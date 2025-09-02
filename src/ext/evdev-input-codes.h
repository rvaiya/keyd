#ifndef EVDEV_INPUT_H
#define EVDEV_INPUT_H

#undef INPUT_PROP_POINTER
#undef INPUT_PROP_DIRECT
#undef INPUT_PROP_BUTTONPAD
#undef INPUT_PROP_SEMI_MT
#undef INPUT_PROP_TOPBUTTONPAD
#undef INPUT_PROP_POINTING_STICK
#undef INPUT_PROP_ACCELEROMETER
#undef INPUT_PROP_MAX
#undef INPUT_PROP_CNT
#undef EV_SYN
#undef EV_KEY
#undef EV_REL
#undef EV_ABS
#undef EV_MSC
#undef EV_SW
#undef EV_LED
#undef EV_SND
#undef EV_REP
#undef EV_FF
#undef EV_PWR
#undef EV_FF_STATUS
#undef EV_MAX
#undef EV_CNT
#undef SYN_REPORT
#undef SYN_CONFIG
#undef SYN_MT_REPORT
#undef SYN_DROPPED
#undef SYN_MAX
#undef SYN_CNT
#undef KEY_RESERVED
#undef KEY_ESC
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9
#undef KEY_0
#undef KEY_MINUS
#undef KEY_EQUAL
#undef KEY_BACKSPACE
#undef KEY_TAB
#undef KEY_Q
#undef KEY_W
#undef KEY_E
#undef KEY_R
#undef KEY_T
#undef KEY_Y
#undef KEY_U
#undef KEY_I
#undef KEY_O
#undef KEY_P
#undef KEY_LEFTBRACE
#undef KEY_RIGHTBRACE
#undef KEY_ENTER
#undef KEY_LEFTCTRL
#undef KEY_A
#undef KEY_S
#undef KEY_D
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_SEMICOLON
#undef KEY_APOSTROPHE
#undef KEY_GRAVE
#undef KEY_LEFTSHIFT
#undef KEY_BACKSLASH
#undef KEY_Z
#undef KEY_X
#undef KEY_C
#undef KEY_V
#undef KEY_B
#undef KEY_N
#undef KEY_M
#undef KEY_COMMA
#undef KEY_DOT
#undef KEY_SLASH
#undef KEY_RIGHTSHIFT
#undef KEY_KPASTERISK
#undef KEY_LEFTALT
#undef KEY_SPACE
#undef KEY_CAPSLOCK
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_NUMLOCK
#undef KEY_SCROLLLOCK
#undef KEY_KP7
#undef KEY_KP8
#undef KEY_KP9
#undef KEY_KPMINUS
#undef KEY_KP4
#undef KEY_KP5
#undef KEY_KP6
#undef KEY_KPPLUS
#undef KEY_KP1
#undef KEY_KP2
#undef KEY_KP3
#undef KEY_KP0
#undef KEY_KPDOT
#undef KEY_ZENKAKUHANKAKU
#undef KEY_102ND
#undef KEY_F11
#undef KEY_F12
#undef KEY_RO
#undef KEY_KATAKANA
#undef KEY_HIRAGANA
#undef KEY_HENKAN
#undef KEY_KATAKANAHIRAGANA
#undef KEY_MUHENKAN
#undef KEY_KPJPCOMMA
#undef KEY_KPENTER
#undef KEY_RIGHTCTRL
#undef KEY_KPSLASH
#undef KEY_SYSRQ
#undef KEY_RIGHTALT
#undef KEY_LINEFEED
#undef KEY_HOME
#undef KEY_UP
#undef KEY_PAGEUP
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_END
#undef KEY_DOWN
#undef KEY_PAGEDOWN
#undef KEY_INSERT
#undef KEY_DELETE
#undef KEY_MACRO
#undef KEY_MUTE
#undef KEY_VOLUMEDOWN
#undef KEY_VOLUMEUP
#undef KEY_POWER
#undef KEY_KPEQUAL
#undef KEY_KPPLUSMINUS
#undef KEY_PAUSE
#undef KEY_SCALE
#undef KEY_KPCOMMA
#undef KEY_HANGEUL
#undef KEY_HANGUEL
#undef KEY_HANJA
#undef KEY_YEN
#undef KEY_LEFTMETA
#undef KEY_RIGHTMETA
#undef KEY_COMPOSE
#undef KEY_STOP
#undef KEY_AGAIN
#undef KEY_PROPS
#undef KEY_UNDO
#undef KEY_FRONT
#undef KEY_COPY
#undef KEY_OPEN
#undef KEY_PASTE
#undef KEY_FIND
#undef KEY_CUT
#undef KEY_HELP
#undef KEY_MENU
#undef KEY_CALC
#undef KEY_SETUP
#undef KEY_SLEEP
#undef KEY_WAKEUP
#undef KEY_FILE
#undef KEY_SENDFILE
#undef KEY_DELETEFILE
#undef KEY_XFER
#undef KEY_PROG1
#undef KEY_PROG2
#undef KEY_WWW
#undef KEY_MSDOS
#undef KEY_COFFEE
#undef KEY_SCREENLOCK
#undef KEY_ROTATE_DISPLAY
#undef KEY_DIRECTION
#undef KEY_CYCLEWINDOWS
#undef KEY_MAIL
#undef KEY_BOOKMARKS
#undef KEY_COMPUTER
#undef KEY_BACK
#undef KEY_FORWARD
#undef KEY_CLOSECD
#undef KEY_EJECTCD
#undef KEY_EJECTCLOSECD
#undef KEY_NEXTSONG
#undef KEY_PLAYPAUSE
#undef KEY_PREVIOUSSONG
#undef KEY_STOPCD
#undef KEY_RECORD
#undef KEY_REWIND
#undef KEY_PHONE
#undef KEY_ISO
#undef KEY_CONFIG
#undef KEY_HOMEPAGE
#undef KEY_REFRESH
#undef KEY_EXIT
#undef KEY_MOVE
#undef KEY_EDIT
#undef KEY_SCROLLUP
#undef KEY_SCROLLDOWN
#undef KEY_KPLEFTPAREN
#undef KEY_KPRIGHTPAREN
#undef KEY_NEW
#undef KEY_REDO
#undef KEY_F13
#undef KEY_F14
#undef KEY_F15
#undef KEY_F16
#undef KEY_F17
#undef KEY_F18
#undef KEY_F19
#undef KEY_F20
#undef KEY_F21
#undef KEY_F22
#undef KEY_F23
#undef KEY_F24
#undef KEY_PLAYCD
#undef KEY_PAUSECD
#undef KEY_PROG3
#undef KEY_PROG4
#undef KEY_ALL_APPLICATIONS
#undef KEY_DASHBOARD
#undef KEY_SUSPEND
#undef KEY_CLOSE
#undef KEY_PLAY
#undef KEY_FASTFORWARD
#undef KEY_BASSBOOST
#undef KEY_PRINT
#undef KEY_HP
#undef KEY_CAMERA
#undef KEY_SOUND
#undef KEY_QUESTION
#undef KEY_EMAIL
#undef KEY_CHAT
#undef KEY_SEARCH
#undef KEY_CONNECT
#undef KEY_FINANCE
#undef KEY_SPORT
#undef KEY_SHOP
#undef KEY_ALTERASE
#undef KEY_CANCEL
#undef KEY_BRIGHTNESSDOWN
#undef KEY_BRIGHTNESSUP
#undef KEY_MEDIA
#undef KEY_SWITCHVIDEOMODE
#undef KEY_KBDILLUMTOGGLE
#undef KEY_KBDILLUMDOWN
#undef KEY_KBDILLUMUP
#undef KEY_SEND
#undef KEY_REPLY
#undef KEY_FORWARDMAIL
#undef KEY_SAVE
#undef KEY_DOCUMENTS
#undef KEY_BATTERY
#undef KEY_BLUETOOTH
#undef KEY_WLAN
#undef KEY_UWB
#undef KEY_UNKNOWN
#undef KEY_VIDEO_NEXT
#undef KEY_VIDEO_PREV
#undef KEY_BRIGHTNESS_CYCLE
#undef KEY_BRIGHTNESS_AUTO
#undef KEY_BRIGHTNESS_ZERO
#undef KEY_DISPLAY_OFF
#undef KEY_WWAN
#undef KEY_WIMAX
#undef KEY_RFKILL
#undef KEY_MICMUTE
#undef BTN_MISC
#undef BTN_0
#undef BTN_1
#undef BTN_2
#undef BTN_3
#undef BTN_4
#undef BTN_5
#undef BTN_6
#undef BTN_7
#undef BTN_8
#undef BTN_9
#undef BTN_MOUSE
#undef BTN_LEFT
#undef BTN_RIGHT
#undef BTN_MIDDLE
#undef BTN_SIDE
#undef BTN_EXTRA
#undef BTN_FORWARD
#undef BTN_BACK
#undef BTN_TASK
#undef BTN_JOYSTICK
#undef BTN_TRIGGER
#undef BTN_THUMB
#undef BTN_THUMB2
#undef BTN_TOP
#undef BTN_TOP2
#undef BTN_PINKIE
#undef BTN_BASE
#undef BTN_BASE2
#undef BTN_BASE3
#undef BTN_BASE4
#undef BTN_BASE5
#undef BTN_BASE6
#undef BTN_DEAD
#undef BTN_GAMEPAD
#undef BTN_SOUTH
#undef BTN_A
#undef BTN_EAST
#undef BTN_B
#undef BTN_C
#undef BTN_NORTH
#undef BTN_X
#undef BTN_WEST
#undef BTN_Y
#undef BTN_Z
#undef BTN_TL
#undef BTN_TR
#undef BTN_TL2
#undef BTN_TR2
#undef BTN_SELECT
#undef BTN_START
#undef BTN_MODE
#undef BTN_THUMBL
#undef BTN_THUMBR
#undef BTN_DIGI
#undef BTN_TOOL_PEN
#undef BTN_TOOL_RUBBER
#undef BTN_TOOL_BRUSH
#undef BTN_TOOL_PENCIL
#undef BTN_TOOL_AIRBRUSH
#undef BTN_TOOL_FINGER
#undef BTN_TOOL_MOUSE
#undef BTN_TOOL_LENS
#undef BTN_TOOL_QUINTTAP
#undef BTN_STYLUS3
#undef BTN_TOUCH
#undef BTN_STYLUS
#undef BTN_STYLUS2
#undef BTN_TOOL_DOUBLETAP
#undef BTN_TOOL_TRIPLETAP
#undef BTN_TOOL_QUADTAP
#undef BTN_WHEEL
#undef BTN_GEAR_DOWN
#undef BTN_GEAR_UP
#undef KEY_OK
#undef KEY_SELECT
#undef KEY_GOTO
#undef KEY_CLEAR
#undef KEY_POWER2
#undef KEY_OPTION
#undef KEY_INFO
#undef KEY_TIME
#undef KEY_VENDOR
#undef KEY_ARCHIVE
#undef KEY_PROGRAM
#undef KEY_CHANNEL
#undef KEY_FAVORITES
#undef KEY_EPG
#undef KEY_PVR
#undef KEY_MHP
#undef KEY_LANGUAGE
#undef KEY_TITLE
#undef KEY_SUBTITLE
#undef KEY_ANGLE
#undef KEY_FULL_SCREEN
#undef KEY_ZOOM
#undef KEY_MODE
#undef KEY_KEYBOARD
#undef KEY_ASPECT_RATIO
#undef KEY_SCREEN
#undef KEY_PC
#undef KEY_TV
#undef KEY_TV2
#undef KEY_VCR
#undef KEY_VCR2
#undef KEY_SAT
#undef KEY_SAT2
#undef KEY_CD
#undef KEY_TAPE
#undef KEY_RADIO
#undef KEY_TUNER
#undef KEY_PLAYER
#undef KEY_TEXT
#undef KEY_DVD
#undef KEY_AUX
#undef KEY_MP3
#undef KEY_AUDIO
#undef KEY_VIDEO
#undef KEY_DIRECTORY
#undef KEY_LIST
#undef KEY_MEMO
#undef KEY_CALENDAR
#undef KEY_RED
#undef KEY_GREEN
#undef KEY_YELLOW
#undef KEY_BLUE
#undef KEY_CHANNELUP
#undef KEY_CHANNELDOWN
#undef KEY_FIRST
#undef KEY_LAST
#undef KEY_AB
#undef KEY_NEXT
#undef KEY_RESTART
#undef KEY_SLOW
#undef KEY_SHUFFLE
#undef KEY_BREAK
#undef KEY_PREVIOUS
#undef KEY_DIGITS
#undef KEY_TEEN
#undef KEY_TWEN
#undef KEY_VIDEOPHONE
#undef KEY_GAMES
#undef KEY_ZOOMIN
#undef KEY_ZOOMOUT
#undef KEY_ZOOMRESET
#undef KEY_WORDPROCESSOR
#undef KEY_EDITOR
#undef KEY_SPREADSHEET
#undef KEY_GRAPHICSEDITOR
#undef KEY_PRESENTATION
#undef KEY_DATABASE
#undef KEY_NEWS
#undef KEY_VOICEMAIL
#undef KEY_ADDRESSBOOK
#undef KEY_MESSENGER
#undef KEY_DISPLAYTOGGLE
#undef KEY_BRIGHTNESS_TOGGLE
#undef KEY_SPELLCHECK
#undef KEY_LOGOFF
#undef KEY_DOLLAR
#undef KEY_EURO
#undef KEY_FRAMEBACK
#undef KEY_FRAMEFORWARD
#undef KEY_CONTEXT_MENU
#undef KEY_MEDIA_REPEAT
#undef KEY_10CHANNELSUP
#undef KEY_10CHANNELSDOWN
#undef KEY_IMAGES
#undef KEY_NOTIFICATION_CENTER
#undef KEY_PICKUP_PHONE
#undef KEY_HANGUP_PHONE
#undef KEY_LINK_PHONE
#undef KEY_DEL_EOL
#undef KEY_DEL_EOS
#undef KEY_INS_LINE
#undef KEY_DEL_LINE
#undef KEY_FN
#undef KEY_FN_ESC
#undef KEY_FN_F1
#undef KEY_FN_F2
#undef KEY_FN_F3
#undef KEY_FN_F4
#undef KEY_FN_F5
#undef KEY_FN_F6
#undef KEY_FN_F7
#undef KEY_FN_F8
#undef KEY_FN_F9
#undef KEY_FN_F10
#undef KEY_FN_F11
#undef KEY_FN_F12
#undef KEY_FN_1
#undef KEY_FN_2
#undef KEY_FN_D
#undef KEY_FN_E
#undef KEY_FN_F
#undef KEY_FN_S
#undef KEY_FN_B
#undef KEY_FN_RIGHT_SHIFT
#undef KEY_BRL_DOT1
#undef KEY_BRL_DOT2
#undef KEY_BRL_DOT3
#undef KEY_BRL_DOT4
#undef KEY_BRL_DOT5
#undef KEY_BRL_DOT6
#undef KEY_BRL_DOT7
#undef KEY_BRL_DOT8
#undef KEY_BRL_DOT9
#undef KEY_BRL_DOT10
#undef KEY_NUMERIC_0
#undef KEY_NUMERIC_1
#undef KEY_NUMERIC_2
#undef KEY_NUMERIC_3
#undef KEY_NUMERIC_4
#undef KEY_NUMERIC_5
#undef KEY_NUMERIC_6
#undef KEY_NUMERIC_7
#undef KEY_NUMERIC_8
#undef KEY_NUMERIC_9
#undef KEY_NUMERIC_STAR
#undef KEY_NUMERIC_POUND
#undef KEY_NUMERIC_A
#undef KEY_NUMERIC_B
#undef KEY_NUMERIC_C
#undef KEY_NUMERIC_D
#undef KEY_CAMERA_FOCUS
#undef KEY_WPS_BUTTON
#undef KEY_TOUCHPAD_TOGGLE
#undef KEY_TOUCHPAD_ON
#undef KEY_TOUCHPAD_OFF
#undef KEY_CAMERA_ZOOMIN
#undef KEY_CAMERA_ZOOMOUT
#undef KEY_CAMERA_UP
#undef KEY_CAMERA_DOWN
#undef KEY_CAMERA_LEFT
#undef KEY_CAMERA_RIGHT
#undef KEY_ATTENDANT_ON
#undef KEY_ATTENDANT_OFF
#undef KEY_ATTENDANT_TOGGLE
#undef KEY_LIGHTS_TOGGLE
#undef BTN_DPAD_UP
#undef BTN_DPAD_DOWN
#undef BTN_DPAD_LEFT
#undef BTN_DPAD_RIGHT
#undef KEY_ALS_TOGGLE
#undef KEY_ROTATE_LOCK_TOGGLE
#undef KEY_REFRESH_RATE_TOGGLE
#undef KEY_BUTTONCONFIG
#undef KEY_TASKMANAGER
#undef KEY_JOURNAL
#undef KEY_CONTROLPANEL
#undef KEY_APPSELECT
#undef KEY_SCREENSAVER
#undef KEY_VOICECOMMAND
#undef KEY_ASSISTANT
#undef KEY_KBD_LAYOUT_NEXT
#undef KEY_EMOJI_PICKER
#undef KEY_DICTATE
#undef KEY_CAMERA_ACCESS_ENABLE
#undef KEY_CAMERA_ACCESS_DISABLE
#undef KEY_CAMERA_ACCESS_TOGGLE
#undef KEY_ACCESSIBILITY
#undef KEY_DO_NOT_DISTURB
#undef KEY_BRIGHTNESS_MIN
#undef KEY_BRIGHTNESS_MAX
#undef KEY_KBDINPUTASSIST_PREV
#undef KEY_KBDINPUTASSIST_NEXT
#undef KEY_KBDINPUTASSIST_PREVGROUP
#undef KEY_KBDINPUTASSIST_NEXTGROUP
#undef KEY_KBDINPUTASSIST_ACCEPT
#undef KEY_KBDINPUTASSIST_CANCEL
#undef KEY_RIGHT_UP
#undef KEY_RIGHT_DOWN
#undef KEY_LEFT_UP
#undef KEY_LEFT_DOWN
#undef KEY_ROOT_MENU
#undef KEY_MEDIA_TOP_MENU
#undef KEY_NUMERIC_11
#undef KEY_NUMERIC_12
#undef KEY_AUDIO_DESC
#undef KEY_3D_MODE
#undef KEY_NEXT_FAVORITE
#undef KEY_STOP_RECORD
#undef KEY_PAUSE_RECORD
#undef KEY_VOD
#undef KEY_UNMUTE
#undef KEY_FASTREVERSE
#undef KEY_SLOWREVERSE
#undef KEY_DATA
#undef KEY_ONSCREEN_KEYBOARD
#undef KEY_PRIVACY_SCREEN_TOGGLE
#undef KEY_SELECTIVE_SCREENSHOT
#undef KEY_NEXT_ELEMENT
#undef KEY_PREVIOUS_ELEMENT
#undef KEY_AUTOPILOT_ENGAGE_TOGGLE
#undef KEY_MARK_WAYPOINT
#undef KEY_SOS
#undef KEY_NAV_CHART
#undef KEY_FISHING_CHART
#undef KEY_SINGLE_RANGE_RADAR
#undef KEY_DUAL_RANGE_RADAR
#undef KEY_RADAR_OVERLAY
#undef KEY_TRADITIONAL_SONAR
#undef KEY_CLEARVU_SONAR
#undef KEY_SIDEVU_SONAR
#undef KEY_NAV_INFO
#undef KEY_BRIGHTNESS_MENU
#undef KEY_MACRO1
#undef KEY_MACRO2
#undef KEY_MACRO3
#undef KEY_MACRO4
#undef KEY_MACRO5
#undef KEY_MACRO6
#undef KEY_MACRO7
#undef KEY_MACRO8
#undef KEY_MACRO9
#undef KEY_MACRO10
#undef KEY_MACRO11
#undef KEY_MACRO12
#undef KEY_MACRO13
#undef KEY_MACRO14
#undef KEY_MACRO15
#undef KEY_MACRO16
#undef KEY_MACRO17
#undef KEY_MACRO18
#undef KEY_MACRO19
#undef KEY_MACRO20
#undef KEY_MACRO21
#undef KEY_MACRO22
#undef KEY_MACRO23
#undef KEY_MACRO24
#undef KEY_MACRO25
#undef KEY_MACRO26
#undef KEY_MACRO27
#undef KEY_MACRO28
#undef KEY_MACRO29
#undef KEY_MACRO30
#undef KEY_MACRO_RECORD_START
#undef KEY_MACRO_RECORD_STOP
#undef KEY_MACRO_PRESET_CYCLE
#undef KEY_MACRO_PRESET1
#undef KEY_MACRO_PRESET2
#undef KEY_MACRO_PRESET3
#undef KEY_KBD_LCD_MENU1
#undef KEY_KBD_LCD_MENU2
#undef KEY_KBD_LCD_MENU3
#undef KEY_KBD_LCD_MENU4
#undef KEY_KBD_LCD_MENU5
#undef BTN_TRIGGER_HAPPY
#undef BTN_TRIGGER_HAPPY1
#undef BTN_TRIGGER_HAPPY2
#undef BTN_TRIGGER_HAPPY3
#undef BTN_TRIGGER_HAPPY4
#undef BTN_TRIGGER_HAPPY5
#undef BTN_TRIGGER_HAPPY6
#undef BTN_TRIGGER_HAPPY7
#undef BTN_TRIGGER_HAPPY8
#undef BTN_TRIGGER_HAPPY9
#undef BTN_TRIGGER_HAPPY10
#undef BTN_TRIGGER_HAPPY11
#undef BTN_TRIGGER_HAPPY12
#undef BTN_TRIGGER_HAPPY13
#undef BTN_TRIGGER_HAPPY14
#undef BTN_TRIGGER_HAPPY15
#undef BTN_TRIGGER_HAPPY16
#undef BTN_TRIGGER_HAPPY17
#undef BTN_TRIGGER_HAPPY18
#undef BTN_TRIGGER_HAPPY19
#undef BTN_TRIGGER_HAPPY20
#undef BTN_TRIGGER_HAPPY21
#undef BTN_TRIGGER_HAPPY22
#undef BTN_TRIGGER_HAPPY23
#undef BTN_TRIGGER_HAPPY24
#undef BTN_TRIGGER_HAPPY25
#undef BTN_TRIGGER_HAPPY26
#undef BTN_TRIGGER_HAPPY27
#undef BTN_TRIGGER_HAPPY28
#undef BTN_TRIGGER_HAPPY29
#undef BTN_TRIGGER_HAPPY30
#undef BTN_TRIGGER_HAPPY31
#undef BTN_TRIGGER_HAPPY32
#undef BTN_TRIGGER_HAPPY33
#undef BTN_TRIGGER_HAPPY34
#undef BTN_TRIGGER_HAPPY35
#undef BTN_TRIGGER_HAPPY36
#undef BTN_TRIGGER_HAPPY37
#undef BTN_TRIGGER_HAPPY38
#undef BTN_TRIGGER_HAPPY39
#undef BTN_TRIGGER_HAPPY40
#undef KEY_MIN_INTERESTING
#undef KEY_MAX
#undef KEY_CNT
#undef REL_X
#undef REL_Y
#undef REL_Z
#undef REL_RX
#undef REL_RY
#undef REL_RZ
#undef REL_HWHEEL
#undef REL_DIAL
#undef REL_WHEEL
#undef REL_MISC
#undef REL_RESERVED
#undef REL_WHEEL_HI_RES
#undef REL_HWHEEL_HI_RES
#undef REL_MAX
#undef REL_CNT
#undef ABS_X
#undef ABS_Y
#undef ABS_Z
#undef ABS_RX
#undef ABS_RY
#undef ABS_RZ
#undef ABS_THROTTLE
#undef ABS_RUDDER
#undef ABS_WHEEL
#undef ABS_GAS
#undef ABS_BRAKE
#undef ABS_HAT0X
#undef ABS_HAT0Y
#undef ABS_HAT1X
#undef ABS_HAT1Y
#undef ABS_HAT2X
#undef ABS_HAT2Y
#undef ABS_HAT3X
#undef ABS_HAT3Y
#undef ABS_PRESSURE
#undef ABS_DISTANCE
#undef ABS_TILT_X
#undef ABS_TILT_Y
#undef ABS_TOOL_WIDTH
#undef ABS_VOLUME
#undef ABS_PROFILE
#undef ABS_MISC
#undef ABS_RESERVED
#undef ABS_MT_SLOT
#undef ABS_MT_TOUCH_MAJOR
#undef ABS_MT_TOUCH_MINOR
#undef ABS_MT_WIDTH_MAJOR
#undef ABS_MT_WIDTH_MINOR
#undef ABS_MT_ORIENTATION
#undef ABS_MT_POSITION_X
#undef ABS_MT_POSITION_Y
#undef ABS_MT_TOOL_TYPE
#undef ABS_MT_BLOB_ID
#undef ABS_MT_TRACKING_ID
#undef ABS_MT_PRESSURE
#undef ABS_MT_DISTANCE
#undef ABS_MT_TOOL_X
#undef ABS_MT_TOOL_Y
#undef ABS_MAX
#undef ABS_CNT
#undef SW_LID
#undef SW_TABLET_MODE
#undef SW_HEADPHONE_INSERT
#undef SW_RFKILL_ALL
#undef SW_RADIO
#undef SW_MICROPHONE_INSERT
#undef SW_DOCK
#undef SW_LINEOUT_INSERT
#undef SW_JACK_PHYSICAL_INSERT
#undef SW_VIDEOOUT_INSERT
#undef SW_CAMERA_LENS_COVER
#undef SW_KEYPAD_SLIDE
#undef SW_FRONT_PROXIMITY
#undef SW_ROTATE_LOCK
#undef SW_LINEIN_INSERT
#undef SW_MUTE_DEVICE
#undef SW_PEN_INSERTED
#undef SW_MACHINE_COVER
#undef SW_MAX
#undef SW_CNT
#undef MSC_SERIAL
#undef MSC_PULSELED
#undef MSC_GESTURE
#undef MSC_RAW
#undef MSC_SCAN
#undef MSC_TIMESTAMP
#undef MSC_MAX
#undef MSC_CNT
#undef LED_NUML
#undef LED_CAPSL
#undef LED_SCROLLL
#undef LED_COMPOSE
#undef LED_KANA
#undef LED_SLEEP
#undef LED_SUSPEND
#undef LED_MUTE
#undef LED_MISC
#undef LED_MAIL
#undef LED_CHARGING
#undef LED_MAX
#undef LED_CNT
#undef REP_DELAY
#undef REP_PERIOD
#undef REP_MAX
#undef REP_CNT
#undef SND_CLICK
#undef SND_BELL
#undef SND_TONE
#undef SND_MAX
#undef SND_CNT


