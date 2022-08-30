/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYD_H_
#define KEYD_H_

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "macro.h"
#include "device.h"
#include "error.h"
#include "keyboard.h"
#include "keys.h"
#include "vkbd.h"
#include "string.h"

#define MAX_IPC_MESSAGE_SIZE 4096

#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

#define die(fmt, ...) \
	do { \
		fprintf(stderr, "FATAL: "fmt"\n", ##__VA_ARGS__); \
		exit(-1); \
	} while (0)

#define warn(fmt, ...) fprintf(stderr, "\033[31;1mERROR:\033[0m "fmt"\n", ##__VA_ARGS__)

enum event_type {
	EV_DEV_ADD,
	EV_DEV_REMOVE,
	EV_DEV_EVENT,
	EV_FD_ACTIVITY,
	EV_FD_ERR,
	EV_TIMEOUT,
};

struct event {
	enum event_type type;
	struct device *dev;
	struct device_event *devev;
	int timeleft;
	int fd;
};

struct ipc_message {
	enum {
		IPC_SUCCESS,
		IPC_FAIL,

		IPC_BIND,
		IPC_INPUT,
		IPC_MACRO,
		IPC_RELOAD,
		IPC_LAYER_LISTEN,
	} type;
	
	char data[MAX_IPC_MESSAGE_SIZE];
	size_t sz;
};

int monitor(int argc, char *argv[]);
int run_daemon(int argc, char *argv[]);

void evloop_add_fd(int fd);
int evloop(int (*event_handler) (struct event *ev));

void xwrite(int fd, const void *buf, size_t sz);
void xread(int fd, void *buf, size_t sz);

int ipc_create_server();
int ipc_connect();

#endif
