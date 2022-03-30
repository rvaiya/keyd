/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "keyd.h"
#include "unicode.h"
#include "ini.h"

#define MAX_ARGS 5

/* modifies the input string */
static int parse_fn(char *s,
		    char **name,
		    char *args[MAX_ARGS],
		    size_t *nargs)
{
	char *c, *arg;

	c = s;
	while (*c && *c != '(')
		c++;

	if (!*c)
		return -1;

	*name = s;
	*c++ = 0;

	while (*c == ' ')
		c++;

	*nargs = 0;
	arg = c;
	while (1) {
		int plvl = 0;

		while (*c) {
			switch (*c) {
			case '\\':
				if (*(c+1)) {
					c+=2;
					continue;
				}
				break;
			case '(':
				plvl++;
				break;
			case ')':
				plvl--;

				if (plvl == -1)
					goto exit;
				break;
			case ',':
				if (plvl == 0)
					goto exit;
				break;
			}

			c++;
		}
exit:

		if (!*c)
			return -1;

		assert(*nargs < MAX_ARGS);
		args[(*nargs)++] = arg;

		if (*c == ')') {
			*c = 0;
			return 0;
		}

		*c++ = 0;
		while (*c == ' ')
			c++;
		arg = c;
	}
}

static uint8_t parse_code(const char *s)
{
	size_t i;

	for (i = 0; i < 256; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if ((ent->name && !strcmp(ent->name, s)) || 
		    (ent->alt_name && !strcmp(ent->alt_name, s)))
			return i;
	}

	return 0;
}

static int parse_sequence(const char *s, uint8_t *codep, uint8_t *modsp)
{
	const char *c = s;
	size_t i;

	if (!*s)
		return -1;

	uint8_t mods = 0;

	while (c[1] == '-') {
		switch (*c) {
		case 'C':
			mods |= MOD_CTRL;
			break;
		case 'M':
			mods |= MOD_SUPER;
			break;
		case 'A':
			mods |= MOD_ALT;
			break;
		case 'S':
			mods |= MOD_SHIFT;
			break;
		case 'G':
			mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		c += 2;
	}

	for (i = 0; i < 256; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name) {
			if (ent->shifted_name &&
			    !strcmp(ent->shifted_name, c)) {

				mods |= MOD_SHIFT;

				if (modsp)
					*modsp = mods;

				if (codep)
					*codep = i;

				return 0;
			} else if (!strcmp(ent->name, c) ||
				   (ent->alt_name && !strcmp(ent->alt_name, c))) {

				if (modsp)
					*modsp = mods;

				if (codep)
					*codep = i;

				return 0;
			}
		}
	}

	return -1;
}

/* 
 * Returns the character size in bytes, or 0 in the case of the empty string.
 */
static int utf8_read_char(const char *_s, uint32_t *code)
{
	const unsigned char *s = (const unsigned char*)_s;

	if (!s[0])
		return 0;

	if (s[0] >= 0xF0) {
		assert(s[1]);
		assert(s[2]);
		assert(s[3]);
		*code = (s[0] & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
		return 4;
	} else if (s[0] >= 0xE0) {
		assert(s[1]);
		assert(s[2]);
		*code = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
		return 3;
	} else if (s[0] >= 0xC0) {
		assert(s[1]);
		*code = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
		return 2;
	} else {
		*code = s[0] & 0x7F;
		return 1;
	}
}

static int utf8_strlen(const char *s)
{
	uint32_t code;
	int csz;
	int n = 0;

	while ((csz = utf8_read_char(s, &code))) {
		n++;
		s+=csz;
	}

	return n;
}

static void macro_add(struct macro *m, uint8_t type, uint16_t data)
{
	assert(m->sz < MAX_MACRO_SIZE);

	m->entries[m->sz].type = type;
	m->entries[m->sz].data = data;

	m->sz++;
}

static int do_parse_macro(struct macro *macro, char *s)
{
	char *tok;
	macro->sz = 0;

	for (tok = strtok(s, " "); tok; tok = strtok(NULL, " ")) {
		uint8_t code, mods;
		size_t len = strlen(tok);

		if (!parse_sequence(tok, &code, &mods)) {
			macro_add(macro, MACRO_KEYSEQUENCE, (mods << 8) | code);
		} else if (len > 1 && tok[len-2] == 'm' && tok[len-1] == 's') {
			macro_add(macro, MACRO_TIMEOUT, atoi(tok));
		} else if (strchr(tok, '+')) {
			char *saveptr;
			char *key;

			for (key = strtok_r(tok, "+", &saveptr); key; key = strtok_r(NULL, "+", &saveptr)) {
				if (parse_sequence(key, &code, &mods))
					return -1;

				macro_add(macro, MACRO_HOLD, code);
			}

			macro_add(macro, MACRO_RELEASE, 0);
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
							macro_add(macro, MACRO_KEYSEQUENCE, i);
							break;
						}

						if (shiftname && shiftname[0] == tok[0] && shiftname[1] == 0) {
							macro_add(macro, MACRO_KEYSEQUENCE, (MOD_SHIFT << 8) | i);
							break;
						}
					}
				} else if ((xcode = lookup_xcompose_code(codepoint)) > 0)
					macro_add(macro, MACRO_UNICODE, xcode);

				tok += chrsz;
			}
		}
	}

	return 0;
}