#define INPUT_PROP_POINTER		0x00	
#define INPUT_PROP_DIRECT		0x01	
#define INPUT_PROP_BUTTONPAD		0x02	
#define INPUT_PROP_SEMI_MT		0x03	
#define INPUT_PROP_TOPBUTTONPAD		0x04	
#define INPUT_PROP_POINTING_STICK	0x05	
#define INPUT_PROP_ACCELEROMETER	0x06	
#define INPUT_PROP_MAX			0x1f
#define INPUT_PROP_CNT			(INPUT_PROP_MAX + 1)
#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03
#define EV_MSC			0x04
#define EV_SW			0x05
#define EV_LED			0x11
#define EV_SND			0x12
#define EV_REP			0x14
#define EV_FF			0x15
#define EV_PWR			0x16
#define EV_FF_STATUS		0x17
#define EV_MAX			0x1f
#define EV_CNT			(EV_MAX+1)
#define SYN_REPORT		0
#define SYN_CONFIG		1
#define SYN_MT_REPORT		2
#define SYN_DROPPED		3
#define SYN_MAX			0xf
#define SYN_CNT			(SYN_MAX+1)
#define KEY_RESERVED		0
#define KEY_ESC			1
#define KEY_1			2
#define KEY_2			3
#define KEY_3			4
#define KEY_4			5
#define KEY_5			6
#define KEY_6			7
#define KEY_7			8
#define KEY_8			9
#define KEY_9			10
#define KEY_0			11
#define KEY_MINUS		12
#define KEY_EQUAL		13
#define KEY_BACKSPACE		14
#define KEY_TAB			15
#define KEY_Q			16
#define KEY_W			17
#define KEY_E			18
#define KEY_R			19
#define KEY_T			20
#define KEY_Y			21
#define KEY_U			22
#define KEY_I			23
#define KEY_O			24
#define KEY_P			25
#define KEY_LEFTBRACE		26
#define KEY_RIGHTBRACE		27
#define KEY_ENTER		28
#define KEY_LEFTCTRL		29
#define KEY_A			30
#define KEY_S			31
#define KEY_D			32
#define KEY_F			33
#define KEY_G			34
#define KEY_H			35
#define KEY_J			36
#define KEY_K			37
#define KEY_L			38
#define KEY_SEMICOLON		39
#define KEY_APOSTROPHE		40
#define KEY_GRAVE		41
#define KEY_LEFTSHIFT		42
#define KEY_BACKSLASH		43
#define KEY_Z			44
#define KEY_X			45
#define KEY_C			46
#define KEY_V			47
#define KEY_B			48
#define KEY_N			49
#define KEY_M			50
#define KEY_COMMA		51
#define KEY_DOT			52
#define KEY_SLASH		53
#define KEY_RIGHTSHIFT		54
#define KEY_KPASTERISK		55
#define KEY_LEFTALT		56
#define KEY_SPACE		57
#define KEY_CAPSLOCK		58
#define KEY_F1			59
#define KEY_F2			60
#define KEY_F3			61
#define KEY_F4			62
#define KEY_F5			63
#define KEY_F6			64
#define KEY_F7			65
#define KEY_F8			66
#define KEY_F9			67
#define KEY_F10			68
#define KEY_NUMLOCK		69
#define KEY_SCROLLLOCK		70
#define KEY_KP7			71
#define KEY_KP8			72
#define KEY_KP9			73
#define KEY_KPMINUS		74
#define KEY_KP4			75
#define KEY_KP5			76
#define KEY_KP6			77
#define KEY_KPPLUS		78
#define KEY_KP1			79
#define KEY_KP2			80
#define KEY_KP3			81
#define KEY_KP0			82
#define KEY_KPDOT		83
#define KEY_ZENKAKUHANKAKU	85
#define KEY_102ND		86
#define KEY_F11			87
#define KEY_F12			88
#define KEY_RO			89
#define KEY_KATAKANA		90
#define KEY_HIRAGANA		91
#define KEY_HENKAN		92
#define KEY_KATAKANAHIRAGANA	93
#define KEY_MUHENKAN		94
#define KEY_KPJPCOMMA		95
#define KEY_KPENTER		96
#define KEY_RIGHTCTRL		97
#define KEY_KPSLASH		98
#define KEY_SYSRQ		99
#define KEY_RIGHTALT		100
#define KEY_LINEFEED		101
#define KEY_HOME		102
#define KEY_UP			103
#define KEY_PAGEUP		104
#define KEY_LEFT		105
#define KEY_RIGHT		106
#define KEY_END			107
#define KEY_DOWN		108
#define KEY_PAGEDOWN		109
#define KEY_INSERT		110
#define KEY_DELETE		111
#define KEY_MACRO		112
#define KEY_MUTE		113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_POWER		116	
#define KEY_KPEQUAL		117
#define KEY_KPPLUSMINUS		118
#define KEY_PAUSE		119
#define KEY_SCALE		120	
#define KEY_KPCOMMA		121
#define KEY_HANGEUL		122
#define KEY_HANGUEL		KEY_HANGEUL
#define KEY_HANJA		123
#define KEY_YEN			124
#define KEY_LEFTMETA		125
#define KEY_RIGHTMETA		126
#define KEY_COMPOSE		127
#define KEY_STOP		128	
#define KEY_AGAIN		129
#define KEY_PROPS		130	
#define KEY_UNDO		131	
#define KEY_FRONT		132
#define KEY_COPY		133	
#define KEY_OPEN		134	
#define KEY_PASTE		135	
#define KEY_FIND		136	
#define KEY_CUT			137	
#define KEY_HELP		138	
#define KEY_MENU		139	
#define KEY_CALC		140	
#define KEY_SETUP		141
#define KEY_SLEEP		142	
#define KEY_WAKEUP		143	
#define KEY_FILE		144	
#define KEY_SENDFILE		145
#define KEY_DELETEFILE		146
#define KEY_XFER		147
#define KEY_PROG1		148
#define KEY_PROG2		149
#define KEY_WWW			150	
#define KEY_MSDOS		151
#define KEY_COFFEE		152	
#define KEY_SCREENLOCK		KEY_COFFEE
#define KEY_ROTATE_DISPLAY	153	
#define KEY_DIRECTION		KEY_ROTATE_DISPLAY
#define KEY_CYCLEWINDOWS	154
#define KEY_MAIL		155
#define KEY_BOOKMARKS		156	
#define KEY_COMPUTER		157
#define KEY_BACK		158	
#define KEY_FORWARD		159	
#define KEY_CLOSECD		160
#define KEY_EJECTCD		161
#define KEY_EJECTCLOSECD	162
#define KEY_NEXTSONG		163
#define KEY_PLAYPAUSE		164
#define KEY_PREVIOUSSONG	165
#define KEY_STOPCD		166
#define KEY_RECORD		167
#define KEY_REWIND		168
#define KEY_PHONE		169	
#define KEY_ISO			170
#define KEY_CONFIG		171	
#define KEY_HOMEPAGE		172	
#define KEY_REFRESH		173	
#define KEY_EXIT		174	
#define KEY_MOVE		175
#define KEY_EDIT		176
#define KEY_SCROLLUP		177
#define KEY_SCROLLDOWN		178
#define KEY_KPLEFTPAREN		179
#define KEY_KPRIGHTPAREN	180
#define KEY_NEW			181	
#define KEY_REDO		182	
#define KEY_F13			183
#define KEY_F14			184
#define KEY_F15			185
#define KEY_F16			186
#define KEY_F17			187
#define KEY_F18			188
#define KEY_F19			189
#define KEY_F20			190
#define KEY_F21			191
#define KEY_F22			192
#define KEY_F23			193
#define KEY_F24			194
#define KEY_PLAYCD		200
#define KEY_PAUSECD		201
#define KEY_PROG3		202
#define KEY_PROG4		203
#define KEY_ALL_APPLICATIONS	204	
#define KEY_DASHBOARD		KEY_ALL_APPLICATIONS
#define KEY_SUSPEND		205
#define KEY_CLOSE		206	
#define KEY_PLAY		207
#define KEY_FASTFORWARD		208
#define KEY_BASSBOOST		209
#define KEY_PRINT		210	
#define KEY_HP			211
#define KEY_CAMERA		212
#define KEY_SOUND		213
#define KEY_QUESTION		214
#define KEY_EMAIL		215
#define KEY_CHAT		216
#define KEY_SEARCH		217
#define KEY_CONNECT		218
#define KEY_FINANCE		219	
#define KEY_SPORT		220
#define KEY_SHOP		221
#define KEY_ALTERASE		222
#define KEY_CANCEL		223	
#define KEY_BRIGHTNESSDOWN	224
#define KEY_BRIGHTNESSUP	225
#define KEY_MEDIA		226
#define KEY_SWITCHVIDEOMODE	227	
#define KEY_KBDILLUMTOGGLE	228
#define KEY_KBDILLUMDOWN	229
#define KEY_KBDILLUMUP		230
#define KEY_SEND		231	
#define KEY_REPLY		232	
#define KEY_FORWARDMAIL		233	
#define KEY_SAVE		234	
#define KEY_DOCUMENTS		235
#define KEY_BATTERY		236
#define KEY_BLUETOOTH		237
#define KEY_WLAN		238
#define KEY_UWB			239
#define KEY_UNKNOWN		240
#define KEY_VIDEO_NEXT		241	
#define KEY_VIDEO_PREV		242	
#define KEY_BRIGHTNESS_CYCLE	243	
#define KEY_BRIGHTNESS_AUTO	244	
#define KEY_BRIGHTNESS_ZERO	KEY_BRIGHTNESS_AUTO
#define KEY_DISPLAY_OFF		245	
#define KEY_WWAN		246	
#define KEY_WIMAX		KEY_WWAN
#define KEY_RFKILL		247	
#define KEY_MICMUTE		248	
#define BTN_MISC		0x100
#define BTN_0			0x100
#define BTN_1			0x101
#define BTN_2			0x102
#define BTN_3			0x103
#define BTN_4			0x104
#define BTN_5			0x105
#define BTN_6			0x106
#define BTN_7			0x107
#define BTN_8			0x108
#define BTN_9			0x109
#define BTN_MOUSE		0x110
#define BTN_LEFT		0x110
#define BTN_RIGHT		0x111
#define BTN_MIDDLE		0x112
#define BTN_SIDE		0x113
#define BTN_EXTRA		0x114
#define BTN_FORWARD		0x115
#define BTN_BACK		0x116
#define BTN_TASK		0x117
#define BTN_JOYSTICK		0x120
#define BTN_TRIGGER		0x120
#define BTN_THUMB		0x121
#define BTN_THUMB2		0x122
#define BTN_TOP			0x123
#define BTN_TOP2		0x124
#define BTN_PINKIE		0x125
#define BTN_BASE		0x126
#define BTN_BASE2		0x127
#define BTN_BASE3		0x128
#define BTN_BASE4		0x129
#define BTN_BASE5		0x12a
#define BTN_BASE6		0x12b
#define BTN_DEAD		0x12f
#define BTN_GAMEPAD		0x130
#define BTN_SOUTH		0x130
#define BTN_A			BTN_SOUTH
#define BTN_EAST		0x131
#define BTN_B			BTN_EAST
#define BTN_C			0x132
#define BTN_NORTH		0x133
#define BTN_X			BTN_NORTH
#define BTN_WEST		0x134
#define BTN_Y			BTN_WEST
#define BTN_Z			0x135
#define BTN_TL			0x136
#define BTN_TR			0x137
#define BTN_TL2			0x138
#define BTN_TR2			0x139
#define BTN_SELECT		0x13a
#define BTN_START		0x13b
#define BTN_MODE		0x13c
#define BTN_THUMBL		0x13d
#define BTN_THUMBR		0x13e
#define BTN_DIGI		0x140
#define BTN_TOOL_PEN		0x140
#define BTN_TOOL_RUBBER		0x141
#define BTN_TOOL_BRUSH		0x142
#define BTN_TOOL_PENCIL		0x143
#define BTN_TOOL_AIRBRUSH	0x144
#define BTN_TOOL_FINGER		0x145
#define BTN_TOOL_MOUSE		0x146
#define BTN_TOOL_LENS		0x147
#define BTN_TOOL_QUINTTAP	0x148	
#define BTN_STYLUS3		0x149
#define BTN_TOUCH		0x14a
#define BTN_STYLUS		0x14b
#define BTN_STYLUS2		0x14c
#define BTN_TOOL_DOUBLETAP	0x14d
#define BTN_TOOL_TRIPLETAP	0x14e
#define BTN_TOOL_QUADTAP	0x14f	
#define BTN_WHEEL		0x150
#define BTN_GEAR_DOWN		0x150
#define BTN_GEAR_UP		0x151
#define KEY_OK			0x160
#define KEY_SELECT		0x161
#define KEY_GOTO		0x162
#define KEY_CLEAR		0x163
#define KEY_POWER2		0x164
#define KEY_OPTION		0x165
#define KEY_INFO		0x166	
#define KEY_TIME		0x167
#define KEY_VENDOR		0x168
#define KEY_ARCHIVE		0x169
#define KEY_PROGRAM		0x16a	
#define KEY_CHANNEL		0x16b
#define KEY_FAVORITES		0x16c
#define KEY_EPG			0x16d
#define KEY_PVR			0x16e	
#define KEY_MHP			0x16f
#define KEY_LANGUAGE		0x170
#define KEY_TITLE		0x171
#define KEY_SUBTITLE		0x172
#define KEY_ANGLE		0x173
#define KEY_FULL_SCREEN		0x174	
#define KEY_ZOOM		KEY_FULL_SCREEN
#define KEY_MODE		0x175
#define KEY_KEYBOARD		0x176
#define KEY_ASPECT_RATIO	0x177	
#define KEY_SCREEN		KEY_ASPECT_RATIO
#define KEY_PC			0x178	
#define KEY_TV			0x179	
#define KEY_TV2			0x17a	
#define KEY_VCR			0x17b	
#define KEY_VCR2		0x17c	
#define KEY_SAT			0x17d	
#define KEY_SAT2		0x17e
#define KEY_CD			0x17f	
#define KEY_TAPE		0x180	
#define KEY_RADIO		0x181
#define KEY_TUNER		0x182	
#define KEY_PLAYER		0x183
#define KEY_TEXT		0x184
#define KEY_DVD			0x185	
#define KEY_AUX			0x186
#define KEY_MP3			0x187
#define KEY_AUDIO		0x188	
#define KEY_VIDEO		0x189	
#define KEY_DIRECTORY		0x18a
#define KEY_LIST		0x18b
#define KEY_MEMO		0x18c	
#define KEY_CALENDAR		0x18d
#define KEY_RED			0x18e
#define KEY_GREEN		0x18f
#define KEY_YELLOW		0x190
#define KEY_BLUE		0x191
#define KEY_CHANNELUP		0x192	
#define KEY_CHANNELDOWN		0x193	
#define KEY_FIRST		0x194
#define KEY_LAST		0x195	
#define KEY_AB			0x196
#define KEY_NEXT		0x197
#define KEY_RESTART		0x198
#define KEY_SLOW		0x199
#define KEY_SHUFFLE		0x19a
#define KEY_BREAK		0x19b
#define KEY_PREVIOUS		0x19c
#define KEY_DIGITS		0x19d
#define KEY_TEEN		0x19e
#define KEY_TWEN		0x19f
#define KEY_VIDEOPHONE		0x1a0	
#define KEY_GAMES		0x1a1	
#define KEY_ZOOMIN		0x1a2	
#define KEY_ZOOMOUT		0x1a3	
#define KEY_ZOOMRESET		0x1a4	
#define KEY_WORDPROCESSOR	0x1a5	
#define KEY_EDITOR		0x1a6	
#define KEY_SPREADSHEET		0x1a7	
#define KEY_GRAPHICSEDITOR	0x1a8	
#define KEY_PRESENTATION	0x1a9	
#define KEY_DATABASE		0x1aa	
#define KEY_NEWS		0x1ab	
#define KEY_VOICEMAIL		0x1ac	
#define KEY_ADDRESSBOOK		0x1ad	
#define KEY_MESSENGER		0x1ae	
#define KEY_DISPLAYTOGGLE	0x1af	
#define KEY_BRIGHTNESS_TOGGLE	KEY_DISPLAYTOGGLE
#define KEY_SPELLCHECK		0x1b0   
#define KEY_LOGOFF		0x1b1   
#define KEY_DOLLAR		0x1b2
#define KEY_EURO		0x1b3
#define KEY_FRAMEBACK		0x1b4	
#define KEY_FRAMEFORWARD	0x1b5
#define KEY_CONTEXT_MENU	0x1b6	
#define KEY_MEDIA_REPEAT	0x1b7	
#define KEY_10CHANNELSUP	0x1b8	
#define KEY_10CHANNELSDOWN	0x1b9	
#define KEY_IMAGES		0x1ba	
#define KEY_NOTIFICATION_CENTER	0x1bc	
#define KEY_PICKUP_PHONE	0x1bd	
#define KEY_LINK_PHONE		0x1bf
#define KEY_HANGUP_PHONE	0x1be	
#define KEY_DEL_EOL		0x1c0
#define KEY_DEL_EOS		0x1c1
#define KEY_INS_LINE		0x1c2
#define KEY_DEL_LINE		0x1c3
#define KEY_FN			0x1d0
#define KEY_FN_ESC		0x1d1
#define KEY_FN_F1		0x1d2
#define KEY_FN_F2		0x1d3
#define KEY_FN_F3		0x1d4
#define KEY_FN_F4		0x1d5
#define KEY_FN_F5		0x1d6
#define KEY_FN_F6		0x1d7
#define KEY_FN_F7		0x1d8
#define KEY_FN_F8		0x1d9
#define KEY_FN_F9		0x1da
#define KEY_FN_F10		0x1db
#define KEY_FN_F11		0x1dc
#define KEY_FN_F12		0x1dd
#define KEY_FN_1		0x1de
#define KEY_FN_2		0x1df
#define KEY_FN_D		0x1e0
#define KEY_FN_E		0x1e1
#define KEY_FN_F		0x1e2
#define KEY_FN_S		0x1e3
#define KEY_FN_B		0x1e4
#define KEY_FN_RIGHT_SHIFT	0x1e5
#define KEY_BRL_DOT1		0x1f1
#define KEY_BRL_DOT2		0x1f2
#define KEY_BRL_DOT3		0x1f3
#define KEY_BRL_DOT4		0x1f4
#define KEY_BRL_DOT5		0x1f5
#define KEY_BRL_DOT6		0x1f6
#define KEY_BRL_DOT7		0x1f7
#define KEY_BRL_DOT8		0x1f8
#define KEY_BRL_DOT9		0x1f9
#define KEY_BRL_DOT10		0x1fa
#define KEY_NUMERIC_0		0x200	
#define KEY_NUMERIC_1		0x201	
#define KEY_NUMERIC_2		0x202
#define KEY_NUMERIC_3		0x203
#define KEY_NUMERIC_4		0x204
#define KEY_NUMERIC_5		0x205
#define KEY_NUMERIC_6		0x206
#define KEY_NUMERIC_7		0x207
#define KEY_NUMERIC_8		0x208
#define KEY_NUMERIC_9		0x209
#define KEY_NUMERIC_STAR	0x20a
#define KEY_NUMERIC_POUND	0x20b
#define KEY_NUMERIC_A		0x20c	
#define KEY_NUMERIC_B		0x20d
#define KEY_NUMERIC_C		0x20e
#define KEY_NUMERIC_D		0x20f
#define KEY_CAMERA_FOCUS	0x210
#define KEY_WPS_BUTTON		0x211	
#define KEY_TOUCHPAD_TOGGLE	0x212	
#define KEY_TOUCHPAD_ON		0x213
#define KEY_TOUCHPAD_OFF	0x214
#define KEY_CAMERA_ZOOMIN	0x215
#define KEY_CAMERA_ZOOMOUT	0x216
#define KEY_CAMERA_UP		0x217
#define KEY_CAMERA_DOWN		0x218
#define KEY_CAMERA_LEFT		0x219
#define KEY_CAMERA_RIGHT	0x21a
#define KEY_ATTENDANT_ON	0x21b
#define KEY_ATTENDANT_OFF	0x21c
#define KEY_ATTENDANT_TOGGLE	0x21d	
#define KEY_LIGHTS_TOGGLE	0x21e	
#define BTN_DPAD_UP		0x220
#define BTN_DPAD_DOWN		0x221
#define BTN_DPAD_LEFT		0x222
#define BTN_DPAD_RIGHT		0x223
#define KEY_ALS_TOGGLE		0x230	
#define KEY_ROTATE_LOCK_TOGGLE	0x231	
#define KEY_REFRESH_RATE_TOGGLE	0x232	
#define KEY_BUTTONCONFIG		0x240	
#define KEY_TASKMANAGER		0x241	
#define KEY_JOURNAL		0x242	
#define KEY_CONTROLPANEL		0x243	
#define KEY_APPSELECT		0x244	
#define KEY_SCREENSAVER		0x245	
#define KEY_VOICECOMMAND		0x246	
#define KEY_ASSISTANT		0x247	
#define KEY_KBD_LAYOUT_NEXT	0x248	
#define KEY_EMOJI_PICKER	0x249	
#define KEY_DICTATE		0x24a	
#define KEY_CAMERA_ACCESS_ENABLE	0x24b
#define KEY_CAMERA_ACCESS_DISABLE	0x24c
#define KEY_CAMERA_ACCESS_TOGGLE	0x24d
#define KEY_ACCESSIBILITY	0x24e
#define KEY_DO_NOT_DISTURB	0x24f
#define KEY_BRIGHTNESS_MIN		0x250	
#define KEY_BRIGHTNESS_MAX		0x251	
#define KEY_KBDINPUTASSIST_PREV		0x260
#define KEY_KBDINPUTASSIST_NEXT		0x261
#define KEY_KBDINPUTASSIST_PREVGROUP		0x262
#define KEY_KBDINPUTASSIST_NEXTGROUP		0x263
#define KEY_KBDINPUTASSIST_ACCEPT		0x264
#define KEY_KBDINPUTASSIST_CANCEL		0x265
#define KEY_RIGHT_UP			0x266
#define KEY_RIGHT_DOWN			0x267
#define KEY_LEFT_UP			0x268
#define KEY_LEFT_DOWN			0x269
#define KEY_ROOT_MENU			0x26a 
#define KEY_MEDIA_TOP_MENU		0x26b
#define KEY_NUMERIC_11			0x26c
#define KEY_NUMERIC_12			0x26d
#define KEY_AUDIO_DESC			0x26e
#define KEY_3D_MODE			0x26f
#define KEY_NEXT_FAVORITE		0x270
#define KEY_STOP_RECORD			0x271
#define KEY_PAUSE_RECORD		0x272
#define KEY_VOD				0x273 
#define KEY_UNMUTE			0x274
#define KEY_FASTREVERSE			0x275
#define KEY_SLOWREVERSE			0x276
#define KEY_DATA			0x277
#define KEY_ONSCREEN_KEYBOARD		0x278
#define KEY_PRIVACY_SCREEN_TOGGLE	0x279
#define KEY_SELECTIVE_SCREENSHOT	0x27a
#define KEY_NEXT_ELEMENT               0x27b
#define KEY_PREVIOUS_ELEMENT           0x27c
#define KEY_AUTOPILOT_ENGAGE_TOGGLE    0x27d
#define KEY_MARK_WAYPOINT              0x27e
#define KEY_SOS                                0x27f
#define KEY_NAV_CHART                  0x280
#define KEY_FISHING_CHART              0x281
#define KEY_SINGLE_RANGE_RADAR         0x282
#define KEY_DUAL_RANGE_RADAR           0x283
#define KEY_RADAR_OVERLAY              0x284
#define KEY_TRADITIONAL_SONAR          0x285
#define KEY_CLEARVU_SONAR              0x286
#define KEY_SIDEVU_SONAR               0x287
#define KEY_NAV_INFO                   0x288
#define KEY_BRIGHTNESS_MENU            0x289
#define KEY_MACRO1			0x290
#define KEY_MACRO2			0x291
#define KEY_MACRO3			0x292
#define KEY_MACRO4			0x293
#define KEY_MACRO5			0x294
#define KEY_MACRO6			0x295
#define KEY_MACRO7			0x296
#define KEY_MACRO8			0x297
#define KEY_MACRO9			0x298
#define KEY_MACRO10			0x299
#define KEY_MACRO11			0x29a
#define KEY_MACRO12			0x29b
#define KEY_MACRO13			0x29c
#define KEY_MACRO14			0x29d
#define KEY_MACRO15			0x29e
#define KEY_MACRO16			0x29f
#define KEY_MACRO17			0x2a0
#define KEY_MACRO18			0x2a1
#define KEY_MACRO19			0x2a2
#define KEY_MACRO20			0x2a3
#define KEY_MACRO21			0x2a4
#define KEY_MACRO22			0x2a5
#define KEY_MACRO23			0x2a6
#define KEY_MACRO24			0x2a7
#define KEY_MACRO25			0x2a8
#define KEY_MACRO26			0x2a9
#define KEY_MACRO27			0x2aa
#define KEY_MACRO28			0x2ab
#define KEY_MACRO29			0x2ac
#define KEY_MACRO30			0x2ad
#define KEY_MACRO_RECORD_START		0x2b0
#define KEY_MACRO_RECORD_STOP		0x2b1
#define KEY_MACRO_PRESET_CYCLE		0x2b2
#define KEY_MACRO_PRESET1		0x2b3
#define KEY_MACRO_PRESET2		0x2b4
#define KEY_MACRO_PRESET3		0x2b5
#define KEY_KBD_LCD_MENU1		0x2b8
#define KEY_KBD_LCD_MENU2		0x2b9
#define KEY_KBD_LCD_MENU3		0x2ba
#define KEY_KBD_LCD_MENU4		0x2bb
#define KEY_KBD_LCD_MENU5		0x2bc
#define BTN_TRIGGER_HAPPY		0x2c0
#define BTN_TRIGGER_HAPPY1		0x2c0
#define BTN_TRIGGER_HAPPY2		0x2c1
#define BTN_TRIGGER_HAPPY3		0x2c2
#define BTN_TRIGGER_HAPPY4		0x2c3
#define BTN_TRIGGER_HAPPY5		0x2c4
#define BTN_TRIGGER_HAPPY6		0x2c5
#define BTN_TRIGGER_HAPPY7		0x2c6
#define BTN_TRIGGER_HAPPY8		0x2c7
#define BTN_TRIGGER_HAPPY9		0x2c8
#define BTN_TRIGGER_HAPPY10		0x2c9
#define BTN_TRIGGER_HAPPY11		0x2ca
#define BTN_TRIGGER_HAPPY12		0x2cb
#define BTN_TRIGGER_HAPPY13		0x2cc
#define BTN_TRIGGER_HAPPY14		0x2cd
#define BTN_TRIGGER_HAPPY15		0x2ce
#define BTN_TRIGGER_HAPPY16		0x2cf
#define BTN_TRIGGER_HAPPY17		0x2d0
#define BTN_TRIGGER_HAPPY18		0x2d1
#define BTN_TRIGGER_HAPPY19		0x2d2
#define BTN_TRIGGER_HAPPY20		0x2d3
#define BTN_TRIGGER_HAPPY21		0x2d4
#define BTN_TRIGGER_HAPPY22		0x2d5
#define BTN_TRIGGER_HAPPY23		0x2d6
#define BTN_TRIGGER_HAPPY24		0x2d7
#define BTN_TRIGGER_HAPPY25		0x2d8
#define BTN_TRIGGER_HAPPY26		0x2d9
#define BTN_TRIGGER_HAPPY27		0x2da
#define BTN_TRIGGER_HAPPY28		0x2db
#define BTN_TRIGGER_HAPPY29		0x2dc
#define BTN_TRIGGER_HAPPY30		0x2dd
#define BTN_TRIGGER_HAPPY31		0x2de
#define BTN_TRIGGER_HAPPY32		0x2df
#define BTN_TRIGGER_HAPPY33		0x2e0
#define BTN_TRIGGER_HAPPY34		0x2e1
#define BTN_TRIGGER_HAPPY35		0x2e2
#define BTN_TRIGGER_HAPPY36		0x2e3
#define BTN_TRIGGER_HAPPY37		0x2e4
#define BTN_TRIGGER_HAPPY38		0x2e5
#define BTN_TRIGGER_HAPPY39		0x2e6
#define BTN_TRIGGER_HAPPY40		0x2e7
#define KEY_MIN_INTERESTING	KEY_MUTE
#define KEY_MAX			0x2ff
#define KEY_CNT			(KEY_MAX+1)
#define REL_X			0x00
#define REL_Y			0x01
#define REL_Z			0x02
#define REL_RX			0x03
#define REL_RY			0x04
#define REL_RZ			0x05
#define REL_HWHEEL		0x06
#define REL_DIAL		0x07
#define REL_WHEEL		0x08
#define REL_MISC		0x09
#define REL_RESERVED		0x0a
#define REL_WHEEL_HI_RES	0x0b
#define REL_HWHEEL_HI_RES	0x0c
#define REL_MAX			0x0f
#define REL_CNT			(REL_MAX+1)
#define ABS_X			0x00
#define ABS_Y			0x01
#define ABS_Z			0x02
#define ABS_RX			0x03
#define ABS_RY			0x04
#define ABS_RZ			0x05
#define ABS_THROTTLE		0x06
#define ABS_RUDDER		0x07
#define ABS_WHEEL		0x08
#define ABS_GAS			0x09
#define ABS_BRAKE		0x0a
#define ABS_HAT0X		0x10
#define ABS_HAT0Y		0x11
#define ABS_HAT1X		0x12
#define ABS_HAT1Y		0x13
#define ABS_HAT2X		0x14
#define ABS_HAT2Y		0x15
#define ABS_HAT3X		0x16
#define ABS_HAT3Y		0x17
#define ABS_PRESSURE		0x18
#define ABS_DISTANCE		0x19
#define ABS_TILT_X		0x1a
#define ABS_TILT_Y		0x1b
#define ABS_TOOL_WIDTH		0x1c
#define ABS_VOLUME		0x20
#define ABS_PROFILE		0x21
#define ABS_MISC		0x28
#define ABS_RESERVED		0x2e
#define ABS_MT_SLOT		0x2f	
#define ABS_MT_TOUCH_MAJOR	0x30	
#define ABS_MT_TOUCH_MINOR	0x31	
#define ABS_MT_WIDTH_MAJOR	0x32	
#define ABS_MT_WIDTH_MINOR	0x33	
#define ABS_MT_ORIENTATION	0x34	
#define ABS_MT_POSITION_X	0x35	
#define ABS_MT_POSITION_Y	0x36	
#define ABS_MT_TOOL_TYPE	0x37	
#define ABS_MT_BLOB_ID		0x38	
#define ABS_MT_TRACKING_ID	0x39	
#define ABS_MT_PRESSURE		0x3a	
#define ABS_MT_DISTANCE		0x3b	
#define ABS_MT_TOOL_X		0x3c	
#define ABS_MT_TOOL_Y		0x3d	
#define ABS_MAX			0x3f
#define ABS_CNT			(ABS_MAX+1)
#define SW_LID			0x00  
#define SW_TABLET_MODE		0x01  
#define SW_HEADPHONE_INSERT	0x02  
#define SW_RFKILL_ALL		0x03  
#define SW_RADIO		SW_RFKILL_ALL	
#define SW_MICROPHONE_INSERT	0x04  
#define SW_DOCK			0x05  
#define SW_LINEOUT_INSERT	0x06  
#define SW_JACK_PHYSICAL_INSERT 0x07  
#define SW_VIDEOOUT_INSERT	0x08  
#define SW_CAMERA_LENS_COVER	0x09  
#define SW_KEYPAD_SLIDE		0x0a  
#define SW_FRONT_PROXIMITY	0x0b  
#define SW_ROTATE_LOCK		0x0c  
#define SW_LINEIN_INSERT	0x0d  
#define SW_MUTE_DEVICE		0x0e  
#define SW_PEN_INSERTED		0x0f  
#define SW_MACHINE_COVER	0x10  
#define SW_MAX			0x10
#define SW_CNT			(SW_MAX+1)
#define MSC_SERIAL		0x00
#define MSC_PULSELED		0x01
#define MSC_GESTURE		0x02
#define MSC_RAW			0x03
#define MSC_SCAN		0x04
#define MSC_TIMESTAMP		0x05
#define MSC_MAX			0x07
#define MSC_CNT			(MSC_MAX+1)
#define LED_NUML		0x00
#define LED_CAPSL		0x01
#define LED_SCROLLL		0x02
#define LED_COMPOSE		0x03
#define LED_KANA		0x04
#define LED_SLEEP		0x05
#define LED_SUSPEND		0x06
#define LED_MUTE		0x07
#define LED_MISC		0x08
#define LED_MAIL		0x09
#define LED_CHARGING		0x0a
#define LED_MAX			0x0f
#define LED_CNT			(LED_MAX+1)
#define REP_DELAY		0x00
#define REP_PERIOD		0x01
#define REP_MAX			0x01
#define REP_CNT			(REP_MAX+1)
#define SND_CLICK		0x00
#define SND_BELL		0x01
#define SND_TONE		0x02
#define SND_MAX			0x07

#define SND_CNT			(SND_MAX+1)

#endif
