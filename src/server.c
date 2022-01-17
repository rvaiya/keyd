#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "server.h"
#include "keyd.h"
#include "keyboard.h"
#include "error.h"

static int execute_mapping(const char *exp, char response[MAX_RESPONSE_SIZE])
{
	if (!active_keyboard) {
		fprintf(stderr, "Failed to determine active keyboard\n");
		return -1;
	}

	if (kbd_execute_expression(active_keyboard, exp) < 0) {
		sprintf(response, "ERROR parsing %s: %s\n", exp, errstr);
		return -1;
	}

	return 0;
}

static int process_message(uint8_t type,
			   const char *payload,
			   char response[MAX_RESPONSE_SIZE])
{
	switch (type) {
		case MSG_PING:
			strcpy(response, "PONG\n");
			break;
		case MSG_RESET:
			reset_keyboards();
			break;
		case MSG_RELOAD:
			reload_config();
			break;
		case MSG_MAPPING:
			return execute_mapping(payload, response);
			break;
	}

	return 0;
}

int create_server_socket()
{
	int sd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	struct sockaddr_un addr = {0};

	if (sd < 0) {
		perror("socket");
		exit(-1);
	}
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET, sizeof(addr.sun_path)-1);

	unlink(SOCKET);
	if (bind(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		perror("bind");
		exit(-1);
	}

	if (listen(sd, 20) < 0) {
		perror("listen");
		exit(-1);
	}

	chmod(SOCKET, 0660);
	fcntl(sd, F_SETFL, O_NONBLOCK);
	return sd;
}

void server_process_connections(int sd)
{
	char payload[MAX_REQUEST_SIZE];
	char response[MAX_RESPONSE_SIZE] = "";
	int con;

	while((con = accept(sd, NULL, NULL)) > 0) {
		int rc;
		int sz;
		uint8_t type;

		if (read(con, &type, sizeof(type)) < 0) {
			perror("read");
			exit(-1);
		}

		sz = read(con, &payload, sizeof(payload));
		if (sz <= 0) {
			perror("read");
			exit(-1);
		}

		payload[sz-1] = 0;

		rc = process_message(type, payload, response);

		write(con, response, strlen(response));
		write(con, &rc, sizeof rc);
		close(con);
	}
}
