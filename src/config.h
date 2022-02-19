#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DEVICE_IDS 32
#define MAX_CONFIG_NAME 256
#define MAX_LAYERS 32
#define MAX_MACROS 32

#include "layer.h"

struct config {
	struct layer layers[MAX_LAYERS];
	struct macro macros[MAX_MACROS];

	/* The index of the default layout within the config layer table. */
	int default_layout;

	size_t nr_layers;
};

int		 config_lookup_layer(struct config *config, const char *name);
int		 config_create_layer(struct config *config, const char *name, uint8_t mods);

int		 config_execute_expression(struct config *config, const char *str);
const char	*config_find_path(const char *dir, uint16_t vendor, uint16_t product);

int		 config_parse(struct config *config, const char *path);

#endif
