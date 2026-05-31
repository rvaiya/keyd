/*
 * keyd - A key remapping daemon.
 *
 * macOS keycode translation: KEYD (evdev-compatible) ↔ CGKeyCode (macOS virtual keycodes).
 */
#ifndef MACOS_KEYCODES_H
#define MACOS_KEYCODES_H

#include <stdint.h>

/* Returns the CGKeyCode for a given KEYD code, or 0xFFFF if unmapped. */
uint16_t keyd_to_cgkey(uint8_t keyd_code);

/* Returns the KEYD code for a given CGKeyCode (0–127), or 0 if unmapped. */
uint8_t cgkey_to_keyd_code(uint16_t cgkey);

#endif
