#include "keyd.h"

/*
 * Parses expressions of the form: C-t hello enter.
 * Returns 0 on success. Mangles the input string.
 */

int macro_parse(char *s, struct macro *macro)
{
	char *tok;

	#define ADD_ENTRY(t, d) do { \
		if (macro->sz >= ARRAY_SIZE(macro->entries)) { \
			err("maximum macro size (%d) exceeded", ARRAY_SIZE(macro->entries)); \
			return -1; \
		} \
		macro->entries[macro->sz].type = t; \
		macro->entries[macro->sz].data = d; \
		macro->sz++; \
	} while(0)

	macro->sz = 0;
	for (tok = strtok(s, " "); tok; tok = strtok(NULL, " ")) {
		uint8_t code, mods;
		size_t len = strlen(tok);

		if (!parse_key_sequence(tok, &code, &mods)) {
			ADD_ENTRY(MACRO_KEYSEQUENCE, (mods << 8) | code);
		} else if (strchr(tok, '+')) {
			char *saveptr;
			char *key;

			for (key = strtok_r(tok, "+", &saveptr); key; key = strtok_r(NULL, "+", &saveptr)) {
				size_t len = strlen(key);

				if (is_timeval(key))
					ADD_ENTRY(MACRO_TIMEOUT, atoi(key));
				else if (!parse_key_sequence(key, &code, &mods))
					ADD_ENTRY(MACRO_HOLD, code);
				else {
					err("%s is not a valid key", key);
					return -1;
				}
			}

			ADD_ENTRY(MACRO_RELEASE, 0);
		} else if (is_timeval(tok)) {
			ADD_ENTRY(MACRO_TIMEOUT, atoi(tok));
		} else {
			uint32_t codepoint;
			int chrsz;

			while ((chrsz=utf8_read_char(tok, &codepoint))) {
				int i;
				int xcode;

				if (chrsz == 1 && codepoint < 128) {
					for (i = 0; i < 256; i++) {
						const char *name = keycode_table[i].name;
						const char *shiftname = keycode_table[i].shifted_name;

						if (name && name[0] == tok[0] && name[1] == 0) {
							ADD_ENTRY(MACRO_KEYSEQUENCE, i);
							break;
						}

						if (shiftname && shiftname[0] == tok[0] && shiftname[1] == 0) {
							ADD_ENTRY(MACRO_KEYSEQUENCE, (MOD_SHIFT << 8) | i);
							break;
						}
					}
				} else if ((xcode = unicode_lookup_index(codepoint)) > 0)
					ADD_ENTRY(MACRO_UNICODE, xcode);

				tok += chrsz;
			}
		}
	}

	return 0;

	#undef ADD_ENTRY
}

void macro_execute(void (*output)(uint8_t, uint8_t),
		   const struct macro *macro, size_t timeout)
{
	size_t i;
	int hold_start = -1;

	for (i = 0; i < macro->sz; i++) {
		const struct macro_entry *ent = &macro->entries[i];

		switch (ent->type) {
			size_t j;
			uint16_t idx;
			uint8_t codes[4];
			uint8_t code, mods;

		case MACRO_HOLD:
			if (hold_start == -1)
				hold_start = i;

			output(ent->data, 1);

			break;
		case MACRO_RELEASE:
			if (hold_start != -1) {
				size_t j;

				for (j = hold_start; j < i; j++) {
					const struct macro_entry *ent = &macro->entries[j];
					output(ent->data, 0);
				}

				hold_start = -1;
			}
			break;
		case MACRO_UNICODE:
			idx = ent->data;

			unicode_get_sequence(idx, codes);

			for (j = 0; j < 4; j++) {
				output(codes[j], 1);
				output(codes[j], 0);
			}

			break;
		case MACRO_KEYSEQUENCE:
			code = ent->data;
			mods = ent->data >> 8;

			for (j = 0; j < ARRAY_SIZE(modifiers); j++) {
				uint8_t code = modifiers[j].key;
				uint8_t mask = modifiers[j].mask;

				if (mods & mask)
					output(code, 1);
			}

			if (mods && timeout)
				usleep(timeout);

			output(code, 1);
			output(code, 0);

			for (j = 0; j < ARRAY_SIZE(modifiers); j++) {
				uint8_t code = modifiers[j].key;
				uint8_t mask = modifiers[j].mask;

				if (mods & mask)
					output(code, 0);
			}


			break;
		case MACRO_TIMEOUT:
			usleep(ent->data * 1E3);
			break;
		}

		if (timeout)
			usleep(timeout);
	}
}
