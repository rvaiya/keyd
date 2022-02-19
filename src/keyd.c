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
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/inotify.h>

#include "keys.h"
#include "config.h"
#include "keyboard.h"
#include "keyd.h"
#include "server.h"
#include "vkbd.h"
#include "error.h"

#define VIRTUAL_KEYBOARD_NAME "keyd virtual keyboard"

static struct vkbd *vkbd = NULL;

uint8_t keystate[MAX_KEYS] = { 0 };
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

static void send_repetitions()
{
	size_t i;

	/* Inefficient, but still reasonably fast (<100us) */
	for (i = 0; i < sizeof keystate / sizeof keystate[0]; i++) {
		if (keystate[i])
			send_key(i, 2);
	}
}

void reset_vkbd()
{
	size_t code;
	for (code = 0; code < MAX_KEYS; code++) {
		if (keystate[code])
			send_key(code, 0);
	}
}

void send_key(int code, int state)
{
	keystate[code] = state;
	vkbd_send_key(vkbd, code, state);
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
	const char *devname;
	const char *config_path;

	struct keyboard *kbd;
	struct config *config = NULL;
	uint16_t  vendor_id, product_id;

	if (!(devname = evdev_device_name(devnode))) {
		fprintf(stderr, "WARNING: Failed to obtain device info for %s, skipping..\n", devnode);
		return -1;
	}

	/* Don't manage keyd's devices. */
	if (!strcmp(devname, VIRTUAL_KEYBOARD_NAME))
		return -1;

	for (kbd = keyboards; kbd; kbd = kbd->next) {
		if (!strcmp(kbd->devnode, devnode)) {
			dbg("Already managing %s.", devnode);
			return -1;
		}
	}

	if (evdev_device_id(devnode, &vendor_id, &product_id) < 0) {
		fprintf(stderr, "WARNING: Failed to obtain device id for %s (%s)\n", devnode, devname);
		return -1;
	}

	config_path = config_find_path(CONFIG_DIR, vendor_id, product_id);
	if (!config_path) {
		fprintf(stderr, "No config found for %s (%04x:%04x)\n", devname, vendor_id, product_id);
		return -1;
	}

	kbd = calloc(1, sizeof(struct keyboard));
	if (config_parse(&kbd->config, config_path) < 0) {
		fprintf(stderr, "ERROR: failed to parse %s\n", config_path);
		free(kbd);
		return -1;
	}

	if ((fd = open(devnode, O_RDONLY | O_NONBLOCK)) < 0) {
		fprintf(stderr, "WARNING: Failed to open %s (%s)\n", devnode, devname);
		perror("open");

		free(kbd);
		return -1;
	}

	if (evdev_grab_keyboard(fd) < 0) {
		info("Failed to grab %04x:%04x, ignoring...\n", vendor_id, product_id);
		free(kbd);

		return -1;
	}

	kbd->fd = fd;

	kbd->original_config = kbd->config;
	kbd->layout = kbd->config.default_layout;

	strcpy(kbd->devnode, devnode);

	kbd->next = keyboards;
	keyboards = kbd;

	active_keyboard = kbd;

	info("Managing %s (%04x:%04x) (%s)", devname, vendor_id, product_id, config_path);
	return 0;
}

static void scan_keyboards()
{
	int i, n;
	char *devs[MAX_KEYBOARDS];

	evdev_get_keyboard_nodes(devs, &n);

	for (i = 0; i < n; i++)
		manage_keyboard(devs[i]);
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

	scan_keyboards();
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

static void panic_check(uint8_t code, int state)
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
					if (ev.code >= MAX_KEYS) {
						info("Out of bounds evdev keycode: %d", ev.code);
						continue;
					}

					if (ev.type == EV_KEY && ev.value != 2) {
						const char *name = keycode_table[ev.code].name;
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

	evdev_get_keyboard_nodes(devnodes, &sz);

	for (i = 0; i < sz; i++) {
		fd = open(devnodes[i], O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			perror("open");
			exit(-1);
		}
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

static int create_inotify_fd()
{
	int fd = inotify_init1(IN_NONBLOCK);
	if (fd < 0) {
		perror("inotify");
		exit(-1);
	}

	int wd = inotify_add_watch(fd, "/dev/input", IN_CREATE | IN_DELETE);
	if (wd < 0) {
		perror("inotify");
		exit(-1);
	}

	return fd;
}

static void process_inotify_events(int fd)
{
	int len;
	char buf[4096];

	while (1) {
		struct inotify_event *ev;
		if ((len = read(fd, buf, sizeof buf)) <= 0)
			return;

		for (char *ptr = buf; ptr < buf + len; ptr +=  sizeof(struct inotify_event) + ev->len) {
			char path[1024];
			ev = (struct inotify_event*) ptr;

			if (strstr(ev->name, "ev") != ev->name)
				continue;

			sprintf(path, "/dev/input/%s", ev->name);

			if (ev->mask & IN_CREATE &&  evdev_is_keyboard(path))
				manage_keyboard(path);
			else if (ev->mask & IN_DELETE)
				destroy_keyboard(path);
		}
	}
}


static void main_loop()
{
	struct keyboard *kbd;
	int inotifyfd;
	int sd;

	long timeout = 0; /* in ns */
	long last_ts = 0;
	struct keyboard *timeout_kbd = NULL;

	nice(-20);

	scan_keyboards();

	inotifyfd = create_inotify_fd();
	sd = create_server_socket();

	pipe(sigfds);
	signal(SIGUSR1, usr1);

	while (1) {
		int maxfd;
		fd_set fds;
		int ret;

		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(inotifyfd, &fds);
		FD_SET(sd, &fds);
		FD_SET(sigfds[0], &fds);

		maxfd = inotifyfd > sigfds[0] ? inotifyfd : sigfds[0];
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

			if (FD_ISSET(inotifyfd, &fds))
				process_inotify_events(inotifyfd);

			for (kbd = keyboards; kbd; kbd = kbd->next) {
				int fd = kbd->fd;

				if (FD_ISSET(fd, &fds)) {
					struct input_event ev;

					while (read(fd, &ev, sizeof(ev)) > 0) {
						switch (ev.type) {
						case EV_KEY:
							switch (ev.code) {
							case BTN_LEFT:
								vkbd_send_button(vkbd, 1, ev.value);
								break;
							case BTN_MIDDLE:
								vkbd_send_button(vkbd, 2, ev.value);
								break;
							case BTN_RIGHT:
								vkbd_send_button(vkbd, 3, ev.value);
								break;
							default:
								if (ev.code >= MAX_KEYS) {
									dbg("Out of bounds evdev keycode: %d %d", ev.code, ev.value);
									break;
								}

								if (ev.value == 2) {
									/* Wayland and X both ignore repeat events but VTs seem to require them. */
									send_repetitions();
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

	while (kbd) {
		struct keyboard *tmp = kbd;
		kbd = kbd->next;
		free(tmp);
	}

	free_vkbd(vkbd);
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

			for (i = 0; i < MAX_KEYS; i++)
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
	vkbd = vkbd_init(VIRTUAL_KEYBOARD_NAME);

	main_loop();
}
