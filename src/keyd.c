/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */


#include "keyd.h"

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
	printf("keyd " VERSION "\n");
}

static void print_help()
{
	printf("usage: keyd [command] [option]\n\n"
	       "Commands:\n"
	       "    monitor                        Start in monitor mode.\n"
	       "    reload                         Reload all config files.\n"
	       "    bind <binding> [<binding>...]  Add the supplied bindings.\n"
	       "Options:\n"
	       "    -l, --list-keys    List key names.\n"
	       "    -v, --version      Print the current version and exit.\n"
	       "    -h, --help         Print help and exit.\n");
}

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


static int eval_expressions(char *exps[], size_t n)
{
	size_t i;
	int ret = 0;

	for (i = 0; i < n; i++) {
		if (ipc_exec(IPC_BIND, exps[i], strlen(exps[i])))
			ret = -1;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int c;
	int eval_flag = 0;
	int monitor_flag = 0;

	debug_level =
	    atoi(getenv("KEYD_DEBUG") ? getenv("KEYD_DEBUG") : "");

	dbg("Debug mode activated");

	signal(SIGTERM, exit);
	signal(SIGINT, exit);
	signal(SIGPIPE, SIG_IGN);

	if (argc > 1 && !strcmp(argv[1], "reload")) {
		ipc_exec(IPC_RELOAD, NULL, 0);
		return 0;
	}

	if (argc > 1 && !strcmp(argv[1], "bind")) {
		return eval_expressions(argv + 2, argc - 2);
	}


	struct option opts[] = {
		{ "list-keys", no_argument, NULL, 'l' },
		{ "version", no_argument, NULL, 'v' },
		{ "monitor", no_argument, NULL, 'm' },
		{ "expression", no_argument, NULL, 'e' },
		{ "help", no_argument, NULL, 'h' },
	};

	while ((c = getopt_long(argc, argv, "hlvme", opts, NULL)) > 0) {
		switch (c) {
		case 'l':
			print_keys();
			return 0;
		case 'v':
			print_version();
			return 0;
		case 'm':
			monitor_flag = 1;
			break;
		case 'e':
			eval_flag = 1;
			break;
		default:
			print_help();
			return 0;
		}
	}

	if (eval_flag) {
		return eval_expressions(argv + optind, argc - optind);
	}

	if (monitor_flag)
		monitor();
	else
		run_daemon();

	return 0;
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
