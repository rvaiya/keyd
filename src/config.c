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

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ini.h"
#include "keys.h"
#include "error.h"
#include "descriptor.h"
#include "layer.h"
#include "config.h"

static struct config *configs = NULL;

static int config_add_mapping(struct config *config, const char *layer, const char *str);
static int config_add_layer(struct config *config, const char *str);

/*
 * Parse a value of the form 'key = value'. The value may contain =
 * and the key may itself be = as a special case. The returned
 * values are pointers within the modified input string.
 */

static int parse_kvp(char *s, char **key, char **value)
{
	char *last_space = NULL;
	char *c = s;

	/* Allow the first character to be = as a special case. */
	if (*c == '=')
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

			if (!*s)
				return -1;

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

/* Return up to two keycodes associated with the given name. */
static int lookup_keycodes(const char *name, uint8_t *code1, uint8_t *code2)
{
	size_t i;

	/*
	 * If the name is a modifier like 'control' we associate it with both
	 * corresponding key codes (e.g 'rightcontrol'/'leftcontrol')
	 */
	for (i = 0; i < sizeof(modifier_table)/sizeof(modifier_table[0]); i++) {
		struct modifier_table_ent *m = &modifier_table[i];

		if (!strcmp(m->name, name)) {
			*code1 = m->code1;
			*code2 = m->code2;

			return 0;
		}
	}

	for (i = 0; i < MAX_KEYS; i++) {
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

int config_create_layer(struct config *config, const char *name, uint8_t mods)
{
	struct layer *layer = &config->layers[config->nr_layers++];

	layer->mods = mods;
	strncpy(layer->name, name, sizeof(layer->name)-1);

	return config->nr_layers-1;
}

int config_lookup_layer(struct config *config, const char *name)
{
	size_t i;

	for (i = 0; i < config->nr_layers; i++) {
		if (!strcmp(config->layers[i].name, name))
			return i;
	}

	return -1;
}


/* Returns the index within the layer table of the newly created layout. */
static int config_create_layout(struct config *config, const char *name)
{
	size_t code;
	struct layer *layout;

	int layout_idx = config_create_layer(config, name, 0);
	layout = &config->layers[layout_idx];
		
	layout->is_layout = 1;

	for (code = 0; code < MAX_KEYS-1; code++) {
		struct descriptor *d = &layout->keymap[code];

		d->op = OP_KEYSEQ;
		d->args[0].sequence.code = code;
		d->args[0].sequence.mods = 0;
	}

	config_add_mapping(config, name, "shift = layer(shift)");
	config_add_mapping(config, name, "control = layer(control)");
	config_add_mapping(config, name, "meta = layer(meta)");
	config_add_mapping(config, name, "alt = layer(alt)");
	config_add_mapping(config, name, "altgr = layer(altgr)");

	return layout_idx;
}

static void config_init(struct config *config)
{
	bzero(config, sizeof(*config));

	/* Create the default modifier layers. */

	config_add_layer(config, "control:C");
	config_add_layer(config, "meta:M");
	config_add_layer(config, "shift:S");
	config_add_layer(config, "altgr:G");
	config_add_layer(config, "alt:A");

	config->default_layout = config_create_layout(config, "main");
}

/*
 * Consumes a string of the form name:type and creates a layer
 * within the provided config. If the layer already exists
 * then its attributes will remain unchanged.
 */
static int config_add_layer(struct config *config, const char *str)
{
	uint8_t mods;
	char *name;
	char *type;

	static char s[512];

	strncpy(s, str, sizeof(s)-1);

	name = strtok(s, ":");
	type = strtok(NULL, ":");

	if (config_lookup_layer(config, name) != -1) {
		err("%s already exists, cannot redefine it.", name);
		return -1;
	}

	/* Create the layer. */

	if (type) {
		if (!strcmp(type, "layout"))
			config_create_layout(config, name);
		else if (!parse_modset(type, &mods))
			config_create_layer(config, name, mods);
		else {
			err("WARNING: \"%s\" is not a valid layer type (must be \"layout\" or a valid modifier set).\n", type);

			return -1;
		}
	} else
		config_create_layer(config, name, 0);

	return 0;
}

int config_execute_expression(struct config *config, const char *exp)
{
	char *d, *layer, *mapping;
	size_t len = strlen(exp);
	static char s[1024];

	assert(len < sizeof(s)-1);
	memcpy(s, exp, len+1);

	if (len > 1 && s[0] == '[' && s[len-1] == ']') {
		s[len-1] = 0;
		return config_add_layer(config, s+1);
	}

	d = strchr(s, '.');
	if (d) {
		*d = 0;

		layer = s;
		mapping = d+1;
	} else {
		layer = "main";
		mapping = s;
	}


	return config_add_mapping(config, layer, mapping);
}

/*
 * Consumes a string of the form `<key> = <descriptor>` and adds the
 * corresponding mapping to the layer within the supplied config.
 * The layer must exist or else be a valid modifier set.
 */
static int config_add_mapping(struct config *config, const char *layer_name, const char *str)
{
	uint8_t code1, code2;
	char *key, *descstr;
	int idx;
	struct descriptor d;

	static char s[1024];

	strncpy(s, str, sizeof(s)-1);

	if (parse_kvp(s, &key, &descstr) < 0) {
		err("Invalid key value pair.");
		return -1;
	}

	if (lookup_keycodes(key, &code1, &code2) < 0) {
		err("%s is not a valid key.", key);
		return -1;
	}

	idx = config_lookup_layer(config, layer_name);
	if(idx == -1) {
		err("%s is not a valid layer", layer_name);
		return -1;
	}

	if (parse_descriptor(descstr, &d, config) < 0)
		return -1;

	if (code1)
		config->layers[idx].keymap[code1] = d;

	if (code2)
		config->layers[idx].keymap[code2] = d;

	return 0;
}

static int parse(struct config *config, char *str, const char *path)
{
	size_t i;

	struct ini ini;
	struct ini_section *section;

	config_init(config);

	if (ini_parse(str, &ini, NULL) < 0)
		return -1;

	/* First pass: create all layers based on section headers.  */
	for (i = 0; i < ini.nr_sections; i++) {
		section = &ini.sections[i];

		if (!strcmp(section->name, "ids"))
			continue;

		if (config_lookup_layer(config, section->name) != -1)
			continue;

		if (config_add_layer(config, section->name) < 0)
			fprintf(stderr, "ERROR %s:%zd: %s\n", path, section->lnum, errstr);
	}

	/* Populate each layer. */
	for (i = 0; i < ini.nr_sections; i++) {
		size_t j;
		char *name;
		section = &ini.sections[i];

		if (!strcmp(section->name, "ids"))
		    continue;

		name = strtok(section->name, ":");

		for (j = 0; j < section->nr_entries;j++) {
			struct ini_entry *ent = &section->entries[j];

			if (config_add_mapping(config, name, ent->line) < 0)
				fprintf(stderr, "ERROR %s:%zd: %s\n", path, ent->lnum, errstr);
		}
	}

	return 0;
}

int config_parse(struct config *config, const char *path)
{
	int ret;

	char *data = read_file(path);
	ret = parse(config, data, path);
	free(data);

	if (ret < 0)
		fprintf(stderr, "Failed to parse %s\n", path);

	return ret;
}

/* 
 * returns 1 in the case of a match and 2
 * in the case of an exact match.
 */
static int config_check_match(const char *path, uint16_t vendor, uint16_t product)
{
	char line[32];
	int line_sz = 0;

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 0;
	}

	int seen_ids = 0;
	int wildcard = 0;

	while (1) {
		char buf[1024];
		int i;
		int n = read(fd, buf, sizeof buf);

		if (!n)
			break;

		for (i = 0; i < n; i++) {
			switch (buf[i]) {
				case ' ':
					break;
				case '[':
					if (seen_ids)
						goto end;
					else if (line_sz < sizeof(line)-1)
						line[line_sz++] = buf[i];

					break;
				case '\n':
					line[line_sz] = 0;

					if (!seen_ids && strstr(line, "[ids]") == line) {
						seen_ids++;
					} else if (seen_ids && line_sz) {
						if (line[0] == '*' && line[1] == 0) {
							wildcard = 1;
						} else {
							char *id = line;
							uint16_t p, v;
							int ret;

							if (line[0] == '-')
								id++;

							if (line[0] != '#') {
								ret = sscanf(id, "%hx:%hx", &v, &p);

								if (ret == 2 && v == vendor && p == product)
									return wildcard ? 0 : 2;
							}
						}
					}

					line_sz = 0;
					break;
				default:
					if (line_sz < sizeof(line)-1)
						line[line_sz++] = buf[i];

					break;
			}
		}
	}
end:

	return wildcard;
}

/* 
 * scan a directory for the most appropriate match for a given vendor/product
 * pair and return the result. returns NULL if not match is found.
 */
const char *config_find_path(const char *dir, uint16_t vendor, uint16_t product)
{
	static char result[1024];
	DIR *dh = opendir(dir);
	struct dirent *ent;
	int priority = 0;

	if (!dh) {
		perror("opendir");
		return NULL;
	}

	while ((ent = readdir(dh))) {
		char path[1024];
		int len;

		if (ent->d_type == DT_DIR)
			continue;

		len = snprintf(path, sizeof path, "%s/%s", dir, ent->d_name);
		if (len >= 5 && !strcmp(path+len-5, ".conf")) {
			int ret = config_check_match(path, vendor, product);
			if (ret && ret > priority) {
				priority = ret;
				strcpy(result, path);
			}
		}
	}

	closedir(dh);
	return priority ? result : NULL;
}
