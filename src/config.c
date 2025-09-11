/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#include "keyd.h"
#include "ini.h"
#include "keys.h"
#include "log.h"
#include "string.h"
#include "unicode.h"

#define MAX_LINES 4096
#define MAX_FILE_SIZE 65536
#define MAX_INCLUDES 128

struct srcmap_entry {
	const char *path;
	size_t line;
};

struct srcmap {
	char paths[MAX_INCLUDES+1][PATH_MAX];
	size_t num_paths;

	struct srcmap_entry entries[MAX_LINES];
};

static size_t current_line = 0;
static const char *current_file = NULL;

static size_t nr_warnings = 0;

static void config_warn(const char *fmt, ...)
{
	va_list ap;
	char buf[1024];

	if (current_file)
		snprintf(buf, sizeof buf, "\ty{WARNING:} b{%s}:r{%zd}: %s\n", current_file, current_line + 1, fmt);
	else
		snprintf(buf, sizeof buf, "\ty{WARNING:} %s\n", fmt);

	nr_warnings++;
	va_start(ap, fmt);
	_vkeyd_log(buf, ap);
	va_end(ap);
}

static struct {
	const char *name;
	const char *preferred_name;
	uint8_t op;
	enum {
		ARG_EMPTY,

		ARG_MACRO,
		ARG_LAYER,
		ARG_LAYOUT,
		ARG_TIMEOUT,
		ARG_SENSITIVITY,
		ARG_KEYSEQUENCE_DESCRIPTOR,
		ARG_DESCRIPTOR,
	} args[MAX_DESCRIPTOR_ARGS];
} actions[] =  {
	{ "swap", 	NULL,	OP_SWAP,	{ ARG_LAYER } },
	{ "clear", 	NULL,	OP_CLEAR,	{} },
	{ "oneshot", 	NULL,	OP_ONESHOT,	{ ARG_LAYER } },
	{ "toggle", 	NULL,	OP_TOGGLE,	{ ARG_LAYER } },

	{ "clearm", 	NULL,	OP_CLEARM,	{ ARG_MACRO } },
	{ "swapm", 	NULL,	OP_SWAPM,	{ ARG_LAYER, ARG_MACRO } },
	{ "togglem", 	NULL,	OP_TOGGLEM,	{ ARG_LAYER, ARG_MACRO } },
	{ "layerm", 	NULL,	OP_LAYERM,	{ ARG_LAYER, ARG_MACRO } },
	{ "oneshotm", 	NULL,	OP_ONESHOTM,	{ ARG_LAYER, ARG_MACRO } },
	{ "oneshotk", 	NULL,	OP_ONESHOTK,	{ ARG_LAYER, ARG_KEYSEQUENCE_DESCRIPTOR } },

	{ "layer", 	NULL,	OP_LAYER,	{ ARG_LAYER } },

	{ "overload", 	NULL,	OP_OVERLOAD,			{ ARG_LAYER, ARG_DESCRIPTOR } },
	{ "overloadt", 	NULL,	OP_OVERLOAD_TIMEOUT,		{ ARG_LAYER, ARG_DESCRIPTOR, ARG_TIMEOUT } },
	{ "overloadt2", NULL,	OP_OVERLOAD_TIMEOUT_TAP,	{ ARG_LAYER, ARG_DESCRIPTOR, ARG_TIMEOUT } },

	{ "overloadi",	NULL,	OP_OVERLOAD_IDLE_TIMEOUT, { ARG_DESCRIPTOR, ARG_DESCRIPTOR, ARG_TIMEOUT } },
	{ "timeout", 	NULL,	OP_TIMEOUT,	{ ARG_DESCRIPTOR, ARG_TIMEOUT, ARG_DESCRIPTOR } },

	{ "macro2", 	NULL,	OP_MACRO2,	{ ARG_TIMEOUT, ARG_TIMEOUT, ARG_MACRO } },
	{ "setlayout", 	NULL,	OP_LAYOUT,	{ ARG_LAYOUT } },

	{ "repeat", 	NULL,	OP_REPEAT,	{} },

	/* Experimental */
	{ "scrollon", 	NULL,	OP_SCROLL_TOGGLE_ON,		{ARG_SENSITIVITY} },
	{ "scrolloff", 	NULL,	OP_SCROLL_TOGGLE_OFF,		{} },
	{ "scrollt", 	NULL,	OP_SCROLL_TOGGLE,		{ARG_SENSITIVITY} },
	{ "scroll", 	NULL,	OP_SCROLL,			{ARG_SENSITIVITY} },

	/* TODO: deprecate */
	{ "overload2", 	"overloadt",	OP_OVERLOAD_TIMEOUT,		{ ARG_LAYER, ARG_DESCRIPTOR, ARG_TIMEOUT } },
	{ "overload3", 	"overloadt2",	OP_OVERLOAD_TIMEOUT_TAP,	{ ARG_LAYER, ARG_DESCRIPTOR, ARG_TIMEOUT } },
	{ "toggle2", 	"togglem",	OP_TOGGLEM,			{ ARG_LAYER, ARG_MACRO } },
	{ "swap2", 	"swapm",	OP_SWAPM,			{ ARG_LAYER, ARG_MACRO } },
};

