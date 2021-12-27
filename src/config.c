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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "ini.h"
#include "keys.h"
#include "error.h"
#include "descriptor.h"
#include "layer.h"
#include "config.h"
#include <string.h>

static struct config *fallback_config = NULL;
static struct config *configs = NULL;

/*
 * Parse a value of the form 'key = value'. The value may contain =
 * and the key may itself be = as a special case. The returned
 * values are pointers within the modified input string.
 */

static int parse_kvp(char *s, char **key, char **value)
{
	char *last_space = NULL;
	char *c = s;

	if (*c == '=')		//Allow the first character to be = as a special case.
		c++;

	while (*c) {
		switch (*c) {
		case '=':
			if (last_space)
				*last_space = 0;
			else
				*c = 0;
			c++;

			while (*c && *c == ' ')
				c++;

			*key = s;
			*value = c;

			return 0;
		case ' ':
			if (!last_space)
				last_space = c;
			break;
		default:
			last_space = NULL;
			break;
		}

		c++;
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

static uint32_t lookup_keycode(const char *name)
{
	int i;

	for (i = 0; i < KEY_MAX; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name &&
		    (!strcmp(ent->name, name) ||
		     (ent->alt_name && !strcmp(ent->alt_name, name))))
			return i;
	}

	return 0;
}

static char *read_file(const char *path)
{
	struct stat st;
	size_t sz;
	int fd;
	char *data;
	ssize_t n = 0, nr;

	if (stat(path, &st)) {
		perror("stat");
		exit(-1);
	}

	sz = st.st_size;
	data = malloc(sz + 1);

	fd = open(path, O_RDONLY);
	while ((nr = read(fd, data + n, sz - n))) {
		n += nr;
	}

	data[sz] = '\0';
	return data;
}

static struct layer *lookup_layer(const char *name, void *_config)
{
	struct config *config = (struct config *) (_config);
	size_t i;
	uint16_t mods;

	for (i = 0; i < config->nr_layers; i++) {
		struct layer *layer = config->layers[i];

		if (!strcmp(layer->name, name))
			return layer;
	}

	/* Autovivify modifier layers as required. */

	if (!parse_modset(name, &mods)) {
		struct layer *layer = create_layer(name, mods, 0);

		config->layers[config->nr_layers++] = layer;
		return layer;
	}

	return NULL;
}

static int parse_header(const char *s,
			char name[MAX_LAYER_NAME_LEN],
			char type[MAX_LAYER_NAME_LEN]) 
{
	char *d;
	size_t typelen, namelen;
	size_t len = strlen(s);

	if (!(d = strchr(s, ':'))) {
		if (len >= MAX_LAYER_NAME_LEN)
			return -1;
		else {
			strcpy(name, s);
			type[0] = 0;
			return 0;
		}
	}

	namelen = d - s;
	typelen = len - namelen - 1;
	if (namelen >= MAX_LAYER_NAME_LEN || typelen >= MAX_LAYER_NAME_LEN)
		return -1;

	memcpy(name, s, namelen);
	memcpy(type, s+namelen+1, typelen);

	name[namelen] = 0;
	type[typelen] = 0;

	return 0;
}

static void parse_id_section(struct config *config, struct ini_section *section) 
{
	size_t i;

	for (i = 0; i < section->nr_entries; i++) {
		struct ini_entry *ent = &section->entries[i];

		uint16_t product_id, vendor_id;

		/*  Applies to all device ids except the ones explicitly listed within the config. */

		if (!strcmp(ent->line, "*")) {
			if (fallback_config) {
				fprintf(stderr, "WARNING: multiple fallback configs detected: %s, %s. Ignoring %s\n",
					fallback_config->name,
					config->name,
					config->name);

				return;
			}

			printf("Using %s as a fallback config.\n", config->name);
			fallback_config = config;
			continue;
		}

		if (sscanf (ent->line, "-%hx:%hx", &product_id, &vendor_id) == 2) {
			if (fallback_config != config) {
				fprintf(stderr, "ERROR: -<id> is not permitted in non fallback configs.\n");
				continue;
			}

			config->device_ids[config->nr_device_ids++] = product_id << 16 | vendor_id;
		} else if (sscanf(ent->line, "%hx:%hx", &product_id, &vendor_id) == 2) {
			assert(config->nr_device_ids < MAX_DEVICE_IDS);
			config->device_ids[config->nr_device_ids++] = product_id << 16 | vendor_id;
		} else {
			fprintf (stderr,
				"ERROR %s:%zu: Invalid product id: %s\n",
				config->name, ent->lnum, ent->line);
		}
	}
}

/*
 * Ownership of the input string is forfeit, it will eventually be freed when
 * the config is destroyed.
 */

static int parse_config(const char *config_name, char *str, struct config *config)
{
	size_t i, j;

	struct ini ini;

	struct ini_section *layer_sections[MAX_LAYERS];
	struct layer *layers[MAX_LAYERS];
	size_t nr_layers = 0;


	if (ini_parse(str, &ini, NULL) < 0) {
		fprintf(stderr, "ERROR: %s is not a valid config (missing [main]?)\n", config_name);
		return -1;
	}

	config->nr_layers = 1;
	config->nr_device_ids = 0;

	config->layers[0] = create_layer("main", 0, 1);

	assert(strlen(config_name) < MAX_CONFIG_NAME);
	strcpy(config->name, config_name);

	/* 
	 * First pass, create layers so they are available
	 * for lookup during descriptor parsing.
	 */

	for (i = 0; i < ini.nr_sections; i++) {
		struct ini_section *section = &ini.sections[i];
		uint16_t mods;
		struct layer *layer;
		char name[MAX_LAYER_NAME_LEN], type[MAX_LAYER_NAME_LEN];

		if (!strcmp(section->name, "ids")) {
			parse_id_section(config, section);
			continue;
		} 

		if (parse_header(section->name, name, type) < 0) {
			fprintf(stderr,
				"ERROR %s:%zu: Invalid header.\n",
				config_name,
				section->lnum);
			continue;
		}

		layer = lookup_layer(name, (void*)config);

		if (!layer) { //If the layer doesn't exist, create it.
			if (!strcmp(type, "layout")) {
				layer = create_layer(name, 0, 1);
			} else if (!parse_modset(type, &mods)) {
				layer = create_layer(name, mods, 0);
			} else if (strcmp(type, "")) {
				fprintf(stderr,
					"WARNING %s:%zu: \"%s\" is not a valid layer type "
					" (must be \"layout\" or a valid modifier set).\n",
					config_name, section->lnum, type);
				continue;
			}

			config->layers[config->nr_layers++] = layer;
		}

		layers[nr_layers] = layer;
		layer_sections[nr_layers++] = section;
	}

	/* Parse each entry section entry and build the layer keymap. */

	for (i = 0; i < nr_layers; i++) {
		struct ini_section *section = layer_sections[i];
		struct layer *layer = layers[i];

		/* Populate the layer described by the section. */

		for (j = 0; j < section->nr_entries; j++) {
			struct ini_entry *ent = &section->entries[j];

			char *k, *v;
			uint16_t code;
			uint16_t mod;

			struct descriptor desc;

			if (parse_kvp(ent->line, &k, &v) < 0) {
				fprintf(stderr,
					"ERROR %s:%zu: Invalid key value pair.\n",
					config_name, ent->lnum);
				continue;
			}

			code = lookup_keycode(k);

			if (!code) {
				fprintf(stderr,
					"ERROR %s:%zu: %s is not a valid key.\n",
					config_name, ent->lnum, k);
				continue;
			}

			if (parse_descriptor(v, &desc, lookup_layer, config) < 0) {
				fprintf(stderr, "ERROR %s:%zu: %s\n", config_name,
					ent->lnum, errstr);
				continue;
			}

			layer_set_descriptor(layer, code, &desc);
		}
	}

	return 0;
}

static void post_process_config(const struct config *config)
{
	size_t i;
	uint16_t code;

	/* 
	 * Convert all modifier keycodes into their corresponding layer
	 * counterparts for consistency. This allows us to avoid explicitly
	 * accounting for modifier layer/modifier keycode overlap within the
	 * remapping logic and provides the user the ability to remap stock
	 * modifiers using their eponymous layer names.
	 */

	for (i = 0; i < config->nr_layers; i++) {
		struct layer *layer = config->layers[i];

		for (code = 0; code < KEY_MAX; code++) {
			struct descriptor *d = layer_get_descriptor(layer, code);

			if (d && d->op == OP_KEYSEQ) {
				uint16_t key = d->args[0].sequence.code;
				struct layer *modlayer = NULL;

				switch(key) {
				case KEY_RIGHTSHIFT:
				case KEY_LEFTSHIFT:
					modlayer = lookup_layer("S", (void*)config);
					break;
				case KEY_RIGHTALT:
					modlayer = lookup_layer("G", (void*)config);
					break;
				case KEY_RIGHTCTRL:
				case KEY_LEFTCTRL:
					modlayer = lookup_layer("C", (void*)config);
					break;
				case KEY_RIGHTMETA:
				case KEY_LEFTMETA:
					modlayer = lookup_layer("M", (void*)config);
					break;
				case KEY_LEFTALT:
					modlayer = lookup_layer("A", (void*)config);
					break;
				}

				if (modlayer) {
					d->op = OP_LAYER;
					d->args[0].layer = modlayer;
				}
			}

		}
	}
}

struct config *add_config(const char *config_name, char *str)
{
	struct config *config = malloc(sizeof(struct config));

	if (parse_config(config_name, str, config) < 0) {
		free(config);
		return NULL;
	}

	post_process_config(config);

	config->next = configs;
	configs = config;

	return config;
}

void free_configs()
{
	struct config *config = configs;

	while (config) {
		size_t i;
		struct config *tmp;

		for (i = 0; i < config->nr_layers; i++)
			free_layer(config->layers[i]);

		tmp = config;
		config = config->next;
		free(tmp);
	}

	fallback_config = NULL;
	configs = NULL;
}

int read_config_dir(const char *dir)
{
	free_configs();

	DIR *dh = opendir(dir);
	struct dirent *de;

	if (!dh)
		return -1;

	while ((de = readdir(dh))) {
		size_t len = strlen(de->d_name);

		if (len > 4 && !strcmp(de->d_name+len-4, ".cfg")) {
			fprintf(stderr, "NOTE: %s looks like an old (v1) config. See the man page for details on the current config format.\n", de->d_name);
		}

		if (len > 5 && !strcmp(de->d_name+len-5, ".conf")) {
			char path[PATH_MAX];
			char *data;

			sprintf(path, "%s/%s", dir, de->d_name);

			fprintf(stderr, "Parsing %s\n", path);
			data = read_file(path);

			add_config(path, data);
			free(data);
		}
	}

	closedir(dh);
	return 0;
}

struct config *lookup_config(uint32_t device_id)
{
	struct config *config = configs;
	int excluded = 0;

	while (config) {
		size_t i;

		for (i = 0; i < config->nr_device_ids; i++) {
			if (config->device_ids[i] == device_id) {
				if (fallback_config == config)
					excluded++;
				else
					return config;
			}
		}

		config = config->next;
	}

	if (fallback_config && !excluded)
		return fallback_config;

	return NULL;
}
