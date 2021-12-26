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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>
#include "config.h"
#include "keys.h"

#define MAX_ARGS 10

struct keyboard_config *configs = NULL;

struct layer_table_ent {
	char name[256];
	char pname[256];

	uint8_t modsonly;
	struct layer *layer;
} layer_table[MAX_LAYERS];

int nlayers = 0;

static int lnum = 0;
static char path[PATH_MAX];

static uint32_t macros[MAX_MACROS][MAX_MACRO_SIZE];
static size_t nmacros = 0;

static int lookup_layer(const char *name);
static int parse_modset(const char *s, uint16_t * mods);

#define err(fmt, ...) fprintf(stderr, "%s: ERROR on line %d: "fmt"\n", path, lnum, ##__VA_ARGS__)

static const char *modseq_to_string(uint16_t mods)
{
	static char s[32];
	int i = 0;
	s[0] = '\0';

	if (mods & MOD_CTRL) {
		strcpy(s + i, "-C");
		i += 2;
	}
	if (mods & MOD_SHIFT) {
		strcpy(s + i, "-S");
		i += 2;
	}

	if (mods & MOD_SUPER) {
		strcpy(s + i, "-M");
		i += 2;
	}

	if (mods & MOD_ALT) {
		strcpy(s + i, "-A");
		i += 2;
	}

	if (mods & MOD_ALT_GR) {
		strcpy(s + i, "-G");
		i += 2;
	}

	if (i)
		return s + 1;
	else
		return s;
}

const char *keyseq_to_string(uint32_t keyseq)
{
	int i = 0;
	static char s[256];

	uint16_t mods = keyseq >> 16;
	uint16_t code = keyseq & 0x00FF;

	const char *key = keycode_table[code].name;

	if (mods & MOD_CTRL) {
		strcpy(s + i, "C-");
		i += 2;
	}
	if (mods & MOD_SHIFT) {
		strcpy(s + i, "S-");
		i += 2;
	}

	if (mods & MOD_SUPER) {
		strcpy(s + i, "M-");
		i += 2;
	}

	if (mods & MOD_ALT) {
		strcpy(s + i, "A-");
		i += 2;
	}

	if (mods & MOD_ALT_GR) {
		strcpy(s + i, "G-");
		i += 2;
	}

	if (key)
		strcpy(s + i, keycode_table[code].name);
	else
		strcpy(s + i, "UNKNOWN");

	return s;
}

//Returns the position in the layer table
static struct layer_table_ent *create_layer(const char *name,
					    const char *pname,
					    uint16_t mods)
{
	struct layer_table_ent *ent = &layer_table[nlayers];

	ent->layer = calloc(1, sizeof(struct layer));
	ent->layer->keymap =
	    calloc(KEY_CNT, sizeof(struct key_descriptor));

	ent->modsonly = 0;
	ent->layer->mods = mods;

	strcpy(ent->pname, pname);
	strcpy(ent->name, name);

	nlayers++;
	assert(nlayers <= MAX_LAYERS);
	return ent;
}

static struct layer_table_ent *create_mod_layer(uint16_t mods)
{
	struct layer_table_ent *ent = create_layer("", "", mods);
	ent->modsonly = 1;

	return ent;
}

static void keyseq_to_desc(uint32_t seq, struct key_descriptor *desc)
{
	desc->action = ACTION_KEYSEQ;
	desc->arg.keyseq = seq;

	//To facilitate simplification of modifier handling convert
	//all traditional modifier keys to their internal layer
	//representations.

	switch (seq) {
	case KEY_LEFTCTRL:
		desc->action = ACTION_LAYER;
		desc->arg.layer = lookup_layer("C");
		break;
	case KEY_LEFTALT:
		desc->action = ACTION_LAYER;
		desc->arg.layer = lookup_layer("A");
		break;
	case KEY_RIGHTALT:
		desc->action = ACTION_LAYER;
		desc->arg.layer = lookup_layer("G");
		break;
	case KEY_LEFTSHIFT:
		desc->action = ACTION_LAYER;
		desc->arg.layer = lookup_layer("S");
		break;
	case KEY_LEFTMETA:
		desc->action = ACTION_LAYER;
		desc->arg.layer = lookup_layer("M");
		break;
	}
}

static struct layer_table_ent *create_main_layer()
{
	int i;
	struct layer_table_ent *ent = create_layer("main", "", 0);

	for (i = 0; i < KEY_CNT; i++)
		keyseq_to_desc(i, &ent->layer->keymap[i]);

	return ent;
}

