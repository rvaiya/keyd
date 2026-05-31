/*
 * keyd - A key remapping daemon.
 *
 * macOS keycode translation tables.
 *
 * KEYD codes are identical to Linux evdev codes for the basic key range (1–127).
 * macOS CGKeyCode values are "virtual keycodes" defined in Carbon/HIToolbox/Events.h.
 * The two numbering schemes are completely independent; this table bridges them.
 */

#include <stdint.h>
#include "../keys.h"
#include "keycodes.h"

/*
 * Forward table: CGKeyCode (index, 0–127) → KEYD code.
 * Entry is 0 for unmapped slots.
 */
static const uint8_t s_cgkey_to_keyd[128] = {
	/* 0x00 kVK_ANSI_A          */ KEYD_A,
	/* 0x01 kVK_ANSI_S          */ KEYD_S,
	/* 0x02 kVK_ANSI_D          */ KEYD_D,
	/* 0x03 kVK_ANSI_F          */ KEYD_F,
	/* 0x04 kVK_ANSI_H          */ KEYD_H,
	/* 0x05 kVK_ANSI_G          */ KEYD_G,
	/* 0x06 kVK_ANSI_Z          */ KEYD_Z,
	/* 0x07 kVK_ANSI_X          */ KEYD_X,
	/* 0x08 kVK_ANSI_C          */ KEYD_C,
	/* 0x09 kVK_ANSI_V          */ KEYD_V,
	/* 0x0A kVK_ISO_Section     */ KEYD_102ND,
	/* 0x0B kVK_ANSI_B          */ KEYD_B,
	/* 0x0C kVK_ANSI_Q          */ KEYD_Q,
	/* 0x0D kVK_ANSI_W          */ KEYD_W,
	/* 0x0E kVK_ANSI_E          */ KEYD_E,
	/* 0x0F kVK_ANSI_R          */ KEYD_R,
	/* 0x10 kVK_ANSI_Y          */ KEYD_Y,
	/* 0x11 kVK_ANSI_T          */ KEYD_T,
	/* 0x12 kVK_ANSI_1          */ KEYD_1,
	/* 0x13 kVK_ANSI_2          */ KEYD_2,
	/* 0x14 kVK_ANSI_3          */ KEYD_3,
	/* 0x15 kVK_ANSI_4          */ KEYD_4,
	/* 0x16 kVK_ANSI_6          */ KEYD_6,
	/* 0x17 kVK_ANSI_5          */ KEYD_5,
	/* 0x18 kVK_ANSI_Equal      */ KEYD_EQUAL,
	/* 0x19 kVK_ANSI_9          */ KEYD_9,
	/* 0x1A kVK_ANSI_7          */ KEYD_7,
	/* 0x1B kVK_ANSI_Minus      */ KEYD_MINUS,
	/* 0x1C kVK_ANSI_8          */ KEYD_8,
	/* 0x1D kVK_ANSI_0          */ KEYD_0,
	/* 0x1E kVK_ANSI_RightBrk   */ KEYD_RIGHTBRACE,
	/* 0x1F kVK_ANSI_O          */ KEYD_O,
	/* 0x20 kVK_ANSI_U          */ KEYD_U,
	/* 0x21 kVK_ANSI_LeftBrk    */ KEYD_LEFTBRACE,
	/* 0x22 kVK_ANSI_I          */ KEYD_I,
	/* 0x23 kVK_ANSI_P          */ KEYD_P,
	/* 0x24 kVK_Return          */ KEYD_ENTER,
	/* 0x25 kVK_ANSI_L          */ KEYD_L,
	/* 0x26 kVK_ANSI_J          */ KEYD_J,
	/* 0x27 kVK_ANSI_Quote      */ KEYD_APOSTROPHE,
	/* 0x28 kVK_ANSI_K          */ KEYD_K,
	/* 0x29 kVK_ANSI_Semicolon  */ KEYD_SEMICOLON,
	/* 0x2A kVK_ANSI_Backslash  */ KEYD_BACKSLASH,
	/* 0x2B kVK_ANSI_Comma      */ KEYD_COMMA,
	/* 0x2C kVK_ANSI_Slash      */ KEYD_SLASH,
	/* 0x2D kVK_ANSI_N          */ KEYD_N,
	/* 0x2E kVK_ANSI_M          */ KEYD_M,
	/* 0x2F kVK_ANSI_Period     */ KEYD_DOT,
	/* 0x30 kVK_Tab             */ KEYD_TAB,
	/* 0x31 kVK_Space           */ KEYD_SPACE,
	/* 0x32 kVK_ANSI_Grave      */ KEYD_GRAVE,
	/* 0x33 kVK_Delete (bksp)   */ KEYD_BACKSPACE,
	/* 0x34 (unused)            */ 0,
	/* 0x35 kVK_Escape          */ KEYD_ESC,
	/* 0x36 kVK_RightCommand    */ KEYD_RIGHTMETA,
	/* 0x37 kVK_Command         */ KEYD_LEFTMETA,
	/* 0x38 kVK_Shift           */ KEYD_LEFTSHIFT,
	/* 0x39 kVK_CapsLock        */ KEYD_CAPSLOCK,
	/* 0x3A kVK_Option          */ KEYD_LEFTALT,
	/* 0x3B kVK_Control         */ KEYD_LEFTCTRL,
	/* 0x3C kVK_RightShift      */ KEYD_RIGHTSHIFT,
	/* 0x3D kVK_RightOption     */ KEYD_RIGHTALT,
	/* 0x3E kVK_RightControl    */ KEYD_RIGHTCTRL,
	/* 0x3F kVK_Function        */ KEYD_FN,
	/* 0x40 kVK_F17             */ KEYD_F17,
	/* 0x41 kVK_KP_Decimal      */ KEYD_KPDOT,
	/* 0x42 (unused)            */ 0,
	/* 0x43 kVK_KP_Multiply     */ KEYD_KPASTERISK,
	/* 0x44 (unused)            */ 0,
	/* 0x45 kVK_KP_Plus         */ KEYD_KPPLUS,
	/* 0x46 (unused)            */ 0,
	/* 0x47 kVK_KP_Clear        */ KEYD_NUMLOCK,
	/* 0x48 kVK_VolumeUp        */ KEYD_VOLUMEUP,
	/* 0x49 kVK_VolumeDown      */ KEYD_VOLUMEDOWN,
	/* 0x4A kVK_Mute            */ KEYD_MUTE,
	/* 0x4B kVK_KP_Divide       */ KEYD_KPSLASH,
	/* 0x4C kVK_KP_Enter        */ KEYD_KPENTER,
	/* 0x4D (unused)            */ 0,
	/* 0x4E kVK_KP_Minus        */ KEYD_KPMINUS,
	/* 0x4F kVK_F18             */ KEYD_F18,
	/* 0x50 kVK_F19             */ KEYD_F19,
	/* 0x51 kVK_KP_Equals       */ KEYD_KPEQUAL,
	/* 0x52 kVK_KP_0            */ KEYD_KP0,
	/* 0x53 kVK_KP_1            */ KEYD_KP1,
	/* 0x54 kVK_KP_2            */ KEYD_KP2,
	/* 0x55 kVK_KP_3            */ KEYD_KP3,
	/* 0x56 kVK_KP_4            */ KEYD_KP4,
	/* 0x57 kVK_KP_5            */ KEYD_KP5,
	/* 0x58 kVK_KP_6            */ KEYD_KP6,
	/* 0x59 kVK_KP_7            */ KEYD_KP7,
	/* 0x5A kVK_F20             */ KEYD_F20,
	/* 0x5B kVK_KP_8            */ KEYD_KP8,
	/* 0x5C kVK_KP_9            */ KEYD_KP9,
	/* 0x5D kVK_JIS_Yen         */ KEYD_YEN,
	/* 0x5E kVK_JIS_Underscore  */ KEYD_RO,
	/* 0x5F kVK_JIS_KP_Comma    */ KEYD_KPJPCOMMA,
	/* 0x60 kVK_F5              */ KEYD_F5,
	/* 0x61 kVK_F6              */ KEYD_F6,
	/* 0x62 kVK_F7              */ KEYD_F7,
	/* 0x63 kVK_F3              */ KEYD_F3,
	/* 0x64 kVK_F8              */ KEYD_F8,
	/* 0x65 kVK_F9              */ KEYD_F9,
	/* 0x66 kVK_JIS_Eisu        */ KEYD_KATAKANAHIRAGANA,
	/* 0x67 kVK_F11             */ KEYD_F11,
	/* 0x68 kVK_JIS_Kana        */ KEYD_KATAKANA,
	/* 0x69 kVK_F13             */ KEYD_F13,
	/* 0x6A kVK_F16             */ KEYD_F16,
	/* 0x6B kVK_F14             */ KEYD_F14,
	/* 0x6C (unused)            */ 0,
	/* 0x6D kVK_F10             */ KEYD_F10,
	/* 0x6E (unused)            */ 0,
	/* 0x6F kVK_F12             */ KEYD_F12,
	/* 0x70 (unused)            */ 0,
	/* 0x71 kVK_F15             */ KEYD_F15,
	/* 0x72 kVK_Help            */ KEYD_INSERT,
	/* 0x73 kVK_Home            */ KEYD_HOME,
	/* 0x74 kVK_PageUp          */ KEYD_PAGEUP,
	/* 0x75 kVK_ForwardDelete   */ KEYD_DELETE,
	/* 0x76 kVK_F4              */ KEYD_F4,
	/* 0x77 kVK_End             */ KEYD_END,
	/* 0x78 kVK_F2              */ KEYD_F2,
	/* 0x79 kVK_PageDown        */ KEYD_PAGEDOWN,
	/* 0x7A kVK_F1              */ KEYD_F1,
	/* 0x7B kVK_LeftArrow       */ KEYD_LEFT,
	/* 0x7C kVK_RightArrow      */ KEYD_RIGHT,
	/* 0x7D kVK_DownArrow       */ KEYD_DOWN,
	/* 0x7E kVK_UpArrow         */ KEYD_UP,
	/* 0x7F (unused)            */ 0,
};

uint8_t cgkey_to_keyd_code(uint16_t cgkey)
{
	if (cgkey >= 128)
		return 0;
	return s_cgkey_to_keyd[cgkey];
}

uint16_t keyd_to_cgkey(uint8_t keyd_code)
{
	uint16_t i;
	for (i = 0; i < 128; i++) {
		if (s_cgkey_to_keyd[i] == keyd_code && keyd_code != 0)
			return i;
	}
	return 0xFFFF;
}
