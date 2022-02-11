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
	int pfd;
};

static int is_mouse_button(size_t code)
{
	return (((code) >= BTN_LEFT && (code) <= BTN_TASK) || 
		((code) >= BTN_0 && (code) <= BTN_9));
}

static int create_virtual_pointer(const char *name)
{
	uint16_t code;
	struct uinput_setup usetup;

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
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

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x0FAC;
	usetup.id.product = 0x0ADE;
	strcpy(usetup.name, name);

	ioctl(fd, UI_DEV_SETUP, &usetup);
	ioctl(fd, UI_DEV_CREATE);

	return fd;
}

static int create_virtual_keyboard(const char *name)
{
	size_t code;
	struct uinput_setup usetup;

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("open uinput");
		exit(-1);
	}

	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	for (code = 0; code < KEY_MAX; code++) {
		/* skip mouse buttons to prevent X from identifying the virtual device as a mouse */
		if (is_mouse_button(code))
			continue;

		if (keycode_table[code].name)
			ioctl(fd, UI_SET_KEYBIT, code);
	}

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

void vkbd_send(const struct vkbd *vkbd, uint16_t code, int state)
{
	struct input_event ev;
	int ofd = vkbd->fd;

	if (is_mouse_button(code)) {
		if (vkbd->pfd == -1) {
			((struct vkbd *)vkbd)->pfd = create_virtual_pointer("keyd virtual pointer");
		}

		ofd = vkbd->pfd;
	}

	ev.type = EV_KEY;
	ev.code = code;
	ev.value = state;

	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	write(ofd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;


	write(ofd, &ev, sizeof(ev));
}

void free_vkbd(struct vkbd *vkbd)
{
	close(vkbd->fd);
	free(vkbd);
}
