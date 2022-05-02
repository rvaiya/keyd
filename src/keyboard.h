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
	struct layer *dl;
};

struct keyboard {
	struct device *dev;

	struct config config;

	struct layer_table layer_table;

	/* 
	 * Cache descriptors to preserve code->descriptor
	 * mappings in the event of mid-stroke layer changes.
	 */
	struct cache_entry cache[CACHE_SIZE];

	uint8_t transient_mods;

	uint8_t last_pressed_output_code;
	uint8_t last_pressed_code;
	uint8_t last_layer_code;

	uint8_t oneshot_latch;

	uint8_t inhibit_modifier_guard;

	struct macro *active_macro;
	struct layer *active_macro_layer;

	long macro_repeat_timeout;

	struct {
		uint8_t active;

		uint8_t code;

		struct descriptor d1;
		struct descriptor d2;

		struct layer *dl;
	} pending_timeout;

	uint8_t keystate[256];
};

long kbd_process_key_event(struct keyboard *kbd, uint8_t code, int pressed);
void kbd_reset(struct keyboard *kbd);
int kbd_execute_expression(struct keyboard *kbd, const char *exp);

#endif
