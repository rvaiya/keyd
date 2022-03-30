/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include "keyd.h"

/* config variables */

static const char *virtual_keyboard_name;
static const char *config_dir;
static const char *socket_file;

/* local state */

static struct device devices[MAX_DEVICES];
static size_t nr_devices = 0;
static struct keyboard *active_kbd = NULL;

/* loop() callback functions */

static void (*device_add_cb) (struct device *dev);
static void (*device_remove_cb) (struct device *dev);

/* returns a minimum timeout value */
static int (*device_event_cb) (struct device *dev, uint8_t code, uint8_t processed);

/* globals */

struct vkbd *vkbd;
char errstr[2048];
int debug_level;


static void daemon_remove_cb(struct device *dev)
{
	struct keyboard *kbd = dev->data;

	if (kbd)
		free(kbd);

	active_kbd = NULL;

	printf("device removed: %04x:%04x %s (%s)\n",
	       dev->vendor_id,
	       dev->product_id,
	       dev->name,
	       dev->path);
}

static void daemon_add_cb(struct device *dev)
{
	struct keyboard *kbd;
	const char *config_path = config_find_path(config_dir, dev->vendor_id, dev->product_id);

	dev->data = NULL;

	/*
	 * NOTE: Some mice can emit keys and consequently appear as keyboards.
	 * Conversely, some keyboards with a builtin trackpad can emit mouse
	 * events. There doesn't appear to be a reliable way to distinguish
	 * between these two, so we take a permissive approach and leave it up to
	 * the user to blacklist mice which emit key events.
	 */
	if (!dev->is_keyboard)
		return;

	printf("device added: %04x:%04x %s (%s)\n",
	       dev->vendor_id,
	       dev->product_id,
	       dev->name,
	       dev->path);

	if (!config_path) {
		printf("\tignored (no matching config)\n");
		return;
	}

	kbd = calloc(1, sizeof(struct keyboard));
	if (config_parse(&kbd->config, config_path) < 0) {
		free(kbd);
		printf("\tfailed to parse %s\n", config_path);
		return;
	}

	if (device_grab(dev) < 0) {
		free(kbd);
		printf("\tgrab failed\n");
		return;
	}

	printf("\tmatched %s\n", config_path);


	memcpy(&kbd->layer_table, &kbd->config.layer_table, sizeof(kbd->layer_table));

	kbd->dev = dev;
	dev->data = kbd;
}

static void panic_check(uint8_t code, uint8_t pressed)
{
	static uint8_t enter, backspace, escape;
	switch (code) {
		case KEYD_ENTER:
			enter = pressed;
			break;
		case KEYD_BACKSPACE:
			backspace = pressed;
			break;
		case KEYD_ESC:
			escape = pressed;
			break;
	}

	if (backspace && enter && escape)
		exit(-1);
}

static int daemon_event_cb(struct device *dev, uint8_t code, uint8_t pressed)
{
	struct keyboard *kbd = NULL;

	if (!dev) {
		/* timeout */
		kbd = active_kbd;
	} else if (dev->data) {
		kbd = dev->data;
	} else if (code >= KEYD_LEFT_MOUSE && code <= KEYD_MOUSE_2) {
		code = KEYD_EXTERNAL_MOUSE_BUTTON;

		kbd = active_kbd;
	}

	if (!kbd)
		return 0;

	panic_check(code, pressed);
	active_kbd = kbd;

	return kbd_process_key_event(kbd, code, pressed);
}

static int ipc_cb(int fd, const char *input)
{
	int ret = 0;

	if (!strcmp(input, "ping")) {
		char s[] = "pong\n";
		write(fd, s, sizeof s);

		return 0;
	} else if (!strcmp(input, "reset")) {
		if (!active_kbd)
			return -1;

		kbd_reset(active_kbd);
	}  else {
		if (!active_kbd)
			return -1;

		ret = kbd_execute_expression(active_kbd, input);
		if (ret < 0) {
			write(fd, "ERROR: ", 7);
			write(fd, errstr, strlen(errstr));
			write(fd, "\n\x00", 2);
		}
	}

	return ret;
}


