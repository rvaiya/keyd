#ifndef KEYD_H
#define KEYD_H

#include <linux/input-event-codes.h>
#include <stdint.h>

#define MAX_KEYBOARDS 256
extern uint8_t keystate[KEY_CNT];

extern int debug;

void dbg(const char *fmt, ...);

void	set_mods(uint16_t mods);
void	send_key(int code, int pressed);
void	reload_config();
void	reset_keyboards();
void	reset_vkbd();

int		 evdev_get_keyboard_nodes(char **devs, int *ndevs);
int		 evdev_is_keyboard(const char *devnode);
uint32_t	 evdev_device_id(const char *devnode);
const char	*evdev_device_name(const char *devnode);

#endif
