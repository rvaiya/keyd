#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

#include <stdint.h>

struct vkbd;

struct vkbd *vkbd_init(const char *name);
void vkbd_send(const struct vkbd *vkbd, uint16_t code, int state);
void vkbd_move_mouse(const struct vkbd *vkbd, int x, int y);

void free_vkbd(struct vkbd *vkbd);

#endif
