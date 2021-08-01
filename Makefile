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
	-mkdir -p $(DESTDIR)/etc/keyd
	-mkdir -p $(DESTDIR)/usr/lib/systemd/system
	-mkdir -p $(DESTDIR)/usr/bin
	-mkdir -p $(DESTDIR)/usr/share/man/man1

	install -m755 keyd.service $(DESTDIR)/usr/lib/systemd/system
	install -m755 bin/keyd $(DESTDIR)/usr/bin
	install -m644 keyd.1.gz $(DESTDIR)/usr/share/man/man1
