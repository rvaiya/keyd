#include "keyd.h"

#define MAX_AUX_FDS 32

static int aux_fds[MAX_AUX_FDS];
static size_t nr_aux_fds = 0;

struct device device_table[MAX_DEVICES];
size_t device_table_sz;

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

	struct pollfd pfds[MAX_DEVICES+MAX_AUX_FDS+1];

	struct event ev;

	monfd = devmon_create();
	device_table_sz = device_scan(device_table);

	for (i = 0; i < device_table_sz; i++) {
		ev.type = EV_DEV_ADD;
		ev.dev = &device_table[i];

		event_handler(&ev);
	}

	while (1) {
		int removed = 0;

		int start_time;
		int elapsed;

		pfds[0].fd = monfd;
		pfds[0].events = POLLIN;

		for (i = 0; i < device_table_sz; i++) {
			pfds[i+1].fd = device_table[i].fd;
			pfds[i+1].events = POLLIN | POLLERR;
		}

		for (i = 0; i < nr_aux_fds; i++) {
			pfds[i+device_table_sz+1].fd = aux_fds[i];
			pfds[i+device_table_sz+1].events = POLLIN | POLLERR;
		}

		start_time = get_time_ms();
		poll(pfds, device_table_sz+nr_aux_fds+1, timeout > 0 ? timeout : -1);
		ev.timestamp = get_time_ms();
		elapsed = ev.timestamp - start_time;

		if (timeout > 0 && elapsed >= timeout) {
			ev.type = EV_TIMEOUT;
			ev.dev = NULL;
			ev.devev = NULL;
			timeout = event_handler(&ev);
		} else {
			timeout -= elapsed;
		}

		for (i = 0; i < device_table_sz; i++) {
			if (pfds[i+1].revents) {
				struct device_event *devev;

				while ((devev = device_read_event(&device_table[i]))) {
					if (devev->type == DEV_REMOVED) {
						ev.type = EV_DEV_REMOVE;
						ev.dev = &device_table[i];

						timeout = event_handler(&ev);

						device_table[i].fd = -1;
						removed = 1;
						break;
					} else {
						//Handle device event
						panic_check(devev->code, devev->pressed);

						ev.type = EV_DEV_EVENT;
						ev.devev = devev;
						ev.dev = &device_table[i];

						timeout = event_handler(&ev);
					}
				}
			}
		}

		for (i = 0; i < nr_aux_fds; i++) {
			short events = pfds[i+device_table_sz+1].revents;

			if (events) {
				ev.type = events & POLLERR ? EV_FD_ERR : EV_FD_ACTIVITY;
				ev.fd = aux_fds[i];

				timeout = event_handler(&ev);
			}
		}


		if (pfds[0].revents) {
			struct device dev;

			while (devmon_read_device(monfd, &dev) == 0) {
				assert(device_table_sz < MAX_DEVICES);
				device_table[device_table_sz++] = dev;

				ev.type = EV_DEV_ADD;
				ev.dev = &device_table[device_table_sz-1];

				timeout = event_handler(&ev);
			}
		}

		if (removed) {
			size_t n = 0;
			for (i = 0; i < device_table_sz; i++) {
				if (device_table[i].fd != -1)
					device_table[n++] = device_table[i];
			}
			device_table_sz = n;
		}

	}

	return 0;
}

void evloop_add_fd(int fd)
{
	assert(nr_aux_fds < MAX_AUX_FDS);
	aux_fds[nr_aux_fds++] = fd;
}
