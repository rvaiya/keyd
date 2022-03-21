/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYD_H_
#define KEYD_H_

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <grp.h>

#include "device.h"
#include "keyboard.h"
#include "vkbd.h"
#include "ipc.h"

#define MAX_MESSAGE_SIZE 4096

#define dbg(fmt, ...) { \
	if (debug_level) \
		fprintf(stderr, "DEBUG: %s:%d: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__); \
}

#define err(fmt, ...) snprintf(errstr, sizeof(errstr), fmt, ##__VA_ARGS__);

extern int debug_level;
extern char errstr[2048];
extern struct vkbd *vkbd;

int create_server_socket(const char *socket_file);

#endif
