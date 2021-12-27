#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DEVICE_IDS 32
#define MAX_CONFIG_NAME 256
#define MAX_LAYERS 32

#include "layer.h"

struct config {
	char name[MAX_CONFIG_NAME];
	uint32_t device_ids[MAX_DEVICE_IDS];

	/* The first two layers are the default main and modifier layouts. */
	struct layer *layers[MAX_LAYERS];

	size_t nr_device_ids;
	size_t nr_layers;

	struct config *next;
};

	
struct config *add_config(const char *config_name, char *str);
struct config *lookup_config(uint32_t device_id);
int read_config_dir(const char *dir);
void free_configs();

#endif
