/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef IPC_H
#define IPC_H

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <limits.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_MESSAGE_SIZE 4096

enum ipc_messsage_type {
	IPC_SUCCESS,
	IPC_FAIL,

	IPC_EXEC,
	IPC_EXEC_ALL,
};

int ipc_create_server(const char *path);
int ipc_connect(const char *path);

void ipc_readmsg(int sd, enum ipc_messsage_type *type, char data[MAX_MESSAGE_SIZE], size_t *sz);
void ipc_writemsg(int sd, enum ipc_messsage_type type, const char *data, size_t sz);

#endif
