/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef __H_LAYER_
#define __H_LAYER_
	
#include "keys.h"
#include "descriptor.h"

#define MAX_LAYER_NAME_LEN	32
#define MAX_COMPOSITE_LAYERS	8
#define MAX_TIMEOUTS		32

#define MAX_MACRO_SIZE	64
#define MAX_MACROS	256

#define MAX_COMMAND_LEN	256
#define MAX_COMMANDS	64

#define LT_NORMAL	0
#define LT_LAYOUT	1
#define LT_COMPOSITE	2

#define LF_TOGGLED	0x1
#define LF_ONESHOT	0x2

/*
 * A layer is a map from keycodes to descriptors. It may optionally
 * contain one or more modifiers which are applied to the base layout in
 * the event that no matching descriptor is found in the keymap. For
 * consistency, modifiers are internally mapped to eponymously named
 * layers consisting of the corresponding modifier and an empty keymap.
 */

struct layer {
	char name[MAX_LAYER_NAME_LEN+1];

	size_t nr_layers;
	int layers[MAX_COMPOSITE_LAYERS];

	int type;
	uint8_t mods;

	struct descriptor keymap[256];

	/* state */
	uint8_t active;
	uint8_t flags;
	long activation_time;
};

struct command {
	char cmd[MAX_COMMAND_LEN+1];
};

struct timeout {
	uint16_t timeout;
	struct descriptor d1;
	struct descriptor d2;
};

struct macro_entry {
	enum {
		MACRO_KEYSEQUENCE,
		MACRO_HOLD,
		MACRO_RELEASE,
		MACRO_UNICODE,
		MACRO_TIMEOUT
	} type;

	uint16_t data;
};

/*
 * A series of key sequences optionally punctuated by
 * timeouts
 */
struct macro {
	struct macro_entry entries[MAX_MACRO_SIZE];

	int32_t timeout;
	int32_t repeat_timeout;
	uint32_t sz;
};

struct layer_table {
	struct layer layers[MAX_LAYERS];
	size_t nr_layers;

	struct timeout timeouts[MAX_TIMEOUTS];
	struct macro macros[MAX_MACROS];
	struct command commands[MAX_COMMANDS];

	size_t nr_macros;
	size_t nr_timeouts;
	size_t nr_commands;
};

#endif
