#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"

static int create_client_socket()
{
	int sd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	struct sockaddr_un addr = {0};

	if (sd < 0) {
		perror("socket");
		exit(-1);
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET, sizeof(addr.sun_path)-1);

	if (connect(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		perror("bind");
		exit(-1);
	}

	return sd;
}

int client_send_message(uint8_t type, const char *payload)
{
	int con;
	int rc;
	size_t sz;
	char response[MAX_RESPONSE_SIZE];

	con = create_client_socket();

	if (write(con, &type, sizeof type) < 0) {
		perror("write");
		exit(-1);
	}

	if (write(con, payload, strlen(payload)+1) < 0) {
		perror("write");
		exit(-1);
	}

	sz = read(con, response, sizeof response);

	read(con, &rc, sizeof rc);

	write(1, response, sz);

	return rc;
}
