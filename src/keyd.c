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

#ifdef __FreeBSD__
#include <dev/evdev/input.h>
#include <dev/evdev/uinput.h>
#else
#include <linux/input.h>
#include <linux/uinput.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <libudev.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <grp.h>

#include "keys.h"
#include "config.h"
#include "keyboard.h"
#include "keyd.h"
#include "server.h"
#include "vkbd.h"
#include "error.h"

#define VIRTUAL_KEYBOARD_NAME "keyd virtual keyboard"
#define MAX_KEYBOARDS 256

static struct vkbd *vkbd = NULL;

static struct udev *udev;
static struct udev_monitor *udevmon;
uint8_t keystate[KEY_CNT] = { 0 };
static int sigfds[2];
struct keyboard *active_keyboard = NULL;

static struct keyboard *keyboards = NULL;

int debug = 0;

void dbg(const char *fmt, ...)
{
	va_list ap;

	if (!debug)
		return;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void info(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static void udev_type(struct udev_device *dev, int *iskbd, int *ismouse)
{
	if (iskbd)
		*iskbd = 0;
	if (ismouse)
		*ismouse = 0;

	const char *path = udev_device_get_devnode(dev);

	if (!path || !strstr(path, "event"))	/* Filter out non evdev devices. */
		return;

	struct udev_list_entry *prop;
	udev_list_entry_foreach(prop,
				udev_device_get_properties_list_entry(dev))
	{
		if (!strcmp
		    (udev_list_entry_get_name(prop), "ID_INPUT_KEYBOARD")
		    && !strcmp(udev_list_entry_get_value(prop), "1")) {
			if (iskbd)
				*iskbd = 1;
		}

		if (!strcmp
		    (udev_list_entry_get_name(prop), "ID_INPUT_MOUSE")
		    && !strcmp(udev_list_entry_get_value(prop), "1")) {
			if (ismouse)
				*ismouse = 1;
		}
	}
}

static uint32_t evdev_device_id(const char *devnode)
{
	struct input_id info;

	int fd = open(devnode, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	if (ioctl(fd, EVIOCGID, &info) == -1) {
		perror("ioctl");
		exit(-1);
	}

	close(fd);
	return info.vendor << 16 | info.product;
}

static const char *evdev_device_name(const char *devnode)
{
	static char name[256];

	int fd = open(devnode, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), &name) == -1)
		return NULL;

	close(fd);
	return name;
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
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices)
		die("Failed to get device list.");

	*sz = 0;
	udev_list_entry_foreach(ent, devices) {
		int iskbd, ismouse;

		const char *name = udev_list_entry_get_name(ent);;
		struct udev_device *dev = udev_device_new_from_syspath(udev, name);
		const char *path = udev_device_get_devnode(dev);

		udev_type(dev, &iskbd, &ismouse);

		if (iskbd) {
			dbg("Detected keyboard node %s (%s) ismouse: %d",
			    name, evdev_device_name(path), ismouse);

			nodes[*sz] = malloc(strlen(path) + 1);
			strcpy(nodes[*sz], path);
			(*sz)++;
			assert(*sz <= MAX_KEYBOARDS);
		} else if (path) {
			dbg("Ignoring %s (%s)", evdev_device_name(path), path);
		}

		udev_device_unref(dev);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}

static void send_repetitions()
{
	size_t i;

	/* Inefficient, but still reasonably fast (<100us) */
	for (i = 0; i < sizeof keystate / sizeof keystate[0]; i++) {
		if (keystate[i])
			send_key(i, 2);
	}
}

void set_mods(uint16_t mods)
{
	static uint16_t state = 0;
	size_t i;

	uint16_t diff = mods ^ state;

	if (MOD_CTRL & diff)
		send_key(KEY_LEFTCTRL, !!(MOD_CTRL & mods));
	if (MOD_ALT_GR & diff)
		send_key(KEY_RIGHTALT, !!(MOD_ALT_GR & mods));
	if (MOD_SHIFT & diff)
		send_key(KEY_LEFTSHIFT, !!(MOD_SHIFT & mods));
	if (MOD_SUPER & diff)
		send_key(KEY_LEFTMETA, !!(MOD_SUPER & mods));
	if (MOD_ALT & diff)
		send_key(KEY_LEFTALT, !!(MOD_ALT & mods));

	state = mods;
}

/* Block on the given keyboard nodes until no keys are depressed. */
static void await_keyboard_neutrality(char **devs, int n)
{
	int fds[MAX_KEYBOARDS];
	int maxfd = 0;
	int i;

	dbg("Awaiting keyboard neutrality.");
	for (i = 0; i < n; i++) {
		if ((fds[i] = open(devs[i], O_RDONLY | O_NONBLOCK)) < 0)
			die("open");

		if (fds[i] > maxfd)
			maxfd = fds[i];
	}

	/*
	 * There is a race condition here since it is possible for a key down
	 * event to be generated before keyd is launched, in that case we hope a
	 * repeat event is generated within the first 300ms. If we miss the
	 * keydown event and the repeat event is not generated within the first
	 * 300ms it is possible for this to yield a false positive. In practice
	 * this seems to work fine. Given the stateless nature of evdev I am not
	 * aware of a better way to achieve this.
	 */
	while (1) {
		struct timeval tv = {
			.tv_usec = 300000
		};

		struct input_event ev;
		int i;
		fd_set fdset;

		FD_ZERO(&fdset);
		for (i = 0; i < n; i++)
			FD_SET(fds[i], &fdset);

		select(maxfd + 1, &fdset, NULL, NULL, &tv);

		for (i = 0; i < n; i++) {
			if (FD_ISSET(fds[i], &fdset)) {
				while (read(fds[i], &ev, sizeof ev) > 0) {
					if (ev.type == EV_KEY) {
						keystate[ev.code] = ev.value;
						dbg("keystate[%d]: %d", ev.code, ev.value);
					}
				}
			}
		}

		for (i = 0; i < KEY_CNT; i++)
			if (keystate[i])
				break;

		if (i == KEY_CNT)
			break;
	}

	for (i = 0; i < n; i++)
		close(fds[i]);

	dbg("Keyboard neutrality achieved");
}

void reset_vkbd()
{
	uint16_t code;
	for (code = 0; code < KEY_MAX; code++) {
		if (keystate[code])
			send_key(code, 0);
	}
}

void send_key(int code, int state)
{
	keystate[code] = state;
	vkbd_send(vkbd, code, state);
}

void reset_keyboards()
{
	struct keyboard *kbd;

	for (kbd = keyboards; kbd; kbd = kbd->next)
		kbd_reset(kbd);
}

static int manage_keyboard(const char *devnode)
{
	int fd;
	const char *name = evdev_device_name(devnode);
	struct keyboard *kbd;
	struct config *config = NULL;
	uint32_t id;
	uint16_t  vendor_id, product_id;

	/* Don't manage keyd's devices. */
	if (!strcmp(name, VIRTUAL_KEYBOARD_NAME))
		return -1;

	for (kbd = keyboards; kbd; kbd = kbd->next) {
		if (!strcmp(kbd->devnode, devnode)) {
			dbg("Already managing %s.", devnode);
			return -1;
		}
	}

	id = evdev_device_id(devnode);
	vendor_id = id >> 16;
	product_id = id & 0xFFFF;

	config = lookup_config(id);

	if (!config) {
		fprintf(stderr, "No config found for %s (%04x:%04x)\n", evdev_device_name(devnode), vendor_id, product_id);
		return -1;
	}

	if ((fd = open(devnode, O_RDONLY | O_NONBLOCK)) < 0) {
		perror("open");
		exit(1);
	}

	/* Grab the keyboard. */
	if (ioctl(fd, EVIOCGRAB, (void *) 1) < 0) {
		info("Failed to grab %04x:%04x, ignoring...\n", vendor_id, product_id);
		perror("EVIOCGRAB");
		close(fd);
		return -1;
	}

	kbd = calloc(1, sizeof(struct keyboard));
	kbd->fd = fd;

	/* TODO: optimize */
	kbd->config = *config;
	kbd->original_config = *config;

	kbd->layout = config->default_layout;

	strcpy(kbd->devnode, devnode);

	kbd->next = keyboards;
	keyboards = kbd;

	active_keyboard = kbd;
	info("Managing %s (%04x:%04x) (%s)", evdev_device_name(devnode), vendor_id, product_id, config->name);
	return 0;
}

static void scan_keyboards(int wait)
{
	int i, n;
	char *devs[MAX_KEYBOARDS];

	get_keyboard_nodes(devs, &n);
	if (wait)
		await_keyboard_neutrality(devs, n);

	for (i = 0; i < n; i++) {
		manage_keyboard(devs[i]);
		free(devs[i]);
	}
}

/* TODO: optimize */
void reload_config()
{
	struct keyboard *kbd = keyboards;

	while (kbd) {
		struct keyboard *tmp = kbd;

		ioctl(kbd->fd, EVIOCGRAB, (void *) 0);
		close(kbd->fd);
		kbd = kbd->next;
		free(tmp);
	}

	keyboards = NULL;

	free_configs();
	read_config_dir(CONFIG_DIR);
	scan_keyboards(0);
}

static int destroy_keyboard(const char *devnode)
{
	struct keyboard **ent = &keyboards;

	while (*ent) {
		if (!strcmp((*ent)->devnode, devnode)) {
			dbg("Destroying %s", devnode);
			struct keyboard *kbd = *ent;
			*ent = kbd->next;

			/* Attempt to ungrab the the keyboard (assuming it still exists) */
			ioctl(kbd->fd, EVIOCGRAB, (void *) 1);

			close(kbd->fd);
			free(kbd);

			return 1;
		}

		ent = &(*ent)->next;
	}

	return 0;
}

static void monitor_cleanup()
{
	struct termios tinfo;

	tcgetattr(0, &tinfo);
	tinfo.c_lflag |= ECHO;
	tcsetattr(0, TCSANOW, &tinfo);
}

static void panic_check(uint16_t code, int state)
{
	static int n = 0;

	switch (code) {
	case KEY_BACKSPACE:
	case KEY_ENTER:
	case KEY_ESC:
		if (state == 1)
			n++;
		else if (state == 0)
			n--;

		break;
	}

	if (n == 3)
		die("Termination key sequence triggered (backspace+escape+enter), terminating.");
}


static void evdev_monitor_loop(int *fds, int sz)
{
	struct input_event ev;
	fd_set fdset;
	int i;
	struct stat finfo;
	int ispiped;

	struct {
		char name[256];
		uint16_t product_id;
		uint16_t vendor_id;
	} info_table[256];

	fstat(1, &finfo);
	ispiped = finfo.st_mode & S_IFIFO;

	for (i = 0; i < sz; i++) {
		struct input_id info;

		int fd = fds[i];
		if (ioctl(fd, EVIOCGID, &info) == -1) {
			perror("ioctl");
			exit(-1);
		}

		info_table[fd].product_id = info.product;
		info_table[fd].vendor_id = info.vendor;

		if (ioctl
		    (fd, EVIOCGNAME(sizeof(info_table[0].name)),
		     info_table[fd].name) == -1) {
			perror("ioctl");
			exit(-1);
		}
	}

	while (1) {
		int i;
		int maxfd = 1;

		FD_ZERO(&fdset);

		/*
		 * Proactively monitor stdout for pipe closures instead of waiting
		 * for a failed write to generate SIGPIPE.
		 */
		if (ispiped)
			FD_SET(1, &fdset);

		for (i = 0; i < sz; i++) {
			if (maxfd < fds[i])
				maxfd = fds[i];
			FD_SET(fds[i], &fdset);
		}

		select(maxfd + 1, &fdset, NULL, NULL, NULL);

		if (FD_ISSET(1, &fdset) && read(1, NULL, 0) == -1) { /* STDOUT closed. */
			/* Re-enable echo. */
			exit(0);
		}

		for (i = 0; i < sz; i++) {
			int fd = fds[i];

			if (FD_ISSET(fd, &fdset)) {
				while (read(fd, &ev, sizeof(ev)) > 0) {
					if (ev.type == EV_KEY
					    && ev.value != 2) {
						const char *name =
						    keycode_table[ev.code].
						    name;
						if (name) {
							printf("%s\t%04x:%04x\t%s %s\n",
							       info_table[fd].name,
							       info_table[fd].vendor_id,
							       info_table[fd].product_id,
							       name, ev.value == 0 ? "up" : "down");

							fflush(stdout);
						} else
							info("Unrecognized keycode: %d", ev.code);
					} else if (ev.type != EV_SYN) {
						dbg("%s: Event: (%d, %d, %d)", info_table[fd].name, ev.type, ev.value, ev.code);
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

	struct termios tinfo;

	/* Disable terminal echo so keys don't appear twice. */
	tcgetattr(0, &tinfo);
	tinfo.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &tinfo);

	signal(SIGINT, exit);
	signal(SIGTERM, exit);
	atexit(monitor_cleanup);

	get_keyboard_nodes(devnodes, &sz);

	for (i = 0; i < sz; i++) {
		fd = open(devnodes[i], O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			perror("open");
			exit(-1);
		}
		free(devnodes[i]);
		fds[nfds++] = fd;
	}

	evdev_monitor_loop(fds, nfds);

	return 0;
}

static void usr1(int status)
{
	char c = ':';
	write(sigfds[1], &c, 1);
}

/* Relative time in ns. */
long get_time()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return (tv.tv_sec*1E9)+tv.tv_nsec;
}

static void main_loop()
{
	struct keyboard *kbd;
	int monfd;
	int sd;

	long timeout = 0; /* in ns */
	long last_ts = 0;
	struct keyboard *timeout_kbd = NULL;

	nice(-20);

	scan_keyboards(1);

	udev = udev_new();
	udevmon = udev_monitor_new_from_netlink(udev, "udev");

	if (!udev)
		die("Can't create udev.");

	udev_monitor_filter_add_match_subsystem_devtype(udevmon, "input", NULL);
	udev_monitor_enable_receiving(udevmon);

	monfd = udev_monitor_get_fd(udevmon);
	sd = create_server_socket();

	pipe(sigfds);
	signal(SIGUSR1, usr1);

	while (1) {
		int maxfd;
		fd_set fds;
		struct udev_device *dev;
		int ret;

		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(monfd, &fds);
		FD_SET(sd, &fds);
		FD_SET(sigfds[0], &fds);

		maxfd = monfd > sigfds[0] ? monfd : sigfds[0];
		maxfd = sd > maxfd ? sd : maxfd;

		for (kbd = keyboards; kbd; kbd = kbd->next) {
			int fd = kbd->fd;

			maxfd = maxfd > fd ? maxfd : fd;
			FD_SET(fd, &fds);
		}

		tv.tv_sec = (timeout/1E3) / 1E6;
		tv.tv_usec = (long)(timeout/1E3) % (long)1E6;

		if ((ret=select(maxfd + 1, &fds, NULL, NULL, timeout > 0 ? &tv : NULL)) >= 0) {
			long time = get_time();
			long elapsed;

			elapsed = time - last_ts;
			last_ts = time;

			timeout -= elapsed;

			if (timeout_kbd && timeout <= 0) {
				timeout = kbd_process_key_event(timeout_kbd, 0, 0) * 1E6;

				if (timeout <= 0)
					timeout_kbd = NULL;
			}

			if (FD_ISSET(sd, &fds))
				server_process_connections(sd);

			if (FD_ISSET(sigfds[0], &fds)) {
				char c;
				read(sigfds[0], &c, 1);
				info("Received SIGUSR1, reloading config");
				reload_config();
			}

			if (FD_ISSET(monfd, &fds)) {
				int iskbd;
				dev = udev_monitor_receive_device(udevmon);
				const char *devnode = udev_device_get_devnode(dev);
				udev_type(dev, &iskbd, NULL);

				if (devnode && iskbd) {
					const char *action =
					    udev_device_get_action(dev);

					if (!strcmp(action, "add"))
						manage_keyboard(devnode);
					else if (!strcmp(action, "remove"))
						destroy_keyboard(devnode);
					else
						dbg("udev: action %s %s",
						    action, devnode);
				}
				udev_device_unref(dev);
			}


			for (kbd = keyboards; kbd; kbd = kbd->next) {
				int fd = kbd->fd;

				if (FD_ISSET(fd, &fds)) {
					struct input_event ev;

					while (read(fd, &ev, sizeof(ev)) > 0) {
						switch (ev.type) {
						case EV_KEY:
							if (ev.value == 2) {
								/* Wayland and X both ignore repeat events but VTs seem to require them. */
								send_repetitions ();
							} else {
								panic_check(ev.code, ev.value);

								active_keyboard = kbd;
								timeout = kbd_process_key_event(kbd, ev.code, ev.value) * 1E6;

								if (timeout > 0)
									timeout_kbd = kbd;
								else
									timeout_kbd = NULL;
							}
							break;
						case EV_REL: /* Pointer motion events */
							if (ev.code == REL_X)
								vkbd_move_mouse(vkbd, ev.value, 0);
							else if (ev.code == REL_Y)
								vkbd_move_mouse(vkbd, 0, ev.value);

							break;
						case EV_MSC:
						case EV_SYN:
							break;
						default:
							dbg("Unrecognized event: (type: %d, code: %d, value: %d)", ev.type, ev.code, ev.value);
						}
					}
				}
			}
		}
	}
}


static void cleanup()
{
	info("cleaning up and terminating...");

	struct keyboard *kbd = keyboards;
	free_configs();

	while (kbd) {
		struct keyboard *tmp = kbd;
		kbd = kbd->next;
		free(tmp);
	}

	free_vkbd(vkbd);
	udev_unref(udev);
	udev_monitor_unref(udevmon);
	unlink(SOCKET);
}

static void daemonize()
{
	int fd;

	info("Daemonizing...");
	info("Log output will be stored in %s", LOG_FILE);

	fd = open(LOG_FILE, O_CREAT | O_APPEND | O_WRONLY, 0600);

	if (fd < 0) {
		perror("Failed to open log file");
		exit(-1);
	}


	if (fork())
		exit(0);
	if (fork())
		exit(0);

	close(0);
	close(1);
	close(2);

	dup2(fd, 1);
	dup2(fd, 2);
}

static void lock()
{
	int fd;

	if ((fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0600)) == -1) {
		perror("flock open");
		exit(1);
	}

	if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		fprintf(stderr, "ERROR: Another instance of keyd is already running.\n");
		exit(-1);
	}
}

static void chgid()
{
	struct group *g = getgrnam("keyd");

	if (!g) {
		fprintf(stderr, "WARNING: failed to set effective group to \"keyd\" (make sure the group exists)\n");
	} else {
		if (setgid(g->gr_gid)) {
			perror("setgid");
			exit(-1);
		}
	}
}

int main(int argc, char *argv[])
{
	int daemonize_flag = 0;

	if (getenv("KEYD_DEBUG"))
		debug = atoi(getenv("KEYD_DEBUG"));

	dbg("Debug mode enabled.");

	if (argc > 1) {
		if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
			fprintf(stderr, "keyd version: %s (%s)\n", VERSION, GIT_COMMIT_HASH);
			return 0;
		} else if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--daemonize")) {
			daemonize_flag++;
			;;
		} else if (!strcmp(argv[1], "-e") || !strcmp(argv[1], "--expression")) {
			int i;
			int rc = 0;
			const int n = argc - 2;
			argv+=2;

			if (!n) {
				fprintf(stderr, "ERROR: -e must be followed by one or more arguments.\n");
				return -1;
			}



			if (n == 1 && !strcmp(argv[0], "ping"))
				return client_send_message(MSG_PING, "");

			for (i = 0; i < n; i++) {
				const char *mapping = argv[i];

				if (!strcmp(argv[i], "reset")) {
					client_send_message(MSG_RESET, "");
					continue;
				}

				if (client_send_message(MSG_MAPPING, mapping))
				    rc = -1;
			}

			return rc;
			;;
		} else if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--monitor")) {
			return monitor_loop();
		} else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--list")) {
			size_t i;

			for (i = 0; i < KEY_MAX; i++)
				if (keycode_table[i].name) {
					const struct keycode_table_ent *ent
					    = &keycode_table[i];
					printf("%s\n", ent->name);
					if (ent->alt_name)
						printf("%s\n",
						       ent->alt_name);
					if (ent->shifted_name)
						printf("%s\n",
						       ent->shifted_name);
				}
			return 0;
		} else {
			if (strcmp(argv[1], "-h")
			    && strcmp(argv[1], "--help"))
				fprintf(stderr,
					"%s is not a valid option.\n",
					argv[1]);

			fprintf(stderr,
				"Usage: %s [options]\n\n"
				"Options:\n"
				"\t-m, --monitor                                monitor mode\n"
				"\t-e, --expression <mapping> [<mapping>...]    add the supplied mappings to the current config\n"
				"\t-l, --list                                   list all key names\n"
				"\t-d, --daemonize                              fork and start as a daemon\n"
				"\t-v, --version                                print version\n"
				"\t-h, --help                                   print this help message\n", argv[0]);

			return 0;
		}
	}

	chgid();
	lock();

	if (daemonize_flag)
		daemonize();

	signal(SIGINT, exit);
	signal(SIGTERM, exit);
	atexit(cleanup);

	info("Starting keyd v%s (%s).", VERSION, GIT_COMMIT_HASH);
	read_config_dir(CONFIG_DIR);
	vkbd = vkbd_init(VIRTUAL_KEYBOARD_NAME);

	main_loop();
}
