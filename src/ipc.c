/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include "ipc.h"
#include <assert.h>

/* TODO (maybe): settle on an API and publish the protocol. */

static void xwrite(int fd, const void *buf, size_t sz)
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

static void xread(int fd, void *buf, size_t sz)
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

/* TODO: make this more robust (allow for the possibility of untrusted/malicious clients). */
void ipc_readmsg(int sd, enum ipc_messsage_type *type, char data[MAX_MESSAGE_SIZE], size_t *sz)
{
	xread(sd, type, sizeof(*type));
	xread(sd, sz, sizeof(*sz));

	assert(*sz < MAX_MESSAGE_SIZE);

	xread(sd, data, *sz);
}

void ipc_writemsg(int sd, enum ipc_messsage_type type, const char *data, size_t sz)
{
	assert(sz < MAX_MESSAGE_SIZE);

	xwrite(sd, &type, sizeof(type));
	xwrite(sd, &sz, sizeof(sz));
	xwrite(sd, data, sz);
}

/* Establish a client connection to the given socket path. */
int ipc_connect(const char *path)
{
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = {0};

	if (sd < 0) {
		perror("socket");
		exit(-1);
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

	if (connect(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		perror("bind");
		exit(-1);
	}

	return sd;
}

/* Create a listening socket on the supplied path. */
int ipc_create_server(const char *path)
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
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	snprintf(lockpath, sizeof lockpath, "%s.lock", path);
	lfd = open(lockpath, O_CREAT | O_RDONLY, 0600);

	if (lfd < 0) {
		perror("open");
		exit(-1);
	}

	if (flock(lfd, LOCK_EX | LOCK_NB))
		return -1;

	unlink(path);
	if (bind(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		fprintf(stderr, "failed to bind to socket %s\n", path);
		exit(-1);
	}

	if (listen(sd, 20) < 0) {
		perror("listen");
		exit(-1);
	}

	chmod(path, 0660);
	return sd;
}
