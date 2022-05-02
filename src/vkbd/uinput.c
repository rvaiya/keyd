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
#include <linux/uinput.h>

#ifdef __FreeBSD__
#include <dev/evdev/input-event-codes.h>
#else
#include <linux/input-event-codes.h>
#endif

#include "../vkbd.h"
#include "../keys.h"

struct vkbd {
	int fd;
	int pfd;
};

static int is_mouse_button(size_t code)
{
	return (((code) >= BTN_LEFT && (code) <= BTN_TASK) ||
		((code) >= BTN_0 && (code) <= BTN_9));
}

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
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

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

	snprintf(udev.name, sizeof(udev.name), "%s", name);

	if (write(fd, &udev, sizeof udev) < 0) {
		fprintf(stderr, "failed to create uinput device\n");
		exit(-1);
	}

	ioctl(fd, UI_DEV_CREATE);

	return fd;
}

struct vkbd *vkbd_init(const char *name)
{
	struct vkbd *vkbd = calloc(1, sizeof vkbd);
	vkbd->fd = create_virtual_keyboard(name);

	/*
	 * lazily initialize the virtual pointer to avoid presenting an
	 * external mouse if it is unnecessary. This can cause issues higher
	 * up the input stack (e.g libinput touchpad disabling in the presence
	 * of an external mouse)
	 */

	vkbd->pfd = -1;

	return vkbd;
}

void vkbd_move_mouse(const struct vkbd *vkbd, int x, int y)
{
	struct input_event ev;

	if (vkbd->pfd == -1) {
		((struct vkbd *)vkbd)->pfd = create_virtual_pointer("keyd virtual pointer");
	}

	if (x) {
		ev.type = EV_REL;
		ev.code = REL_X;
		ev.value = x;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		write(vkbd->pfd, &ev, sizeof(ev));
	}

	if (y) {
		ev.type = EV_REL;
		ev.code = REL_Y;
		ev.value = y;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		write(vkbd->pfd, &ev, sizeof(ev));
	}

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	write(vkbd->pfd, &ev, sizeof(ev));
}

void vkbd_send_button(const struct vkbd *vkbd, uint8_t btn, int state)
{
	struct input_event ev;

	if (vkbd->pfd == -1) {
		((struct vkbd *)vkbd)->pfd = create_virtual_pointer("keyd virtual pointer");
	}

	switch (btn) {
	case 1:
		ev.code = BTN_LEFT;
		break;
	case 2:
		ev.code = BTN_MIDDLE;
		break;
	case 3:
		ev.code = BTN_RIGHT;
		break;
	default:
		return;
	}

	ev.type = EV_KEY;
	ev.value = state;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	write(vkbd->pfd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	write(vkbd->pfd, &ev, sizeof(ev));
}

void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	struct input_event ev;

	ev.type = EV_KEY;
	ev.code = code;
	ev.value = state;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	write(vkbd->fd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;


	write(vkbd->fd, &ev, sizeof(ev));
}

void free_vkbd(struct vkbd *vkbd)
{
	if (vkbd) {
		close(vkbd->fd);
		free(vkbd);
	}
}
