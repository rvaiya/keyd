#ifndef KEYD_H
#define KEYD_H

extern uint8_t keystate[KEY_CNT];
void set_mods(uint16_t mods);
void send_key(int code, int pressed);
void reload_config();
void reset_keyboards();
void reset_vkbd();

#endif
