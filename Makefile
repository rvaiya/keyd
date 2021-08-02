.PHONY: all clean debug install

DESTDIR=
PREFIX=/usr

VERSION=0.0.1
GIT_HASH=$(shell git describe --no-match --always --abbrev=40 --dirty)
CFLAGS=-DVERSION=\"$(VERSION)\" -DGIT_COMMIT_HASH=\"$(GIT_HASH)\"

all:
	-mkdir bin
	$(CC) $(CFLAGS) -O3 src/*.c -o bin/keyd -ludev
debug:
	-mkdir bin
	$(CC) $(CFLAGS) -Wall -Wextra -pedantic -DDEBUG -g  src/*.c -o bin/keyd -ludev
man:
	pandoc -s -t man man.md | gzip > keyd.1.gz
clean:
	-rm -rf bin
install:
	-mkdir -p $(DESTDIR)/etc/keyd
	-mkdir -p $(DESTDIR)$(PREFIX)/lib/systemd/system
	-mkdir -p $(DESTDIR)$(PREFIX)/bin
	-mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1

	install -m755 keyd.service $(DESTDIR)$(PREFIX)/lib/systemd/system
	install -m755 bin/keyd $(DESTDIR)$(PREFIX)/bin
	install -m644 keyd.1.gz $(DESTDIR)$(PREFIX)/share/man/man1
