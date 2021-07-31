.PHONY: all clean debug install

VERSION=0.0.1
GIT_HASH=$(shell git describe --no-match --always --abbrev=40 --dirty)
CFLAGS=-DVERSION=\"$(VERSION)\" -DGIT_COMMIT_HASH=\"$(GIT_HASH)\"

all:
	-mkdir bin
	$(CC) $(CFLAGS) -O3 src/*.c -o bin/keyd -ludev
debug:
	-mkdir bin
	$(CC) $(CFLAGS) -Wall -Wextra -DDEBUG -g  src/*.c -o bin/keyd
man:
	pandoc -s -t man man.md | gzip > keyd.1.gz
clean:
	-rm -rf bin
install:
	-mkdir /etc/keyd
	install -Dm755 keyd.service $(DESTDIR)/lib/systemd/system
	install -Dm755 bin/keyd $(DESTDIR)/bin/keyd
	install -Dm644 keyd.1.gz $(DESTDIR)/share/man/man1/keyd.1.gz
