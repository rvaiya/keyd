/* Copyright © 2019 Raheman Vaiya.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <libudev.h>
#include <stdint.h>
#include <stdarg.h>
#include <linux/uinput.h>
#include <stdlib.h>
#include <fcntl.h>
#include "keys.h"
#include "config.h"

#define UINPUT_DEVICE_NAME "keyd virtual keyboard"
#define MAX_KEYBOARDS 256
#define LOCK_FILE "/var/lock/keyd.lock"
#define LOG_FILE "/var/log/keyd.log" //Only used when running as a daemon.

#ifdef DEBUG
	#define dbg(fmt, ...) warn("%s:%d: "fmt, __FILE__, __LINE__, ## __VA_ARGS__)
#else
	#define dbg(...)
#endif

static int ufd = -1;

static struct udev *udev;
static struct udev_monitor *udevmon;
static uint8_t keystate[KEY_CNT] = {0};

//Active keyboard state.
struct keyboard {
	int fd;
	char devnode[256];

	struct keyboard_config *cfg;

	struct keyboard *next;
};

static struct keyboard *keyboards = NULL;

static void warn(char *fmt, ...)
{
	va_list args; 
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static void _die(char *fmt, ...)
{
	va_list args; 
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(-1);
}

#define die(fmt, ...) _die("%s:%d: "fmt, __FILE__, __LINE__, ## __VA_ARGS__)

static int is_keyboard(struct udev_device *dev) 
{
	int is_keyboard = 0;

	const char *path = udev_device_get_devnode(dev);
	if(!path || !strstr(path, "event")) //Filter out non evdev devices.
		return 0;

	struct udev_list_entry *prop;
	udev_list_entry_foreach(prop, udev_device_get_properties_list_entry(dev)) {
		//Some mice can also send keypresses, ignore these
		if(!strcmp(udev_list_entry_get_name(prop), "ID_INPUT_MOUSE") &&
		   !strcmp(udev_list_entry_get_value(prop), "1")) {
			return 0;
		}

		if(!strcmp(udev_list_entry_get_name(prop), "ID_INPUT_KEYBOARD") &&
		   !strcmp(udev_list_entry_get_value(prop), "1")) {
			is_keyboard = 1;
		}
	}

	return is_keyboard;
}

static void get_keyboard_nodes(char *nodes[MAX_KEYBOARDS], int *sz) 
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *ent;

	udev = udev_new();
	if (!udev)
		die("Cannot create udev context.");

	enumerate = udev_enumerate_new(udev);
	if (!enumerate)
		die("Cannot create enumerate context.");

	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices)
		die("Failed to get device list.");

	*sz = 0;
	udev_list_entry_foreach(ent, devices) {
		const char *name = udev_list_entry_get_name(ent);;
		struct udev_device *dev = udev_device_new_from_syspath(udev, name);
		const char *path = udev_device_get_devnode(dev);

		if(is_keyboard(dev)) {
			nodes[*sz] = malloc(strlen(path)+1);
			strcpy(nodes[*sz], path);
			(*sz)++;
			assert(*sz <= MAX_KEYBOARDS);
		}

		udev_device_unref(dev);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}

static int create_uinput_fd() 
{
	size_t i;
	struct uinput_setup usetup;

	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		perror("open");
		exit(-1);
	}

	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	for(i = 0;i < sizeof keycode_strings/sizeof keycode_strings[0];i++) {
		if(keycode_strings[i])
			ioctl(fd, UI_SET_KEYBIT, i);
	}

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x046d;
	usetup.id.product = 0xc52b;
	strcpy(usetup.name, UINPUT_DEVICE_NAME);

	ioctl(fd, UI_DEV_SETUP, &usetup);
	ioctl(fd, UI_DEV_CREATE);

	return fd;
}

static void syn()
{
	static struct input_event ev = {
		.type = EV_SYN,
		.code = 0,
		.value = 0,
	};

	write(ufd, &ev, sizeof(ev));
}

static void send_repetitions()
{
	size_t i;
	struct input_event ev = {
		.type = EV_KEY,
		.value = 2,
		.time.tv_sec = 0,
		.time.tv_usec = 0
	};

	//Inefficient, but still reasonably fast (<100us)
	for(i = 0; i < sizeof keystate / sizeof keystate[0];i++) {
		if(keystate[i]) {
			ev.code = i;
			write(ufd, &ev, sizeof(ev));
			syn();
		}
	}
}

static void send_key(uint16_t code, int is_pressed)
{
	keystate[code] = is_pressed;
	struct input_event ev;

	ev.type = EV_KEY;
	ev.code = code;
	ev.value = is_pressed;
	ev.time.tv_sec = 0;
	ev.time.tv_usec = 0;

	write(ufd, &ev, sizeof(ev));

	syn();
}

static void send_mods(uint16_t mods, int pressed)
{
	if(mods & MOD_CTRL)
		send_key(KEY_LEFTCTRL, pressed);
	if(mods & MOD_SHIFT)
		send_key(KEY_LEFTSHIFT, pressed);
	if(mods & MOD_SUPER)
		send_key(KEY_LEFTMETA, pressed);
	if(mods & MOD_ALT)
		send_key(KEY_LEFTALT, pressed);
	if(mods & MOD_ALT_GR)
		send_key(KEY_RIGHTALT, pressed);
}

static void send_keyseq(uint32_t keyseq, int pressed)
{
	uint16_t mods = keyseq >> 16;
	uint16_t code = keyseq;

	send_mods(mods, pressed);
	send_key(code, pressed);
}

//Where the magic happens
static void process_event(struct keyboard *kbd, struct input_event *ev)
{
	static struct keyboard *current_kbd = NULL;
	static uint16_t oneshotmods = 0;
	static struct key_descriptor *last_pressed = NULL;
	static uint8_t layer = 0;
	static uint8_t main_layer = 0;
	static struct key_descriptor *pressed_keys[KEY_CNT];
	static uint8_t oneshotlayer = 0;

	struct key_descriptor *desc;
	int code = ev->code;
	int pressed = ev->value;
	uint32_t keypressed = 0;

	if(current_kbd != kbd) { //Reset state when switching keyboards.
		main_layer = 0;
		layer = 0;
		current_kbd = kbd;
	}

	if(ev->type != EV_KEY)
		return;

	//Wayland and X both ignore repeat events but VTs seem to require them.
	if(pressed == 2) {
		send_repetitions();
		return;
	}

	//Ensure that key descriptors are consistent accross key up/down event pairs.
	//This is necessary to accomodate layer changes midkey.
	if(pressed_keys[code]) 
		desc = pressed_keys[code];
	else
		desc = &kbd->cfg->layers[layer][code];

	if(pressed)
		pressed_keys[code] = desc;
	else
		pressed_keys[code] = NULL;

	if(oneshotlayer) {
		layer = main_layer;
		oneshotlayer = 0;
	}

	switch(desc->action) {
		uint32_t keyseq;

	case ACTION_KEYSEQ:
		keyseq = desc->arg.keyseq;
		send_keyseq(keyseq, pressed);
		keypressed = keyseq;

		break;
	case ACTION_LAYER:
		if(pressed)
			layer = desc->arg.layer;
		else
			layer = main_layer;
		break;
	case ACTION_LAYER_ONESHOT:
		if(pressed) {
			layer = desc->arg.layer;
		} else {
			if(last_pressed == desc) //If tapped
				oneshotlayer++;
			else
				layer = main_layer;
		}
		break;
	case ACTION_LAYER_TOGGLE:
		if(pressed) {
			main_layer = desc->arg.layer;
			layer = main_layer;
		}
		break;
	case ACTION_ONESHOT:
		if(pressed)
			send_mods(desc->arg.mods, 1);
		else if(last_pressed == desc) //If no key has been interposed make the modifiers transient
			oneshotmods |= desc->arg.mods;
		else //otherwise treat as a normal modifier.
			send_mods(desc->arg.mods, 0);

		break;
	case ACTION_DOUBLE_MODIFIER:
		if(pressed)
			send_mods(desc->arg.mods, 1);
		else {
			send_mods(desc->arg.mods, 0);
			if(last_pressed == desc) {
				send_keyseq(desc->arg2.keyseq, 1);
				send_keyseq(desc->arg2.keyseq, 0);
				keypressed = desc->arg2.keyseq;
			}
		}
		break;
	case ACTION_DOUBLE_LAYER:
		if(pressed) {
			layer = desc->arg.layer;
		} else {
			layer = main_layer;
			if(last_pressed == desc) {
				send_keyseq(desc->arg2.keyseq, 1);
				send_keyseq(desc->arg2.keyseq, 0);
				keypressed = desc->arg2.keyseq;
			}
		}
		break;
	default:
		dbg("Ignoring action %d generated by %d", desc->action, code);
		break;
	}

	if(keypressed && !ISMOD(keypressed & 0xFFFF)) {
		send_mods(oneshotmods, 0);
		oneshotmods = 0;
	}

	if(pressed)
		last_pressed = desc;
}

static const char *evdev_device_name(const char *devnode)
{
	static char name[256];

	int fd = open(devnode, O_RDONLY);
	if(fd < 0) {
		perror("open");
		exit(-1);
	}

	if(ioctl(fd, EVIOCGNAME(sizeof(name)), &name) == -1) {
		perror("ioctl");
		exit(-1);
	}

	close(fd);
	return name;
}

//Block on the given keyboard nodes until no keys are depressed.
void await_keyboard_neutrality(char **devs, int n)
{
	int fds[MAX_KEYBOARDS];
	int maxfd = 0;
	int i;

	dbg("Awaiting keyboard neutrality.");
	for(i = 0;i < n;i++) {
		if((fds[i] = open(devs[i], O_RDONLY | O_NONBLOCK)) < 0)
			die("open");

		if(fds[i] > maxfd)
			maxfd = fds[i];
	}

	//There is a race condition here since it is possible for a key down
	//event to be generated before keyd is launched, in that case we hope a
	//repeat event is generated within the first 300ms. If we miss the
	//keydown event and the repeat event is not generated within the first
	//300ms it is possible for this to yield a false positive. In practice
	//this seems to work fine. Given the stateless nature of evdev I am not 
	//aware of a better way to achieve this.

	while(1) {
		struct timeval tv = {
			.tv_usec = 300000
		};

		struct input_event ev;
		int i;
		fd_set fdset;

		FD_ZERO(&fdset);
		for(i = 0;i < n;i++)
			FD_SET(fds[i], &fdset);

		select(maxfd+1, &fdset, NULL, NULL, &tv);

		for(i = 0;i < n;i++) {
			if(FD_ISSET(fds[i], &fdset)) {
				while(read(fds[i], &ev, sizeof ev) > 0) {
					if(ev.type == EV_KEY) {
						keystate[ev.code] = ev.value;
						dbg("keystate[%d]: %d", ev.code, ev.value);
					}
				}
			}
		}

		for(i = 0;i < KEY_CNT;i++)
			if(keystate[i])
				break;

		if(i == KEY_CNT)
			break;
	}

	for(i = 0;i < n;i++)
		close(fds[i]);

	dbg("Keyboard neutrality achieved");
}

static int manage_keyboard(const char *devnode)
{
	int fd;
	const char *name = evdev_device_name(devnode);
	struct keyboard *kbd;
	struct keyboard_config *cfg = NULL;
	struct keyboard_config *default_cfg = NULL;

	if(!strcmp(name, UINPUT_DEVICE_NAME)) //Don't manage virtual keyboard
		return 0;

	for(cfg = configs;cfg;cfg = cfg->next) {
		if(!strcmp(cfg->name, "default"))
			default_cfg = cfg;

		if(!strcmp(cfg->name, name))
			break;
	}

	if(!cfg) {
		if(default_cfg) {
			warn("No config found for %s (%s), falling back to default.cfg", name, devnode);
			cfg = default_cfg;
		} else {
			//Don't manage keyboards for which there is no configuration.
			warn("No config found for %s (%s), ignoring", name, devnode);
			return 0;
		}
	}

	if((fd = open(devnode, O_RDONLY | O_NONBLOCK)) < 0) {
		perror("open");
		exit(1);
	}

	kbd = malloc(sizeof(struct keyboard));
	kbd->fd = fd;
	kbd->cfg = cfg;

	strcpy(kbd->devnode, devnode);

	//Grab the keyboard.
	if(ioctl(fd, EVIOCGRAB, (void *)1) < 0) {
		perror("EVIOCGRAB"); 
		exit(-1);
	}

	kbd->next = keyboards;
	keyboards = kbd;

	warn("Managing %s", evdev_device_name(devnode));
	return 1;
}

static int destroy_keyboard(const char *devnode)
{
	struct keyboard **ent = &keyboards;

	while(*ent) {
		if(!strcmp((*ent)->devnode, devnode)) {
			dbg("Destroying %s", devnode);
			struct keyboard *kbd = *ent;
			*ent = kbd->next;

			//Attempt to ungrab the the keyboard (assuming it still exists)
			if(ioctl(kbd->fd, EVIOCGRAB, (void *)1) < 0) {
				perror("EVIOCGRAB"); 
			}

			close(kbd->fd);
			free(kbd);

			return 1;
		}

		ent = &(*ent)->next;
	}

	return 0;
}

static void evdev_monitor_loop(int *fds, int sz) 
{
	struct input_event ev;
	fd_set fdset;
	int i;
	char names[256][256];

	for(i = 0;i < sz;i++) {
		int fd = fds[i];
		if(ioctl(fd, EVIOCGNAME(sizeof(names[fd])), names[fd]) == -1) {
			perror("ioctl");
			exit(-1);
		}
	}

	while(1) {
		int i;
		int maxfd = fds[0];

		FD_ZERO(&fdset);
		for(i = 0;i < sz;i++) {
			if(maxfd < fds[i]) maxfd = fds[i];
			FD_SET(fds[i], &fdset);
		}

		select(maxfd+1, &fdset, NULL, NULL, NULL);

		for(i = 0;i < sz;i++) {
			int fd = fds[i];
			if(FD_ISSET(fd, &fdset)) {
				while(read(fd, &ev, sizeof(ev)) > 0) {
					if(ev.type == EV_KEY && ev.value != 2) {
						fprintf(stderr, "%s: %s %s\n",
							names[fd],
							keycode_strings[ev.code],
							ev.value == 0 ? "up" : "down");
					}
				}
			}
		}
	}
}

static int monitor_loop() 
{
	char *devnodes[256];
	int sz, i;
	int fd = -1;
	int fds[256];
	int nfds = 0;

	get_keyboard_nodes(devnodes, &sz);

	for(i = 0;i < sz;i++) {
		fd = open(devnodes[i], O_RDONLY | O_NONBLOCK);
		if(fd < 0) {
			perror("open");
			exit(-1);
		}
		fds[nfds++] = fd;
	}

	evdev_monitor_loop(fds, nfds);

	return 0;
}

static void main_loop() 
{
	struct keyboard *kbd;
	int monfd;

	int i, n;
	char *devs[MAX_KEYBOARDS];

	get_keyboard_nodes(devs, &n);
	await_keyboard_neutrality(devs, n);

	for(i = 0;i < n;i++) {
		manage_keyboard(devs[i]);
		free(devs[i]);
	}

	udev = udev_new();
	udevmon = udev_monitor_new_from_netlink(udev, "udev");

	if (!udev)
		die("Can't create udev.");

	udev_monitor_filter_add_match_subsystem_devtype(udevmon, "input", NULL);
	udev_monitor_enable_receiving(udevmon);

	monfd = udev_monitor_get_fd(udevmon);

	int exit = 0;
	while(!exit) {
		int maxfd;
		fd_set fds;
		struct udev_device *dev;

		FD_ZERO(&fds);
		FD_SET(monfd, &fds);

		maxfd = monfd;

		for(kbd = keyboards;kbd;kbd=kbd->next) {
			int fd = kbd->fd;

			maxfd = maxfd > fd ? maxfd : fd;
			FD_SET(fd, &fds);
		}

		if(select(maxfd+1, &fds, NULL, NULL, NULL) > 0) {
			if(FD_ISSET(monfd, &fds)) {
				dev = udev_monitor_receive_device(udevmon);

				const char *devnode = udev_device_get_devnode(dev);

				if(devnode && is_keyboard(dev)) {
					const char *action = udev_device_get_action(dev);

					if(!strcmp(action, "add"))
						manage_keyboard(devnode);
					else if(!strcmp(action, "remove"))
						destroy_keyboard(devnode);
				}
				udev_device_unref(dev);
			}


			for(kbd = keyboards;kbd;kbd=kbd->next) {
				int fd = kbd->fd;

				if(FD_ISSET(fd, &fds)) {
					struct input_event ev;

					while(read(fd, &ev, sizeof(ev)) > 0) {
						process_event(kbd, &ev);
					}
				}
			}
		}
	}
}


static void cleanup()
{
	struct keyboard *kbd = keyboards;
	config_free();

	while(kbd) {
		struct keyboard *tmp = kbd;
		kbd = kbd->next;
		free(tmp);
	}

	udev_unref(udev);
	udev_monitor_unref(udevmon);
}

static void lock()
{
	int fd;

	if((fd=open(LOCK_FILE, O_CREAT | O_RDWR, 0600)) == -1) {
		perror("flock open");
		exit(1);
	}

	if(flock(fd, LOCK_EX | LOCK_NB) == -1)
		die("Another instance of keyd is already running.");
}


static void exit_signal_handler(int sig)
{
	warn("%s received, cleaning up and terminating...", sig == SIGINT ? "SIGINT" : "SIGTERM");

	cleanup();
	exit(0);
}

void daemonize()
{
	int fd = open(LOG_FILE, O_APPEND|O_WRONLY);

	warn("Daemonizing.");
	warn("Log output will be stored in %s", LOG_FILE);

	if(fork()) exit(0);
	if(fork()) exit(0);

	close(0);
	close(1);
	close(2);

	dup2(fd, 1);
	dup2(fd, 2);
}

int main(int argc, char *argv[])
{
	if(argc > 1) {
		if(!strcmp(argv[1], "-v")) {
			fprintf(stderr, "keyd version: %s (%s)\n", VERSION, GIT_COMMIT_HASH);
			return 0;
		} else if(!strcmp(argv[1], "-m")) {
			return monitor_loop();
		} else if(!strcmp(argv[1], "-l")) {
			size_t i;
			for(i = 0; i < sizeof(keycode_strings)/sizeof(keycode_strings[0]);i++)
				if(keycode_strings[i])
					printf("%s\n", keycode_strings[i]);
			return 0;
		}
	}

	lock();

	signal(SIGINT, exit_signal_handler);
	signal(SIGTERM, exit_signal_handler);

	if(argc > 1 && !strcmp(argv[1], "-d"))
		daemonize();

	warn("Starting keyd.");
	config_generate();
	ufd = create_uinput_fd();

	main_loop();
}
