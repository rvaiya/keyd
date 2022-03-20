/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <stdint.h>
#include "keys.h"

const struct modifier_table_ent modifier_table[MAX_MOD] = {
	{"control", MOD_CTRL, KEY_LEFTCTRL, KEY_RIGHTCTRL},
	{"shift", MOD_SHIFT, KEY_LEFTSHIFT, KEY_RIGHTSHIFT},
	{"meta", MOD_SUPER, KEY_LEFTMETA, KEY_RIGHTMETA},

	{"alt", MOD_ALT, KEY_LEFTALT, 0},
	{"altgr", MOD_ALT_GR, KEY_RIGHTALT, 0},
};

const struct keycode_table_ent keycode_table[256] = {
	[KEY_ESC] = { "esc", "escape", NULL },
	[KEY_1] = { "1", NULL, "!" },
	[KEY_2] = { "2", NULL, "@" },
	[KEY_3] = { "3", NULL, "#" },
	[KEY_4] = { "4", NULL, "$" },
	[KEY_5] = { "5", NULL, "%" },
	[KEY_6] = { "6", NULL, "^" },
	[KEY_7] = { "7", NULL, "&" },
	[KEY_8] = { "8", NULL, "*" },
	[KEY_9] = { "9", NULL, "(" },
	[KEY_0] = { "0", NULL, ")" },
	[KEY_MINUS] = { "-", "minus", "_" },
	[KEY_EQUAL] = { "=", "equal", "+" },
	[KEY_BACKSPACE] = { "backspace", NULL, NULL },
	[KEY_TAB] = { "tab", NULL, NULL },
	[KEY_Q] = { "q", NULL, "Q" },
	[KEY_W] = { "w", NULL, "W" },
	[KEY_E] = { "e", NULL, "E" },
	[KEY_R] = { "r", NULL, "R" },
	[KEY_T] = { "t", NULL, "T" },
	[KEY_Y] = { "y", NULL, "Y" },
	[KEY_U] = { "u", NULL, "U" },
	[KEY_I] = { "i", NULL, "I" },
	[KEY_O] = { "o", NULL, "O" },
	[KEY_P] = { "p", NULL, "P" },
	[KEY_LEFTBRACE] = { "[", "leftbrace", "{" },
	[KEY_RIGHTBRACE] = { "]", "rightbrace", "}" },
	[KEY_ENTER] = { "enter", NULL, NULL },
	[KEY_LEFTCTRL] = { "leftcontrol", "", NULL },
	[84] = { "iso-level3-shift", NULL, NULL }, //Oddly missing from input-event-codes.h, appears to be used as altgr in an english keymap on X
	[KEY_A] = { "a", NULL, "A" },
	[KEY_S] = { "s", NULL, "S" },
	[KEY_D] = { "d", NULL, "D" },
	[KEY_F] = { "f", NULL, "F" },
	[KEY_G] = { "g", NULL, "G" },
	[KEY_H] = { "h", NULL, "H" },
	[KEY_J] = { "j", NULL, "J" },
	[KEY_K] = { "k", NULL, "K" },
	[KEY_L] = { "l", NULL, "L" },
	[KEY_SEMICOLON] = { ";", "semicolon", ":" },
	[KEY_APOSTROPHE] = { "'", "apostrophe", "\"" },
	[KEY_GRAVE] = { "`", "grave", "~" },
	[KEY_LEFTSHIFT] = { "leftshift", "", NULL },
	[KEY_BACKSLASH] = { "\\", "backslash", "|" },
	[KEY_Z] = { "z", NULL, "Z" },
	[KEY_X] = { "x", NULL, "X" },
	[KEY_C] = { "c", NULL, "C" },
	[KEY_V] = { "v", NULL, "V" },
	[KEY_B] = { "b", NULL, "B" },
	[KEY_N] = { "n", NULL, "N" },
	[KEY_M] = { "m", NULL, "M" },
	[KEY_COMMA] = { ",", "comma", "<" },
	[KEY_DOT] = { ".", "dot", ">" },
	[KEY_SLASH] = { "/", "slash", "?" },
	[KEY_RIGHTSHIFT] = { "rightshift", NULL, NULL },
	[KEY_KPASTERISK] = { "kpasterisk", NULL, NULL },
	[KEY_LEFTALT] = { "leftalt", "", NULL },
	[KEY_SPACE] = { "space", NULL, NULL },
	[KEY_CAPSLOCK] = { "capslock", NULL, NULL },
	[KEY_F1] = { "f1", NULL, NULL },
	[KEY_F2] = { "f2", NULL, NULL },
	[KEY_F3] = { "f3", NULL, NULL },
	[KEY_F4] = { "f4", NULL, NULL },
	[KEY_F5] = { "f5", NULL, NULL },
	[KEY_F6] = { "f6", NULL, NULL },
	[KEY_F7] = { "f7", NULL, NULL },
	[KEY_F8] = { "f8", NULL, NULL },
	[KEY_F9] = { "f9", NULL, NULL },
	[KEY_F10] = { "f10", NULL, NULL },
	[KEY_NUMLOCK] = { "numlock", NULL, NULL },
	[KEY_SCROLLLOCK] = { "scrolllock", NULL, NULL },
	[KEY_KP7] = { "kp7", NULL, NULL },
	[KEY_KP8] = { "kp8", NULL, NULL },
	[KEY_KP9] = { "kp9", NULL, NULL },
	[KEY_KPMINUS] = { "kpminus", NULL, NULL },
	[KEY_KP4] = { "kp4", NULL, NULL },
	[KEY_KP5] = { "kp5", NULL, NULL },
	[KEY_KP6] = { "kp6", NULL, NULL },
	[KEY_KPPLUS] = { "kpplus", NULL, NULL },
	[KEY_KP1] = { "kp1", NULL, NULL },
	[KEY_KP2] = { "kp2", NULL, NULL },
	[KEY_KP3] = { "kp3", NULL, NULL },
	[KEY_KP0] = { "kp0", NULL, NULL },
	[KEY_KPDOT] = { "kpdot", NULL, NULL },
	[KEY_ZENKAKUHANKAKU] = { "zenkakuhankaku", NULL, NULL },
	[KEY_102ND] = { "102nd", NULL, NULL },
	[KEY_F11] = { "f11", NULL, NULL },
	[KEY_F12] = { "f12", NULL, NULL },
	[KEY_RO] = { "ro", NULL, NULL },
	[KEY_KATAKANA] = { "katakana", NULL, NULL },
	[KEY_HIRAGANA] = { "hiragana", NULL, NULL },
	[KEY_HENKAN] = { "henkan", NULL, NULL },
	[KEY_KATAKANAHIRAGANA] = { "katakanahiragana", NULL, NULL },
	[KEY_MUHENKAN] = { "muhenkan", NULL, NULL },
	[KEY_KPJPCOMMA] = { "kpjpcomma", NULL, NULL },
	[KEY_KPENTER] = { "kpenter", NULL, NULL },
	[KEY_RIGHTCTRL] = { "rightcontrol", NULL, NULL },
	[KEY_KPSLASH] = { "kpslash", NULL, NULL },
	[KEY_SYSRQ] = { "sysrq", NULL, NULL },
	[KEY_RIGHTALT] = { "rightalt", NULL, NULL },
	[KEY_LINEFEED] = { "linefeed", NULL, NULL },
	[KEY_HOME] = { "home", NULL, NULL },
	[KEY_UP] = { "up", NULL, NULL },
	[KEY_PAGEUP] = { "pageup", NULL, NULL },
	[KEY_LEFT] = { "left", NULL, NULL },
	[KEY_RIGHT] = { "right", NULL, NULL },
	[KEY_END] = { "end", NULL, NULL },
	[KEY_DOWN] = { "down", NULL, NULL },
	[KEY_PAGEDOWN] = { "pagedown", NULL, NULL },
	[KEY_INSERT] = { "insert", NULL, NULL },
	[KEY_DELETE] = { "delete", NULL, NULL },
	[KEY_MACRO] = { "macro", NULL, NULL },
	[KEY_MUTE] = { "mute", NULL, NULL },
	[KEY_VOLUMEDOWN] = { "volumedown", NULL, NULL },
	[KEY_VOLUMEUP] = { "volumeup", NULL, NULL },
	[KEY_POWER] = { "power", NULL, NULL },
	[KEY_KPEQUAL] = { "kpequal", NULL, NULL },
	[KEY_KPPLUSMINUS] = { "kpplusminus", NULL, NULL },
	[KEY_PAUSE] = { "pause", NULL, NULL },
	[KEY_SCALE] = { "scale", NULL, NULL },
	[KEY_KPCOMMA] = { "kpcomma", NULL, NULL },
	[KEY_HANGEUL] = { "hangeul", NULL, NULL },
	[KEY_HANJA] = { "hanja", NULL, NULL },
	[KEY_YEN] = { "yen", NULL, NULL },
	[KEY_LEFTMETA] = { "leftmeta", "", NULL },
	[KEY_RIGHTMETA] = { "rightmeta", NULL, NULL },
	[KEY_COMPOSE] = { "compose", NULL, NULL },
	[KEY_STOP] = { "stop", NULL, NULL },
	[KEY_AGAIN] = { "again", NULL, NULL },
	[KEY_PROPS] = { "props", NULL, NULL },
	[KEY_UNDO] = { "undo", NULL, NULL },
	[KEY_FRONT] = { "front", NULL, NULL },
	[KEY_COPY] = { "copy", NULL, NULL },
	[KEY_OPEN] = { "open", NULL, NULL },
	[KEY_PASTE] = { "paste", NULL, NULL },
	[KEY_FIND] = { "find", NULL, NULL },
	[KEY_CUT] = { "cut", NULL, NULL },
	[KEY_HELP] = { "help", NULL, NULL },
	[KEY_MENU] = { "menu", NULL, NULL },
	[KEY_CALC] = { "calc", NULL, NULL },
	[KEY_SETUP] = { "setup", NULL, NULL },
	[KEY_SLEEP] = { "sleep", NULL, NULL },
	[KEY_WAKEUP] = { "wakeup", NULL, NULL },
	[KEY_FILE] = { "file", NULL, NULL },
	[KEY_SENDFILE] = { "sendfile", NULL, NULL },
	[KEY_DELETEFILE] = { "deletefile", NULL, NULL },
	[KEY_XFER] = { "xfer", NULL, NULL },
	[KEY_PROG1] = { "prog1", NULL, NULL },
	[KEY_PROG2] = { "prog2", NULL, NULL },
	[KEY_WWW] = { "www", NULL, NULL },
	[KEY_MSDOS] = { "msdos", NULL, NULL },
	[KEY_COFFEE] = { "coffee", NULL, NULL },
	[KEY_ROTATE_DISPLAY] = { "display", NULL, NULL },
	[KEY_CYCLEWINDOWS] = { "cyclewindows", NULL, NULL },
	[KEY_MAIL] = { "mail", NULL, NULL },
	[KEY_BOOKMARKS] = { "bookmarks", NULL, NULL },
	[KEY_COMPUTER] = { "computer", NULL, NULL },
	[KEY_BACK] = { "back", NULL, NULL },
	[KEY_FORWARD] = { "forward", NULL, NULL },
	[KEY_CLOSECD] = { "closecd", NULL, NULL },
	[KEY_EJECTCD] = { "ejectcd", NULL, NULL },
	[KEY_EJECTCLOSECD] = { "ejectclosecd", NULL, NULL },
	[KEY_NEXTSONG] = { "nextsong", NULL, NULL },
	[KEY_PLAYPAUSE] = { "playpause", NULL, NULL },
	[KEY_PREVIOUSSONG] = { "previoussong", NULL, NULL },
	[KEY_STOPCD] = { "stopcd", NULL, NULL },
	[KEY_RECORD] = { "record", NULL, NULL },
	[KEY_REWIND] = { "rewind", NULL, NULL },
	[KEY_PHONE] = { "phone", NULL, NULL },
	[KEY_ISO] = { "iso", NULL, NULL },
	[KEY_CONFIG] = { "config", NULL, NULL },
	[KEY_HOMEPAGE] = { "homepage", NULL, NULL },
	[KEY_REFRESH] = { "refresh", NULL, NULL },
	[KEY_EXIT] = { "exit", NULL, NULL },
	[KEY_MOVE] = { "move", NULL, NULL },
	[KEY_EDIT] = { "edit", NULL, NULL },
	[KEY_SCROLLUP] = { "scrollup", NULL, NULL },
	[KEY_SCROLLDOWN] = { "scrolldown", NULL, NULL },
	[KEY_KPLEFTPAREN] = { "kpleftparen", NULL, NULL },
	[KEY_KPRIGHTPAREN] = { "kprightparen", NULL, NULL },
	[KEY_NEW] = { "new", NULL, NULL },
	[KEY_REDO] = { "redo", NULL, NULL },
	[KEY_F13] = { "f13", NULL, NULL },
	[KEY_F14] = { "f14", NULL, NULL },
	[KEY_F15] = { "f15", NULL, NULL },
	[KEY_F16] = { "f16", NULL, NULL },
	[KEY_F17] = { "f17", NULL, NULL },
	[KEY_F18] = { "f18", NULL, NULL },
	[KEY_F19] = { "f19", NULL, NULL },
	[KEY_F20] = { "f20", NULL, NULL },
	[KEY_F21] = { "f21", NULL, NULL },
	[KEY_F22] = { "f22", NULL, NULL },
	[KEY_F23] = { "f23", NULL, NULL },
	[KEY_F24] = { "f24", NULL, NULL },
	[KEY_PLAYCD] = { "playcd", NULL, NULL },
	[KEY_PAUSECD] = { "pausecd", NULL, NULL },
	[KEY_PROG3] = { "prog3", NULL, NULL },
	[KEY_PROG4] = { "prog4", NULL, NULL },
	[KEY_DASHBOARD] = { "dashboard", NULL, NULL },
	[KEY_SUSPEND] = { "suspend", NULL, NULL },
	[KEY_CLOSE] = { "close", NULL, NULL },
	[KEY_PLAY] = { "play", NULL, NULL },
	[KEY_FASTFORWARD] = { "fastforward", NULL, NULL },
	[KEY_BASSBOOST] = { "bassboost", NULL, NULL },
	[KEY_PRINT] = { "print", NULL, NULL },
	[KEY_HP] = { "hp", NULL, NULL },
	[KEY_CAMERA] = { "camera", NULL, NULL },
	[KEY_SOUND] = { "sound", NULL, NULL },
	[KEY_QUESTION] = { "question", NULL, NULL },
	[KEY_EMAIL] = { "email", NULL, NULL },
	[KEY_CHAT] = { "chat", NULL, NULL },
	[KEY_SEARCH] = { "search", NULL, NULL },
	[KEY_CONNECT] = { "connect", NULL, NULL },
	[KEY_FINANCE] = { "finance", NULL, NULL },
	[KEY_SPORT] = { "sport", NULL, NULL },
	[KEY_SHOP] = { "shop", NULL, NULL },
	[KEY_ALTERASE] = { "alterase", NULL, NULL },
	[KEY_CANCEL] = { "cancel", NULL, NULL },
	[KEY_BRIGHTNESSDOWN] = { "brightnessdown", NULL, NULL },
	[KEY_BRIGHTNESSUP] = { "brightnessup", NULL, NULL },
	[KEY_MEDIA] = { "media", NULL, NULL },
	[KEY_SWITCHVIDEOMODE] = { "switchvideomode", NULL, NULL },
	[KEY_KBDILLUMTOGGLE] = { "kbdillumtoggle", NULL, NULL },
	[KEY_KBDILLUMDOWN] = { "kbdillumdown", NULL, NULL },
	[KEY_KBDILLUMUP] = { "kbdillumup", NULL, NULL },
	[KEY_SEND] = { "send", NULL, NULL },
	[KEY_REPLY] = { "reply", NULL, NULL },
	[KEY_FORWARDMAIL] = { "forwardmail", NULL, NULL },
	[KEY_SAVE] = { "save", NULL, NULL },
	[KEY_DOCUMENTS] = { "documents", NULL, NULL },
	[KEY_BATTERY] = { "battery", NULL, NULL },
	[KEY_BLUETOOTH] = { "bluetooth", NULL, NULL },
	[KEY_WLAN] = { "wlan", NULL, NULL },
	[KEY_UWB] = { "uwb", NULL, NULL },
	[KEY_UNKNOWN] = { "unknown", NULL, NULL },
	[KEY_VIDEO_NEXT] = { "next", NULL, NULL },
	[KEY_VIDEO_PREV] = { "prev", NULL, NULL },
	[KEY_BRIGHTNESS_CYCLE] = { "cycle", NULL, NULL },
	[KEY_BRIGHTNESS_AUTO] = { "auto", NULL, NULL },
	[KEY_DISPLAY_OFF] = { "off", NULL, NULL },
	[KEY_WWAN] = { "wwan", NULL, NULL },
	[KEY_RFKILL] = { "rfkill", NULL, NULL },
	[KEY_MICMUTE] = { "micmute", NULL, NULL },
	[KEY_LEFT_MOUSE] = { "leftmouse", NULL, NULL },
	[KEY_RIGHT_MOUSE] = { "rightmouse", NULL, NULL },
	[KEY_MIDDLE_MOUSE] = { "middlemouse", NULL, NULL },
	[KEY_MOUSE_1] = { "mouse1", NULL, NULL },
	[KEY_MOUSE_2] = { "mouse2", NULL, NULL },
};

