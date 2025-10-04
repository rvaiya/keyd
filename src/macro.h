#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>
#include <stdlib.h>

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
	struct macro_entry entries[256];

	uint32_t sz;
};


long macro_execute(void (*output)(void *, uint8_t, uint8_t),
		   void *ctx,
		   const struct macro *macro,
		   size_t timeout);

int macro_parse(char *s, struct macro *macro);
#endif
