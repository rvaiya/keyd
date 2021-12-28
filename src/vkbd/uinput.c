#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>

#include "../vkbd.h"
#include "../keys.h"

struct vkbd {
	int fd;
};

static int create_virtual_keyboard(const char *name)
{
	size_t i;
	struct uinput_setup usetup;

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	for (i = 0; i < KEY_MAX; i++) {
		if (keycode_table[i].name)
			ioctl(fd, UI_SET_KEYBIT, i);
	}

	ioctl(fd, UI_SET_RELBIT, REL_X);
	ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
	ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
	ioctl(fd, UI_SET_RELBIT, REL_Y);
	ioctl(fd, UI_SET_RELBIT, REL_Z);


	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x0FAC;
	usetup.id.product = 0x0ADE;
	strcpy(usetup.name, name);

	ioctl(fd, UI_DEV_SETUP, &usetup);
	ioctl(fd, UI_DEV_CREATE);

	return fd;
}

struct vkbd *vkbd_init(const char *name)
{
	struct vkbd *vkbd = calloc(1, sizeof vkbd);
	vkbd->fd = create_virtual_keyboard(name);

	return vkbd;
}

void vkbd_move_mouse(const struct vkbd *vkbd, int x, int y)
{
	struct input_event ev;

	if (x) {
		ev.type = EV_REL;
		ev.code = REL_X;
		ev.value = x;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		write(vkbd->fd, &ev, sizeof(ev));
	}

	if (y) {
		ev.type = EV_REL;
		ev.code = REL_Y;
		ev.value = y;

		ev.time.tv_sec = 0;
		ev.time.tv_usec = 0;

		write(vkbd->fd, &ev, sizeof(ev));
	}

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	write(vkbd->fd, &ev, sizeof(ev));
}

void vkbd_send(const struct vkbd *vkbd, int code, int state)
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
	close(vkbd->fd);
	free(vkbd);
}
