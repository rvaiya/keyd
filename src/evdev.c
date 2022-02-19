#include "keyd.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <dirent.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdlib.h>
#include <unistd.h>

int evdev_is_keyboard(const char *devnode)
{
	int fd = open(devnode, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "is_keyboard: Failed to open %s\n", devnode);
		return 0;
	}

	uint8_t keymask[(KEY_CNT+7)/8];

	if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof keymask), keymask) < 0) {
		perror("ioctl");
		fprintf(stderr, "WARNING: Failed to retrieve key bit for %s\n", devnode);
		return 0;
	}

	int has_a = keymask[KEY_A/8] & (0x01 << (KEY_A%8));
	int has_d = keymask[KEY_D/8] & (0x01 << (KEY_D%8));

	close(fd);

	return has_a && has_d;
}


struct worker {
	pthread_t tid;
	char path[1024];
	int result;
} workers[256];

static void *is_keyboard_worker(void *worker)
{
	struct worker *w = (struct worker*) worker;
	w->result = evdev_is_keyboard(w->path);
}

/*
 * Note: the returned array is owned by the function and
 * should not be freed by the caller. Successive invocations
 * invalidate devs.
 */

int evdev_get_keyboard_nodes(char **devs, int *ndevs)
{
	struct dirent *ent;
	int nkbds = 0;
	int n = 0;

	DIR *dh = opendir("/dev/input/");
	if (!dh) {
		perror("opendir /dev/input");
		exit(-1);
	}

	n = 0;

	while((ent = readdir(dh))) {
		char path[1024];
		if (strstr(ent->d_name, "event") == ent->d_name) {
			assert(n < sizeof(workers)/sizeof(workers[0]));
			struct worker *w = &workers[n++];
			snprintf(w->path, sizeof(w->path), "/dev/input/%s", ent->d_name);
			pthread_create(&w->tid, NULL, is_keyboard_worker, w);
		}
	}

	closedir(dh);

	*ndevs = 0;
	while(n--) {
		pthread_join(workers[n].tid, NULL);

		if(workers[n].result)
			devs[(*ndevs)++] = workers[n].path;
	}
}

const char *evdev_device_name(const char *devnode)
{
	static char name[256];

	int fd = open(devnode, O_RDONLY);
	if (fd < 0) {
		perror("open name");
		return NULL;
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), &name) == -1)
		return NULL;

	close(fd);
	return name;
}

int evdev_device_id(const char *devnode, uint16_t *vendor, uint16_t *product)
{
	struct input_id info;

	int fd = open(devnode, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	if (ioctl(fd, EVIOCGID, &info) == -1) {
		perror("ioctl");
		return -1;
	}

	close(fd);

	*vendor = info.vendor;
	*product = info.product;

	return 0;
}

int evdev_grab_keyboard(int fd)
{
	size_t i;
	uint8_t state[KEY_MAX / 8 + 1];

	/*
	 * await neutral key state to ensure any residual
	 * key up events propagate.
	 */

	while (1) {
		int n = 0;
		memset(state, 0, sizeof(state));

		if (ioctl(fd, EVIOCGKEY(sizeof state), state) < 0) {
			perror("ioctl EVIOCGKEY");
			return -1;
		}

		for (i = 0; i < KEY_MAX; i++) {
			if ((state[i/8] >> (i % 8)) & 0x1)
				n++;
		}

		if (n == 0)
			break;
	}

	if (ioctl(fd, EVIOCGRAB, (void *) 1) < 0) {
		perror("EVIOCGRAB");
		return -1;
	}

	return 0;
}
