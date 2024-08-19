/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <stdint.h>
#include <string.h>
#include "keys.h"

const struct modifier modifiers[MAX_MOD] = {
	{MOD_ALT, KEYD_LEFTALT},
	{MOD_ALT_GR, KEYD_RIGHTALT},
	{MOD_SHIFT, KEYD_LEFTSHIFT},
	{MOD_SUPER, KEYD_LEFTMETA},
	{MOD_CTRL, KEYD_LEFTCTRL},
};

const struct keycode_table_ent keycode_table[256] = {
	[KEYD_ESC] = { "esc", "escape", NULL },
	[KEYD_1] = { "1", NULL, "!" },
	[KEYD_2] = { "2", NULL, "@" },
	[KEYD_3] = { "3", NULL, "#" },
	[KEYD_4] = { "4", NULL, "$" },
	[KEYD_5] = { "5", NULL, "%" },
	[KEYD_6] = { "6", NULL, "^" },
	[KEYD_7] = { "7", NULL, "&" },
	[KEYD_8] = { "8", NULL, "*" },
	[KEYD_9] = { "9", NULL, "(" },
	[KEYD_0] = { "0", NULL, ")" },
	[KEYD_MINUS] = { "-", "minus", "_" },
	[KEYD_EQUAL] = { "=", "equal", "+" },
	[KEYD_BACKSPACE] = { "backspace", NULL, NULL },
	[KEYD_TAB] = { "tab", NULL, NULL },
	[KEYD_Q] = { "q", NULL, "Q" },
	[KEYD_W] = { "w", NULL, "W" },
	[KEYD_E] = { "e", NULL, "E" },
	[KEYD_R] = { "r", NULL, "R" },
	[KEYD_T] = { "t", NULL, "T" },
	[KEYD_Y] = { "y", NULL, "Y" },
	[KEYD_U] = { "u", NULL, "U" },
	[KEYD_I] = { "i", NULL, "I" },
	[KEYD_O] = { "o", NULL, "O" },
	[KEYD_P] = { "p", NULL, "P" },
	[KEYD_LEFTBRACE] = { "[", "leftbrace", "{" },
	[KEYD_RIGHTBRACE] = { "]", "rightbrace", "}" },
	[KEYD_ENTER] = { "enter", NULL, NULL },
	[KEYD_LEFTCTRL] = { "leftcontrol", "", NULL },
	[KEYD_IS_LEVEL3_SHIFT] = { "iso-level3-shift", NULL, NULL }, //Oddly missing from input-event-codes.h, appears to be used as altgr in an english keymap on X
	[KEYD_A] = { "a", NULL, "A" },
	[KEYD_S] = { "s", NULL, "S" },
	[KEYD_D] = { "d", NULL, "D" },
	[KEYD_F] = { "f", NULL, "F" },
	[KEYD_G] = { "g", NULL, "G" },
	[KEYD_H] = { "h", NULL, "H" },
	[KEYD_J] = { "j", NULL, "J" },
	[KEYD_K] = { "k", NULL, "K" },
	[KEYD_L] = { "l", NULL, "L" },
	[KEYD_SEMICOLON] = { ";", "semicolon", ":" },
	[KEYD_APOSTROPHE] = { "'", "apostrophe", "\"" },
	[KEYD_GRAVE] = { "`", "grave", "~" },
	[KEYD_LEFTSHIFT] = { "leftshift", "", NULL },
	[KEYD_BACKSLASH] = { "\\", "backslash", "|" },
	[KEYD_Z] = { "z", NULL, "Z" },
	[KEYD_X] = { "x", NULL, "X" },
	[KEYD_C] = { "c", NULL, "C" },
	[KEYD_V] = { "v", NULL, "V" },
	[KEYD_B] = { "b", NULL, "B" },
	[KEYD_N] = { "n", NULL, "N" },
	[KEYD_M] = { "m", NULL, "M" },
	[KEYD_COMMA] = { ",", "comma", "<" },
	[KEYD_DOT] = { ".", "dot", ">" },
	[KEYD_SLASH] = { "/", "slash", "?" },
	[KEYD_RIGHTSHIFT] = { "rightshift", NULL, NULL },
	[KEYD_KPASTERISK] = { "kpasterisk", NULL, NULL },
	[KEYD_LEFTALT] = { "leftalt", "", NULL },
	[KEYD_SPACE] = { "space", NULL, NULL },
	[KEYD_CAPSLOCK] = { "capslock", NULL, NULL },
	[KEYD_F1] = { "f1", NULL, NULL },
	[KEYD_F2] = { "f2", NULL, NULL },
	[KEYD_F3] = { "f3", NULL, NULL },
	[KEYD_F4] = { "f4", NULL, NULL },
	[KEYD_F5] = { "f5", NULL, NULL },
	[KEYD_F6] = { "f6", NULL, NULL },
	[KEYD_F7] = { "f7", NULL, NULL },
	[KEYD_F8] = { "f8", NULL, NULL },
	[KEYD_F9] = { "f9", NULL, NULL },
	[KEYD_F10] = { "f10", NULL, NULL },
	[KEYD_NUMLOCK] = { "numlock", NULL, NULL },
	[KEYD_SCROLLLOCK] = { "scrolllock", NULL, NULL },
	[KEYD_KP7] = { "kp7", NULL, NULL },
	[KEYD_KP8] = { "kp8", NULL, NULL },
	[KEYD_KP9] = { "kp9", NULL, NULL },
	[KEYD_KPMINUS] = { "kpminus", NULL, NULL },
	[KEYD_KP4] = { "kp4", NULL, NULL },
	[KEYD_KP5] = { "kp5", NULL, NULL },
	[KEYD_KP6] = { "kp6", NULL, NULL },
	[KEYD_KPPLUS] = { "kpplus", NULL, NULL },
	[KEYD_KP1] = { "kp1", NULL, NULL },
	[KEYD_KP2] = { "kp2", NULL, NULL },
	[KEYD_KP3] = { "kp3", NULL, NULL },
	[KEYD_KP0] = { "kp0", NULL, NULL },
	[KEYD_KPDOT] = { "kpdot", NULL, NULL },
	[KEYD_ZENKAKUHANKAKU] = { "zenkakuhankaku", NULL, NULL },
	[KEYD_102ND] = { "102nd", NULL, NULL },
	[KEYD_F11] = { "f11", NULL, NULL },
	[KEYD_F12] = { "f12", NULL, NULL },
	[KEYD_RO] = { "ro", NULL, NULL },
	[KEYD_KATAKANA] = { "katakana", NULL, NULL },
	[KEYD_HIRAGANA] = { "hiragana", NULL, NULL },
	[KEYD_HENKAN] = { "henkan", NULL, NULL },
	[KEYD_KATAKANAHIRAGANA] = { "katakanahiragana", NULL, NULL },
	[KEYD_MUHENKAN] = { "muhenkan", NULL, NULL },
	[KEYD_KPJPCOMMA] = { "kpjpcomma", NULL, NULL },
	[KEYD_KPENTER] = { "kpenter", NULL, NULL },
	[KEYD_RIGHTCTRL] = { "rightcontrol", NULL, NULL },
	[KEYD_KPSLASH] = { "kpslash", NULL, NULL },
	[KEYD_SYSRQ] = { "sysrq", NULL, NULL },
	[KEYD_RIGHTALT] = { "rightalt", NULL, NULL },
	[KEYD_LINEFEED] = { "linefeed", NULL, NULL },
	[KEYD_HOME] = { "home", NULL, NULL },
	[KEYD_UP] = { "up", NULL, NULL },
	[KEYD_PAGEUP] = { "pageup", NULL, NULL },
	[KEYD_LEFT] = { "left", NULL, NULL },
	[KEYD_RIGHT] = { "right", NULL, NULL },
	[KEYD_END] = { "end", NULL, NULL },
	[KEYD_DOWN] = { "down", NULL, NULL },
	[KEYD_PAGEDOWN] = { "pagedown", NULL, NULL },
	[KEYD_INSERT] = { "insert", NULL, NULL },
	[KEYD_DELETE] = { "delete", NULL, NULL },
	[KEYD_MACRO] = { "macro", NULL, NULL },
	[KEYD_MUTE] = { "mute", NULL, NULL },
	[KEYD_VOLUMEDOWN] = { "volumedown", NULL, NULL },
	[KEYD_VOLUMEUP] = { "volumeup", NULL, NULL },
	[KEYD_POWER] = { "power", NULL, NULL },
	[KEYD_KPEQUAL] = { "kpequal", NULL, NULL },
	[KEYD_KPPLUSMINUS] = { "kpplusminus", NULL, NULL },
	[KEYD_PAUSE] = { "pause", NULL, NULL },
	[KEYD_SCALE] = { "scale", NULL, NULL },
	[KEYD_KPCOMMA] = { "kpcomma", NULL, NULL },
	[KEYD_HANGEUL] = { "hangeul", NULL, NULL },
	[KEYD_HANJA] = { "hanja", NULL, NULL },
	[KEYD_YEN] = { "yen", NULL, NULL },
	[KEYD_LEFTMETA] = { "leftmeta", "", NULL },
	[KEYD_RIGHTMETA] = { "rightmeta", NULL, NULL },
	[KEYD_COMPOSE] = { "compose", NULL, NULL },
	[KEYD_STOP] = { "stop", NULL, NULL },
	[KEYD_AGAIN] = { "again", NULL, NULL },
	[KEYD_PROPS] = { "props", NULL, NULL },
	[KEYD_UNDO] = { "undo", NULL, NULL },
	[KEYD_FRONT] = { "front", NULL, NULL },
	[KEYD_COPY] = { "copy", NULL, NULL },
	[KEYD_OPEN] = { "open", NULL, NULL },
	[KEYD_PASTE] = { "paste", NULL, NULL },
	[KEYD_FIND] = { "find", NULL, NULL },
	[KEYD_CUT] = { "cut", NULL, NULL },
	[KEYD_HELP] = { "help", NULL, NULL },
	[KEYD_MENU] = { "menu", NULL, NULL },
	[KEYD_CALC] = { "calc", NULL, NULL },
	[KEYD_SETUP] = { "setup", NULL, NULL },
	[KEYD_SLEEP] = { "sleep", NULL, NULL },
	[KEYD_WAKEUP] = { "wakeup", NULL, NULL },
	[KEYD_FILE] = { "file", NULL, NULL },
	[KEYD_SENDFILE] = { "sendfile", NULL, NULL },
	[KEYD_DELETEFILE] = { "deletefile", NULL, NULL },
	[KEYD_XFER] = { "xfer", NULL, NULL },
	[KEYD_PROG1] = { "prog1", NULL, NULL },
	[KEYD_PROG2] = { "prog2", NULL, NULL },
	[KEYD_WWW] = { "www", NULL, NULL },
	[KEYD_MSDOS] = { "msdos", NULL, NULL },
	[KEYD_COFFEE] = { "coffee", NULL, NULL },
	[KEYD_ROTATE_DISPLAY] = { "display", NULL, NULL },
	[KEYD_CYCLEWINDOWS] = { "cyclewindows", NULL, NULL },
	[KEYD_MAIL] = { "mail", NULL, NULL },
	[KEYD_BOOKMARKS] = { "bookmarks", NULL, NULL },
	[KEYD_COMPUTER] = { "computer", NULL, NULL },
	[KEYD_BACK] = { "back", NULL, NULL },
	[KEYD_FORWARD] = { "forward", NULL, NULL },
	[KEYD_CLOSECD] = { "closecd", NULL, NULL },
	[KEYD_EJECTCD] = { "ejectcd", NULL, NULL },
	[KEYD_EJECTCLOSECD] = { "ejectclosecd", NULL, NULL },
	[KEYD_NEXTSONG] = { "nextsong", NULL, NULL },
	[KEYD_PLAYPAUSE] = { "playpause", NULL, NULL },
	[KEYD_PREVIOUSSONG] = { "previoussong", NULL, NULL },
	[KEYD_STOPCD] = { "stopcd", NULL, NULL },
	[KEYD_RECORD] = { "record", NULL, NULL },
	[KEYD_REWIND] = { "rewind", NULL, NULL },
	[KEYD_PHONE] = { "phone", NULL, NULL },
	[KEYD_ISO] = { "iso", NULL, NULL },
	[KEYD_CONFIG] = { "config", NULL, NULL },
	[KEYD_HOMEPAGE] = { "homepage", NULL, NULL },
	[KEYD_REFRESH] = { "refresh", NULL, NULL },
	[KEYD_EXIT] = { "exit", NULL, NULL },
	[KEYD_MOVE] = { "move", NULL, NULL },
	[KEYD_EDIT] = { "edit", NULL, NULL },
	[KEYD_KPLEFTPAREN] = { "kpleftparen", NULL, NULL },
	[KEYD_KPRIGHTPAREN] = { "kprightparen", NULL, NULL },
	[KEYD_NEW] = { "new", NULL, NULL },
	[KEYD_REDO] = { "redo", NULL, NULL },
	[KEYD_F13] = { "f13", NULL, NULL },
	[KEYD_F14] = { "f14", NULL, NULL },
	[KEYD_F15] = { "f15", NULL, NULL },
	[KEYD_F16] = { "f16", NULL, NULL },
	[KEYD_F17] = { "f17", NULL, NULL },
	[KEYD_F18] = { "f18", NULL, NULL },
	[KEYD_F19] = { "f19", NULL, NULL },
	[KEYD_F20] = { "f20", NULL, NULL },
	[KEYD_F21] = { "f21", NULL, NULL },
	[KEYD_F22] = { "f22", NULL, NULL },
	[KEYD_F23] = { "f23", NULL, NULL },
	[KEYD_F24] = { "f24", NULL, NULL },
	[KEYD_PLAYCD] = { "playcd", NULL, NULL },
	[KEYD_PAUSECD] = { "pausecd", NULL, NULL },
	[KEYD_PROG3] = { "prog3", NULL, NULL },
	[KEYD_PROG4] = { "prog4", NULL, NULL },
	[KEYD_DASHBOARD] = { "dashboard", NULL, NULL },
	[KEYD_SUSPEND] = { "suspend", NULL, NULL },
	[KEYD_CLOSE] = { "close", NULL, NULL },
	[KEYD_PLAY] = { "play", NULL, NULL },
	[KEYD_FASTFORWARD] = { "fastforward", NULL, NULL },
	[KEYD_BASSBOOST] = { "bassboost", NULL, NULL },
	[KEYD_PRINT] = { "print", NULL, NULL },
	[KEYD_HP] = { "hp", NULL, NULL },
	[KEYD_CAMERA] = { "camera", NULL, NULL },
	[KEYD_SOUND] = { "sound", NULL, NULL },
	[KEYD_QUESTION] = { "question", NULL, NULL },
	[KEYD_EMAIL] = { "email", NULL, NULL },
	[KEYD_CHAT] = { "chat", NULL, NULL },
	[KEYD_SEARCH] = { "search", NULL, NULL },
	[KEYD_CONNECT] = { "connect", NULL, NULL },
	[KEYD_FINANCE] = { "finance", NULL, NULL },
	[KEYD_SCROLL_UP] = { "scrollup", NULL, NULL },
    [KEYD_SCROLL_DOWN] = { "scrolldown", NULL, NULL },
	[KEYD_VOICECOMMAND] = { "voicecommand", NULL, NULL },
	[KEYD_CANCEL] = { "cancel", NULL, NULL },
	[KEYD_BRIGHTNESSDOWN] = { "brightnessdown", NULL, NULL },
	[KEYD_BRIGHTNESSUP] = { "brightnessup", NULL, NULL },
	[KEYD_MEDIA] = { "media", NULL, NULL },
	[KEYD_SWITCHVIDEOMODE] = { "switchvideomode", NULL, NULL },
	[KEYD_KBDILLUMTOGGLE] = { "kbdillumtoggle", NULL, NULL },
	[KEYD_KBDILLUMDOWN] = { "kbdillumdown", NULL, NULL },
	[KEYD_KBDILLUMUP] = { "kbdillumup", NULL, NULL },
	[KEYD_SEND] = { "send", NULL, NULL },
	[KEYD_REPLY] = { "reply", NULL, NULL },
	[KEYD_FORWARDMAIL] = { "forwardmail", NULL, NULL },
	[KEYD_SAVE] = { "save", NULL, NULL },
	[KEYD_DOCUMENTS] = { "documents", NULL, NULL },
	[KEYD_BATTERY] = { "battery", NULL, NULL },
	[KEYD_BLUETOOTH] = { "bluetooth", NULL, NULL },
	[KEYD_WLAN] = { "wlan", NULL, NULL },
	[KEYD_UWB] = { "uwb", NULL, NULL },
	[KEYD_UNKNOWN] = { "unknown", NULL, NULL },
	[KEYD_VIDEO_NEXT] = { "next", NULL, NULL },
	[KEYD_VIDEO_PREV] = { "prev", NULL, NULL },
	[KEYD_BRIGHTNESS_CYCLE] = { "cycle", NULL, NULL },
	[KEYD_BRIGHTNESS_AUTO] = { "auto", NULL, NULL },
	[KEYD_DISPLAY_OFF] = { "off", NULL, NULL },
	[KEYD_WWAN] = { "wwan", NULL, NULL },
	[KEYD_RFKILL] = { "rfkill", NULL, NULL },
	[KEYD_MICMUTE] = { "micmute", NULL, NULL },
	[KEYD_LEFT_MOUSE] = { "leftmouse", NULL, NULL },
	[KEYD_RIGHT_MOUSE] = { "rightmouse", NULL, NULL },
	[KEYD_MIDDLE_MOUSE] = { "middlemouse", NULL, NULL },
	[KEYD_MOUSE_1] = { "mouse1", NULL, NULL },
	[KEYD_MOUSE_2] = { "mouse2", NULL, NULL },
	[KEYD_MOUSE_BACK] = { "mouseback", NULL, NULL },
	[KEYD_MOUSE_FORWARD] = { "mouseforward", NULL, NULL },
	[KEYD_FN] = { "fn", NULL, NULL },
	[KEYD_ZOOM] = { "zoom", NULL, NULL },
	[KEYD_NOOP] = { "noop", NULL, NULL },
};

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

int parse_key_sequence(const char *s, uint8_t *codep, uint8_t *modsp)
{
	const char *c = s;
	size_t i;

	if (!*s)
		return -1;

	uint8_t mods = 0;

	while (c[1] == '-') {
		switch (*c) {
		case 'C':
			mods |= MOD_CTRL;
			break;
		case 'M':
			mods |= MOD_SUPER;
			break;
		case 'A':
			mods |= MOD_ALT;
			break;
		case 'S':
			mods |= MOD_SHIFT;
			break;
		case 'G':
			mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		c += 2;
	}

	for (i = 0; i < 256; i++) {
		const struct keycode_table_ent *ent = &keycode_table[i];

		if (ent->name) {
			if (ent->shifted_name &&
			    !strcmp(ent->shifted_name, c)) {

				mods |= MOD_SHIFT;

				if (modsp)
					*modsp = mods;

				if (codep)
					*codep = i;

				return 0;
			} else if (!strcmp(ent->name, c) ||
				   (ent->alt_name && !strcmp(ent->alt_name, c))) {

				if (modsp)
					*modsp = mods;

				if (codep)
					*codep = i;

				return 0;
			}
		}
	}

	return -1;
}

