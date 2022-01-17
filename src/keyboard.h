#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "config.h"
#include "layer.h"

#define MAX_ACTIVE_KEYS 32

extern struct keyboard *active_keyboard;

struct active_layer {
	int layer;
	int oneshot;
};

/* Represents a currently depressed key */
struct active_key {
	uint16_t code;

	struct descriptor d;
	int dl; /* The layer from which the descriptor was drawn. */
};

/* Active keyboard state. */
struct keyboard {
	int fd;
	char devnode[256];
	uint32_t id;

	struct active_layer active_layers[MAX_LAYERS];
	size_t nr_active_layers;

	struct config original_config;
	struct config config;
	int layout;

	struct active_key active_keys[MAX_ACTIVE_KEYS];
	size_t nr_active_keys;

	uint16_t last_pressed_keycode;

	struct keyboard *next;
};

long	kbd_process_key_event(struct keyboard *kbd, uint16_t code, int pressed);

void	kbd_reset(struct keyboard *kbd);
int	kbd_execute_expression(struct keyboard *kbd, const char *exp);

#endif
