/*
 * keyd - A key remapping daemon.
 *
 * macOS input backend: replaces device.c and evloop.c.
 *
 * Keyboard input is captured via CGEventTap (Quartz Events), which intercepts
 * all keyboard events at the HID level before they reach applications.  The tap
 * runs on a dedicated CFRunLoop thread and writes compact event records into a
 * pipe.  The main thread polls the pipe read-end alongside the IPC socket with
 * the same poll(2) loop structure used by the Linux evloop.
 *
 * Requires: Accessibility permission (System Settings → Privacy & Security →
 * Accessibility) OR running as root.
 *
 * Link with: -framework CoreGraphics -framework ApplicationServices
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ApplicationServices/ApplicationServices.h>

#include "../keyd.h"
#include "keycodes.h"

/* -------------------------------------------------------------------------
 * Globals — exported from evloop.c on Linux; defined here on macOS.
 * ---------------------------------------------------------------------- */

struct device device_table[MAX_DEVICES];
size_t device_table_sz;

/* -------------------------------------------------------------------------
 * Internal state
 * ---------------------------------------------------------------------- */

#define MAX_AUX_FDS 32

static int    s_aux_fds[MAX_AUX_FDS];
static size_t s_nr_aux_fds;

/* Pipe: tap thread writes, main thread reads (non-blocking). */
static int s_event_pipe[2] = {-1, -1};

/*
 * Tag injected events so the tap can recognise and pass them through.
 * Written into kCGEventSourceUserData on every event we post.
 */
#define KEYD_EVENT_MARKER ((int64_t)0x6B657964ULL)  /* 'k','e','y','d' */

/* -------------------------------------------------------------------------
 * Modifier press/release inference (kCGEventFlagsChanged)
 *
 * kCGEventFlagsChanged carries a new combined-flags word, not a direction.
 * We infer press vs. release per-key using a per-CGKeyCode latch array.
 * ---------------------------------------------------------------------- */

static uint8_t s_mod_down[128]; /* indexed by CGKeyCode */

static CGEventFlags modifier_flag_bit(uint16_t cgkey)
{
	switch (cgkey) {
	case 0x38: case 0x3C: return kCGEventFlagMaskShift;
	case 0x3B: case 0x3E: return kCGEventFlagMaskControl;
	case 0x3A: case 0x3D: return kCGEventFlagMaskAlternate;
	case 0x37: case 0x36: return kCGEventFlagMaskCommand;
	case 0x39:            return kCGEventFlagMaskAlphaShift;
	case 0x3F:            return kCGEventFlagMaskSecondaryFn;
	default:              return 0;
	}
}

static int flags_changed_pressed(uint16_t cgkey, CGEventFlags new_flags)
{
	int pressed;

	if (cgkey >= 128)
		return 0;

	CGEventFlags bit = modifier_flag_bit(cgkey);

	if (bit) {
		int flag_set = (new_flags & bit) != 0;
		int was_down = s_mod_down[cgkey];
		/*
		 * Press:   flag just became set AND we weren't tracking this key
		 *          (handles simultaneous L/R modifiers of the same type).
		 * Release: anything else (flag cleared, or key was already tracked).
		 */
		pressed = (flag_set && !was_down) ? 1 : 0;
	} else {
		pressed = !s_mod_down[cgkey];
	}

	s_mod_down[cgkey] = (uint8_t)pressed;
	return pressed;
}

/* -------------------------------------------------------------------------
 * Compact event record written through the pipe
 * (sizeof == 3, well within PIPE_BUF → atomic writes guaranteed)
 * ---------------------------------------------------------------------- */

struct raw_event {
	uint16_t cgkey;
	uint8_t  pressed;
};

/* -------------------------------------------------------------------------
 * CGEventTap callback
 * ---------------------------------------------------------------------- */

