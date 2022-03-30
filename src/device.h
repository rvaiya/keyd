/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#define DEV_MOUSE	0
#define DEV_KEY		1
#define DEV_REMOVED	2

#define MAX_DEVICES	64

struct device {
	/* 
	 * A file descriptor that can be used to monitor events subsequently read with
	 * device_read_event(). 
	 */
	int fd;

	uint8_t is_keyboard;
	uint16_t product_id;
	uint16_t vendor_id;
	char name[64];
	char path[256];

	/* Reserved for the user. */
	void *data;
};

struct device_event {
	uint8_t type;

	uint8_t code;
	uint8_t pressed;
	uint8_t x;
	uint8_t y;
};


struct device_event	*device_read_event(struct device *dev);

int			 device_scan(struct device devices[MAX_DEVICES]);
int		 	 device_grab(struct device *dev);
int		 	 device_ungrab(struct device *dev);

int		 devmon_create();
struct device	*devmon_read_device(int fd);
void		 device_set_led(const struct device *dev, int led, int state);

#endif
