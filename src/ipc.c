/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include "keyd.h"

/* TODO (maybe): settle on an API and publish the protocol. */

static void chgid()
{
	struct group *g = getgrnam("keyd");

	if (!g) {
		fprintf(stderr,
			"WARNING: failed to set effective group to \"keyd\" (make sure the group exists)\n");
	} else {
		if (setgid(g->gr_gid)) {
			perror("setgid");
			exit(-1);
		}
	}
}

int ipc_connect()
{
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = {0};

	if (sd < 0) {
		perror("socket");
		exit(-1);
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

	if (connect(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		fprintf(stderr, "ERROR: Failed to connect to \"" SOCKET_PATH "\", make sure the daemon is running and you have permission to access the socket.\n");
		exit(-1);
	}

	return sd;
}

int ipc_create_server()
{
	char lockpath[PATH_MAX];
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	int lfd;
	struct sockaddr_un addr = {0};

	chgid();

	if (sd < 0) {
		perror("socket");
		exit(-1);
	}
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);
	snprintf(lockpath, sizeof lockpath, "%s.lock", SOCKET_PATH);
	lfd = open(lockpath, O_CREAT | O_RDONLY, 0600);

	if (lfd < 0) {
		perror("open");
		exit(-1);
	}

	if (flock(lfd, LOCK_EX | LOCK_NB))
		return -1;

	unlink(SOCKET_PATH);
	if (bind(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		fprintf(stderr, "failed to bind to socket %s\n", SOCKET_PATH);
		exit(-1);
	}

	if (listen(sd, 20) < 0) {
		perror("listen");
		exit(-1);
	}

	chmod(SOCKET_PATH, 0660);

	return sd;
}