static CGEventRef tap_callback(CGEventTapProxy proxy,
				CGEventType type,
				CGEventRef event,
				void *ctx)
{
	struct raw_event raw;

	(void)proxy;

	/* The tap can be disabled by macOS on timeout; re-enable it. */
	if (type == kCGEventTapDisabledByTimeout ||
	    type == kCGEventTapDisabledByUserInput) {
		CGEventTapEnable(*(CFMachPortRef *)ctx, true);
		return event;
	}

	/* Let events we injected ourselves pass through untouched. */
	if (CGEventGetIntegerValueField(event, kCGEventSourceUserData) ==
	    KEYD_EVENT_MARKER)
		return event;

	raw.cgkey = (uint16_t)CGEventGetIntegerValueField(
	    event, kCGKeyboardEventKeycode);

	if (type == kCGEventKeyDown) {
		raw.pressed = 1;
	} else if (type == kCGEventKeyUp) {
		raw.pressed = 0;
	} else if (type == kCGEventFlagsChanged) {
		CGEventFlags flags = CGEventGetFlags(event);
		raw.pressed = (uint8_t)flags_changed_pressed(raw.cgkey, flags);
	} else {
		return event;
	}

	/*
	 * Non-blocking write.  If the pipe buffer is momentarily full we lose
	 * one event, which is far preferable to blocking the tap thread (which
	 * triggers a timeout-disable cascade).
	 */
	write(s_event_pipe[1], &raw, sizeof raw);

	return NULL; /* suppress the original event */
}

/* -------------------------------------------------------------------------
 * CGEventTap thread
 * ---------------------------------------------------------------------- */

static void *tap_thread(void *arg)
{
	(void)arg;
	static CFMachPortRef s_port; /* static lifetime; set before callback fires */

	CGEventMask mask =
	    CGEventMaskBit(kCGEventKeyDown)      |
	    CGEventMaskBit(kCGEventKeyUp)        |
	    CGEventMaskBit(kCGEventFlagsChanged);

	/*
	 * Pass &s_port as context so the callback can call CGEventTapEnable()
	 * if macOS disables the tap on timeout.  s_port is assigned before
	 * CGEventTapEnable() is called, so it is valid the first time the
	 * callback fires.
	 */
	s_port = CGEventTapCreate(
	    kCGHIDEventTap,
	    kCGHeadInsertEventTap,
	    kCGEventTapOptionDefault,
	    mask,
	    tap_callback,
	    &s_port);

	if (!s_port) {
		fprintf(stderr,
		    "keyd: failed to create CGEventTap.\n"
		    "      Grant Accessibility permission to keyd in:\n"
		    "      System Settings → Privacy & Security → Accessibility\n"
		    "      (or run keyd as root)\n");
		exit(1);
	}

	CFRunLoopSourceRef src =
	    CFMachPortCreateRunLoopSource(kCFAllocatorDefault, s_port, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);
	CGEventTapEnable(s_port, true);

	keyd_log("keyd: CGEventTap active\n");

	CFRunLoopRun();

	CFRelease(src);
	CFRelease(s_port);
	return NULL;
}

/* -------------------------------------------------------------------------
 * Panic-sequence check (mirrors evloop.c on Linux)
 * ---------------------------------------------------------------------- */

static void panic_check(uint8_t code, uint8_t pressed)
{
	static uint8_t enter, backspace, escape;
	switch (code) {
	case KEYD_ENTER:     enter     = pressed; break;
	case KEYD_BACKSPACE: backspace = pressed; break;
	case KEYD_ESC:       escape    = pressed; break;
	}
	if (backspace && enter && escape)
		die("panic sequence detected");
}

/* -------------------------------------------------------------------------
 * Device interface (declared in device.h)
 *
 * We expose one synthetic device whose fd is the event-pipe read-end.
 * ---------------------------------------------------------------------- */

int device_scan(struct device devices[MAX_DEVICES])
{
	assert(s_event_pipe[0] != -1);

	memset(&devices[0], 0, sizeof devices[0]);
	devices[0].fd           = s_event_pipe[0];
	devices[0].grabbed      = 1;
	devices[0].capabilities = CAP_KEYBOARD | CAP_KEY;
	devices[0].is_virtual   = 0;
	strncpy(devices[0].id,   "0000:0000:00000001", sizeof devices[0].id   - 1);
	strncpy(devices[0].name, "CGEventTap",         sizeof devices[0].name - 1);

	return 1;
}

int device_grab(struct device *dev)
{
	(void)dev;
	dev->grabbed = 1;
	return 0; /* CGEventTap already captures all keyboards globally */
}

int device_ungrab(struct device *dev)
{
	(void)dev;
	dev->grabbed = 0;
	return 0;
}

struct device_event *device_read_event(struct device *dev)
{
	static struct device_event devev;
	struct raw_event raw;

	(void)dev;

