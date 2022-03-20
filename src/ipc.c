/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include "ipc.h"

/* Establish a client connection to the given socket path. */
static int client_connect(const char *path)
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

static int readmsg(int sd, char buf[MAX_MESSAGE_SIZE])
{
	int n = 0;

	while (1) {
		int ret = read(sd, buf+n, MAX_MESSAGE_SIZE-n);
		if (ret < 0)
			return -1;

		n += ret;
		if (n > 1 && buf[n-1] == 0 && buf[n-2] == 0)
			return n-2;

		assert(n < MAX_MESSAGE_SIZE);
	}
}

/* 
 * Consume a \x00\x00 terminated input string from the supplied connection and
 * delegate processing to the provided callback. The return value of 'handler'
 * will ultimately be returned by the corresponding ipc_run() call on the
 * client and all output written to 'output_fd' will be printed to the client's
 * standard output stream.
 */

void ipc_server_process_connection(int sd, int (*handler) (int output_fd, const char *input))
{
	char input[MAX_MESSAGE_SIZE];
	uint8_t ret = 0;

	if (readmsg(sd, input) < 0) {
		fprintf(stderr, "ipc: failed to read input\n");
		return;
	}

	ret = handler(sd, input);

	write(sd, &ret, 1);
	write(sd, "\x00\x00", 2);
	close(sd);
}

int ipc_run(const char *socket, const char *input)
{	
	int n;
	uint8_t ret;
	char buf[MAX_MESSAGE_SIZE];

	int sd = client_connect(socket);

	if (sd < 0)
		return -1;

	write(sd, input, strlen(input));
	write(sd, "\x00\x00", 2); 

	n = readmsg(sd, buf);
	if (n < 0)
		return -1;

	printf("%s", buf);

	ret = buf[n-1];
	return (int)ret;
}
