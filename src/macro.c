/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <string.h>
#include <assert.h>

#include "macro.h"
#include "keys.h"
#include "error.h"
#include "unicode.h"
#include "string.h"

static void add_entry(struct macro *m, uint8_t type, uint16_t data)
{
	assert(m->sz < MAX_MACRO_SIZE);

	m->entries[m->sz].type = type;
	m->entries[m->sz].data = data;

	m->sz++;
}

static int parse(struct macro *macro, char *s)
{
	char *tok;
	macro->sz = 0;

	for (tok = strtok(s, " "); tok; tok = strtok(NULL, " ")) {
		uint8_t code, mods;
		size_t len = strlen(tok);

		if (!parse_key_sequence(tok, &code, &mods)) {
			add_entry(macro, MACRO_KEYSEQUENCE, (mods << 8) | code);
		} else if (strchr(tok, '+')) {
			char *saveptr;
			char *key;

			for (key = strtok_r(tok, "+", &saveptr); key; key = strtok_r(NULL, "+", &saveptr)) {
				size_t len = strlen(key);

				if (len > 1 && key[len-2] == 'm' && key[len-1] == 's')
					add_entry(macro, MACRO_TIMEOUT, atoi(key));
				else if (!parse_key_sequence(key, &code, &mods))
					add_entry(macro, MACRO_HOLD, code);
				else
					return -1;
			}

			add_entry(macro, MACRO_RELEASE, 0);
		} else if (len > 1 && tok[len-2] == 'm' && tok[len-1] == 's') {
			add_entry(macro, MACRO_TIMEOUT, atoi(tok));
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
							add_entry(macro, MACRO_KEYSEQUENCE, i);
							break;
						}

						if (shiftname && shiftname[0] == tok[0] && shiftname[1] == 0) {
							add_entry(macro, MACRO_KEYSEQUENCE, (MOD_SHIFT << 8) | i);
							break;
						}
					}
				} else if ((xcode = lookup_xcompose_code(codepoint)) > 0)
					add_entry(macro, MACRO_UNICODE, xcode);

				tok += chrsz;
			}
		}
	}

	return 0;
}

int parse_macro(const char *exp, struct macro *macro)
{
	char s[MAX_MACROEXP_LEN+1];
	int len = strlen(exp);

	if (len > MAX_MACROEXP_LEN) {
		err("macro exceeds maximum macro length (%d)", MAX_MACROEXP_LEN);
		return -1;
	}

	strcpy(s, exp);
	str_escape(s);
	len = strlen(s);

	if (!parse_key_sequence(s, NULL, NULL) || utf8_strlen(s) == 1) {
		return parse(macro, s);
	} else if (strstr(s, "macro(") == s && s[len-1] == ')') {
		s[len-1] = 0;

		return parse(macro, s+6);
	} else
		return -1;
}
