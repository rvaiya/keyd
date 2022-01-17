#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DEVICE_IDS 32
#define MAX_CONFIG_NAME 256
#define MAX_LAYERS 32
#define MAX_MACROS 32

#include "layer.h"

struct config {
	char	 name[MAX_CONFIG_NAME];
	uint32_t device_ids[MAX_DEVICE_IDS];

	int has_wildcard;

	/* The first two layers are the default main and modifier layouts. */
	struct layer layers[MAX_LAYERS];
	struct macro macros[MAX_MACROS];

	/* The index of the default layout within the config layer table. */
	int default_layout;

	size_t nr_device_ids;
	size_t nr_layers;

	struct config *next;
};

	
struct config	*lookup_config(uint32_t device_id);

int	config_lookup_layer(struct config *config, const char *name);
int	config_create_layer(struct config *config, const char *name, uint16_t mods);

int	config_execute_expression(struct config *config, const char *str);
int	read_config_dir(const char *dir);

void	free_configs();

#endif