static size_t escape(char *s)
{
	int i = 0;
	int n = 0;

	for (i = 0; s[i]; i++) {
		if (s[i] == '\\') {
			switch (s[i+1]) {
			case 'n':
				s[n++] = '\n';
				break;
			case 't':
				s[n++] = '\t';
				break;
			case '\\':
				s[n++] = '\\';
				break;
			case ')':
				s[n++] = ')';
				break;
			case '(':
				s[n++] = '(';
				break;
			case 0:
				s[n] = 0;
				return n;
			default:
				s[n++] = '\\';
				s[n++] = s[i+1];
				break;
			}

			i++;
		} else {
			s[n++] = s[i];
		}
	}

	s[n] = 0;

	return n;
}


static int parse_macro(const char *exp, struct macro *macro)
{
	char s[MAX_MACROEXP_LEN];
	int len = strlen(exp);

	if (len >= MAX_MACROEXP_LEN) {
		err("macro exceeds maximum macro length (%d)", MAX_MACROEXP_LEN);
		return -1;
	}

	strcpy(s, exp);
	escape(s);
	len = strlen(s);

	if (!parse_sequence(s, NULL, NULL)) {
		return do_parse_macro(macro, s);
	} else if (strstr(s, "macro(") == s && s[len-1] == ')') {
		s[len-1] = 0;

		return do_parse_macro(macro, s+6);
	} else
		return -1;
}

/* Return up to two keycodes associated with the given name. */
static int lookup_keycodes(const char *name, uint8_t *code1, uint8_t *code2)
{
	size_t i;

	/*
	 * If the name is a modifier like 'control' we associate it with both
	 * corresponding key codes (e.g 'rightcontrol'/'leftcontrol')
	 */
	for (i = 0; i < MAX_MOD; i++) {
		const struct modifier_table_ent *mod = &modifier_table[i];

		if (!strcmp(mod->name, name)) {
			*code1 = mod->code1;
			*code2 = mod->code2;

			return 0;
		}
	}

	for (i = 0; i < 256; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name &&
		    (!strcmp(ent->name, name) ||
		     (ent->alt_name && !strcmp(ent->alt_name, name)))) {
			*code1 = i;
			*code2 = 0;

			return 0;
		}
	}

	return -1;
}

int layer_table_lookup(const struct layer_table *lt, const char *name)
{
	size_t i;

	for (i = 0; i < lt->nr; i++)
		if (!strcmp(lt->layers[i].name, name))
			return i;

	return -1;
}

/*
 * Consumes a string of the form `[<layer>.]<key> = <descriptor>` and adds the
 * mapping to the corresponding layer in the layer_table.
 */

int layer_table_add_entry(struct layer_table *lt, const char *exp)
{
	uint8_t code1, code2;
	char *keystr, *descstr, *c, *s;
	char *layername = "main";
	struct descriptor d;
	struct layer *layer;
	int idx;

	static char buf[MAX_EXP_LEN];

	if (strlen(exp) >= MAX_EXP_LEN) {
		err("%s exceeds maximum expression length (%d)", exp, MAX_EXP_LEN);
		return -1;
	}

	strcpy(buf, exp);
	s = buf;

	if ((c = strchr(s, '.'))) {
		layername = s;
		*c = 0;
		s = c+1;
	}

	if (parse_kvp(s, &keystr, &descstr) < 0) {
		err("Invalid key value pair.");
		return -1;
	}

	idx = layer_table_lookup(lt, layername);

	if (idx == -1) {
		err("%s is not a valid layer", layername);
		return -1;
	}

	layer = &lt->layers[idx];

	if (lookup_keycodes(keystr, &code1, &code2) < 0) {
		err("%s is not a valid key.", keystr);
		return -1;
	}

	if (parse_descriptor(descstr, &d, lt) < 0)
		return -1;

	if (code1)
		layer->keymap[code1] = d;

	if (code2)
		layer->keymap[code2] = d;

	return 0;
}

/*
 * Populate the provided layer described by `desc`, which is a string of the
 * form "<layer>[:<type>]".  The provided layer table is used to look up
 * constituent layers in the case of a composite descriptor string
 * (e.g "layer1+layer2").
 */

