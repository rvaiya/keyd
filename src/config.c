/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
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
#include "layer.h"
#include "error.h"
#include "config.h"

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

int config_get_layer_index(const struct config *config, const char *name)
{
	size_t i;

	for (i = 0; i < config->nr_layers; i++)
		if (!strcmp(config->layers[i].name, name))
			return i;

	return -1;
}


/*
 * Consumes a string of the form `[<layer>.]<key> = <descriptor>` and adds the
 * mapping to the corresponding layer in the config.
 */

int config_add_entry(struct config *config, const char *exp)
{
	uint8_t code1, code2;
	char *keyname, *descstr, *dot, *paren, *s;
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

	dot = strchr(s, '.');
	paren = strchr(s, '(');

	if (dot && (!paren || dot < paren)) {
		layername = s;
		*dot = 0;
		s = dot+1;
	}

	if (parse_kvp(s, &keyname, &descstr) < 0) {
		err("Invalid key value pair.");
		return -1;
	}

	idx = config_get_layer_index(config, layername);

	if (idx == -1) {
		err("%s is not a valid layer", layername);
		return -1;
	}

	layer = &config->layers[idx];

	if (lookup_keycodes(keyname, &code1, &code2) < 0) {
		err("%s is not a valid key.", keyname);
		return -1;
	}

	if (parse_descriptor(descstr, &d, config) < 0)
		return -1;

	if (code1)
		layer->keymap[code1] = d;

	if (code2)
		layer->keymap[code2] = d;

	return 0;
}

/*
 * returns:
 * 	1 if the layer exists
 * 	0 if the layer was created successfully
 * 	< 0 on error
 */
static int config_add_layer(struct config *config, const char *s)
{
	int ret;

	char buf[MAX_LAYER_NAME_LEN];
	char *name;

	strcpy(buf, s);
	name = strtok(buf, ":");

	if (name && config_get_layer_index(config, name) != -1)
			return 1;

	if (config->nr_layers >= MAX_LAYERS) {
		err("max layers (%d) exceeded", MAX_LAYERS);
		return -1;
	}

	ret = create_layer(&config->layers[config->nr_layers], s, config);

	if (ret < 0)
		return -1;

	config->nr_layers++;
	return 0;
}

static void config_init(struct config *config)
{
	size_t i;
	struct descriptor *km;

	memset(config, 0, sizeof *config);

	config_add_layer(config, "main");

	config_add_layer(config, "control:C");
	config_add_layer(config, "meta:M");
	config_add_layer(config, "shift:S");
	config_add_layer(config, "altgr:G");
	config_add_layer(config, "alt:A");

	km = config->layers[0].keymap;

	for (i = 0; i < 256; i++) {
		km[i].op = OP_KEYSEQUENCE;
		km[i].args[0].code = i;
		km[i].args[1].mods = 0;
	}

	for (i = 0; i < MAX_MOD; i++) {
		struct descriptor *ent1 = &km[modifier_table[i].code1];
		struct descriptor *ent2 = &km[modifier_table[i].code2];

		int idx = config_get_layer_index(config, modifier_table[i].name);

		assert(idx != -1);

		ent1->op = OP_LAYER;
		ent1->args[0].idx = idx;

		ent2->op = OP_LAYER;
		ent2->args[0].idx = idx;
	}

	/* In ms */
	config->macro_timeout = 600;
	config->macro_repeat_timeout = 50;

}

static void parse_globals(const char *path, struct config *config, struct ini_section *section)
{
	size_t i;

	for (i = 0; i < section->nr_entries;i++) {
		char *key, *val;
		struct ini_entry *ent = &section->entries[i];
		if (parse_kvp(ent->line, &key, &val)) {
			fprintf(stderr, "\tERROR %s:%zd: malformed config entry\n", path, ent->lnum);
			continue;
		}

		if (!strcmp(key, "macro_timeout"))
			config->macro_timeout = atoi(val);
		else if (!strcmp(key, "macro_repeat_timeout"))
			config->macro_repeat_timeout = atoi(val);
		else if (!strcmp(key, "layer_indicator"))
			config->layer_indicator = atoi(val);
		else
			fprintf(stderr, "\tERROR %s:%zd: %s is not a valid global option.\n",
					path,
					ent->lnum,
					key);
	}
}

int config_parse(struct config *config, const char *path)
{
	size_t i;

	struct ini *ini;
	struct ini_section *section;

	config_init(config);

	if (!(ini = ini_parse_file(path, NULL)))
		return -1;

	/* First pass: create all layers based on section headers.  */
	for (i = 0; i < ini->nr_sections; i++) {
		section = &ini->sections[i];

		if (!strcmp(section->name, "ids") ||
		    !strcmp(section->name, "global"))
			continue;


		if (config_add_layer(config, section->name) < 0)
			fprintf(stderr, "\tERROR %s:%zd: %s\n", path, section->lnum, errstr);
	}

	/* Populate each layer. */
	for (i = 0; i < ini->nr_sections; i++) {
		size_t j;
		char *layername;
		section = &ini->sections[i];

		if (!strcmp(section->name, "ids"))
			continue;

		if (!strcmp(section->name, "global")) {
			parse_globals(path, config, section);
			continue;
		}

		layername = strtok(section->name, ":");

		for (j = 0; j < section->nr_entries;j++) {
			char entry[MAX_EXP_LEN];
			struct ini_entry *ent = &section->entries[j];

			snprintf(entry, sizeof entry, "%s.%s", layername, ent->line);

			if (config_add_entry(config, entry) < 0)
				fprintf(stderr, "\tERROR %s:%zd: %s\n", path, ent->lnum, errstr);
		}
	}

	return 0;
}

/*
 * Returns 1 in the case of a match and 2
 * in the case of an exact match.
 */
static int config_check_match(const char *path, uint16_t vendor, uint16_t product)
{
	char line[32];
	size_t line_sz = 0;

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
							uint8_t omit = 0;
							uint16_t p, v;
							int ret;

							if (line[0] == '-') {
								omit = 1;
								id++;
							}

							if (line[0] != '#') {
								ret = sscanf(id, "%hx:%hx", &v, &p);

								if (ret == 2 && v == vendor && p == product) {
									close(fd);
									return omit ? 0 : 2;
								}
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

	close(fd);
	return wildcard;
}

/*
 * Scan a directory for the most appropriate match for a given vendor/product
 * pair and return the result. returns NULL if not match is found.
 */
const char *find_config_path(const char *dir, uint16_t vendor, uint16_t product, uint8_t *is_exact_match)
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

	*is_exact_match = priority == 2;
	return priority ? result : NULL;
}