static int exists_and_is_relative(const char *parent_dir, const char *path)
{
	char p1[PATH_MAX];
	char p2[PATH_MAX];

	if (!realpath(parent_dir, p1))
		return 0;

	if (!realpath(path, p2))
		return 0;

	return !strncmp(p2, p1, strlen(p1));
}

static int resolve_include_path(const char *config_path, const char *include_path, char *resolved_path)
{
	size_t len;
	size_t ret;
	char config_dir[PATH_MAX-1];

	snprintf(config_dir, sizeof config_dir, "%s", config_path);
	if (!dirname(config_dir))
		return -1;

	assert(strlen(config_dir) + strlen(include_path) + 2 < PATH_MAX);

	len = strlen(config_dir);
	strcpy(resolved_path, config_dir);
	resolved_path[len] = '/';
	strcpy(resolved_path + len + 1, include_path);

	if (exists_and_is_relative(config_dir, resolved_path))
		return 0;

	snprintf(resolved_path, PATH_MAX, DATA_DIR"/%s", include_path);
	return !exists_and_is_relative(DATA_DIR, resolved_path);
}

static void append_line(char *buf, size_t buf_sz, size_t *off, const char *line)
{
	size_t len = strlen(line);
	assert(*off + len + 2 < buf_sz);

	memcpy(buf + *off, line, len);
	buf[*off + len] = '\n';
	buf[*off + len + 1] = 0;

	*off += len + 1;
}

static const char *read_line(FILE *fh)
{
	static char line[4096];
	size_t n = 0;
	int c;

	while (1) {
		c = fgetc(fh);
		if (c == -1 || c == '\n')
			break;

		assert(n < (sizeof(line)-1));
		line[n++] = c;
	}

	if (n == 0 && c == -1)
		return NULL;

	line[n] = 0;
	return line;
}

static char *read_config_file(const char *path, struct srcmap *srcmap)
{
	FILE *fh;
	const char *line;

	const char include_prefix[] = "include ";
	const size_t include_prefix_len = sizeof(include_prefix) - 1;

	size_t off = 0;
	static char output[MAX_FILE_SIZE];

	size_t config_line_num = 0;
	size_t output_line_num = 0;

	if (!(fh = fopen(path, "r")))
		return NULL;

	srcmap->num_paths = 1;
	snprintf(srcmap->paths[0], sizeof srcmap->paths[0], "%s", path);

	while ((line = read_line(fh))) {
		current_line = config_line_num;
		current_file = path;

		if (!strncmp(line, include_prefix, include_prefix_len)) {
			char *include_path;

			assert(srcmap->num_paths < ARRAY_SIZE(srcmap->paths));

			include_path = srcmap->paths[srcmap->num_paths];
			if (!resolve_include_path(path, line + include_prefix_len, include_path)) {
				FILE *fh;
				size_t include_line_num = 0;

				if (!(fh = fopen(include_path, "r"))) {
					config_warn("failed to open %s", include_path);
					continue;
				}

				srcmap->num_paths++;
				while ((line = read_line(fh))) {
					append_line(output, sizeof output, &off, line);

					assert(output_line_num < ARRAY_SIZE(srcmap->entries));
					srcmap->entries[output_line_num].path = include_path;
					srcmap->entries[output_line_num].line = include_line_num++;

					output_line_num++;
				}

				fclose(fh);
			} else {
				config_warn("failed to resolve include path %s", line + include_prefix_len);
			}
		} else {
			append_line(output, sizeof output, &off, line);

			assert(output_line_num < ARRAY_SIZE(srcmap->entries));
			srcmap->entries[output_line_num].line = config_line_num;
			srcmap->entries[output_line_num].path = srcmap->paths[0];

			output_line_num++;
		}

		config_line_num++;
	}

	fclose(fh);
	return output;
}


