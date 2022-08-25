#include "keyd.h"

static void set_echo(int set)
{
	if (!isatty(1))
		return;

	struct termios tinfo;

	tcgetattr(1, &tinfo);

	if (set)
		tinfo.c_lflag |= ECHO;
	else
		tinfo.c_lflag &= ~ECHO;

	tcsetattr(1, TCSANOW, &tinfo);
}

static void cleanup()
{
	set_echo(1);
}

int event_handler(struct event *ev)
{
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

			printf("%s\t%04x:%04x\t%s %s\n",
			       ev->dev->name,
			       ev->dev->vendor_id,
			       ev->dev->product_id, name, ev->devev->pressed ? "down" : "up");
	       }
		break;
	case EV_FD_ACTIVITY:
		exit(0);
		break;
	default:
		break;
	}

	fflush(stdout);
	fflush(stderr);

	return 0;
}

int monitor(int argc, char *argv[])
{
	set_echo(0);

	evloop_add_fd(1);

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	atexit(cleanup);

	evloop(event_handler);

	return 0;
}
