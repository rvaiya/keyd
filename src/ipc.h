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


int	ipc_create_server(const char *path);
void	ipc_server_process_connection(int sd, int (*handler) (int fd, const char *input));
int	ipc_run(const char *socket, const char *input);

#endif
