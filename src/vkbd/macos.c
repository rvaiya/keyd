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
 * Link with: -framework CoreGraphics -framework ApplicationServices
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ApplicationServices/ApplicationServices.h>

#include "../keyd.h"
#include "../macos/keycodes.h"

#define KEYD_EVENT_MARKER ((int64_t)0x6B657964ULL)

struct vkbd {
	int _dummy; /* CGEventPost needs no persistent device handle */
};

struct vkbd *vkbd_init(const char *name)
{
	(void)name;
	struct vkbd *v = calloc(1, sizeof *v);
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
