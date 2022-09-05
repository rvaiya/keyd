/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyd.h"
#include "unicode.h"
#include "config.h"
#include "device.h"

#define MAX_ACTIVE_KEYS	32
#define CACHE_SIZE	16 //Effectively nkro

struct cache_entry {
	uint8_t code;
	struct descriptor d;
	int dl;
};

struct key_event {
	uint8_t code;
	uint8_t pressed;
	int timestamp;
};

/* May correspond to more than one physical input device. */
struct keyboard {
	const struct config *original_config;
	struct config config;

	/*
	 * Cache descriptors to preserve code->descriptor
	 * mappings in the event of mid-stroke layer changes.
	 */
	struct cache_entry cache[CACHE_SIZE];

	uint8_t last_pressed_output_code;
	uint8_t last_pressed_code;
	uint8_t last_layer_code;

	uint8_t oneshot_latch;

	uint8_t inhibit_modifier_guard;

	struct macro *active_macro;
	int active_macro_layer;

	long macro_repeat_timeout;
	int timeout;
	int last_event_ts;

	struct {
		uint8_t active;
		uint8_t code;

		uint8_t layer;
		struct descriptor *action;
		uint8_t dl;

		struct key_event queued[32];
		uint8_t n;
	} overload2;

	struct {
		long activation_time;

		uint8_t active;
		uint8_t toggled;
		uint8_t oneshot;
	} layer_state[MAX_LAYERS];

	struct {
		uint8_t active;

		uint8_t code;

		struct descriptor d1;
		struct descriptor d2;

		int dl;
	} pending_timeout;

	uint8_t keystate[256];
	void (*output) (uint8_t code, uint8_t state);
	void (*layer_observer) (struct keyboard *kbd, const char *layer, int state);
};

struct keyboard *new_keyboard(struct config *config,
			      void (*sink) (uint8_t code, uint8_t pressed),
			      void (*layer_observer)(struct keyboard *kbd, const char *name, int state));

long kbd_process_events(struct keyboard *kbd, const struct key_event *events, size_t n);
int kbd_eval(struct keyboard *kbd, const char *exp);
void kbd_reset(struct keyboard *kbd);

#endif