/* Return up to two keycodes associated with the given name. */
static uint8_t lookup_keycode(const char *name)
{
	size_t i;

	for (i = 0; i < 256; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name &&
		    (!strcmp(ent->name, name) ||
		     (ent->alt_name && !strcmp(ent->alt_name, name)))) {
			return i;
		}
	}

	return 0;
}

static struct descriptor *layer_lookup_chord(struct layer *layer, uint8_t *keys, size_t n)
{
	size_t i;

	for (i = 0; i < layer->nr_chords; i++) {
		size_t j;
		size_t nm = 0;
		struct chord *chord = &layer->chords[i];

		for (j = 0; j < n; j++) {
			size_t k;
			for (k = 0; k < chord->sz; k++)
				if (keys[j] == chord->keys[k]) {
					nm++;
					break;
				}
		}

		if (nm == n)
			return &chord->d;
	}

	return NULL;
}

/*
 * Consumes a string of the form `[<layer>.]<key> = <descriptor>` and adds the
 * mapping to the corresponding layer in the config.
 */

static int set_layer_entry(const struct config *config,
			   struct layer *layer, char *key,
			   const struct descriptor *d)
{
	size_t i;
	int found = 0;

	if (strchr(key, '+')) {
		//TODO: Handle aliases
		char *tok;
		struct descriptor *ld;
		uint8_t keys[ARRAY_SIZE(layer->chords[0].keys)];
		size_t n = 0;

		for (tok = strtok(key, "+"); tok; tok = strtok(NULL, "+")) {
			uint8_t code = lookup_keycode(tok);
			if (!code) {
				err("%s is not a valid key", tok);
				return -1;
			}

			if (n >= ARRAY_SIZE(keys)) {
				err("chords cannot contain more than %ld keys", n);
				return -1;
			}

			keys[n++] = code;
		}


		if ((ld = layer_lookup_chord(layer, keys, n))) {
			*ld = *d;
		} else {
			struct chord *chord;
			if (layer->nr_chords >= ARRAY_SIZE(layer->chords)) {
				err("max chords exceeded(%ld)", layer->nr_chords);
				return -1;
			}

			chord = &layer->chords[layer->nr_chords];
			memcpy(chord->keys, keys, sizeof keys);
			chord->sz = n;
			chord->d = *d;

			layer->nr_chords++;
		}
	} else {
		for (i = 0; i < 256; i++) {
			if (!strcmp(config->aliases[i], key)) {
				layer->keymap[i] = *d;
				found = 1;
			}
		}

		if (!found) {
			uint8_t code;

			if (!(code = lookup_keycode(key))) {
				err("%s is not a valid key or alias", key);
				return -1;
			}

			layer->keymap[code] = *d;

		}
	}

	return 0;
}

static int new_layer(char *s, const struct config *config, struct layer *layer)
{
	uint8_t mods;
	char *name;
	char *type;

	name = strtok(s, ":");
	type = strtok(NULL, ":");

	strcpy(layer->name, name);

	layer->nr_chords = 0;

	if (strchr(name, '+')) {
		char *layername;
		int n = 0;

		layer->type = LT_COMPOSITE;
		layer->nr_constituents = 0;

		if (type) {
			err("composite layers cannot have a type.");
			return -1;
		}

		for (layername = strtok(name, "+"); layername; layername = strtok(NULL, "+")) {
			int idx = config_get_layer_index(config, layername);

			if (idx < 0) {
				err("%s is not a valid layer", layername);
				return -1;
			}

			if (n >= ARRAY_SIZE(layer->constituents)) {
				err("max composite layers (%d) exceeded", ARRAY_SIZE(layer->constituents));
				return -1;
			}

			layer->constituents[layer->nr_constituents++] = idx;
		}

	} else if (type && !strcmp(type, "layout")) {
			layer->type = LT_LAYOUT;
	} else if (type && !parse_modset(type, &mods)) {
			layer->type = LT_NORMAL;
			layer->mods = mods;
	} else {
		if (type)
			config_warn("\"%s\" is not a valid layer type, ignoring", type);

		layer->type = LT_NORMAL;
		layer->mods = 0;
	}


	return 0;
}

