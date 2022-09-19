/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <limits.h>
#include "macro.h"

#define MAX_LAYER_NAME_LEN	32
#define MAX_DESCRIPTOR_ARGS	3

#define MAX_LAYERS		32
#define MAX_EXP_LEN		512


#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


enum op {
	OP_KEYSEQUENCE = 1,

	OP_ONESHOT,
	OP_ONESHOTM,
	OP_LAYERM,
	OP_SWAP,
	OP_SWAPM,
	OP_LAYER,
	OP_LAYOUT,
	OP_CLEAR,
	OP_CLEARM,
	OP_OVERLOAD,
	OP_OVERLOAD_TIMEOUT,
	OP_OVERLOAD_TIMEOUT_TAP,
	OP_TOGGLE,
	OP_TOGGLEM,

	OP_MACRO,
	OP_MACRO2,
	OP_COMMAND,
	OP_TIMEOUT
};

union descriptor_arg {
	uint8_t code;
	uint8_t mods;
	int16_t idx;
	uint16_t sz;
	uint16_t timeout;
};

/* Describes the intended purpose of a key (corresponds to an 'action' in user parlance). */

struct descriptor {
	enum op op;
	union descriptor_arg args[MAX_DESCRIPTOR_ARGS];
};

/*
 * A layer is a map from keycodes to descriptors. It may optionally
 * contain one or more modifiers which are applied to the base layout in
 * the event that no matching descriptor is found in the keymap. For
 * consistency, modifiers are internally mapped to eponymously named
 * layers consisting of the corresponding modifier and an empty keymap.
 */

struct layer {
	char name[MAX_LAYER_NAME_LEN+1];

	enum {
		LT_NORMAL,
		LT_LAYOUT,
		LT_COMPOSITE,
	} type;

	uint8_t mods;
	struct descriptor keymap[256];

	/* Used for composite layers. */
	size_t nr_constituents;
	int constituents[8];
};

struct command {
	char cmd[256];
};

struct config {
	char path[PATH_MAX];
	struct layer layers[MAX_LAYERS];

	/* Auxiliary descriptors used by layer bindings. */
	struct descriptor descriptors[64];
	struct macro macros[256];
	struct command commands[64];
	char aliases[256][32];

	uint8_t wildcard;
	uint32_t ids[64];
	uint32_t excluded_ids[64];

	size_t nr_ids;
	size_t nr_excluded_ids;
	size_t nr_layers;
	size_t nr_macros;
	size_t nr_descriptors;
	size_t nr_commands;

	long macro_timeout;
	long macro_sequence_timeout;
	long macro_repeat_timeout;

	uint8_t layer_indicator;
	char default_layout[MAX_LAYER_NAME_LEN];
};

int config_parse(struct config *config, const char *path);
int config_add_entry(struct config *config, const char *exp);
int config_get_layer_index(const struct config *config, const char *name);

int config_check_match(struct config *config, uint32_t id);

#endif
