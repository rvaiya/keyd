/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <linux/uinput.h>

#ifdef __FreeBSD__
#include <dev/evdev/input-event-codes.h>
#else
#include <linux/input-event-codes.h>
#endif

#define REPEAT_INTERVAL 40
#define REPEAT_TIMEOUT 200

#include "../keyd.h"

struct vkbd {
	int fd;
	int pfd;
};

pthread_mutex_t repeater_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t repeater_cond = PTHREAD_COND_INITIALIZER;
static uint8_t repeat_key = 0;

static int create_virtual_keyboard(const char *name)
{
	int ret;
	size_t code;
	struct uinput_user_dev udev = {0};

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		perror("open uinput");
		exit(-1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_KEY)) {
		perror("ioctl set_evbit");
		exit(-1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_SYN)) {
		perror("ioctl set_evbit");
		exit(-1);
	}

	for (code = 0; code < 256; code++) {
		if (keycode_table[code].name) {
			if (ioctl(fd, UI_SET_KEYBIT, code)) {
				perror("ioctl set_keybit");
				exit(-1);
			}
		}
	}

	udev.id.bustype = BUS_USB;
	udev.id.vendor = 0x0FAC;
	udev.id.product = 0x0ADE;

	snprintf(udev.name, sizeof(udev.name), "%s", name);

	/*
	 * We use this in favour of the newer UINPUT_DEV_SETUP
	 * ioctl in order to support older kernels.
	 *
	 * See: https://github.com/torvalds/linux/commit/052876f8e5aec887d22c4d06e54aa5531ffcec75
	 */
	ret = write(fd, &udev, sizeof udev);

	if (ret < 0) {
		fprintf(stderr, "failed to create uinput device\n");
		exit(-1);
	}

	if (ioctl(fd, UI_DEV_CREATE)) {
		perror("ioctl dev_create");
		exit(-1);
	}

	return fd;
}

static int create_virtual_pointer(const char *name)
{
	uint16_t code;
	struct uinput_user_dev udev = {0};

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	ioctl(fd, UI_SET_ABSBIT, ABS_X);
	ioctl(fd, UI_SET_ABSBIT, ABS_Y);
	ioctl(fd, UI_SET_RELBIT, REL_X);
	ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
	ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
	ioctl(fd, UI_SET_RELBIT, REL_Y);
	ioctl(fd, UI_SET_RELBIT, REL_Z);

	for (code = BTN_LEFT; code <= BTN_TASK; code++)
		ioctl(fd, UI_SET_KEYBIT, code);

	for (code = BTN_0; code <= BTN_9; code++)
		ioctl(fd, UI_SET_KEYBIT, code);

	udev.id.bustype = BUS_USB;
	udev.id.vendor = 0x0FAC;
	udev.id.product = 0x1ADE;

	udev.absmax[ABS_X] = 1024;
	udev.absmax[ABS_Y] = 1024;

	snprintf(udev.name, sizeof(udev.name), "%s", name);

	if (write(fd, &udev, sizeof udev) < 0) {
		fprintf(stderr, "failed to create uinput device\n");
		exit(-1);
	}

	ioctl(fd, UI_DEV_CREATE);

	return fd;
}

/*
 * Sleep for timeout miliseconds or until repeater_cond is signaled.
 * Returns 0 if sleep completed without interruption.
 */
int wait_for_repeat_cond(long timeout)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	ts.tv_nsec += timeout * 1E6;

	ts.tv_sec += ts.tv_nsec / 1E9;
	ts.tv_nsec %= (long) 1E9;

	return pthread_cond_timedwait(&repeater_cond, &repeater_mtx, &ts) != ETIMEDOUT;
}

