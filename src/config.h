/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DEVICE_IDS 32
#define MAX_CONFIG_NAME 256

#include "layer.h"

struct config {
	struct layer_table layer_table;
};

const char	*config_find_path(const char *dir, uint16_t vendor, uint16_t product);
int		 config_parse(struct config *config, const char *path);

#endif
