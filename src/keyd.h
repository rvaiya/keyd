#ifndef KEYD_H
#define KEYD_H

#include "keys.h"
#include <stdint.h>

#define MAX_KEYBOARDS 256
extern uint8_t keystate[MAX_KEYS];

extern int debug;

void dbg(const char *fmt, ...);

void	send_key(int code, int pressed);
void	reload_config();
void	reset_keyboards();
void	reset_vkbd();

int		 evdev_get_keyboard_nodes(char **devs, int *ndevs);
int		 evdev_is_keyboard(const char *devnode);
uint32_t	 evdev_device_id(const char *devnode);
const char	*evdev_device_name(const char *devnode);

#endif
