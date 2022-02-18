#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

#include <stdint.h>

struct vkbd;

struct vkbd	*vkbd_init(const char *name);
void		 vkbd_move_mouse(const struct vkbd *vkbd, int x, int y);
void		 vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state);
void		 vkbd_send_button(const struct vkbd *vkbd, uint8_t btn, int state);

void		 free_vkbd(struct vkbd *vkbd);

#endif
