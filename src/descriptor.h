#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

#define MAX_MACROS 256
#define MAX_MACRO_SIZE 32

enum op {
	OP_KEYSEQ,

	OP_ONESHOT,
	OP_SWAP,
	OP_LAYER,
	OP_OVERLOAD,
	OP_OVERLOAD_TIMEOUT,
	OP_TOGGLE,
	OP_RESET,

	OP_LAYOUT,

	OP_MACRO,
};

struct key_sequence {
	uint16_t mods;
	uint16_t code;
};

struct macro_entry {
	enum {
		MACRO_KEYSEQUENCE,
		MACRO_HOLD,
		MACRO_RELEASE,
		MACRO_TIMEOUT
	} type;

	union {
		struct key_sequence sequence;
		uint32_t timeout;
	} data;
};

/*
 * A series of key sequences optionally punctuated by
 * timeouts
 */
struct macro {
	struct macro_entry entries[MAX_MACRO_SIZE];
	size_t sz;
};

/* Describes the intended purpose of a key. */

struct descriptor {
	enum op op;

	union {
		struct key_sequence sequence;
		struct layer *layer;
		struct macro *macro;
		size_t sz;
		size_t timeout;
	} args[3];
};

/*
 * Creates a descriptor from the given string which describes a key action. A
 * function which maps layer names to indices must also be supplied for layer
 * lookup. The function should return -1 to indicate the absence of layer.
 * Potentially modifies the input string in the process. layer_index_fn_arg
 * gets passed back to layer_index_fn unaltered.
 */

int parse_descriptor(char *s,
		     struct descriptor *desc,
		     struct layer *(*layer_lookup_fn)(const char *name, void *arg),
		     void *layer_lookup_fn_arg);

#endif
