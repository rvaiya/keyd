/*
 * keyd - A key remapping daemon.
 *
 * macOS virtual keyboard output backend.
 *
 * Key events are injected via CGEventPost (Quartz Events).
 * Mouse movement and scrolling use CGEventPost with the appropriate event
 * types.  Every injected event is tagged with KEYD_EVENT_MARKER in the
 * kCGEventSourceUserData field so that our own CGEventTap (in macos/input.c)
 * can recognise and pass them through instead of suppressing them.
 *
 * Key repeat: CGEventPost is one-shot (unlike uinput on Linux which has
 * built-in kernel repeat).  A background thread fires kCGEventKeyDown with
 * kCGKeyboardEventAutorepeat=1 at the system repeat delay/interval to
 * replicate the uinput behaviour.
 *
 * Link with: -framework CoreGraphics -framework ApplicationServices
 */

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ApplicationServices/ApplicationServices.h>

#include "../keyd.h"
#include "../macos/keycodes.h"

#define KEYD_EVENT_MARKER ((int64_t)0x6B657964ULL)

struct vkbd {
	int _dummy; /* CGEventPost needs no persistent device handle */
};

/* -------------------------------------------------------------------------
 * Software key repeat
 *
 * CGEventPost injected events do not auto-repeat; we simulate it here.
 * Only one key repeats at a time (the most recently pressed non-modifier).
 * ---------------------------------------------------------------------- */

static pthread_t       s_repeat_tid;
static pthread_mutex_t s_repeat_mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  s_repeat_cond = PTHREAD_COND_INITIALIZER;
static uint16_t        s_repeat_key  = 0;
static int             s_repeat_armed = 0; /* 1 when a key is held for repeat */
static int             s_repeat_gen  = 0;  /* bumped on every arm/cancel */

static int is_modifier_cgkey(uint16_t cgkey)
{
	switch (cgkey) {
	case 0x38: case 0x3C: /* left/right shift */
	case 0x3B: case 0x3E: /* left/right control */
	case 0x3A: case 0x3D: /* left/right option */
	case 0x37: case 0x36: /* left/right command */
	case 0x3F:            /* fn */
	case 0x39:            /* caps lock */
		return 1;
	default:
		return 0;
	}
}

/* Read the system key-repeat settings (units of 1/60 s → ms). */
static void get_repeat_settings(long *delay_ms, long *interval_ms)
{
	Boolean ok;
	CFIndex v;

	v = CFPreferencesGetAppIntegerValue(CFSTR("InitialKeyRepeat"),
	    kCFPreferencesAnyApplication, &ok);
	*delay_ms = (ok && v > 0) ? v * 1000 / 60 : 500;

	v = CFPreferencesGetAppIntegerValue(CFSTR("KeyRepeat"),
	    kCFPreferencesAnyApplication, &ok);
	*interval_ms = (ok && v > 0) ? v * 1000 / 60 : 33;
}

static void sleep_ms(long ms)
{
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
	nanosleep(&ts, NULL);
}

static void post_key_repeat(uint16_t cgkey)
{
	CGEventRef ev = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)cgkey, true);
	if (!ev)
		return;
	CGEventSetIntegerValueField(ev, kCGKeyboardEventAutorepeat, 1);
	CGEventSetIntegerValueField(ev, kCGEventSourceUserData, KEYD_EVENT_MARKER);
	CGEventPost(kCGHIDEventTap, ev);
	CFRelease(ev);
}

static void *repeat_thread_fn(void *arg)
{
	long delay_ms, interval_ms;
	(void)arg;
	get_repeat_settings(&delay_ms, &interval_ms);

	for (;;) {
		int gen;
		uint16_t key;

		pthread_mutex_lock(&s_repeat_mtx);
		while (!s_repeat_armed)
			pthread_cond_wait(&s_repeat_cond, &s_repeat_mtx);
		key = s_repeat_key;
		gen = s_repeat_gen;
		pthread_mutex_unlock(&s_repeat_mtx);

		sleep_ms(delay_ms);

		pthread_mutex_lock(&s_repeat_mtx);
		int cancelled = (s_repeat_gen != gen);
		pthread_mutex_unlock(&s_repeat_mtx);
		if (cancelled)
			continue;

		while (1) {
			pthread_mutex_lock(&s_repeat_mtx);
			cancelled = (s_repeat_gen != gen);
			pthread_mutex_unlock(&s_repeat_mtx);
			if (cancelled)
				break;

			post_key_repeat(key);
			sleep_ms(interval_ms);
		}
	}
	return NULL;
}

struct vkbd *vkbd_init(const char *name)
{
	(void)name;
	struct vkbd *v = calloc(1, sizeof *v);

	pthread_t tid;
	pthread_create(&tid, NULL, repeat_thread_fn, NULL);
	pthread_detach(tid);
	s_repeat_tid = tid;

	return v;
}

void free_vkbd(struct vkbd *vkbd)
{
	free(vkbd);
}

/* -------------------------------------------------------------------------
 * Key injection
 * ---------------------------------------------------------------------- */

static void post_key(uint16_t cgkey, int pressed)
{
	CGEventRef ev = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)cgkey,
	                                           pressed ? true : false);
	if (!ev)
		return;

	CGEventSetIntegerValueField(ev, kCGEventSourceUserData, KEYD_EVENT_MARKER);
	CGEventPost(kCGHIDEventTap, ev);
	CFRelease(ev);
}