void write_key_event(const struct vkbd *vkbd, uint8_t code, int state)
{
	static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	struct input_event ev;
	int fd;
	int is_btn;

	pthread_mutex_lock(&mtx);

	fd = vkbd->fd;

	ev.type = EV_KEY;

	is_btn = 1;
	switch (code) {
		case KEYD_LEFT_MOUSE:	ev.code = BTN_LEFT; break;
		case KEYD_MIDDLE_MOUSE:	ev.code = BTN_MIDDLE; break;
		case KEYD_RIGHT_MOUSE:	ev.code = BTN_RIGHT; break;
		case KEYD_MOUSE_1:	ev.code = BTN_SIDE; break;
		case KEYD_MOUSE_2:	ev.code = BTN_EXTRA; break;
		default:
			ev.code = code;
			is_btn = 0;
			break;
	}

	/*
	 * Send all buttons through the virtual pointer
	 * to prevent X from identifying the virtual
	 * keyboard as a mouse.
	 */
	if (is_btn) {
		fd = vkbd->pfd;

		/*
		 * Give key events preceding a mouse click
		 * a chance to propagate to avoid event
		 * order transposition. A bit kludegy,
		 * but better than waiting for all events
		 * to propagate and then checking them
		 * on re-entry.
		 *
		 * TODO: fixme (maybe)
		 */
		usleep(1000);
	}

	ev.value = state;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	xwrite(fd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;


	xwrite(fd, &ev, sizeof(ev));

	pthread_mutex_unlock(&mtx);
}

/*
 * Unlike X/Wayland, VTs rely on repeat events generated
 * by the kernel drivers, so we have to simulate these.
 */
static void *repeater(void *arg)
{
	struct vkbd *vkbd = arg;

	while(1) {
		while (!repeat_key)
			pthread_cond_wait(&repeater_cond, &repeater_mtx);

		if (wait_for_repeat_cond(REPEAT_TIMEOUT))
			continue;

		while (!wait_for_repeat_cond(REPEAT_INTERVAL))
			write_key_event(vkbd, repeat_key, 2);
	}

	return NULL;
}

struct vkbd *vkbd_init(const char *name)
{
	pthread_t tid;

	struct vkbd *vkbd = calloc(1, sizeof vkbd);
	vkbd->fd = create_virtual_keyboard(name);
	vkbd->pfd = create_virtual_pointer("keyd virtual pointer");

	pthread_create(&tid, NULL, repeater, vkbd);

	return vkbd;
}

void vkbd_mouse_move(const struct vkbd *vkbd, int x, int y)
{
	struct input_event ev;

	if (x) {
		ev.type = EV_REL;
		ev.code = REL_X;
		ev.value = x;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		xwrite(vkbd->pfd, &ev, sizeof(ev));
	}

	if (y) {
		ev.type = EV_REL;
		ev.code = REL_Y;
		ev.value = y;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		xwrite(vkbd->pfd, &ev, sizeof(ev));
	}

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	xwrite(vkbd->pfd, &ev, sizeof(ev));
}

void vkbd_mouse_scroll(const struct vkbd *vkbd, int x, int y)
{
	struct input_event ev;

	ev.type = EV_REL;
	ev.code = REL_WHEEL;
	ev.value = y;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	xwrite(vkbd->pfd, &ev, sizeof(ev));

	ev.type = EV_REL;
	ev.code = REL_HWHEEL;
	ev.value = x;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	xwrite(vkbd->pfd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	xwrite(vkbd->pfd, &ev, sizeof(ev));
}

void vkbd_mouse_move_abs(const struct vkbd *vkbd, int x, int y)
{
	struct input_event ev;

	if (x) {
		ev.type = EV_ABS;
		ev.code = ABS_X;
		ev.value = x;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		xwrite(vkbd->pfd, &ev, sizeof(ev));
	}

	if (y) {
		ev.type = EV_ABS;
		ev.code = ABS_Y;
		ev.value = y;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		xwrite(vkbd->pfd, &ev, sizeof(ev));
	}

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	xwrite(vkbd->pfd, &ev, sizeof(ev));
}

void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	dbg("output %s %s", KEY_NAME(code), state == 1 ? "down" : "up");

	if (state) {
		pthread_mutex_lock(&repeater_mtx);
		repeat_key = code;
		pthread_mutex_unlock(&repeater_mtx);
		pthread_cond_signal(&repeater_cond);
	} else {
		pthread_mutex_lock(&repeater_mtx);
		repeat_key = 0;
		pthread_mutex_unlock(&repeater_mtx);
		pthread_cond_signal(&repeater_cond);
	}

	write_key_event(vkbd, code, state);
}

void free_vkbd(struct vkbd *vkbd)
{
	if (vkbd) {
		close(vkbd->fd);
		free(vkbd);
	}
}