/*
 * Returns:
 * 	1 if the layer exists
 * 	0 if the layer was created successfully
 * 	< 0 on error
 */
static int config_add_layer(struct config *config, const char *s)
{
	int ret;
	char buf[MAX_LAYER_NAME_LEN+1];
	char *name;

	if (strlen(s) >= sizeof buf) {
		err("%s exceeds maximum section length (%d) (ignoring)", s, MAX_LAYER_NAME_LEN);
		return -1;
	}

	strcpy(buf, s);
	name = strtok(buf, ":");

	if (config_get_layer_index(config, name) != -1)
			return 1;

	if (config->nr_layers >= MAX_LAYERS) {
		err("max layers (%d) exceeded", MAX_LAYERS);
		return -1;
	}

	strcpy(buf, s);
	ret = new_layer(buf, config, &config->layers[config->nr_layers]);

	if (ret < 0)
		return -1;

	config->nr_layers++;
	return 0;
}

/* Modifies the input string */
static int parse_fn(char *s,
		    char **name,
		    char *args[5],
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

		if (arg != c) {
			assert(*nargs < 5);
			args[(*nargs)++] = arg;
		}

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

/*
 * Parses macro expression placing the result
 * in the supplied macro struct.
 *
 * Returns:
 *   0 on success
 *   -1 in the case of an invalid macro expression
 *   > 0 for all other errors
 */

int parse_macro_expression(const char *s, struct macro *macro)
{
	uint8_t code, mods;

	#define ADD_ENTRY(t, d) do { \
		if (macro->sz >= ARRAY_SIZE(macro->entries)) { \
			err("maximum macro size (%d) exceeded", ARRAY_SIZE(macro->entries)); \
			return 1; \
		} \
		macro->entries[macro->sz].type = t; \
		macro->entries[macro->sz].data = d; \
		macro->sz++; \
	} while(0)

	size_t len = strlen(s);

	char buf[1024];
	char *ptr = buf;

	if (len >= sizeof(buf)) {
		err("macro size exceeded maximum size (%ld)\n", sizeof(buf));
		return -1;
	}

	strcpy(buf, s);

	if (!strncmp(ptr, "macro(", 6) && ptr[len-1] == ')') {
		ptr[len-1] = 0;
		ptr += 6;
		str_escape(ptr);
	} else if (parse_key_sequence(ptr, &code, &mods) && utf8_strlen(ptr) != 1) {
		err("Invalid macro");
		return -1;
	}

	return macro_parse(ptr, macro) == 0 ? 0 : 1;
}

static int parse_command(const char *s, struct command *command)
{
	int len = strlen(s);

	if (len == 0 || strstr(s, "command(") != s || s[len-1] != ')')
		return -1;

	if (len > (int)sizeof(command->cmd)) {
		err("max command length (%ld) exceeded\n", sizeof(command->cmd));
		return 1;
	}

	strcpy(command->cmd, s+8);
	command->cmd[len-9] = 0;
	str_escape(command->cmd);

	return 0;
}

static int parse_descriptor(char *s,
			    struct descriptor *d,
			    struct config *config)
{
	char *fn = NULL;
	char *args[5];
	size_t nargs = 0;
	uint8_t code, mods;
	int ret;
	struct macro macro;
	struct command cmd;

	if (!s || !s[0]) {
		d->op = 0;
		return 0;
	}

