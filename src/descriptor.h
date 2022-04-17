/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

struct layer;
struct layer_table;

#define MAX_LAYERS 32
#define MAX_EXP_LEN 512
#define MAX_MACROEXP_LEN 512

#define MAX_DESCRIPTOR_LEN 256

enum op {
	OP_UNDEFINED,
	OP_KEYSEQUENCE,

	OP_ONESHOT,
	OP_SWAP,
	OP_LAYER,
	OP_OVERLOAD,
	OP_TOGGLE,

	OP_MACRO,
	OP_TIMEOUT
};

/* Describes the intended purpose of a key. */

struct descriptor {
	enum op op;

	union {
		uint8_t code;
		uint8_t mods;
		int16_t idx;
		uint16_t sz;
		uint16_t timeout;
	} args[3];
};

/*
 * Creates a descriptor from the given string which describes a key action.
 * Potentially modifying the input string in the process.
 */

int parse_descriptor(const char *descstr,
		     struct descriptor *d,
		     struct layer_table *lt);

int layer_table_add_entry(struct layer_table *lt, const char *exp);
int layer_table_lookup(const struct layer_table *lt, const char *name);

int create_layer(struct layer *layer, const char *desc, const struct layer_table *lt);
#endif