static void monitor_remove_cb(struct device *dev)
{
	fprintf(stderr, "device removed: %04x:%04x (%s)\n",
			dev->vendor_id,
			dev->product_id,
			dev->name);
}

static void monitor_add_cb(struct device *dev)
{
	fprintf(stderr, "device added: %04x:%04x (%s)\n",
			dev->vendor_id,
			dev->product_id,
			dev->name);

}

static int monitor_event_cb(struct device *dev, uint8_t code, uint8_t pressed)
{
	const char *name = keycode_table[code].name;

	if (name) {
		printf("%s\t%04x:%04x\t%s %s\n",
				dev->name,
				dev->vendor_id,
				dev->product_id,
				name,
				pressed ? "down" : "up");
	}

	return 0;
}


static void set_echo(int set)
{
	struct termios tinfo;

	tcgetattr(1, &tinfo);

	if (set)
		tinfo.c_lflag |= ECHO;
	else
		tinfo.c_lflag &= ~ECHO;

	tcsetattr(1, TCSANOW, &tinfo);
}

static long get_time_ms()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec*1E3+ts.tv_nsec/1E6;
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

static void init_devices(struct device devices[MAX_DEVICES], int exclude_vkbd)
{
	size_t i;

	size_t n = device_scan(devices);

	nr_devices = 0;
	for (i = 0; i < n; i++) {
		if (exclude_vkbd && devices[i].vendor_id == 0x0FAC)
			continue;

		devices[nr_devices++] = devices[i];
	}
}

static int loop(int monitor_mode)
{
	int timeout_start = 0;
	int timeout = 0;

	size_t i;

	int monfd = devmon_create();
	int ipcfd = -1;

	struct pollfd pfds[MAX_DEVICES];

	int nfds = 0;

	if (monitor_mode) {
		init_devices(devices, 0);
	} else {
		init_devices(devices, 1);
		ipcfd = ipc_create_server(socket_file);
		if (ipcfd < 0) {
			fprintf(stderr, "ERROR: failed to create %s (another instance running?)\n", socket_file);
			exit(-1);
		}

		printf("socket: %s\n", socket_file);
	}

	pfds[nfds].fd = 1;
	pfds[nfds++].events = POLLERR;

	pfds[nfds].fd = monfd;
	pfds[nfds++].events = POLLIN;

	pfds[nfds].fd = ipcfd;
	pfds[nfds++].events = POLLIN;

	for (i = 0; i < nr_devices; i++)
		device_add_cb(&devices[i]);

	while (1) {
		int prune = 0;
		int poll_timeout = -1;

		for (i = 0; i < nr_devices; i++) {
			pfds[i+nfds].fd = devices[i].fd;
			pfds[i+nfds].events = POLLIN;
		}

		if (timeout)
			poll_timeout = get_time_ms() - timeout_start;
		else
			poll_timeout = -1;

		poll(pfds, nr_devices+nfds, poll_timeout);

		if (timeout) {
			int elapsed = get_time_ms() - timeout_start;
			if (elapsed >= timeout) {
				timeout = device_event_cb(NULL,  0, 0);
				timeout_start = get_time_ms();
			}
		}

		/* pipe closed, proactively terminate. */
		if (pfds[0].revents)
			exit(0);

		if (pfds[1].revents) {
			struct device *dev;
			while ((dev = devmon_read_device(monfd))) {
				assert(nr_devices < MAX_DEVICES);

				if (!monitor_mode && dev->vendor_id == 0x0FAC) /* ignore virtual devices we own */
					continue;

				devices[nr_devices] = *dev;
				device_add_cb(&devices[nr_devices]);
				nr_devices++;
			}
		}

		if (pfds[2].revents) {
			int con = accept(ipcfd, NULL, 0);
			if (con < 0) {
				perror("accept");
				continue;
			}

			ipc_server_process_connection(con, ipc_cb);
		}

		for (i = 0; i < nr_devices; i++) {
			if (pfds[i+nfds].revents) {
				struct device *dev = &devices[i];
				struct device_event *ev = device_read_event(&devices[i]);

				if (ev) {
					if (ev->type == DEV_REMOVED) {
						device_remove_cb(dev);
						devices[i].fd = -1;
						prune = 1;
					} else {
						timeout = device_event_cb(dev,  ev->code, ev->pressed);
						timeout_start = get_time_ms();
					}
				}
			}
		}

		if (prune) {
			int n = 0;
			for (i = 0; i < nr_devices; i++) {
				if (devices[i].fd != -1)
					devices[n++] = devices[i];
			}

			nr_devices = n;
		}
	}
}

