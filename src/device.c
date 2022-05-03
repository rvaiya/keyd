/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include "device.h"
#include "keys.h"


#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/input.h>
#include <sys/inotify.h>

/*
 * Abstract away evdev and inotify.
 *
 * We could make this cleaner by creating a single file descriptor via epoll
 * but this would break FreeBSD compatibility without a dedicated kqueue
 * implementation. A thread based approach was also considered, but
 * inter-thread communication adds too much overhead (~100us).
 *
 * Overview:
 *
 * A 'devmon' is a file descriptor which can be created with devmon_create()
 * and subsequently monitored for new devices read with devmon_read_device().
 *
 * A 'device' always corresponds to a keyboard or mouse from which activity can
 * be monitored with device->fd and events subsequently read using
 * device_read_event().
 *
 * If the event returned by device_read_event() is of type DEV_REMOVED then the
 * corresponding device should be considered invalid by the caller.
 */

/*
 * returns 1 if keyboard, 2 if mouse, and 0 if neither.
 */
static int device_type(int fd)
{
	uint32_t mask[BTN_LEFT/32+1] = {0};

	if (ioctl(fd, EVIOCGBIT(EV_KEY, (BTN_LEFT/32+1)*4), mask) < 0) {
		perror("ioctl");
		return 0;
	}

	/* The first 31 bits correspond to [KEY_ESC-KEY_S] */
	if (mask[0] == 0xFFFFFFFE)
		return 1;
	else if (mask[BTN_LEFT/32] >> BTN_LEFT%32)
		return 2;
	else
		return 0;
}

static int device_init(const char *path, struct device *dev)
{
	int fd;
	int type;

	if ((fd = open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC, 0600)) < 0) {
		fprintf(stderr, "failed to open %s\n", path);
		return -1;
	}

	type = device_type(fd);

	if (type) {
		struct input_id info;

		if (ioctl(fd, EVIOCGNAME(sizeof(dev->name)), dev->name) == -1) {
			perror("ioctl EVIOCGNAME");
			return -1;
		}

		if (ioctl(fd, EVIOCGID, &info) == -1) {
			perror("ioctl EVIOCGID");
			return -1;
		}

		strncpy(dev->path, path, sizeof(dev->path)-1);
		dev->path[sizeof(dev->path)-1] = 0;

		dev->fd = fd;
		dev->is_keyboard = type == 1;
		dev->vendor_id = info.vendor;
		dev->product_id = info.product;

		return 0;
	} else {
		close(fd);
		return -1;
	}

	return -1;
}

struct device_worker {
	pthread_t tid;
	char path[1024];
	struct device dev;
};

static void *device_scan_worker(void *arg)
{
	struct device_worker *w = (struct device_worker *)arg;
	if (device_init(w->path, &w->dev) < 0)
		return NULL;

	return &w->dev;
}

int device_scan(struct device devices[MAX_DEVICES])
{
	int i;
	struct device_worker workers[MAX_DEVICES];
	struct dirent *ent;
	DIR *dh = opendir("/dev/input/");
	int n = 0, ndevs;

	if (!dh) {
		perror("opendir /dev/input");
		exit(-1);
	}

	while((ent = readdir(dh))) {
		if (!strncmp(ent->d_name, "event", 5)) {
			assert(n < MAX_DEVICES);
			struct device_worker *w = &workers[n++];

			snprintf(w->path, sizeof(w->path), "/dev/input/%s", ent->d_name);
			pthread_create(&w->tid, NULL, device_scan_worker, w);
		}
	}

	ndevs = 0;
	for(i = 0; i < n; i++) {
		struct device *d;
		pthread_join(workers[i].tid, (void**)&d);

		if (d)
			devices[ndevs++] = workers[i].dev;
	}

	closedir(dh);
	return ndevs;
}

/*
 * NOTE: Only a single devmon fd may exist. Implementing this properly
 * would involve bookkeeping state for each fd, but this is
 * unnecessary for our use.
 */
int devmon_create()
{
	static int init = 0;
	assert(!init);
	init = 1;

	int fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if (fd < 0) {
		perror("inotify");
		exit(-1);
	}

	int wd = inotify_add_watch(fd, "/dev/input/", IN_CREATE);
	if (wd < 0) {
		perror("inotify");
		exit(-1);
	}

	return fd;
}

