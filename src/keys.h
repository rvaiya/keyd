/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef _KEYS_H_
#define _KEYS_H_
#define _KEYS_H_

#ifdef __FreeBSD__
#include <dev/evdev/input-event-codes.h>
#else
#include <linux/input-event-codes.h>
#endif

#include <stdint.h>
#include <stdlib.h>

#define MOD_ALT_GR 0x10
#define MOD_CTRL 0x8
#define MOD_SHIFT 0x4
#define MOD_SUPER 0x2
#define MOD_ALT 0x1

#define MAX_MOD 5

uint8_t	keycode_to_mod(uint8_t code);
int	parse_modset(const char *s, uint8_t *mods);

struct keycode_table_ent {
	const char *name;
	const char *alt_name;
	const char *shifted_name;
};

struct modifier_table_ent {
	const char *name;
	uint8_t mask;

	uint8_t code1;
	uint8_t code2; /* May be 0. */
};

/* Move evdev mouse codes into byte range. */
#define KEY_LEFT_MOUSE		KEY_MICMUTE+1
#define KEY_RIGHT_MOUSE		KEY_MICMUTE+2
#define KEY_MIDDLE_MOUSE	KEY_MICMUTE+3
#define KEY_MOUSE_1		KEY_MICMUTE+4
#define KEY_MOUSE_2		KEY_MICMUTE+5

extern const struct modifier_table_ent modifier_table[MAX_MOD];
extern const struct keycode_table_ent keycode_table[256];

#endif
