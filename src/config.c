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
#include <limits.h>
#include <unistd.h>
#include "ini.h"
#include "keys.h"
#include "error.h"
#include "descriptor.h"
#include "layer.h"
#include "config.h"
#include <string.h>

static struct config *configs = NULL;

int config_add_mapping(struct config *config, const char *layer, const char *str);
int config_add_layer(struct config *config, const char *str);

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
static int lookup_keycodes(const char *name, uint16_t *code1, uint16_t *code2)
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

	for (i = 0; i < KEY_MAX; i++) {
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

int config_create_layer(struct config *config, const char *name, uint16_t mods)
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
int config_create_layout(struct config *config, const char *name)
{
	uint16_t code;
	struct layer *layout;

	int layout_idx = config_create_layer(config, name, 0);
	layout = &config->layers[layout_idx];
		
	layout->is_layout = 1;

	for (code = 0; code < KEY_MAX; code++) {
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

static void parse_id_section(struct config *config, struct ini_section *section)
{
	size_t i;

	for (i = 0; i < section->nr_entries; i++) {
		struct ini_entry *ent = &section->entries[i];

		uint16_t product_id, vendor_id;

		/*  Applies to all device ids except the ones explicitly listed within the config. */

		if (!strcmp(ent->line, "*")) {
			config->has_wildcard = 1;
			continue;
		}

		if (sscanf (ent->line, "-%hx:%hx", &product_id, &vendor_id) == 2) {
			if (!config->has_wildcard) {
				fprintf(stderr,
					"ERROR %s:%zu: -<id> is not permitted unless the ids section begins with *.\n",
					config->name, ent->lnum);
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

struct config *create_config(const char *name)
{
	struct config *config;

	config = calloc(1, sizeof(struct config));
	strncpy(config->name, name, sizeof(config->name)-1);

	/* Create the default modifier layers. */

	config_add_layer(config, "control:C");
	config_add_layer(config, "meta:M");
	config_add_layer(config, "shift:S");
	config_add_layer(config, "altgr:G");
	config_add_layer(config, "alt:A");

	config->default_layout = config_create_layout(config, "main");
	return config;
}

/*
 * Consumes a string of the form name:type and creates a layer
 * within the provided config. If the layer already exists
 * then its attributes will remain unchanged.
 */
int config_add_layer(struct config *config, const char *str)
{
	uint16_t mods;
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
			fprintf(stderr,
				"WARNING %s: \"%s\" is not a valid layer type "
				" (must be \"layout\" or a valid modifier set).\n",
				config->name, type);

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
int config_add_mapping(struct config *config, const char *layer_name, const char *str)
{
	uint16_t code1, code2;
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

static struct config *parse_ini_string(const char *name, char *str)
{
	size_t i;

	struct ini ini;
	struct config *config = create_config(name);
	struct ini_section *section;

	if (ini_parse(str, &ini, NULL) < 0) {
		fprintf(stderr, "ERROR: %s is not a valid config.", config->name);

		free(config);
		return NULL;
	}

	/* First pass: create all layers based on section headers.  */
	for (i = 0; i < ini.nr_sections; i++) {
		section = &ini.sections[i];

		if (!strcmp(section->name, "ids")) {
			parse_id_section(config, section);
			continue;
		}

		if (config_lookup_layer(config, section->name) != -1)
			continue;

		if (config_add_layer(config, section->name) < 0)
			fprintf(stderr, "ERROR %s:%zd: %s\n", config->name, section->lnum, errstr);
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
				fprintf(stderr, "ERROR %s:%zd: %s\n", config->name, ent->lnum, errstr);
		}
	}

	return config;
}

struct config *config_create_from_ini(const char *config_name, char *str)
{
	struct config *config = parse_ini_string(config_name, str);

	if (!config) {
		free(config);
		return NULL;
	}

	config->next = configs;
	configs = config;

	return config;
}

void free_configs()
{
	struct config *config = configs;

	while (config) {
		struct config *tmp = config;
		config = config->next;

		free(tmp);
	}

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

			config_create_from_ini(path, data);
			free(data);
		}
	}

	closedir(dh);
	return 0;
}

struct config *lookup_config(uint32_t device_id)
{
	struct config *config = configs;
	struct config *result = NULL;

	while (config) {
		size_t i;

		if (config->has_wildcard) {
			int excluded = 0;
			for (i = 0; i < config->nr_device_ids; i++)
				if (config->device_ids[i] == device_id)
					excluded++;

			if (!excluded)
				result = config;
		} else {
			for (i = 0; i < config->nr_device_ids; i++)
				if (config->device_ids[i] == device_id)
					return config;
		}

		config = config->next;
	}

	return result;
}

