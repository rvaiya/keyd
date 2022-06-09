/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 * © 2021 Giorgi Chavchanidze
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../keys.h"
#include "usb-gadget.h"

static uint8_t mods = 0;

static uint8_t keys[6] = {0};

struct vkbd {
	int fd;
};

struct hid_report {
	uint8_t hid_mods;
	uint8_t reserved;
	uint8_t hid_code[6];
};

static int create_virtual_keyboard(void)
{

	int fd = open("/dev/hidg0", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	return fd;
}

static void send_hid_report(const struct vkbd *vkbd)
{

	struct hid_report report;

	for (int i = 0; i < 6; i++)
		report.hid_code[i] = keys[i];

	report.hid_mods = mods;

	write(vkbd->fd, &report, sizeof(report));

}

static uint8_t get_modifier(int code)
{
	switch (code) {
	case KEYD_LEFTSHIFT:
		return HID_SHIFT;
		break;
	case KEYD_RIGHTSHIFT:
		return HID_RIGHTSHIFT;
		break;
	case KEYD_LEFTCTRL:
		return HID_CTRL;
		break;
	case KEYD_RIGHTCTRL:
		return HID_RIGHTCTRL;
		break;
	case KEYD_LEFTALT:
		return HID_ALT;
		break;
	case KEYD_RIGHTALT:
		return HID_ALT_GR;
		break;
	case KEYD_LEFTMETA:
		return HID_SUPER;
		break;
	case KEYD_RIGHTMETA:
		return HID_RIGHTSUPER;
		break;
	default:
		return 0;
		break;
	}

}

static int update_modifier_state(int code, int state)
{
	uint16_t mod = get_modifier(code);

	if (mod) {
		if (state)
			mods |= mod;
		else
			mods &= ~mod;
		return 0;
	}

	return -1;

}

static void update_key_state(uint16_t code, int state)
{
	int i;
	int set = 0;
	uint8_t hid_code = hid_table[code];

	for (i = 0; i < 6; i++) {
		if (keys[i] == hid_code) {
			set = 1;
			if (state == 0)
				keys[i] = 0;
		}
	}
	if (state && !set) {
		for (i = 0; i < 6; i++) {
			if (keys[i] == 0) {
				keys[i] = hid_code;
				break;
			}
		}
	}
}

struct vkbd *vkbd_init(const char *name)
{
	struct vkbd *vkbd = calloc(1, sizeof vkbd);
	vkbd->fd = create_virtual_keyboard();

	return vkbd;
}

void vkbd_mouse_move(const struct vkbd *vkbd, int x, int y)
{
	fprintf(stderr, "usb-gadget: mouse support is not implemented\n");
}

void vkbd_mouse_move_abs(const struct vkbd *vkbd, int x, int y)
{
	fprintf(stderr, "usb-gadget: mouse support is not implemented\n");
}

void vkbd_mouse_scroll(const struct vkbd *vkbd, int x, int y)
{
	fprintf(stderr, "usb-gadget: mouse support is not implemented\n");
}

void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	if (update_modifier_state(code, state) < 0)
		update_key_state(code, state);

	send_hid_report(vkbd);
}

void free_vkbd(struct vkbd *vkbd)
{
	close(vkbd->fd);
	free(vkbd);
}
