/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <string.h>

#include "layer.h"
#include "keys.h"
#include "config.h"
#include "error.h"

/*
 * Populate the provided layer described by `desc`, which is a string of the
 * form "<layer>[:<type>]".  The provided config is used to look up
 * constituent layers in the case of a composite descriptor string
 * (e.g "layer1+layer2").
 */

int create_layer(struct layer *layer, const char *desc, const struct config *cfg)
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
		char *layername;
		int n = 0;
		int layers[MAX_COMPOSITE_LAYERS];

		if (modstr) {
			err("composite layers cannot have a modifier set.");
			return -1;
		}

		for (layername = strtok(name, "+"); layername; layername = strtok(NULL, "+")) {
			int idx = config_get_layer_index(cfg, layername);
			if (idx < 0) {
				err("%s is not a valid layer", layername);
				return -1;
			}

			if (n >= MAX_COMPOSITE_LAYERS) {
				err("max composite layers (%d) exceeded", MAX_COMPOSITE_LAYERS);
				return -1;
			}

			layers[n++] = idx;
		}

		layer->type = LT_COMPOSITE;
		layer->nr_constituents = n;

		memcpy(layer->constituents, layers, sizeof(layer->constituents));
	}  else if (modstr && !parse_modset(modstr, &mods)) {
			layer->type = LT_NORMAL;
			layer->mods = mods;
	} else {
		if (modstr)
			fprintf(stderr, "WARNING: \"%s\" is not a valid modifier set, ignoring\n", modstr);

		layer->type = LT_NORMAL;
		layer->mods = 0;
	}


	dbg2("created [%s] from \"%s\"", layer->name, desc);
	return 0;
}