	if (read(s_event_pipe[0], &raw, sizeof raw) != (ssize_t)sizeof raw)
		return NULL; /* EAGAIN or partial read */

	uint8_t keyd_code = cgkey_to_keyd_code(raw.cgkey);
	if (!keyd_code) {
		dbg("macOS: unmapped CGKeyCode 0x%02x", (unsigned)raw.cgkey);
		return NULL;
	}

	devev.type    = DEV_KEY;
	devev.code    = keyd_code;
	devev.pressed = raw.pressed;

	dbg2("key %s %s", KEY_NAME(devev.code), devev.pressed ? "down" : "up");

	return &devev;
}

void device_set_led(const struct device *dev, int led, int state)
{
	(void)dev;
	(void)led;
	(void)state;
	/* LED control is not available via CGEventTap. */
}

int devmon_create(void)
{
	int fds[2];
	if (pipe(fds) < 0) { perror("devmon_create"); exit(-1); }
	close(fds[1]); /* write-end never used; read-end blocks poll() forever */
	return fds[0];
}

int devmon_read_device(int fd, struct device *dev)
{
	(void)fd;
	(void)dev;
	return -1; /* hotplug not supported in this backend */
}

/* -------------------------------------------------------------------------
 * evloop / evloop_add_fd  (declared in keyd.h, defined in evloop.c on Linux)
 * ---------------------------------------------------------------------- */

void evloop_add_fd(int fd)
{
	assert(s_nr_aux_fds < MAX_AUX_FDS);
	s_aux_fds[s_nr_aux_fds++] = fd;
}

static long get_time_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

int evloop(int (*event_handler)(struct event *ev))
{
	pthread_t tid;
	struct event ev;
	size_t i;
	int timeout = 0;

	if (pipe(s_event_pipe) < 0) { perror("evloop: pipe"); exit(-1); }
	if (fcntl(s_event_pipe[0], F_SETFL, O_NONBLOCK) < 0) {
		perror("evloop: fcntl O_NONBLOCK");
		exit(-1);
	}

	device_table_sz = device_scan(device_table);

	/* Notify the event handler that the synthetic device was added. */
	ev.type = EV_DEV_ADD;
	ev.dev  = &device_table[0];
	event_handler(&ev);

	/* Start the CGEventTap on a background thread. */
	if (pthread_create(&tid, NULL, tap_thread, NULL) != 0) {
		perror("evloop: pthread_create");
		exit(-1);
	}
	pthread_detach(tid);

	/* Main poll loop. */
	while (1) {
		struct pollfd pfds[1 + MAX_AUX_FDS];
		int nfds = 0;
		long start_time, elapsed;

		pfds[nfds].fd     = s_event_pipe[0];
		pfds[nfds].events = POLLIN;
		nfds++;

		for (i = 0; i < s_nr_aux_fds; i++) {
			pfds[nfds].fd     = s_aux_fds[i];
			pfds[nfds].events = POLLIN | POLLERR;
			nfds++;
		}

		start_time = get_time_ms();
		poll(pfds, nfds, timeout > 0 ? timeout : -1);
		ev.timestamp = (int)get_time_ms();
		elapsed      = ev.timestamp - (int)start_time;

		if (timeout > 0 && elapsed >= timeout) {
			ev.type  = EV_TIMEOUT;
			ev.dev   = NULL;
			ev.devev = NULL;
			timeout  = event_handler(&ev);
		} else {
			timeout -= (int)elapsed;
			if (timeout < 0)
				timeout = 0;
		}

		/* Device events from the CGEventTap thread. */
		if (pfds[0].revents & POLLIN) {
			struct device_event *devev;
			while ((devev = device_read_event(&device_table[0]))) {
				panic_check(devev->code, devev->pressed);
				ev.type  = EV_DEV_EVENT;
				ev.devev = devev;
				ev.dev   = &device_table[0];
				timeout  = event_handler(&ev);
			}
		}

		/* Auxiliary fd events (IPC socket, etc.). */
		for (i = 0; i < s_nr_aux_fds; i++) {
			short revents = pfds[1 + i].revents;
			if (revents) {
				ev.type = (revents & POLLERR) ? EV_FD_ERR
							       : EV_FD_ACTIVITY;
				ev.fd   = s_aux_fds[i];
				timeout = event_handler(&ev);
			}
		}
	}

	return 0;
}
