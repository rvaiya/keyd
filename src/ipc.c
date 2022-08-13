/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include "keyd.h"

/* TODO (maybe): settle on an API and publish the protocol. */

void xwrite(int fd, const void *buf, size_t sz)
{
	size_t nwr = 0;
	ssize_t n;

	while(sz != nwr) {
		n = write(fd, buf+nwr, sz-nwr);
		if (n < 0) {
			perror("write");
			exit(-1);
		}
		nwr += n;
	}
}

void xread(int fd, void *buf, size_t sz)
{
	size_t nrd = 0;
	ssize_t n;

	while(sz != nrd) {
		n = read(fd, buf+nrd, sz-nrd);
		if (n < 0) {
			perror("read");
			exit(-1);
		}
		nrd += n;
	}
}

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
		perror("bind");
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

	chgid();
	return sd;
}
