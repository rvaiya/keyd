/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef _KEYS_H_
#define _KEYS_H_
#define _KEYS_H_

#include <stdint.h>
#include <stdlib.h>

#define MOD_ALT_GR	0x10
#define MOD_CTRL	0x8
#define MOD_SHIFT	0x4
#define MOD_SUPER	0x2
#define MOD_ALT		0x1

#define MAX_MOD 5

struct keycode_table_ent {
	const char *name;
	const char *alt_name;
	const char *shifted_name;
};

struct modifier {
	uint8_t mask;
	uint8_t key;
};

#define KEYD_ESC               1
#define KEYD_1                 2
#define KEYD_2                 3
#define KEYD_3                 4
#define KEYD_4                 5
#define KEYD_5                 6
#define KEYD_6                 7
#define KEYD_7                 8
#define KEYD_8                 9
#define KEYD_9                 10
#define KEYD_0                 11
#define KEYD_MINUS             12
#define KEYD_EQUAL             13
#define KEYD_BACKSPACE         14
#define KEYD_TAB               15
#define KEYD_Q                 16
#define KEYD_W                 17
#define KEYD_E                 18
#define KEYD_R                 19
#define KEYD_T                 20
#define KEYD_Y                 21
#define KEYD_U                 22
#define KEYD_I                 23
#define KEYD_O                 24
#define KEYD_P                 25
#define KEYD_LEFTBRACE         26
#define KEYD_RIGHTBRACE        27
#define KEYD_ENTER             28
#define KEYD_LEFTCTRL          29
#define KEYD_A                 30
#define KEYD_S                 31
#define KEYD_D                 32
#define KEYD_F                 33
#define KEYD_G                 34
#define KEYD_H                 35
#define KEYD_J                 36
#define KEYD_K                 37
#define KEYD_L                 38
#define KEYD_SEMICOLON         39
#define KEYD_APOSTROPHE        40
#define KEYD_GRAVE             41
#define KEYD_LEFTSHIFT         42
#define KEYD_BACKSLASH         43
#define KEYD_Z                 44
#define KEYD_X                 45
#define KEYD_C                 46
#define KEYD_V                 47
#define KEYD_B                 48
#define KEYD_N                 49
#define KEYD_M                 50
#define KEYD_COMMA             51
#define KEYD_DOT               52
#define KEYD_SLASH             53
#define KEYD_RIGHTSHIFT        54
#define KEYD_KPASTERISK        55
#define KEYD_LEFTALT           56
#define KEYD_SPACE             57
#define KEYD_CAPSLOCK          58
#define KEYD_F1                59
#define KEYD_F2                60
#define KEYD_F3                61
#define KEYD_F4                62
#define KEYD_F5                63
#define KEYD_F6                64
#define KEYD_F7                65
#define KEYD_F8                66
#define KEYD_F9                67
#define KEYD_F10               68
#define KEYD_NUMLOCK           69
#define KEYD_SCROLLLOCK        70
#define KEYD_KP7               71
#define KEYD_KP8               72
#define KEYD_KP9               73
#define KEYD_KPMINUS           74
#define KEYD_KP4               75
#define KEYD_KP5               76
#define KEYD_KP6               77
#define KEYD_KPPLUS            78
#define KEYD_KP1               79
#define KEYD_KP2               80
#define KEYD_KP3               81
#define KEYD_KP0               82
#define KEYD_KPDOT             83
#define KEYD_IS_LEVEL3_SHIFT   84
#define KEYD_ZENKAKUHANKAKU    85
#define KEYD_102ND             86
#define KEYD_F11               87
#define KEYD_F12               88
#define KEYD_RO                89
#define KEYD_KATAKANA          90
#define KEYD_HIRAGANA          91
#define KEYD_HENKAN            92
#define KEYD_KATAKANAHIRAGANA  93
#define KEYD_MUHENKAN          94
#define KEYD_KPJPCOMMA         95
#define KEYD_KPENTER           96
#define KEYD_RIGHTCTRL         97
#define KEYD_KPSLASH           98
#define KEYD_SYSRQ             99
#define KEYD_RIGHTALT          100
#define KEYD_LINEFEED          101
#define KEYD_HOME              102
#define KEYD_UP                103
#define KEYD_PAGEUP            104
#define KEYD_LEFT              105
#define KEYD_RIGHT             106
#define KEYD_END               107
#define KEYD_DOWN              108
#define KEYD_PAGEDOWN          109
#define KEYD_INSERT            110
#define KEYD_DELETE            111
#define KEYD_MACRO             112
#define KEYD_MUTE              113
#define KEYD_VOLUMEDOWN        114
#define KEYD_VOLUMEUP          115
#define KEYD_POWER             116
#define KEYD_KPEQUAL           117
#define KEYD_KPPLUSMINUS       118
#define KEYD_PAUSE             119
#define KEYD_SCALE             120
#define KEYD_KPCOMMA           121
#define KEYD_HANGEUL           122
#define KEYD_HANJA             123
#define KEYD_YEN               124
#define KEYD_LEFTMETA          125
#define KEYD_RIGHTMETA         126
#define KEYD_COMPOSE           127
#define KEYD_STOP              128
#define KEYD_AGAIN             129
#define KEYD_PROPS             130
#define KEYD_UNDO              131
#define KEYD_FRONT             132
#define KEYD_COPY              133
#define KEYD_OPEN              134
#define KEYD_PASTE             135
#define KEYD_FIND              136
#define KEYD_CUT               137
#define KEYD_HELP              138
#define KEYD_MENU              139
#define KEYD_CALC              140
#define KEYD_SETUP             141
#define KEYD_SLEEP             142
#define KEYD_WAKEUP            143
#define KEYD_FILE              144
#define KEYD_SENDFILE          145
#define KEYD_DELETEFILE        146
#define KEYD_XFER              147
#define KEYD_PROG1             148
#define KEYD_PROG2             149
#define KEYD_WWW               150
#define KEYD_MSDOS             151
#define KEYD_COFFEE            152
#define KEYD_ROTATE_DISPLAY    153
#define KEYD_CYCLEWINDOWS      154
#define KEYD_MAIL              155
#define KEYD_BOOKMARKS         156
#define KEYD_COMPUTER          157
#define KEYD_BACK              158
#define KEYD_FORWARD           159
#define KEYD_CLOSECD           160
#define KEYD_EJECTCD           161
#define KEYD_EJECTCLOSECD      162
#define KEYD_NEXTSONG          163
#define KEYD_PLAYPAUSE         164
#define KEYD_PREVIOUSSONG      165
#define KEYD_STOPCD            166
#define KEYD_RECORD            167
#define KEYD_REWIND            168
#define KEYD_PHONE             169
#define KEYD_ISO               170
#define KEYD_CONFIG            171
#define KEYD_HOMEPAGE          172
#define KEYD_REFRESH           173
#define KEYD_EXIT              174
#define KEYD_MOVE              175
#define KEYD_EDIT              176
#define KEYD_ZOOM              177
#define KEYD_KPLEFTPAREN       179
#define KEYD_KPRIGHTPAREN      180
#define KEYD_NEW               181
#define KEYD_REDO              182
#define KEYD_F13               183
#define KEYD_F14               184
#define KEYD_F15               185
#define KEYD_F16               186
#define KEYD_F17               187
#define KEYD_F18               188
#define KEYD_F19               189
#define KEYD_F20               190
#define KEYD_F21               191
#define KEYD_F22               192
#define KEYD_F23               193
#define KEYD_F24               194
#define KEYD_PLAYCD            200
#define KEYD_PAUSECD           201
#define KEYD_PROG3             202
#define KEYD_PROG4             203
#define KEYD_DASHBOARD         204
#define KEYD_SUSPEND           205
#define KEYD_CLOSE             206
#define KEYD_PLAY              207
#define KEYD_FASTFORWARD       208
#define KEYD_BASSBOOST         209
#define KEYD_PRINT             210
#define KEYD_HP                211
#define KEYD_CAMERA            212
#define KEYD_SOUND             213
#define KEYD_QUESTION          214
#define KEYD_EMAIL             215
#define KEYD_CHAT              216
#define KEYD_SEARCH            217
#define KEYD_CONNECT           218
#define KEYD_FINANCE           219
#define KEYD_VOICECOMMAND      222
#define KEYD_CANCEL            223
#define KEYD_BRIGHTNESSDOWN    224
#define KEYD_BRIGHTNESSUP      225
#define KEYD_MEDIA             226
#define KEYD_SWITCHVIDEOMODE   227
#define KEYD_KBDILLUMTOGGLE    228
#define KEYD_KBDILLUMDOWN      229
#define KEYD_KBDILLUMUP        230
#define KEYD_SEND              231
#define KEYD_REPLY             232
#define KEYD_FORWARDMAIL       233
#define KEYD_SAVE              234
#define KEYD_DOCUMENTS         235
#define KEYD_BATTERY           236
#define KEYD_BLUETOOTH         237
#define KEYD_WLAN              238
#define KEYD_UWB               239
#define KEYD_UNKNOWN           240
#define KEYD_VIDEO_NEXT        241
#define KEYD_VIDEO_PREV        242
#define KEYD_BRIGHTNESS_CYCLE  243
#define KEYD_BRIGHTNESS_AUTO   244
#define KEYD_DISPLAY_OFF       245
#define KEYD_WWAN              246
#define KEYD_RFKILL            247
#define KEYD_MICMUTE           248

/* These deviate from uinput codes. */

#define  KEYD_NOOP             		195
#define  KEYD_EXTERNAL_MOUSE_BUTTON     196
#define  KEYD_CHORD_1			197
#define  KEYD_CHORD_2			198
#define  KEYD_CHORD_MAX			199
#define  KEYD_LEFT_MOUSE       		249
#define  KEYD_MIDDLE_MOUSE     		250
#define  KEYD_RIGHT_MOUSE		251
#define  KEYD_MOUSE_1          		252
#define  KEYD_MOUSE_2          		253
#define  KEYD_MOUSE_BACK       		178
#define  KEYD_SCROLL_UP             220
#define  KEYD_SCROLL_DOWN           221
#define  KEYD_FN               		254
#define  KEYD_MOUSE_FORWARD    		255

#define KEY_NAME(code) (keycode_table[code].name ? keycode_table[code].name : "UNKNOWN")

int parse_modset(const char *s, uint8_t *mods);
int parse_key_sequence(const char *s, uint8_t *code, uint8_t *mods);

extern const struct modifier modifiers[MAX_MOD];
extern const struct keycode_table_ent keycode_table[256];

#endif