static void cleanup()
{
	size_t i;

	for (i = 0; i < nr_devices; i++)
		free(devices[i].data);

	free_vkbd(vkbd);

	if (isatty(1))
		set_echo(1);
}

static void print_keys()
{
	size_t i;
	for (i = 0; i < 256; i++) {
		const char *altname = keycode_table[i].alt_name;
		const char *shiftedname = keycode_table[i].shifted_name;
		const char *name = keycode_table[i].name;

		if (name)
			printf("%s\n", name);
		if (altname)
			printf("%s\n", altname);
		if (shiftedname)
			printf("%s\n", shiftedname);
	}
}

static void print_version()
{
	printf("keyd "VERSION"\n");
}

static void print_help()
{
	printf("usage: keyd [option]\n\n"
			"Options:\n"
			"    -m, --monitor      Start keyd in monitor mode.\n"
			"    -l, --list-keys    List key names.\n"
			"    -v, --version      Print the current version and exit.\n"
			"    -h, --help         Print help and exit.\n");
}

static void eval_expressions(char *exps[], int n)
{
	int i;
	int ret = 0;

	for (i = 0; i < n; i++) {
		int rc;
		if ((rc = ipc_run(socket_file, exps[i])))
			ret = rc;
	}

	exit(ret);
}


#define setvar(var, name, default) \
	var = getenv(name); \
	if (!var) \
		var = default;

int main(int argc, char *argv[])
{
	int monitor_flag = 0;

	setvar(virtual_keyboard_name,	"KEYD_NAME",		"keyd virtual device");
	setvar(config_dir,		"KEYD_CONFIG_DIR",	"/etc/keyd");
	setvar(socket_file,		"KEYD_SOCKET",		"/var/run/keyd.socket");

	debug_level = atoi(getenv("KEYD_DEBUG") ? getenv("KEYD_DEBUG") : "");

	dbg("Debug mode activated");

	atexit(cleanup);

	signal(SIGTERM, exit);
	signal(SIGINT, exit);
	signal(SIGPIPE, SIG_IGN);

	if (argc >= 2) {
		if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--list-keys"))
			print_keys();
		else if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))
			print_version();
		else if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--monitor"))
			monitor_flag = 1;
		else if (!strcmp(argv[1], "-e") || !strcmp(argv[1], "--expression"))
			eval_expressions(argv+2, argc-2);
		else
			print_help();

		if (!monitor_flag)
			exit(0);
	}


	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	if (monitor_flag) {
		device_add_cb = monitor_add_cb;
		device_remove_cb = monitor_remove_cb;
		device_event_cb = monitor_event_cb;

		if (isatty(1))
			set_echo(0);

		loop(1);
	} else {
		device_add_cb = daemon_add_cb;
		device_remove_cb = daemon_remove_cb;
		device_event_cb = daemon_event_cb;

		vkbd = vkbd_init(virtual_keyboard_name);

		printf("Starting keyd "VERSION"\n");

		chgid();
		loop(0);
	}
}