//Returns the index in the layer table or -1.
static int lookup_layer(const char *name)
{
	int i;
	uint16_t mods;

	for (i = 0; i < nlayers; i++) {
		if (!strcmp(layer_table[i].name, name))
			return i;
	}

	if (!parse_modset(name, &mods)) {
		//Check if a dedicated mod layer already exists.
		for (i = 0; i < nlayers; i++) {
			struct layer_table_ent *ent = &layer_table[i];
			if (ent->modsonly && ent->layer->mods == mods)
				return i;
		}

		//Autovivify mod layers which don't exist.

		create_mod_layer(mods);
		return nlayers - 1;
	}

	return -1;
}


static int parse_modset(const char *s, uint16_t * mods)
{
	*mods = 0;

	while (*s) {
		switch (*s) {
		case 'C':
			*mods |= MOD_CTRL;
			break;
		case 'M':
			*mods |= MOD_SUPER;
			break;
		case 'A':
			*mods |= MOD_ALT;
			break;
		case 'S':
			*mods |= MOD_SHIFT;
			break;
		case 'G':
			*mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		if (s[1] == 0)
			return 0;
		else if (s[1] != '-')
			return -1;

		s += 2;
	}

	return 0;
}

static int parse_layer_heading(const char *s, char name[256],
			       char parent[256])
{
	size_t len = strlen(s);
	char *c;

	name[0] = 0;
	parent[0] = 0;

	if (s[0] != '[' || s[len - 1] != ']')
		return -1;

	c = strchr(s, ':');
	if (c) {
		int sz = c - (s + 1);
		memcpy(name, s + 1, sz);
		name[sz] = 0;

		sz = len - 2 - sz - 1;
		memcpy(parent, c + 1, sz);
		parent[sz] = 0;
	} else {
		int sz = len - 2;
		memcpy(name, s + 1, sz);
		name[sz] = 0;
	}

	return 0;
}

static uint32_t parse_keyseq(const char *s)
{
	const char *c = s;
	size_t i;
	uint32_t mods = 0;

	while (*c && c[1] == '-') {
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
			return 0;
			break;
		}

		c += 2;
	}

	for (i = 0; i < KEY_MAX; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name && !strcmp(ent->name, c))
			return (mods << 16) | i;
		if (ent->alt_name && !strcmp(ent->alt_name, c))
			return (mods << 16) | i;
		if (ent->shifted_name && !strcmp(ent->shifted_name, c))
			return (mods | MOD_SHIFT) << 16 | i;
	}

	return 0;
}

uint32_t *parse_macro_fn(const char *s, size_t *szp)
{
	char *_s = strdup(s);
	size_t len = strlen(s);
	uint32_t *macro = macros[nmacros];

	char *tok;
	size_t sz = 0;

	assert(nmacros < MAX_MACROS);

	if (strstr(s, "macro(") != s || s[len - 1] != ')') {
		free(_s);
		return NULL;
	}

	_s[len - 1] = 0;

	for (tok = strtok(_s + 6, " "); tok; tok = strtok(NULL, " ")) {
		uint32_t seq;
		len = strlen(tok);

		if ((seq = parse_keyseq(tok))) {
			macro[sz++] = seq;
			assert(sz < MAX_MACRO_SIZE);
		} else if (len > 1 && tok[len - 1] == 's'
			   && tok[len - 2] == 'm') {
			int len = atoi(tok);
			assert(len <= MAX_TIMEOUT_LEN);

			macro[sz++] = TIMEOUT_KEY(len);
		} else {
			char *c;

			for (c = tok; *c; c++) {
				char s[2];

				s[0] = *c;
				s[1] = 0;

				if (!(seq = parse_keyseq(s))) {
					free(_s);
					return NULL;
				}

				macro[sz++] = seq;
				assert(sz < MAX_MACRO_SIZE);
			}
		}
	}

	free(_s);

	*szp = sz;
	nmacros++;
	return macro;
}

static int parse_kvp(char *s, char **_k, char **_v)
{
	char *v = NULL, *k = NULL;

	while (*s && isspace(*s))
		s++;
	if (!*s)
		return -1;
	k = s;

	if (*s == '=')
		s++;		//Allow the first character to be = as a special case.

	while (*s && !isspace(*s) && *s != '=')
		s++;
	if (!*s)
		return -1;

	while (*s && *s != '=') {
		*s = 0;
		s++;
	}
	if (!*s)
		return -1;
	*s++ = 0;

	while (*s && isspace(*s))
		*s++ = 0;
	if (!*s)
		return -1;
	v = s;

	*_k = k;
	*_v = v;
	return 0;
}

static int parse_fn(char *s, char **fn_name, char *args[MAX_ARGS],
		    size_t *nargs)
{
	int openparen = 0;

	*fn_name = s;
	*nargs = 0;

	while (*s && *s != ')') {
		switch (*s) {
		case '(':
			openparen++;
			*s = '\0';
			s++;

			while (*s && isspace(*s))
				s++;

			if (!*s)
				return -1;

			if (*s == ')') {	//No args
				*s = '\0';
				return 0;
			}

			args[(*nargs)++] = s;

			break;
		case ',':
			*s = '\0';
			s++;
			while (*s && isspace(*s))
				s++;

			if (!*s)
				return -1;

			args[(*nargs)++] = s;
			break;
		}

		s++;
	}

	if (*s != ')')
		return -1;

	*s = '\0';

	return 0;
}

