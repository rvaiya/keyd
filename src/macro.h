/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>

#define MAX_MACRO_SIZE	64
#define MAX_MACROEXP_LEN 512

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

	uint32_t sz;
};

int parse_macro(const char *exp, struct macro *macro);

#endif
