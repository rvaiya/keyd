#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../keys.h"
#include "usb-gadget.h"

uint16_t mods = 0;

struct vkbd {
	int fd;
};

struct hid_report {
	uint16_t mask;
	uint16_t code;
	uint32_t fill;
};

static int create_virtual_keyboard(void)
{

	int fd = open("/dev/hidg0", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		exit(-1);
	}

	return fd;
}

struct vkbd *vkbd_init(const char *name)
{
	struct vkbd *vkbd = calloc(1, sizeof vkbd);
	vkbd->fd = create_virtual_keyboard();

	return vkbd;
}

uint16_t hid_code(uint16_t code)
{
	int codes = sizeof(evdev_to_hid)/sizeof(evdev_to_hid[0]);
	for(int i = 0; i < codes/2; i++) {
		if(code == evdev_to_hid[2*i]) {
			return evdev_to_hid[2*i+1];
		}
	}
	return 0;
}

void send_hid_report (const struct vkbd *vkbd, uint16_t code, uint16_t mods) {

	struct hid_report report;

	report.code = hid_code(code);
	report.mask = mods;
	report.fill = 0;

	write(vkbd->fd,&report,sizeof(report));

}

static int get_modifier(const char *name)
{

	if (!strcmp(name, "leftshift") || !strcmp(name, "rightshift")) {
		return HID_SHIFT;
	} else	if (!strcmp(name, "leftcontrol") || !strcmp(name, "rightcontrol") ) {
		return HID_CTRL;
	} else	if (!strcmp(name, "leftalt")) {
		return HID_ALT;
	} else	if (!strcmp(name, "rightalt")) {
		return HID_ALT_GR;
	} else	if (!strcmp(name, "leftmeta") || !strcmp(name, "rightmeta") ) {
		return HID_SUPER;
	}

	return 0;

}

static int set_modifier_state(const char *name, int state)
{
	uint16_t mod = get_modifier(name);

	if(mod) {
		if (state == 1) {
			mods |= mod;
		} else if (state == 0) {
			mods ^= mod;
		}
	}

	return mod;

}


void vkbd_send(const struct vkbd *vkbd, int code, int state)
{
	if(!set_modifier_state(keycode_table[code].name, state)) {
		if(state) {
			send_hid_report(vkbd, code, mods);
		} else {
			send_hid_report(vkbd, 0, 0);
		}
	}
}

void vkbd_move_mouse(const struct vkbd *vkbd, int x, int y)
{
	// Not implemented
}

void free_vkbd(struct vkbd *vkbd)
{
	close(vkbd->fd);
	free(vkbd);
}
