/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */


#include "keyd.h"

static int ipc_exec(int type, const char *data, size_t sz)
{
	struct ipc_message msg;

	assert(sz <= sizeof(msg.data));

	msg.type = type;
	msg.sz = sz;
	memcpy(msg.data, data, sz);

	int con = ipc_connect();

	if (con < 0) {
		perror("connect");
		exit(-1);
	}

	xwrite(con, &msg, sizeof msg);
	xread(con, &msg, sizeof msg);

	if (msg.sz) {
		write(1, msg.data, msg.sz);
		write(1, "\n", 1);
	}

	return msg.type == IPC_FAIL;

	return type == IPC_FAIL;
}

static int version(int argc, char *argv[])
{
	printf("keyd " VERSION "\n");

	return 0;
}

static int help(int argc, char *argv[])
{
	printf("usage: keyd [-v] [-h] [command] [<args>]\n\n"
	       "Commands:\n"
	       "    monitor                        Print key events in real time.\n"
	       "    list-keys                      Print a list of valid key names.\n"
	       "    reload                         Trigger a reload .\n"
	       "    bind <binding> [<binding>...]  Add the supplied bindings to all loaded configs.\n"
	       "Options:\n"
	       "    -v, --version      Print the current version and exit.\n"
	       "    -h, --help         Print help and exit.\n");

	return 0;
}

static int list_keys(int argc, char *argv[])
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

	return 0;
}


static int add_binding(int argc, char *argv[])
{
	int i;
	int ret = 0;

	for (i = 0; i < argc; i++) {
		if (ipc_exec(IPC_BIND, argv[i], strlen(argv[i])))
			ret = -1;
	}

	return ret;
}

static int reload()
{
	ipc_exec(IPC_RELOAD, NULL, 0);

	return 0;
}

struct {
	const char *name;
	const char *flag;
	const char *long_flag;

	int (*fn)(int argc, char **argv);
} commands[] = {
	{"help", "-h", "--help", help},
	{"version", "-v", "--version", version},

	/* Keep -e and -m for backward compatibility. TODO: remove these at some point. */
	{"monitor", "-m", "--monitor", monitor},
	{"bind", "-e", "--expression", add_binding},

	{"reload", "", "", reload},
	{"list-keys", "", "", list_keys},
};

int main(int argc, char *argv[])
{
	int c;
	size_t i;

	debug_level =
	    atoi(getenv("KEYD_DEBUG") ? getenv("KEYD_DEBUG") : "");

	dbg("Debug mode activated");

	signal(SIGTERM, exit);
	signal(SIGINT, exit);
	signal(SIGPIPE, SIG_IGN);

	if (argc > 1) {
		for (i = 0; i < ARRAY_SIZE(commands); i++)
			if (!strcmp(commands[i].name, argv[1]) ||
				!strcmp(commands[i].flag, argv[1]) ||
				!strcmp(commands[i].long_flag, argv[1])) {
				return commands[i].fn(argc - 2, argv + 2);
			}

		return help(argc, argv);
	}

	run_daemon(argc, argv);
}

/* TODO: find a better place for this. */
void set_led(int led, int state)
{
//TODO: fixme
//      size_t i;
//
//      for (i = 0; i < nr_devices; i++)
//              device_set_led(&devices[i], led, state);
}
