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

static char layers[MAX_LAYERS][MAX_LAYER_NAME_LEN]; //Layer names
static size_t nlayers;
static int parents[MAX_LAYERS];

static int lnum = 0;
static char path[PATH_MAX];

#define err(fmt, ...) fprintf(stderr, "%s: ERROR on line %d: "fmt"\n", path, lnum, ##__VA_ARGS__)

static const char *modseq_to_string(uint8_t mods) {
	static char s[32];
	int i = 0;
	s[0] = '\0';

	if(mods & MOD_CTRL) {
		strcpy(s+i, "-C");
		i+=2;
	} 
	if(mods & MOD_SHIFT) {
		strcpy(s+i, "-S");
		i+=2;
	}

	if(mods & MOD_SUPER) {
		strcpy(s+i, "-M");
		i+=2;
	} 

	if(mods & MOD_ALT) {
		strcpy(s+i, "-A");
		i+=2;
	}

	if(i)
		return s+1;
	else
		return s;
}

static const char *keyseq_to_string(uint32_t keyseq) {
	int i = 0;
	static char s[256];

	uint8_t mods = keyseq >> 16;
	uint16_t code = keyseq & 0x00FF;

	const char *key = keycode_strings[code];

	if(mods & MOD_CTRL) {
		strcpy(s+i, "C-");
		i+=2;
	} 
	if(mods & MOD_SHIFT) {
		strcpy(s+i, "S-");
		i+=2;
	}

	if(mods & MOD_SUPER) {
		strcpy(s+i, "M-");
		i+=2;
	} 

	if(mods & MOD_ALT) {
		strcpy(s+i, "A-");
		i+=2;
	}

	if(key)
		strcpy(s+i, keycode_strings[code]);
	else
		strcpy(s+i, "UNKNOWN");

	return s;
}

static int parse_mods(const char *s, uint8_t *mods) 
{
	*mods = 0;

	while(*s) {
		switch(*s) {
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
		default:
			return -1;
			break;
		}

		if(s[1] == 0)
			return 0;
		else if(s[1] != '-') {
			err("%s is not a valid modifier set", s);
			return -1;
		}

		s+=2;
	}

	return 0;
}

static int parse_keyseq(const char *s, uint16_t *keycode, uint8_t *mods) {
	const char *c = s;
	size_t i;

	*mods = 0;
	*keycode = 0;

	while(*c && c[1] == '-') {
		switch(*c) {
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
		default:
			return -1;
			break;
		}

		c += 2;
	}

	for(i = 0;i < sizeof keycode_strings / sizeof keycode_strings[0];i++) {
		if(keycode_strings[i] && !strcmp(keycode_strings[i], c)) {
			*keycode |= i;
			return 0;
		}
	}

	return -1;
}

static int parse_kvp(char *s, char **_k, char **_v)
{
	char *v = NULL, *k = NULL;

	while(*s && isspace(*s)) s++;
	if(!*s) return -1;
	k = s;

	while(*s && !isspace(*s) && *s != '=') s++;
	if(!*s) return -1;

	while(*s && *s != '=') {
		*s = 0;
		s++;
	}
	if(!*s) return -1;
	*s++ = 0;

	while(*s && isspace(*s)) *s++ = 0;
	if(!*s) return -1;
	v = s;

	*_k = k;
	*_v = v;
	return 0;
}

static void parse_layer_names()
{
	size_t i;
	char *line = NULL;
	size_t n;
	ssize_t len;
	char strparents[MAX_LAYERS][MAX_LAYER_NAME_LEN] = {0};

	FILE *fh = fopen(path, "r");
	if(!fh) {
		fprintf(stderr, "ERROR: Failed to open %s\n", path);
		perror("fopen");
		exit(-1);
	}


	nlayers = 0;
	strcpy(layers[nlayers++], "default");
	while((len=getline(&line, &n, fh)) != -1) {
		char *s = line;

		while(len && isspace(s[0])) {
			s++;
			len--;
		}

		if(len > 2 && s[0] == '[' && s[len-2] == ']') {
			const char *name = s+1;
			size_t idx = nlayers;
			char *c;

			s[len-2] = '\0';

			if((c=strchr(name, ':'))) {
				const char *parent = c+1;
				*c = '\0';

				strcpy(strparents[idx], parent);
			}

			strcpy(layers[idx], name);
			nlayers++;
		}

		free(line);
		line = NULL;
		n = 0;
	}

	free(line);
	fclose(fh);

	for(i = 0;i < nlayers;i++) {
		size_t j;

		parents[i] = -1;

		for(j = 0;j < nlayers;j++) {
			if(!strcmp(layers[j], strparents[i])) {
				parents[i] = j;
			}
		}
		if(strparents[i][0] != 0 && parents[i] == -1) {
			err("%s is not a valid parent layer", strparents[i]);
		}
	}
}

