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

static long get_time_ms()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1E3 + ts.tv_nsec / 1E6;
}

int event_handler(struct event *ev)
{
	static long last_time;
	long ctime = get_time_ms();

	switch (ev->type) {
	const char *name;

	case EV_DEV_ADD:
		printf("device added: %04x:%04x %s (%s)\n",
		       ev->dev->vendor_id, ev->dev->product_id,
		       ev->dev->name, ev->dev->path);
		break;
	case EV_DEV_REMOVE:
		printf("device removed: %04x:%04x %s (%s)\n",
		       ev->dev->vendor_id, ev->dev->product_id,
		       ev->dev->name, ev->dev->path);
		break;
	case EV_DEV_EVENT:
		if (ev->devev->type == DEV_KEY) {
			name = keycode_table[ev->devev->code].name;

			if (time_flag)
				printf("+%ld ms\t", ctime - last_time);

			printf("%s\t%04x:%04x\t%s %s\n",
			       ev->dev->name,
			       ev->dev->vendor_id,
			       ev->dev->product_id, name, ev->devev->pressed ? "down" : "up");
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

	last_time = ctime;
	return 0;
}

int monitor(int argc, char *argv[])
{
	if (argc == 1 && !strcmp(argv[0], "-h")) {
		printf("Usage: keyd monitor [-t]\n\n\t-t: Print the time in milliseconds between events.\n");
		return 0;
	}

	if (argc == 1 && !strcmp(argv[0], "-t"))
		time_flag = 1;

	if (isatty(1))
		set_tflags(ECHO, 0);

	/* Eagerly terminate on pipe closures. */
	if (!isatty(1))
		evloop_add_fd(1);

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	atexit(cleanup);

	evloop(event_handler);

	return 0;
}
