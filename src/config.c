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

int config_add_binding(struct config *config, const char *layer, const char *binding)
{
	char exp[MAX_EXP_LEN];
	snprintf(exp, sizeof exp, "%s.%s", layer, binding);

	return layer_table_add_entry(&config->layer_table, exp);
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

	if (name && layer_table_lookup(&config->layer_table, name) != -1)
			return 1;

	if (config->layer_table.nr_layers >= MAX_LAYERS) {
		err("max layers (%d) exceeded", MAX_LAYERS);
		return -1;
	}

	ret = create_layer(&config->layer_table.layers[config->layer_table.nr_layers],
		s,
		&config->layer_table);

	if (ret < 0)
		return -1;

	config->layer_table.nr_layers++;
	return 0;
}

static void config_init(struct config *config)
{
	size_t i;
	struct descriptor *km;

	bzero(config, sizeof(*config));

	config_add_layer(config, "main");

	config_add_layer(config, "control:C");
	config_add_layer(config, "meta:M");
	config_add_layer(config, "shift:S");
	config_add_layer(config, "altgr:G");
	config_add_layer(config, "alt:A");

	km = config->layer_table.layers[0].keymap;
	for (i = 0; i < 256; i++) {
		km[i].op = OP_KEYSEQUENCE;
		km[i].args[0].code = i;
		km[i].args[1].mods = 0;
	}

	for (i = 0; i < MAX_MOD; i++) {
		struct descriptor *ent1 = &km[modifier_table[i].code1];
		struct descriptor *ent2 = &km[modifier_table[i].code2];

		int idx = layer_table_lookup(&config->layer_table, modifier_table[i].name);

		assert(idx != -1);

		ent1->op = OP_LAYER;
		ent1->args[0].idx = idx;

		ent2->op = OP_LAYER;
		ent2->args[0].idx = idx;
	}

	config->layer_table.layers[0].active = 1;
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
		char *name;
		section = &ini->sections[i];

		if (!strcmp(section->name, "ids"))
			continue;

		if (!strcmp(section->name, "global")) {
			parse_globals(path, config, section);
			continue;
		}

		name = strtok(section->name, ":");

		for (j = 0; j < section->nr_entries;j++) {
			struct ini_entry *ent = &section->entries[j];

			if (config_add_binding(config, name, ent->line) < 0)
				fprintf(stderr, "\tERROR %s:%zd: %s\n", path, ent->lnum, errstr);
		}
	}

	return 0;
}

/*
 * returns 1 in the case of a match and 2
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
							uint16_t p, v;
							int ret;

							if (line[0] == '-')
								id++;

							if (line[0] != '#') {
								ret = sscanf(id, "%hx:%hx", &v, &p);

								if (ret == 2 && v == vendor && p == product) {
									close(fd);
									return wildcard ? 0 : 2;
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