static int parse_fn(char *s, char **fn_name, char *args[MAX_ARGS], size_t *nargs)
{
	int openparen = 0;

	*fn_name = s;
	*nargs = 0;

	while(*s && *s != ')') {
		switch(*s) {
		case '(':
			openparen++;
			*s = '\0';
			s++;

			while(*s && isspace(*s)) 
				s++;

			if(!*s) {
				err("Missing closing parenthesis.");
				return -1;
			}

			if(*s == ')') { //No args
				*s = '\0';
				return 0;
			}

			args[(*nargs)++] = s;

			break;
		case ',':
			*s = '\0';
			s++;
			while(*s && isspace(*s)) 
				s++;

			if(!*s) {
				err("Missing closing parenthesis.");
				return -1;
			}

			args[(*nargs)++] = s;
			break;
		}

		s++;
	}

	if(*s != ')') {
		if(openparen)
			err("Missing closing parenthesis.");
		else
			err("Invalid function or key sequence.");

		return -1;
	}

	*s = '\0';

	return 0;
}

static int parse_val(const char *_s, struct key_descriptor *desc)
{
	uint16_t code;
	uint8_t mods;

	char *s = strdup(_s);
	char *fn;
	char *args[MAX_ARGS];
	size_t nargs;

	if(!parse_keyseq(s, &code, &mods)) {
		desc->action = ACTION_KEYSEQ;
		desc->arg.keyseq = ((uint32_t)mods << 16) | (uint32_t)code;

		goto cleanup;
	} 

	if(parse_fn(s, &fn, args, &nargs))
		goto fail;

	if(!strcmp(fn, "oneshot_layer") && nargs == 1) {
		size_t i;

		for(i = 0;i < nlayers;i++)
			if(!strcmp(args[0], layers[i])) {
				desc->action = ACTION_LAYER_ONESHOT;
				desc->arg.layer = i;

				goto cleanup;
			}

		err("%s is not a valid layer name.", args[0]);
		goto fail;
	} else if(!strcmp(fn, "layer") && nargs == 1) {
		size_t i;

		for(i = 0;i < nlayers;i++)
			if(!strcmp(args[0], layers[i])) {
				desc->action = ACTION_LAYER;
				desc->arg.layer = i;

				goto cleanup;
			}

		err("%s is not a valid layer name.", args[0]);
		goto fail;
	} else if(!strcmp(fn, "layer_toggle") && nargs == 1) {
		size_t i;

		for(i = 0;i < nlayers;i++)
			if(!strcmp(args[0], layers[i])) {
				desc->action = ACTION_LAYER_TOGGLE;
				desc->arg.layer = i;

				goto cleanup;
			}

		err("%s is not a valid layer name.", args[0]);
		goto fail;
	} else if(!strcmp(fn, "oneshot") && nargs == 1) {
		if(parse_mods(args[0], &mods))
			goto fail;

		desc->action = ACTION_ONESHOT;
		desc->arg.mods = mods;

	} else if(!strcmp(fn, "layer_on_hold") && nargs == 2) {
		size_t i;

		desc->action = ACTION_DOUBLE_LAYER;

		if(parse_keyseq(args[1], &code, &mods)) {
			err("%s is not a vaid keysequence.", args[1]);
			goto fail;
		}

		desc->arg2.keyseq = ((uint32_t)mods << 16) | (uint32_t)code;

		for(i = 0;i < nlayers;i++)
			if(!strcmp(args[0], layers[i])) {
				desc->arg.layer = i;
				goto cleanup;
			}


		err("%s is not a valid layer.", args[0]);
		goto fail;
	} else if(!strcmp(fn, "mods_on_hold") && nargs == 2) {
		desc->action = ACTION_DOUBLE_MODIFIER;

		if(parse_mods(args[0], &mods)) 
			goto fail;

		desc->arg.mods = mods;

		if(parse_keyseq(args[1], &code, &mods)) {
			err("%s is not a vaid keysequence.", args[1]);
			goto fail;
		}

		desc->arg2.keyseq = ((uint32_t)mods << 16) | (uint32_t)code;

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

static void parse(struct keyboard_config *cfg)
{
	size_t i, j;
	char *line = NULL;
	size_t n = 0;
	ssize_t len;

	lnum = 0;
	parse_layer_names();

	FILE *fh = fopen(path, "r");

	if(!fh) {
		perror("fopen");
		exit(-1);
	}

	int current_layer = 0;
	struct key_descriptor *layer = cfg->layers[0];

	lnum = 0;

	while((len=getline(&line, &n, fh)) != -1) {
		lnum++;
		char *s = line;

		while(len && isspace(s[0])) { //Trim leading whitespace
			s++;
			len--;
		}

		if(len && s[len-1] == '\n') //Strip tailing newline (not present on EOF)
			s[--len] = '\0';

		if(len == 0 || s[0] == '#') 
			continue;

		if(s[0] == '[' && s[len-1] == ']') {
			layer = cfg->layers[++current_layer];
		} else if(layer) {
			struct key_descriptor desc;
			char *key, *val;

			uint16_t code;
			uint8_t mods;

			if(parse_kvp(s, &key, &val)) {
				err("Invalid key value pair.");
				goto next;
			}

			if(parse_keyseq(key, &code, &mods)) {
				err("'%s' is not a valid key.", key);
				goto next;
			}

			if(mods != 0) {
				err("key cannot contain modifiers.");
				goto next;
			}

			if(parse_val(val, &desc))
				goto next;

			layer[code] = desc;
		}

next:
		free(line);
		line = NULL;
		n = 0;
	}

	free(line);
	fclose(fh);

	for(i = 0;i < nlayers;i++) {
		int p = parents[i];
		if(p != -1) {
			for(j = 0;j < KEY_CNT;j++) {
				if(cfg->layers[i][j].action == ACTION_DEFAULT)
					cfg->layers[i][j] = cfg->layers[p][j];
			}
		}
	}
}

void config_free()
{
	struct keyboard_config *cfg = configs;

	while(cfg) {
		struct keyboard_config *tmp = cfg;
		cfg = cfg->next;
		free(tmp);
	};
}

char* get_config_dir()
{
	char *keyd_config;

	keyd_config = getenv("KEYD_CONFIG_DIR");
	if (keyd_config) {
		DIR *config_dir = opendir(keyd_config);
		if (config_dir) {
			return config_dir;
		}
	}

	return CONFIG_DIR;
}

void config_generate()
{
	struct dirent *ent;
	char *config_dir = get_config_dir();
	DIR *dh = opendir(config_dir);

	if(!dh) {
		perror("opendir");
		exit(-1);
	}

	while((ent=readdir(dh))) {
		size_t i;
		struct keyboard_config *cfg;

		int len = strlen(ent->d_name);
		if(len <= 4 || strcmp(ent->d_name+len-4, ".cfg"))
			continue;


		sprintf(path, "%s/%s", CONFIG_DIR, ent->d_name);

		cfg = calloc(1, sizeof(struct keyboard_config));

		for(i = 0;i < KEY_CNT;i++) {
			struct key_descriptor *desc = &cfg->layers[0][i];
			desc->action = ACTION_KEYSEQ;
			desc->arg.keyseq = i;
		}

		strcpy(cfg->name, ent->d_name);
		cfg->name[len-4] = '\0';

		cfg->next = configs;
		parse(cfg);

		configs = cfg;
	}

	closedir(dh);
	free(config_dir);
}
