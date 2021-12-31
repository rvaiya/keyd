#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../keys.h"
#include "usb-gadget.h"

uint16_t mods = 0;
uint8_t keys[] = {
	0,
	0,
	0,
	0,
	0,
	0
};

struct vkbd {
	int fd;
};

struct hid_report {
	uint16_t hid_mods;
	uint8_t hid_code[6];
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
	if(hid_table[code]) {
		return hid_table[code];
	}

	return 0;
}

void send_hid_report (const struct vkbd *vkbd)
{

	struct hid_report report;

	for (int i = 0; i < 6; i++) {
		report.hid_code[i] = hid_code(keys[i]);
	}
	report.hid_mods = mods;

	write(vkbd->fd,&report,sizeof(report));

}

static int get_modifier(int code)
{
	switch (code) {
	case KEY_LEFTSHIFT:
		return HID_SHIFT;
		break;
	case KEY_RIGHTSHIFT:
		return HID_RIGHTSHIFT;
		break;
	case KEY_LEFTCTRL:
		return HID_CTRL;
		break;
	case KEY_RIGHTCTRL:
		return HID_RIGHTCTRL;
		break;
	case KEY_LEFTALT:
		return HID_ALT;
		break;
	case KEY_RIGHTALT:
		return HID_ALT_GR;
		break;
	case KEY_LEFTMETA:
		return HID_SUPER;
		break;
	case KEY_RIGHTMETA:
		return HID_RIGHTSUPER;
		break;
	default:
		return 0;
		break;
	}

}

static int set_modifier_state(int code, int state)
{
	uint16_t mod = get_modifier(code);

	if(mod) {
		if (state) {
			mods |= mod;
		} else {
			mods &= ~mod;
		}
	}

	return mod;

}

void set_key_state(int code, int state)
{
	int set = 0;

	for (int i = 0; i < 6; i++) {
		if(keys[i] == code) {
			set = 1;
			if(state == 0) {
				keys[i] = 0;
			}
		}
	}
	if(!set) {
		for (int i = 0; i < 6; i++) {
			if(keys[i] == 0 && state && !set) {
				set = 1;
				keys[i] = code;
			}
		}
	}
}


void vkbd_send(const struct vkbd *vkbd, int code, int state)
{
	set_modifier_state(code, state);
	set_key_state(code, state);
	send_hid_report(vkbd);
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
