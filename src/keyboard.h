/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyd.h"
#include "keys.h"
#include "unicode.h"
#include "config.h"
#include "device.h"

#define MAX_ACTIVE_KEYS	32
#define CACHE_SIZE	16 //Effectively nkro

struct keyboard;

struct cache_entry {
	uint8_t code;
	struct descriptor d;
	int dl;
	int layer;
};

struct key_event {
	uint8_t code;
	uint8_t pressed;
	int timestamp;
};

struct output {
	void (*send_key) (uint8_t code, uint8_t state);
	void (*on_layer_change) (const struct keyboard *kbd, const struct layer *layer, uint8_t active);
};

/* May correspond to more than one physical input device. */
struct keyboard {
	const struct config *original_config;
	struct config config;
	struct output output;

	/*
	 * Cache descriptors to preserve code->descriptor
	 * mappings in the event of mid-stroke layer changes.
	 */
	struct cache_entry cache[CACHE_SIZE];

	uint8_t last_pressed_output_code;
	uint8_t last_pressed_code;

	uint8_t oneshot_latch;

	uint8_t inhibit_modifier_guard;

	struct macro *active_macro;
	const char *active_command;
	int active_macro_layer;
	int overload_last_layer_code;

	long macro_timeout;
	long oneshot_timeout;
	long command_timeout;

	long macro_repeat_interval;
	long command_repeat_interval;

	long overload_start_time;

	long last_simple_key_time;

	long timeouts[64];
	size_t nr_timeouts; 

	struct active_chord {
		uint8_t active;
		struct chord chord;
		int layer;
	} active_chords[KEYD_CHORD_MAX-KEYD_CHORD_1+1];

	struct {
		struct key_event queue[32];
		size_t queue_sz;

		const struct chord *match;
		int match_layer;

		uint8_t start_code;
		long last_code_time;

		enum {
			CHORD_RESOLVING,
			CHORD_INACTIVE,
			CHORD_PENDING_DISAMBIGUATION,
			CHORD_PENDING_HOLD_TIMEOUT,
		} state;
	} chord;

	struct {
		uint8_t code;
		uint8_t dl;
		long expire;
		long tap_expiry;

		enum {
			PK_INTERRUPT_ACTION1,
			PK_INTERRUPT_ACTION2,
			PK_UNINTERRUPTIBLE,
			PK_UNINTERRUPTIBLE_TAP_ACTION2,
		} behaviour;

		struct key_event queue[32];
		size_t queue_sz;

		struct descriptor action1;
		struct descriptor action2;
	} pending_key;

	struct {
		long activation_time;

		uint8_t active;
		uint8_t toggled;
		uint8_t oneshot_depth;
	} layer_state[MAX_LAYERS];

	uint8_t keystate[256];

	struct {
		int x;
		int y;

		int sensitivity; /* Mouse units per scroll unit (higher == slower scrolling). */
		int active;
	} scroll;
};

struct keyboard *new_keyboard(struct config *config, const struct output *output);

long kbd_process_events(struct keyboard *kbd, const struct key_event *events, size_t n);
int kbd_eval(struct keyboard *kbd, const char *exp);
void kbd_reset(struct keyboard *kbd);

#endif
