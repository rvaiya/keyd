/* Copyright © 2019 Raheman Vaiya.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "descriptor.h"
#include "layer.h"
#include "keys.h"
#include "error.h"
#include "config.h"

#define MAX_ARGS 5

static int parse_fn(char *s,
		    char **name,
		    char *args[MAX_ARGS],
		    size_t *nargs)
{
	size_t len = strlen(s);
	size_t n = 0;
	char *arg, *c;

	if (len == 0 || s[len-1] != ')')
		return -1;

	s[len-1] = 0;

	if (!(c = strchr(s, '(')))
		return -1;

	*c = '\0';
	*name = s;
	s = c + 1;

	for (arg = strtok(s, ","); arg; arg = strtok(NULL, ",")) {
		while (isspace(arg[0]))
			arg++;

		if (n >= MAX_ARGS)
			return 0;

		args[n++] = arg;
	}

	*nargs = n;

	return 0;
}

static int parse_key_sequence(const char *s, struct key_sequence *seq)
{
	const char *c = s;
	uint16_t code;

	if (!*s)
		return -1;

	seq->mods = 0;

	while (c[1] == '-') {
		switch (*c) {
		case 'C':
			seq->mods |= MOD_CTRL;
			break;
		case 'M':
			seq->mods|= MOD_SUPER;
			break;
		case 'A':
			seq->mods |= MOD_ALT;
			break;
		case 'S':
			seq->mods |= MOD_SHIFT;
			break;
		case 'G':
			seq->mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		c += 2;
	}

	for (code = 0; code < KEY_MAX; code++) {
		const struct keycode_table_ent *ent = &keycode_table[code];

		if (ent->name) {
			if (ent->shifted_name &&
			    !strcmp(ent->shifted_name, c)) {

				seq->mods |= MOD_SHIFT;
				seq->code = code;

				return 0;
			} else if (!strcmp(ent->name, c) ||
				   (ent->alt_name && !strcmp(ent->alt_name, c))) {

				seq->code = code;

				return 0;
			}
		}
	}

	return -1;
}

static int parse_macro_fn(struct macro *macro, char *s)
{
	size_t len = strlen(s);

	char *tok;
	size_t sz = 0;

	if (strstr(s, "macro(") != s || s[len-1] != ')')
		return -1;

	s[len-1] = 0;

	macro->sz = 0;
	for (tok = strtok(s + 6, " "); tok; tok = strtok(NULL, " ")) {
		struct macro_entry ent;
		len = strlen(tok);

		if (!parse_key_sequence(tok, &ent.data.sequence)) {
			assert(sz < MAX_MACRO_SIZE);

			ent.type = MACRO_KEYSEQUENCE;

			macro->entries[sz++] = ent;
		} else if (len > 1 && tok[len-2] == 'm' && tok[len-1] == 's') {
			int len = atoi(tok);

			ent.type = MACRO_TIMEOUT;
			ent.data.timeout = len;

			assert(sz < MAX_MACRO_SIZE);
			macro->entries[sz++] = ent;
		} else {
			char *c;

			if (strchr(tok, '+')) {
				char *saveptr;
				char *key;

				for (key = strtok_r(tok, "+", &saveptr); key; key = strtok_r(NULL, "+", &saveptr)) {
					if (parse_key_sequence(key, &ent.data.sequence) < 0) {
						return -1;
					}

					ent.type = MACRO_HOLD;

					assert(sz < MAX_MACRO_SIZE);
					macro->entries[sz++] = ent;
				}

				ent.type = MACRO_RELEASE;
				assert(sz < MAX_MACRO_SIZE);
				macro->entries[sz++] = ent;
			} else {
				for (c = tok; *c; c++) {
					char s[2];

					s[0] = *c;
					s[1] = 0;

					if (parse_key_sequence(s, &ent.data.sequence) < 0) {
						return -1;
					}

					ent.type = MACRO_KEYSEQUENCE;

					assert(sz < MAX_MACRO_SIZE);
					macro->entries[sz++] = ent;
				}
			}
		}
	}

	macro->sz = sz;
	return 0;
}

static int config_get_free_macro_idx(struct config *config)
{
	size_t i;

	/* Scan the macro table for a free entry. */
	for (i = 0; i < sizeof(config->macros)/sizeof(config->macros[0]); i++)
		if (!config->macros[i].sz)
			return i;

	die("Max macros exceeded in %s\n", config->name);

	return -1;
}

/*
 * Modifies the input string, consumes a function which which is used for
 * resolving layer names as required.
 */
int parse_descriptor(char *s,
		     struct descriptor *desc,
		     struct config *config)
{
	struct key_sequence seq;

	char *fn;
	char *args[MAX_ARGS];
	size_t nargs = 0;
	int macro_idx = config_get_free_macro_idx(config);

	if (!parse_key_sequence(s, &seq)) {
		desc->op = OP_KEYSEQ;

		desc->args[0].sequence = seq;
	} else if (!parse_macro_fn(&config->macros[macro_idx], s)) {
		desc->op = OP_MACRO;
		desc->args[0].idx = macro_idx;
	} else if (!parse_fn(s, &fn, args, &nargs)) {
		int layer_idx;

		if (!strcmp(fn, "layer"))
			desc->op = OP_LAYER;
		else if (!strcmp(fn, "reset"))
			desc->op = OP_RESET;
		else if (!strcmp(fn, "toggle"))
			desc->op = OP_TOGGLE;
		else if (!strcmp(fn, "layout"))
			desc->op = OP_LAYOUT;
		else if (!strcmp(fn, "oneshot"))
			desc->op = OP_ONESHOT;
		else if (!strcmp(fn, "overload"))
			desc->op = OP_OVERLOAD;
		else if (!strcmp(fn, "swap"))
			desc->op = OP_SWAP;
		else {
			err("\"%s\" is not a valid action or key sequence.", s);
			return -1;
		}

		if (desc->op == OP_RESET)
			return 0;

		if (nargs == 0) {
			err("%s requires one or more arguments.", fn);
			return -1;
		}

		layer_idx = config_lookup_layer(config, args[0]);
		if (layer_idx == -1) {
			uint16_t mods;

			/* Autovivify modifier layers. */
			if (!parse_modset(args[0], &mods)) {
				layer_idx = config_create_layer(config, args[0], mods);
				assert(layer_idx > 0);
			} else {
				err("\"%s\" is not a valid layer", args[0]);
				return -1;
			}
		}

		if (desc->op == OP_LAYOUT && !config->layers[layer_idx].is_layout) {
			err("\"%s\" must be a valid layout.", args[0]);
			return -1;
		}

		desc->args[0].idx = layer_idx;
		desc->args[1].sequence = (struct key_sequence){0};

		if (nargs > 1) {
			int ret;
			ret = parse_key_sequence(args[1], &seq);

			if (ret < 0) {
				err("\"%s\" is not a valid key sequence", args[1]);
				return -1;
			}

			desc->args[1].sequence = seq;
		}

		if (desc->op == OP_OVERLOAD && nargs == 3) {
			desc->op = OP_OVERLOAD_TIMEOUT;
			desc->args[2].timeout = atoi(args[2]);
		}
	} else {
		err("\"%s\" is not a valid key sequence or action.", s);
		return -1;
	}

	return 0;
}
