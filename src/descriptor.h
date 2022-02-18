#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

struct config;

#define MAX_MACRO_SIZE 128

enum op {
	OP_UNDEFINED,
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
	uint8_t mods;
	uint8_t code;
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

		uint16_t idx;
		uint16_t sz;
		uint16_t timeout;
	} args[3];
};

/*
 * Creates a descriptor from the given string which describes a key action.
 * Layer and macro indices are relative to the provided config.  Potentially
 * modifies the input string in the process.
 */

int parse_descriptor(char *s,
		     struct descriptor *desc,
		     struct config *config);

#endif