int create_layer(struct layer *layer, const char *desc, const struct layer_table *lt)
{
	uint8_t mods;
	char *name;
	char *modstr;

	static char s[MAX_LAYER_NAME_LEN];

	if (strlen(desc) >= sizeof(s)) {
		err("%s exceeds max layer length (%d)", desc, MAX_LAYER_NAME_LEN);
		return -1;
	}

	strcpy(s, desc);

	name = strtok(s, ":");
	modstr = strtok(NULL, ":");

	strcpy(layer->name, name);

	if (strchr(name, '+')) {
		char *layern;
		int n = 0;
		int layers[MAX_COMPOSITE_LAYERS];

		if (modstr) {
			err("composite layers cannot have a modifier set.");
			return -1;
		}

		for (layern = strtok(name, "+"); layern; layern = strtok(NULL, "+")) {
			int idx = layer_table_lookup(lt, layern);
			if (idx < 0) {
				err("%s is not a valid layer", layern);
				return -1;
			}

			if (n >= MAX_COMPOSITE_LAYERS) {
				err("max composite layers (%d) exceeded", MAX_COMPOSITE_LAYERS);
				return -1;
			}

			layers[n++] = idx;
		}

		layer->type = LT_COMPOSITE;
		layer->nr_layers = n;
		memcpy(layer->layers, layers, sizeof(layer->layers));
	}  else if (modstr && !parse_modset(modstr, &mods)) {
			layer->type = LT_NORMAL;
			layer->mods = mods;
	} else {
		if (modstr)
			fprintf(stderr, "WARNING: \"%s\" is not a valid modifier set, ignoring\n", modstr);

		layer->type = LT_NORMAL;
		layer->mods = 0;
	}


	dbg("created [%s] from \"%s\"", layer->name, desc);
	return 0;
}

/* 
 * Returns:
 *
 * > 0 if exp is a valid macro but the macro table is full
 * < 0 in the case of an invalid macro
 * 0 on success
 */
int set_macro_arg(struct descriptor *d, int idx, struct layer_table *lt, const char *exp)
{
	if (lt->nr_macros >= MAX_MACROS) {
		err("max macros (%d), exceeded", MAX_MACROS);
		return 1;
	}

	if (parse_macro(exp, &lt->macros[lt->nr_macros]) < 0) {
		err("\"%s\" is not a valid macro", exp);
		return -1;
	}

	d->args[idx].idx = lt->nr_macros;

	lt->nr_macros++;

	return 0;
}

/*
 * Modifies the input string. Layers names within the descriptor
 * are resolved using the provided layer table.
 */
int parse_descriptor(const char *descstr,
		     struct descriptor *d,
		     struct layer_table *lt)
{
	char *fn = NULL;
	char *args[MAX_ARGS];
	size_t nargs = 0;
	uint8_t code;
	int idx;
	int ret;

	char fnstr[MAX_DESCRIPTOR_LEN+1];

	if (strlen(descstr) > MAX_DESCRIPTOR_LEN) {
		err("maximum descriptor length exceeded");
		return -1;
	}

	strcpy(fnstr, descstr);

	if ((code = parse_code(descstr))) {
		d->op = OP_KEYCODE;
		d->args[0].code = code;

		/* TODO: fixme. */
		if (keycode_to_mod(code))
			fprintf(stderr,
				"WARNING: mapping modifier keycodes directly may produce unintended results, you probably want layer(<modifier name>) instead\n");
	} else if ((ret=set_macro_arg(d, 0, lt, descstr)) >= 0) {
		if (ret > 0)
			return -1;
		else
			d->op = OP_MACRO;
	} else if (!parse_fn(fnstr, &fn, args, &nargs)) {
		if (!strcmp(fn, "layer"))
			d->op = OP_LAYER;
		else if (!strcmp(fn, "toggle"))
			d->op = OP_TOGGLE;
		else if (!strcmp(fn, "oneshot"))
			d->op = OP_ONESHOT;
		else if (!strcmp(fn, "overload"))
			d->op = OP_OVERLOAD;
		else if (!strcmp(fn, "swap")) {
			d->op = OP_SWAP;
		} else if (!strcmp(fn, "timeout")) {
			d->op = OP_TIMEOUT;
		} else {
			err("\"%s\" is not a valid action or macro.", descstr);
			return -1;
		}

		if (nargs == 0) {
			err("%s requires one or more arguments.", fn);
			return -1;
		}

		if (d->op == OP_TIMEOUT) {
			struct timeout *timeout;

			if (nargs != 3) {
				err("timeout requires 3 arguments.");
				return -1;
			}

			d->op = OP_TIMEOUT;
			d->args[0].idx = lt->nr_timeouts;

			if (lt->nr_timeouts >= MAX_TIMEOUTS) {
				err("max timeouts (%d) exceeded", MAX_TIMEOUTS);
				return -1;
			}

			timeout = &lt->timeouts[lt->nr_timeouts];

			if (parse_descriptor(args[0], &timeout->d1, lt) < 0)
				return -1;

			if (parse_descriptor(args[2], &timeout->d2, lt) < 0)
				return -1;

			timeout->timeout = atoi(args[1]);

			lt->nr_timeouts++;
			return 0;
		}

		idx = layer_table_lookup(lt, args[0]);

		if (idx == -1) {
			err("%s is not a valid layer.", args[0]);
			return -1;
		}

		d->args[0].idx = idx;
		d->args[1].idx = -1;

		if (nargs > 1)
			return set_macro_arg(d, 1, lt, args[1]);
	} else if (utf8_strlen(descstr) == 1) {
		char buf[32];
		sprintf(buf, "macro(%s)", descstr);
		d->op = OP_MACRO;

		if (set_macro_arg(d, 0, lt, buf))
			return -1;
	} else {
		err("invalid key or action");
		return -1;
	}

	return 0;
}
