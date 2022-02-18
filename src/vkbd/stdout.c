/* Build with make vkbd-stdout. */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>

#include "../vkbd.h"
#include "../keys.h"

struct vkbd {};

struct vkbd *vkbd_init(const char *name)
{
	return NULL;
}

void vkbd_move_mouse(const struct vkbd *vkbd, int x, int y)
{
	printf("mouse movement: x: %d, y: %d\n", x, y);
}

void vkbd_send_button(const struct vkbd *vkbd, uint8_t btn, int state)
{
	printf("mouse button: %d, state: %d\n", btn, state);
}


void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	printf("key: %s, state: %d\n", keycode_table[code].name, state);
}

void free_vkbd(struct vkbd *vkbd)
{
}
