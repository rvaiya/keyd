/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <string.h>

#include "layer.h"
#include "ini.h"
#include "error.h"

int layer_table_lookup(const struct layer_table *lt, const char *name)
{
	size_t i;

	for (i = 0; i < lt->nr_layers; i++)
		if (!strcmp(lt->layers[i].name, name))
			return i;

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


/*
 * Consumes a string of the form `[<layer>.]<key> = <descriptor>` and adds the
 * mapping to the corresponding layer in the layer_table.
 */

int layer_table_add_entry(struct layer_table *lt, const char *exp)
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

	idx = layer_table_lookup(lt, layername);

	if (idx == -1) {
		err("%s is not a valid layer", layername);
		return -1;
	}

	layer = &lt->layers[idx];

	if (lookup_keycodes(keyname, &code1, &code2) < 0) {
		err("%s is not a valid key.", keyname);
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