static int parse_descriptor(const char *_s, struct key_descriptor *desc)
{
	uint32_t seq;

	char *s = strdup(_s);
	char *fn;
	char *args[MAX_ARGS];
	size_t nargs, sz;
	uint32_t *macro;

	if ((seq = parse_keyseq(s))) {
		keyseq_to_desc(seq, desc);

		goto cleanup;
	}

	if ((macro = parse_macro_fn(s, &sz))) {
		desc->action = ACTION_MACRO;
		desc->arg.macro = macro;
		desc->arg2.sz = sz;

		goto cleanup;
	}

	if (parse_fn(s, &fn, args, &nargs)) {
		err("%s is not a valid key sequence or action.", s);
		goto fail;
	}

	if (!strcmp(fn, "layer") && nargs == 1) {
		int idx = lookup_layer(args[0]);

		if (idx < 0) {
			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		desc->action = ACTION_LAYER;
		desc->arg.layer = idx;

		goto cleanup;
	} else if (!strcmp(fn, "layert") && nargs == 1) {
		int idx = lookup_layer(args[0]);

		if (idx < 0) {
			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		desc->action = ACTION_LAYER_TOGGLE;
		desc->arg.layer = idx;

		goto cleanup;
	} else if (!strcmp(fn, "layout") && nargs > 0) {
		int idx;

		if ((idx = lookup_layer(args[0])) < 0) {
			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		desc->action = ACTION_LAYOUT;
		desc->arg.layer = idx;

		if (nargs == 1) {
			desc->arg2.layer = idx;
		} else {
			if ((idx = lookup_layer(args[1])) < 0) {
				err("%s is not a valid layer.", args[1]);
				goto fail;
			}

			desc->action = ACTION_LAYOUT;
			desc->arg2.layer = idx;
		}


		goto cleanup;
	} else if (!strcmp(fn, "oneshot") && nargs == 1) {
		int idx;

		if ((idx = lookup_layer(args[0])) < 0) {

			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		desc->action = ACTION_ONESHOT;
		desc->arg.layer = idx;

		goto cleanup;
	} else if (!strcmp(fn, "overload") && nargs == 2) {
		int idx;

		if ((idx = lookup_layer(args[0])) < 0) {
			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		if (!(seq = parse_keyseq(args[1]))) {
			err("%s is not a valid key sequence.", args[1]);
			goto fail;
		}

		desc->action = ACTION_OVERLOAD;
		desc->arg.keyseq = seq;
		desc->arg2.layer = idx;

		goto cleanup;
	} else if (!strcmp(fn, "swap")) {
		int idx;

		if ((idx = lookup_layer(args[0])) < 0) {
			err("%s is not a valid layer.", args[0]);
			goto fail;
		}

		desc->action = ACTION_SWAP;
		desc->arg.layer = idx;
		desc->arg2.keyseq = 0;

		if (nargs == 2) {
			if (!(desc->arg2.keyseq = parse_keyseq(args[1]))) {
				err("%s is not a valid key sequence.",
				    args[1]);
				goto fail;
			}
		}

		goto cleanup;
	} else {
		err("%s is not a valid action or key.", _s);
		goto fail;
	}


      cleanup:
	free(s);
	return 0;

      fail:
	free(s);
	return -1;
}

static void build_layer_table()
{
	ssize_t len;
	char *line = NULL;
	size_t line_sz = 0;
	FILE *fh = fopen(path, "r");

	if (!fh) {
		perror("fopen");
		exit(-1);
	}

	lnum = 0;
	nlayers = 0;

	create_main_layer();
	while ((len = getline(&line, &line_sz, fh)) != -1) {
		char name[256];
		char pname[256];

		lnum++;
		if (line[len - 1] == '\n')
			line[--len] = 0;

		if (!parse_layer_heading(line, name, pname)) {
			uint16_t mods;

			if (!parse_modset(pname, &mods))
				create_layer(name, "", mods);
			else
				create_layer(name, pname, 0);
		}
	}

	free(line);
	fclose(fh);
}

static int parse_layer_entry(char *s, uint16_t * code,
			     struct key_descriptor *desc)
{
	uint32_t seq;
	char *k, *v;

	if (parse_kvp(s, &k, &v)) {
		err("Invalid key value pair.");
		return -1;
	}

	if (!(seq = parse_keyseq(k))) {
		err("'%s' is not a valid key.", k);
		return -1;
	}

	if ((seq >> 16) != 0) {
		err("key cannot contain modifiers.");
		return -1;
	}

	if (parse_descriptor(v, desc))
		return -1;

	*code = seq & 0xFF;
	return 0;
}

void post_process_config(struct keyboard_config *cfg,
			 const char *layout_name,
			 const char *modlayout_name)
{
	cfg->default_layout = -1;
	cfg->default_modlayout = -1;

	int i;
	for (i = 0; i < nlayers; i++) {
		struct layer_table_ent *ent = &layer_table[i];

		if (!strcmp(ent->name, layout_name))
			cfg->default_layout = i;

		if (!strcmp(ent->name, modlayout_name))
			cfg->default_modlayout = i;

		if (ent->pname[0]) {
			int j;

			for (j = 0; j < nlayers; j++) {
				struct layer_table_ent *ent2 =
				    &layer_table[j];

				if (!strcmp(ent->pname, ent2->name)) {
					int k;

					struct layer *dst = ent->layer;
					struct layer *src = ent2->layer;

					for (k = 0; k < KEY_CNT; k++)
						if (dst->keymap[k].
						    action ==
						    ACTION_UNDEFINED)
							dst->keymap[k] =
							    src->keymap[k];
				}
			}
		}
	}

	if (cfg->default_layout == -1) {
		fprintf(stderr, "%s is not a valid default layout",
			layout_name);
		cfg->default_layout = 0;
	}

	if (cfg->default_modlayout == -1) {
		fprintf(stderr,
			"%s is not a valid default modifier layout\n",
			modlayout_name);
		cfg->default_modlayout = 0;
	}
}

static void parse(struct keyboard_config *cfg)
{
	int i;
	char *line = NULL;
	size_t n = 0;
	ssize_t len;
	struct layer *layer;

	char layout_name[256];
	char modlayout_name[256];

	build_layer_table();

	FILE *fh = fopen(path, "r");
	if (!fh) {
		perror("fopen");
		exit(-1);
	}

	lnum = 0;
	layer = layer_table[0].layer;

	strcpy(layout_name, "main");
	strcpy(modlayout_name, "main");

	while ((len = getline(&line, &n, fh)) != -1) {
		char name[256];
		char type[256];

		uint16_t code;
		struct key_descriptor desc;

		size_t nargs;
		char *fnname;
		char *args[MAX_ARGS];


		lnum++;
		char *s = line;

		while (len && isspace(s[0])) {	//Trim leading whitespace
			s++;
			len--;
		}

		if (len && s[len - 1] == '\n')	//Strip tailing newline (not present on EOF)
			s[--len] = '\0';

		if (len == 0 || s[0] == '#')
			continue;

		if (!parse_layer_heading(s, name, type)) {
			int idx = lookup_layer(name);
			assert(idx > 0);
			layer = layer_table[idx].layer;
		} else if (strchr(s, '=')) {
			if (!parse_layer_entry(s, &code, &desc))
				layer->keymap[code] = desc;
			else
				err("Invalid layer entry.");
		} else if (!parse_fn(s, &fnname, args, &nargs)) {
			if (!strcmp(fnname, "layout")) {
				if (nargs == 1) {
					strcpy(layout_name, args[0]);
					strcpy(modlayout_name, args[0]);
				} else if (nargs == 2) {
					strcpy(layout_name, args[0]);
					strcpy(modlayout_name, args[1]);
				}
			}
		}

		free(line);
		line = NULL;
		n = 0;
	}

	free(line);
	fclose(fh);

	post_process_config(cfg, layout_name, modlayout_name);

	for (i = 0; i < nlayers; i++)
		cfg->layers[i] = layer_table[i].layer;

	cfg->nlayers = nlayers;

	return;
}

void config_free()
{
	struct keyboard_config *cfg = configs;

	while (cfg) {
		size_t i;
		struct keyboard_config *tmp = cfg;

		cfg = cfg->next;

		for (i = 0; i < tmp->nlayers; i++) {
			struct layer *layer = tmp->layers[i];

			free(layer->keymap);
			free(layer);
		}

		free(tmp);
	};
}

void config_generate(const char *config_dir)
{
	struct dirent *ent;
	DIR *dh = opendir(config_dir);

	if (!dh) {
		perror("opendir");
		exit(-1);
	}

	while ((ent = readdir(dh))) {
		struct keyboard_config *cfg;

		int len = strlen(ent->d_name);
		if (len <= 4 || strcmp(ent->d_name + len - 4, ".cfg"))
			continue;


		sprintf(path, "%s/%s", config_dir, ent->d_name);

		cfg = calloc(1, sizeof(struct keyboard_config));

		strcpy(cfg->name, ent->d_name);
		cfg->name[len - 4] = '\0';

		parse(cfg);

		cfg->next = configs;
		configs = cfg;
	}

	closedir(dh);
}
