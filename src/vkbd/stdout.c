/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
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

void vkbd_mouse_scroll(const struct vkbd *vkbd, int x, int y)
{
	printf("mouse scroll: x: %d, y: %d\n", x, y);
}

void vkbd_mouse_move(const struct vkbd *vkbd, int x, int y)
{
	printf("mouse movement: x: %d, y: %d\n", x, y);
}

void vkbd_mouse_move_abs(const struct vkbd *vkbd, int x, int y)
{
	printf("absolute mouse movement: x: %d, y: %d\n", x, y);
}

void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	printf("key: %s, state: %d\n", keycode_table[code].name, state);
}

void free_vkbd(struct vkbd *vkbd)
{
}