uint8_t keycode_to_mod(uint8_t code)
{
	switch (code) {
	case KEY_LEFTSHIFT:
	case KEY_RIGHTSHIFT:
		return MOD_SHIFT;
	case KEY_LEFTALT:
		return MOD_ALT;
	case KEY_RIGHTALT:
		return MOD_ALT_GR;
	case KEY_LEFTCTRL:
	case KEY_RIGHTCTRL:
		return MOD_CTRL;
	case KEY_LEFTMETA:
	case KEY_RIGHTMETA:
		return MOD_SUPER;
	}

	return 0;
}

const char *modstring(uint8_t mods)
{
	static char s[16];
	int i = 0;
	s[0] = 0;

	if (MOD_CTRL & mods) {
		s[i++] = 'C';
		s[i++] = '-';
	}

	if (MOD_SUPER & mods) {
		s[i++] = 'M';
		s[i++] = '-';
	}

	if (MOD_ALT_GR & mods) {
		s[i++] = 'G';
		s[i++] = '-';
	}

	if (MOD_SHIFT & mods) {
		s[i++] = 'S';
		s[i++] = '-';
	}

	if (MOD_ALT & mods) {
		s[i++] = 'A';
		s[i++] = '-';
	}

	if(i)
		s[i-1] = 0;

	return s;
}

int parse_modset(const char *s, uint8_t *mods)
{
	*mods = 0;

	while (*s) {
		switch (*s) {
		case 'C':
			*mods |= MOD_CTRL;
			break;
		case 'M':
			*mods |= MOD_SUPER;
			break;
		case 'A':
			*mods |= MOD_ALT;
			break;
		case 'S':
			*mods |= MOD_SHIFT;
			break;
		case 'G':
			*mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		if (s[1] == 0)
			return 0;
		else if (s[1] != '-')
			return -1;

		s += 2;
	}

	return 0;
}
