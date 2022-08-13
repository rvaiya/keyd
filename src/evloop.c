#include "keyd.h"

#define MAX_AUX_FDS 32

static int aux_fds[MAX_AUX_FDS];
static size_t nr_aux_fds = 0;

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
		die("panic sequence detected");
}

static long get_time_ms()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1E3 + ts.tv_nsec / 1E6;
}

int evloop(int (*event_handler) (struct event *ev))
{
	size_t i;
	int timeout = 0;
	int monfd;

	struct device devs[MAX_DEVICES];
	struct pollfd pfds[MAX_DEVICES+MAX_AUX_FDS+1];
	size_t ndevs;

	struct event ev;

	monfd = devmon_create();
	ndevs = device_scan(devs);

	for (i = 0; i < ndevs; i++) {
		ev.type = EV_DEV_ADD;
		ev.dev = &devs[i];

		event_handler(&ev);
	}

	while (1) {
		int removed = 0;

		int start_time;
		int elapsed;

		pfds[0].fd = monfd;
		pfds[0].events = POLLIN;

		for (i = 0; i < ndevs; i++) {
			pfds[i+1].fd = devs[i].fd;
			pfds[i+1].events = POLLIN | POLLERR;
		}

		for (i = 0; i < nr_aux_fds; i++) {
			pfds[i+ndevs+1].fd = aux_fds[i];
			pfds[i+ndevs+1].events = POLLIN | POLLERR;
		}

		start_time = get_time_ms();
		poll(pfds, ndevs+nr_aux_fds+1, timeout > 0 ? timeout : -1);
		elapsed = get_time_ms() - start_time;

		ev.timeleft = timeout;
		if (timeout > 0 && elapsed >= timeout) {
			ev.type = EV_TIMEOUT;
			ev.timeleft = 0;
			timeout = event_handler(&ev);
		} else {
			timeout -= elapsed;
			ev.timeleft = timeout;
		}

		for (i = 0; i < ndevs; i++) {
			if (pfds[i+1].revents) {
				struct device_event *devev;

				while ((devev = device_read_event(&devs[i]))) {
					if (devev->type == DEV_REMOVED) {
						ev.type = EV_DEV_REMOVE;
						ev.dev = &devs[i];

						timeout = event_handler(&ev);

						devs[i].fd = -1;
						removed = 1;
						break;
					} else {
						//Handle device event
						panic_check(devev->code, devev->pressed);

						ev.type = EV_DEV_EVENT;
						ev.devev = devev;
						ev.dev = &devs[i];

						timeout = event_handler(&ev);
					}
				}
			}
		}

		for (i = 0; i < nr_aux_fds; i++) {
			if (pfds[i+ndevs+1].revents) {
				ev.type = EV_FD_ACTIVITY;
				ev.fd = aux_fds[i];

				timeout = event_handler(&ev);
			}
		}


		if (pfds[0].revents) {
			struct device dev;

			while (devmon_read_device(monfd, &dev) == 0) {
				assert(ndevs < MAX_DEVICES);
				devs[ndevs++] = dev;

				ev.type = EV_DEV_ADD;
				ev.dev = &devs[ndevs-1];

				timeout = event_handler(&ev);
			}
		}

		if (removed) {
			size_t n = 0;
			for (i = 0; i < ndevs; i++) {
				if (devs[i].fd != -1)
					devs[n++] = devs[i];
			}
			ndevs = n;
		}

	}

	return 0;
}

void evloop_add_fd(int fd)
{
	assert(nr_aux_fds < MAX_AUX_FDS);
	aux_fds[nr_aux_fds++] = fd;
}
