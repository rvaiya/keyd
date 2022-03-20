/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "config.h"
#include "layer.h"

#define MAX_ACTIVE_KEYS	32
#define CACHE_SIZE	16 //Effectively nkro

struct cache_entry {
	uint8_t code;
	struct descriptor d;
	uint8_t layermods;
};

struct keyboard {
	int fd;

	struct config config;

	struct layer_table layer_table;

	/* state*/

	/* for key up events */
	struct cache_entry cache[CACHE_SIZE];

	uint8_t last_pressed_output_code;
	uint8_t last_pressed_keycode;
	uint8_t last_layer_code;

	struct {
		uint8_t code;
		uint8_t mods;
		struct timeout t;
	} pending_timeout;

	uint8_t keystate[256];
	uint8_t modstate[MAX_MOD];
};

long	kbd_process_key_event(struct keyboard *kbd, uint8_t code, int pressed);
void	kbd_reset(struct keyboard *kbd);
int	kbd_execute_expression(struct keyboard *kbd, const char *exp);

#endif
