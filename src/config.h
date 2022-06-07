/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DEVICE_IDS		32
#define MAX_CONFIG_NAME		256

#define MAX_COMMANDS		64
#define MAX_MACROS		256
#define MAX_AUX_DESCRIPTORS	64

#include "layer.h"
#include "descriptor.h"
#include "macro.h"
#include "command.h"


struct config {
	struct layer layers[MAX_LAYERS];

	/* Auxiliary descriptors used by layer bindings. */
	struct descriptor descriptors[MAX_AUX_DESCRIPTORS];
	struct macro macros[MAX_MACROS];
	struct command commands[MAX_COMMANDS];

	size_t nr_layers;

	size_t nr_macros;
	size_t nr_descriptors;
	size_t nr_commands;

	long macro_timeout;
	long macro_repeat_timeout;
	long layer_indicator;
};

int config_parse(struct config *config, const char *path);
int config_add_entry(struct config *config, const char *exp);
int config_get_layer_index(const struct config *config, const char *name);

const char *find_config_path(const char *dir, uint16_t vendor, uint16_t product, uint8_t *is_exact_match);

#endif