void vkbd_send_key(const struct vkbd *vkbd, uint8_t code, int state)
{
	(void)vkbd;

	dbg("output %s %s", KEY_NAME(code), state ? "down" : "up");

	/* Route mouse button codes through CGEventPost with button event types. */
	{
		CGEventType btn_type  = 0;
		CGMouseButton btn_num = kCGMouseButtonLeft;

		switch (code) {
		case KEYD_LEFT_MOUSE:
			btn_type = state ? kCGEventLeftMouseDown  : kCGEventLeftMouseUp;
			btn_num  = kCGMouseButtonLeft;
			break;
		case KEYD_RIGHT_MOUSE:
			btn_type = state ? kCGEventRightMouseDown : kCGEventRightMouseUp;
			btn_num  = kCGMouseButtonRight;
			break;
		case KEYD_MIDDLE_MOUSE:
			btn_type = state ? kCGEventOtherMouseDown : kCGEventOtherMouseUp;
			btn_num  = kCGMouseButtonCenter;
			break;
		default:
			btn_type = 0;
			break;
		}

		if (btn_type) {
			CGEventRef cursor = CGEventCreate(NULL);
			CGPoint    pos    = CGEventGetLocation(cursor);
			CFRelease(cursor);

			CGEventRef ev = CGEventCreateMouseEvent(NULL, btn_type, pos, btn_num);
			if (ev) {
				CGEventSetIntegerValueField(ev, kCGEventSourceUserData,
				                            KEYD_EVENT_MARKER);
				CGEventPost(kCGHIDEventTap, ev);
				CFRelease(ev);
			}
			return;
		}
	}

	uint16_t cgkey = keyd_to_cgkey(code);
	if (cgkey == 0xFFFF) {
		dbg("macOS: no CGKeyCode mapping for KEYD code %u", (unsigned)code);
		return;
	}

	post_key(cgkey, state);

	/* Arm or cancel the software repeat timer for non-modifier keys. */
	if (!is_modifier_cgkey(cgkey)) {
		pthread_mutex_lock(&s_repeat_mtx);
		if (state) {
			s_repeat_key   = cgkey;
			s_repeat_armed = 1;
			s_repeat_gen++;
			pthread_cond_signal(&s_repeat_cond);
		} else if (s_repeat_key == cgkey && s_repeat_armed) {
			s_repeat_armed = 0;
			s_repeat_gen++;
		}
		pthread_mutex_unlock(&s_repeat_mtx);
	}
}

/* -------------------------------------------------------------------------
 * Mouse movement
 * ---------------------------------------------------------------------- */

void vkbd_mouse_move(const struct vkbd *vkbd, int x, int y)
{
	(void)vkbd;

	/* Get current pointer position to compute absolute target. */
	CGEventRef cursor_ev = CGEventCreate(NULL);
	CGPoint pos = CGEventGetLocation(cursor_ev);
	CFRelease(cursor_ev);

	pos.x += x;
	pos.y += y;

	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
	                                         pos, kCGMouseButtonLeft);
	if (ev) {
		CGEventSetIntegerValueField(ev, kCGEventSourceUserData,
		                            KEYD_EVENT_MARKER);
		CGEventPost(kCGHIDEventTap, ev);
		CFRelease(ev);
	}
}

void vkbd_mouse_move_abs(const struct vkbd *vkbd, int x, int y)
{
	(void)vkbd;

	/*
	 * keyd passes abs coordinates in a 0–1024 space.
	 * Map to the main display's pixel dimensions.
	 */
	CGDirectDisplayID disp = CGMainDisplayID();
	CGFloat w = (CGFloat)CGDisplayPixelsWide(disp);
	CGFloat h = (CGFloat)CGDisplayPixelsHigh(disp);

	CGPoint pos;
	pos.x = (x / 1024.0) * w;
	pos.y = (y / 1024.0) * h;

	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
	                                         pos, kCGMouseButtonLeft);
	if (ev) {
		CGEventSetIntegerValueField(ev, kCGEventSourceUserData,
		                            KEYD_EVENT_MARKER);
		CGEventPost(kCGHIDEventTap, ev);
		CFRelease(ev);
	}
}

/* -------------------------------------------------------------------------
 * Scroll
 * ---------------------------------------------------------------------- */

void vkbd_mouse_scroll(const struct vkbd *vkbd, int x, int y)
{
	(void)vkbd;

	/*
	 * CGEventCreateScrollWheelEvent2 takes unit (lines vs pixels) and up to
	 * three axes.  We use kCGScrollEventUnitLine for coarse step behaviour
	 * consistent with the Linux uinput REL_WHEEL approach.
	 */
	CGEventRef ev = CGEventCreateScrollWheelEvent2(
	    NULL,
	    kCGScrollEventUnitLine,
	    2,       /* number of axes */
	    y,       /* axis 1: vertical */
	    x,       /* axis 2: horizontal */
	    0);

	if (ev) {
		CGEventSetIntegerValueField(ev, kCGEventSourceUserData,
		                            KEYD_EVENT_MARKER);
		CGEventPost(kCGHIDEventTap, ev);
		CFRelease(ev);
	}
}