/*
 * A non blocking call which returns any devices available on the provided
 * monitor descriptor. The return value should not be freed or modified by the calling
 * code. Returns NULL if no devices are available.
 */
struct device *devmon_read_device(int fd)
{
	static struct device ret;

	static char buf[4096];
	static int buf_sz = 0;
	static char *ptr = buf;

	while (1) {
		char path[1024];
		struct inotify_event *ev;

		if (ptr >= (buf+buf_sz)) {
			ptr = buf;
			buf_sz = read(fd, buf, sizeof(buf));
			if (buf_sz == -1) {
				buf_sz = 0;
				return NULL;
			}
		}

		ev = (struct inotify_event*)ptr;
		ptr += sizeof(struct inotify_event) + ev->len;

		snprintf(path, sizeof path, "/dev/input/%s", ev->name);

		if (!device_init(path, &ret))
			return &ret;
	}
}

int device_grab(struct device *dev)
{
	size_t i;
	struct input_event ev;
	uint8_t state[KEY_MAX / 8 + 1];

	/*
	 * await neutral key state to ensure any residual
	 * key up events propagate.
	 */

	while (1) {
		int n = 0;
		memset(state, 0, sizeof(state));

		if (ioctl(dev->fd, EVIOCGKEY(sizeof state), state) < 0) {
			perror("ioctl EVIOCGKEY");
			return -1;
		}

		for (i = 0; i < KEY_MAX; i++) {
			if ((state[i / 8] >> (i % 8)) & 0x1)
				n++;
		}

		if (n == 0)
			break;
	}

	if (ioctl(dev->fd, EVIOCGRAB, (void *) 1) < 0) {
		perror("EVIOCGRAB");
		return -1;
	}

	/* drain any input events before the grab (assumes NONBLOCK is set on the fd) */
	while (read(dev->fd, &ev, sizeof(ev)) > 0) {
	}

	return 0;
}

int device_ungrab(struct device *dev)
{
	return ioctl(dev->fd, EVIOCGRAB, (void *) 0);
}

/*
 * Read a device event from the given device or return
 * NULL if none are available (may happen in the
 * case of a spurious wakeup).
 */
struct device_event *device_read_event(struct device *dev)
{
	struct input_event ev;
	static struct device_event devev;

	if (read(dev->fd, &ev, sizeof(ev)) < 0) {
		if (errno == EAGAIN) {
			return NULL;
		} else {
			devev.type = DEV_REMOVED;
			return &devev;
		}
	}

	if (ev.type == EV_REL && ev.code == REL_WHEEL) {
		devev.type = DEV_KEY;
		devev.code = ev.value > 0 ? KEYD_SCROLL_UP : KEYD_SCROLL_DOWN;
		devev.pressed = 1;

		return &devev;
	} else if (ev.type != EV_KEY || ev.value == 2) {
		return NULL;
	}

	/*
	 * KEYD_* codes <256 correspond to their evdev
	 * counterparts.
	 */
	if (ev.code >= 256) {
		if (ev.code == BTN_LEFT)
			ev.code = KEYD_LEFT_MOUSE;
		else if (ev.code == BTN_MIDDLE)
			ev.code = KEYD_MIDDLE_MOUSE;
		else if (ev.code == BTN_RIGHT)
			ev.code = KEYD_RIGHT_MOUSE;
		else if (ev.code == BTN_SIDE)
			ev.code = KEYD_MOUSE_1;
		else if (ev.code == BTN_EXTRA)
			ev.code = KEYD_MOUSE_2;
		else if (ev.code == KEY_FN)
			ev.code = KEYD_FN;
		else if (ev.code >= BTN_DIGI && ev.code <= BTN_TOOL_QUADTAP)
			;
		else {
			fprintf(stderr, "ERROR: unsupported evdev code: 0x%x\n", ev.code);
			return NULL;
		}
	}

	devev.type = DEV_KEY;
	devev.code = ev.code;
	devev.pressed = ev.value;

	return &devev;
}

void device_set_led(const struct device *dev, int led, int state)
{
	struct input_event ev = {
		.type = EV_LED,
		.code = led,
		.value = state
	};

	write(dev->fd, &ev, sizeof ev);
}