	if (!parse_key_sequence(s, &code, &mods)) {
		size_t i;
		const char *layer = NULL;

		switch (code) {
			case KEYD_LEFTSHIFT:   layer = "shift"; break;
			case KEYD_LEFTCTRL:    layer = "control"; break;
			case KEYD_LEFTMETA:    layer = "meta"; break;
			case KEYD_LEFTALT:     layer = "alt"; break;
			case KEYD_RIGHTALT:    layer = "altgr"; break;
		}

		if (layer) {
			config_warn("You should use b{layer(%s)} instead of assigning to b{%s} directly.", layer, KEY_NAME(code));
			d->op = OP_LAYER;
			d->args[0].idx = config_get_layer_index(config, layer);

			assert(d->args[0].idx != -1);

			return 0;
		}

		d->op = OP_KEYSEQUENCE;
		d->args[0].code = code;
		d->args[1].mods = mods;

		return 0;
	} else if ((ret=parse_command(s, &cmd)) >= 0) {
		if (ret) {
			return -1;
		}

		if (config->nr_commands >= ARRAY_SIZE(config->commands)) {
			err("max commands (%d), exceeded", ARRAY_SIZE(config->commands));
			return -1;
		}


		d->op = OP_COMMAND;
		d->args[0].idx = config->nr_commands;

		config->commands[config->nr_commands++] = cmd;

		return 0;
	} else if ((ret=parse_macro_expression(s, &macro)) >= 0) {
		if (ret)
			return -1;

		if (config->nr_macros >= ARRAY_SIZE(config->macros)) {
			err("max macros (%d), exceeded", ARRAY_SIZE(config->macros));
			return -1;
		}

		d->op = OP_MACRO;
		d->args[0].idx = config->nr_macros;

		config->macros[config->nr_macros++] = macro;

		return 0;
	} else if (!parse_fn(s, &fn, args, &nargs)) {
		int i;
		char buf[1024];

		if (!strcmp(fn, "lettermod")) {
			if (nargs != 4) {
				err("%s requires 4 arguments", fn);
				return -1;
			}

			snprintf(buf, sizeof buf,
				"overloadi(%s, overloadt2(%s, %s, %s), %s)",
				args[1], args[0], args[1], args[3], args[2]);

			if (parse_fn(buf, &fn, args, &nargs)) {
				err("failed to parse %s", buf);
				return -1;
			}
		}

		for (i = 0; i < ARRAY_SIZE(actions); i++) {
			if (!strcmp(actions[i].name, fn)) {
				int j;

				if (actions[i].preferred_name)
					config_warn("%s is deprecated (renamed to %s).", actions[i].name, actions[i].preferred_name);

				d->op = actions[i].op;

				for (j = 0; j < MAX_DESCRIPTOR_ARGS; j++) {
					if (!actions[i].args[j])
						break;
				}

				if ((int)nargs != j) {
					err("%s requires %d %s", actions[i].name, j, j == 1 ? "argument" : "arguments");
					return -1;
				}

				while (j--) {
					int type = actions[i].args[j];
					union descriptor_arg *arg = &d->args[j];
					char *argstr = args[j];
					struct descriptor desc;

					switch (type) {
					case ARG_LAYER:
						if (!strcmp(argstr, "main")) {
							err("the main layer cannot be toggled");
							return -1;
						}

						arg->idx = config_get_layer_index(config, argstr);
						if (arg->idx == -1 || config->layers[arg->idx].type == LT_LAYOUT) {
							err("%s is not a valid layer", argstr);
							return -1;
						}

						break;
					case ARG_LAYOUT:
						arg->idx = config_get_layer_index(config, argstr);
						if (arg->idx == -1 ||
							(arg->idx != 0 && //Treat main as a valid layout
							 config->layers[arg->idx].type != LT_LAYOUT)) {
							err("%s is not a valid layout", argstr);
							return -1;
						}

						break;
					case ARG_KEYSEQUENCE_DESCRIPTOR:
					case ARG_DESCRIPTOR:
						if (parse_descriptor(argstr, &desc, config))
							return -1;

						if (config->nr_descriptors >= ARRAY_SIZE(config->descriptors)) {
							err("maximum descriptors exceeded");
							return -1;
						}

						if (type == ARG_KEYSEQUENCE_DESCRIPTOR && desc.op != OP_KEYSEQUENCE) {
							err("%s is not a valid keysequence", argstr);
							return -1;
						}

						config->descriptors[config->nr_descriptors] = desc;
						arg->idx = config->nr_descriptors++;
						break;
					case ARG_SENSITIVITY:
						arg->sensitivity = atoi(argstr);
						break;
					case ARG_TIMEOUT:
						arg->timeout = atoi(argstr);
						break;
					case ARG_MACRO:
						if (config->nr_macros >= ARRAY_SIZE(config->macros)) {
							err("max macros (%d), exceeded", ARRAY_SIZE(config->macros));
							return -1;
						}

						if (parse_macro_expression(argstr, &config->macros[config->nr_macros])) {
							return -1;
						}

						arg->idx = config->nr_macros;
						config->nr_macros++;

						break;
					default:
						assert(0);
						break;
					}
				}

				return 0;
			}
		}
	}

	err("invalid key or action");
	return -1;
}

