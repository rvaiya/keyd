#ifndef __H_LAYER_
#define __H_LAYER_
	
#include "keys.h"
#include "descriptor.h"

#define MAX_LAYER_NAME_LEN 32

struct keymap_entry {
	uint16_t code;
	struct descriptor descriptor;

	struct keymap_entry *next;
};

/* 
 * A layer is a map from keycodes to descriptors. It may optionally contain one
 * or more modifiers which are applied to the base layout in the event that no
 * matching descriptor is found in the keymap. For consistency, modifiers are
 * internally mapped to eponymously named layers consisting of the
 * corresponding modifier and an empty keymap.
 */

struct layer {
	char name[MAX_LAYER_NAME_LEN];

	int is_layout;
	uint16_t mods;
	struct keymap_entry *_keymap;
};

struct layer *create_layer(const char *name, uint16_t mods, int populate);
void free_layer(struct layer *layer);

struct descriptor *layer_get_descriptor(const struct layer *layer, uint16_t code);
void layer_set_descriptor(struct layer *layer, uint16_t code, const struct descriptor *descriptor);

#endif
