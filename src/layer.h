#ifndef __H_LAYER_
#define __H_LAYER_
	
#include "keys.h"
#include "descriptor.h"

#define MAX_LAYER_NAME_LEN 32

/*
 * A layer is a map from keycodes to descriptors. It may optionally
 * contain one or more modifiers which are applied to the base layout in
 * the event that no matching descriptor is found in the keymap. For
 * consistency, modifiers are internally mapped to eponymously named
 * layers consisting of the corresponding modifier and an empty keymap.
 */

struct layer {
	char name[MAX_LAYER_NAME_LEN];

	int is_layout;
	uint8_t mods;

	struct descriptor keymap[MAX_KEYS];
};

#endif