static void parse_global_section(struct config *config, struct ini_section *section)
{
	size_t i;

	for (i = 0; i < section->nr_entries;i++) {
		struct ini_entry *ent = &section->entries[i];

		if (!strcmp(ent->key, "macro_timeout"))
			config->macro_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "macro_sequence_timeout"))
			config->macro_sequence_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "disable_modifier_guard"))
			config->disable_modifier_guard = atoi(ent->val);
		else if (!strcmp(ent->key, "oneshot_timeout"))
			config->oneshot_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "chord_hold_timeout"))
			config->chord_hold_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "chord_timeout"))
			config->chord_interkey_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "default_layout"))
			snprintf(config->default_layout, sizeof config->default_layout,
				 "%s", ent->val);
		else if (!strcmp(ent->key, "macro_repeat_timeout"))
			config->macro_repeat_timeout = atoi(ent->val);
		else if (!strcmp(ent->key, "layer_indicator"))
			config->layer_indicator = atoi(ent->val);
		else if (!strcmp(ent->key, "overload_tap_timeout"))
			config->overload_tap_timeout = atoi(ent->val);
		else
			config_warn("%s is not a valid global option", ent->key);
	}
}

static void parse_id_section(struct config *config, struct ini_section *section)
{
	size_t i;
	for (i = 0; i < section->nr_entries; i++) {
		uint16_t product, vendor;

		struct ini_entry *ent = &section->entries[i];
		const char *s = ent->key;

		if (!strcmp(s, "*")) {
			config->wildcard = 1;
		} else if (strstr(s, "m:") == s) {
			assert(config->nr_ids < ARRAY_SIZE(config->ids));
			config->ids[config->nr_ids].flags = ID_MOUSE;

			snprintf(config->ids[config->nr_ids++].id, sizeof(config->ids[0].id), "%s", s+2);
		} else if (strstr(s, "k:") == s) {
			assert(config->nr_ids < ARRAY_SIZE(config->ids));
			config->ids[config->nr_ids].flags = ID_KEYBOARD | ID_KEY;

			snprintf(config->ids[config->nr_ids++].id, sizeof(config->ids[0].id), "%s", s+2);
		} else if (strstr(s, "-") == s) {
			assert(config->nr_ids < ARRAY_SIZE(config->ids));
			config->ids[config->nr_ids].flags = ID_EXCLUDED;

			snprintf(config->ids[config->nr_ids++].id, sizeof(config->ids[0].id), "%s", s+1);
		} else if (strlen(s) < sizeof(config->ids[config->nr_ids].id)-1) {
			assert(config->nr_ids < ARRAY_SIZE(config->ids));
			config->ids[config->nr_ids].flags = ID_KEYBOARD | ID_KEY | ID_MOUSE;

			snprintf(config->ids[config->nr_ids++].id, sizeof(config->ids[0].id), "%s", s);
		} else {
			config_warn("%s is not a valid device id", s);
		}
	}
}

static void parse_alias_section(struct config *config, struct ini_section *section)
{
	size_t i;

	for (i = 0; i < section->nr_entries; i++) {
		uint8_t code;
		struct ini_entry *ent = &section->entries[i];
		const char *name = ent->val;

		if ((code = lookup_keycode(ent->key))) {
			ssize_t len = strlen(name);

			if (len >= (ssize_t)sizeof(config->aliases[0])) {
				config_warn("%s exceeds the maximum alias length (%ld)", name, sizeof(config->aliases[0])-1);
			} else {
				uint8_t alias_code;

				if ((alias_code = lookup_keycode(name))) {
					struct descriptor *d = &config->layers[0].keymap[code];

					d->op = OP_KEYSEQUENCE;
					d->args[0].code = alias_code;
					d->args[1].mods = 0;
				}

				strcpy(config->aliases[code], name);
			}
		} else {
			config_warn("failed to define alias %s, %s is not a valid keycode", name, ent->key);
		}
	}
}

