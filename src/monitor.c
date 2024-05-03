#include "keyd.h"

static int time_flag = 0;

static void set_tflags(tcflag_t flags, int val)
{
	if (!isatty(0))
		return;

	struct termios tinfo;

	tcgetattr(0, &tinfo);

	if (val)
		tinfo.c_lflag |= flags;
	else
		tinfo.c_lflag &= ~flags;

	tcsetattr(0, TCSANOW, &tinfo);
}

static void cleanup()
{
	/* Drain STDIN (useful for scripting). */
	set_tflags(ICANON, 0);
	char buf[4096];
	fcntl(0, F_SETFL, O_NONBLOCK);
	while(read(0, buf, sizeof buf) > 0) {}

	set_tflags(ICANON|ECHO, 1);
}

int event_handler(struct event *ev)
{
	static long last_time = 0;

	switch (ev->type) {
	const char *name;

	case EV_DEV_ADD:
		keyd_log("device added: %s %s (%s)\n",
			  ev->dev->id, ev->dev->name, ev->dev->path);
		break;
	case EV_DEV_REMOVE:
		keyd_log("device removed: %s %s (%s)\n",
			  ev->dev->id, ev->dev->name, ev->dev->path);
		break;
	case EV_DEV_EVENT:
		switch (ev->devev->type) {
		case DEV_KEY:
			name = keycode_table[ev->devev->code].name;

			if (time_flag && last_time)
				keyd_log("r{+%ld} ms\t", ev->timestamp - last_time);

			keyd_log("%s\t%s\t%s %s\n",
				 ev->dev->name, ev->dev->id,
				 name, ev->devev->pressed ? "down" : "up");

			break;
		default:
			break;
		}
		break;
	case EV_FD_ERR:
		exit(0);
		break;
	default:
		break;
	}

	fflush(stdout);
	fflush(stderr);

	last_time = ev->timestamp;
	return 0;
}

int monitor(int argc, char *argv[])
{
	struct stat st;

	if (argc == 2 && !strcmp(argv[1], "-h")) {
		printf("Usage: keyd monitor [-t]\n\n\t-t: Print the time in milliseconds between events.\n");
		return 0;
	}

	if (argc == 2 && !strcmp(argv[1], "-t"))
		time_flag = 1;

	if (isatty(1))
		set_tflags(ECHO, 0);

	if (fstat(1, &st)) {
		perror("fstat");
		exit(-1);
	}

	/* If stdout is a process, terminate on pipe closures. */
	if (st.st_mode & S_IFIFO)
		evloop_add_fd(1);

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	atexit(cleanup);

	evloop(event_handler);

	return 0;
}
