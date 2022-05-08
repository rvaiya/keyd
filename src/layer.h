/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef __H_LAYER_
#define __H_LAYER_
	
#include "descriptor.h"
struct config;

#define MAX_LAYER_NAME_LEN	32
#define MAX_COMPOSITE_LAYERS	8

#define MAX_LAYERS	32
#define MAX_EXP_LEN	512

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
	int constituents[MAX_COMPOSITE_LAYERS];
};

int create_layer(struct layer *layer, const char *desc, const struct config *cfg);

#endif