// Returns 0 on success, or else the number of warnings issued.
static int do_parse(struct config *config, char *content, const struct srcmap *srcmap)
{
	size_t i;
	struct ini *ini;

	current_file = NULL;
	current_line = 0;
	nr_warnings = 0;

	if (!(ini = ini_parse_string(content, NULL))) {
		config_warn("Invalid config file (missing [ids] section)");
		return 1;
	}

	/* First pass: create all layers based on section headers.  */
	for (i = 0; i < ini->nr_sections; i++) {
		struct ini_section *section = &ini->sections[i];

		if (!strcmp(section->name, "ids")) {
			parse_id_section(config, section);
		} else if (!strcmp(section->name, "aliases")) {
			parse_alias_section(config, section);
		} else if (!strcmp(section->name, "global")) {
			parse_global_section(config, section);
		} else {
			if (config_add_layer(config, section->name) < 0)
				config_warn("%s", errstr);
		}
	}

	/* Populate each layer. */
	for (i = 0; i < ini->nr_sections; i++) {
		size_t j;
		char *layername;
		struct ini_section *section = &ini->sections[i];

		if (!strcmp(section->name, "ids") ||
		    !strcmp(section->name, "aliases") ||
		    !strcmp(section->name, "global"))
			continue;

		layername = strtok(section->name, ":");

		for (j = 0; j < section->nr_entries;j++) {
			char entry[MAX_EXP_LEN];
			struct ini_entry *ent = &section->entries[j];

			if (srcmap) {
				current_file = srcmap->entries[ent->lnum - 1].path;
				current_line = srcmap->entries[ent->lnum - 1].line;
			} else {
				current_file = "";
				current_line = ent->lnum - 1;
			}

			if (!ent->val) {
				config_warn("invalid binding");
				continue;
			}

			snprintf(entry, sizeof entry, "%s.%s = %s", layername, ent->key, ent->val);

			if (config_add_entry(config, entry) < 0)
				config_warn("%s", errstr);
		}
	}

	return nr_warnings;
}

static void config_init(struct config *config)
{
	size_t nw;
	size_t i;

	memset(config, 0, sizeof *config);

	char default_config[] =
	"[aliases]\n"

	"leftshift = shift\n"
	"rightshift = shift\n"

	"leftalt = alt\n"
	"rightalt = altgr\n"

	"leftmeta = meta\n"
	"rightmeta = meta\n"

	"leftcontrol = control\n"
	"rightcontrol = control\n"

	"[main:layout]\n"

	"shift = layer(shift)\n"
	"alt = layer(alt)\n"
	"altgr = layer(altgr)\n"
	"meta = layer(meta)\n"
	"control = layer(control)\n"

	"[control:C]\n"
	"[shift:S]\n"
	"[meta:M]\n"
	"[alt:A]\n"
	"[altgr:G]\n";

	nw = do_parse(config, default_config, NULL);
	assert(nw == 0);

	/* In ms */
	config->chord_interkey_timeout = 50;
	config->chord_hold_timeout = 0;
	config->oneshot_timeout = 0;

	config->macro_timeout = 600;
	config->macro_repeat_timeout = 50;
}

/*
 * Returns:
 * 	0 on success (no warning)
 * 	n > 0 on partial success (where n is the number of issued warnings)
 * 	< 0 on complete failure
 */
int config_parse(struct config *config, const char *path)
{
	char *content;
	struct srcmap srcmap;

	if (!(content = read_config_file(path, &srcmap)))
		return -1;

	config_init(config);
	snprintf(config->path, sizeof(config->path), "%s", path);
	return do_parse(config, content, &srcmap);
}

int config_check_match(struct config *config, const char *id, uint8_t flags)
{
	size_t i;

	for (i = 0; i < config->nr_ids; i++) {
		//Prefix match to allow matching <product>:<vendor> for backward compatibility.
		if (strstr(id, config->ids[i].id) == id) {
			if (config->ids[i].flags & ID_EXCLUDED) {
				return 0;
			} else if (config->ids[i].flags & flags) {
				return 2;
			}
		}
	}

	return config->wildcard ? 1 : 0;
}

int config_get_layer_index(const struct config *config, const char *name)
{
	size_t i;

	if (!name)
		return -1;

	for (i = 0; i < config->nr_layers; i++)
		if (!strcmp(config->layers[i].name, name))
			return i;

	return -1;
}

/*
 * Adds a binding of the form [<layer>.]<key> = <descriptor expression>
 * to the given config.
 */
int config_add_entry(struct config *config, const char *exp)
{
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

	if (dot && dot != s && (!paren || dot < paren)) {
		layername = s;
		*dot = 0;
		s = dot+1;
	}

	parse_kvp(s, &keyname, &descstr);
	idx = config_get_layer_index(config, layername);

	if (idx == -1) {
		err("%s is not a valid layer", layername);
		return -1;
	}

	layer = &config->layers[idx];

	if (parse_descriptor(descstr, &d, config) < 0)
		return -1;

	return set_layer_entry(config, layer, keyname, &d);
}

