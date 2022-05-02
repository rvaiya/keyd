/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef __H_LAYER_
#define __H_LAYER_
	
#include "keys.h"
#include "macro.h"
#include "command.h"
#include "descriptor.h"

#define MAX_LAYER_NAME_LEN	32
#define MAX_COMPOSITE_LAYERS	8

#define MAX_LAYERS	32
#define MAX_EXP_LEN	512

#define MAX_COMMANDS			64
#define MAX_MACROS			256
#define MAX_LAYER_TABLE_DESCRIPTORS	64

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

	enum {
		LT_NORMAL,
		LT_LAYOUT,
		LT_COMPOSITE,
	} type;

	uint8_t mods;
	struct descriptor keymap[256];

	/* Composite layer constituents. */
	size_t nr_layers;
	int layers[MAX_COMPOSITE_LAYERS];

	/* state */
	uint8_t active;
	uint8_t flags;
	long activation_time;
};

struct layer_table {
	struct layer layers[MAX_LAYERS];
	size_t nr_layers;

	/* Auxiliary descriptors used by layer bindings. */
	struct descriptor descriptors[MAX_LAYER_TABLE_DESCRIPTORS];

	struct macro macros[MAX_MACROS];
	struct command commands[MAX_COMMANDS];

	size_t nr_macros;
	size_t nr_descriptors;
	size_t nr_commands;
};

int layer_table_add_entry(struct layer_table *lt, const char *exp);
int layer_table_lookup(const struct layer_table *lt, const char *name);

int create_layer(struct layer *layer, const char *desc, const struct layer_table *lt);

#endif
