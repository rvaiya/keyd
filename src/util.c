#include "keyd.h"

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

