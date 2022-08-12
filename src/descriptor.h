/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

struct config;
struct layer;
struct layer_table;

#define MAX_DESCRIPTOR_ARGS 3

enum op {
	OP_KEYSEQUENCE = 1,

	OP_ONESHOT,
	OP_SWAP,
	OP_SWAP2,
	OP_LAYER,
	OP_LAYOUT,
	OP_CLEAR,
	OP_OVERLOAD,
	OP_TOGGLE,
	OP_TOGGLE2,

	OP_MACRO,
	OP_MACRO2,
	OP_COMMAND,
	OP_TIMEOUT
};

typedef union {
	uint8_t code;
	uint8_t mods;
	int16_t idx;
	uint16_t sz;
	uint16_t timeout;
} descriptor_arg_t;

/* Describes the intended purpose of a key (corresponds to an 'action' in user parlance). */

struct descriptor {
	enum op op;

	descriptor_arg_t args[MAX_DESCRIPTOR_ARGS];
};

/*
 * Creates a descriptor from the given string which describes a key action.
 * Potentially modifying the input string in the process.
 */

int parse_descriptor(const char *descstr,
		     struct descriptor *d,
		     struct config *config);

#endif
