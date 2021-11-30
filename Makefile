.PHONY: all clean install uninstall

DESTDIR=
PREFIX=/usr

LOCK_FILE="/var/lock/keyd.lock"
LOG_FILE="/var/log/keyd.log"
CONFIG_DIR="/etc/keyd"

VERSION=1.3.1
GIT_HASH=$(shell git describe --no-match --always --abbrev=40 --dirty)

CFLAGS=-DVERSION=\"$(VERSION)\" \
	-DGIT_COMMIT_HASH=\"$(GIT_HASH)\" \
	-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
	-DLOG_FILE=\"$(LOG_FILE)\" \
	-DLOCK_FILE=\"$(LOCK_FILE)\"\
	-I/usr/local/include\
	-L/usr/local/lib

all:
	mkdir -p bin
	$(CC) $(CFLAGS) -O3 src/*.c -o bin/keyd -ludev
debug:
	$(CC) $(CFLAGS) -O3 src/*.c -o bin/keyd -ludev -pedantic -Wall -Wextra -g
man:
	pandoc -s -t man man.md | gzip > keyd.1.gz
clean:
	-rm -rf bin
install:
	mkdir -p $(DESTDIR)/etc/keyd
	mkdir -p $(DESTDIR)$(PREFIX)/lib/systemd/system
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1

	install -m644 keyd.service $(DESTDIR)$(PREFIX)/lib/systemd/system
	install -m755 bin/keyd $(DESTDIR)$(PREFIX)/bin
	install -m644 keyd.1.gz $(DESTDIR)$(PREFIX)/share/man/man1
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service\
		bin/keyd $(DESTDIR)$(PREFIX)/bin/keyd\
		$(DESTDIR)$(PREFIX)/share/man/man1/keyd.1.gz

