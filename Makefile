.PHONY: all clean install install-usb-gadget uninstall uninstall-usb-gadget debug man
DESTDIR=
PREFIX=/usr

VERSION=2.3.0-rc
COMMIT=$(shell git describe --no-match --always --abbrev=7 --dirty)

CFLAGS+=-DVERSION=\"v$(VERSION)\ \($(COMMIT)\)\" \
	-I/usr/local/include \
	-L/usr/local/lib

platform+=$(shell uname -s)

ifeq ($(platform), Linux)
	COMPAT_FILES=
else
	LDFLAGS+=-linotify
	COMPAT_FILES=
endif

all: vkbd-uinput
vkbd-%:
	mkdir -p bin
	$(CC) $(CFLAGS) -O3 $(COMPAT_FILES) src/*.c src/vkbd/$(@:vkbd-%=%).c -o bin/keyd -lpthread $(LDFLAGS)
debug:
	CFLAGS="-pedantic -Wall -Wextra -g" $(MAKE)
man:
	scdoc < keyd.md | gzip > keyd.1.gz
	scdoc < keyd-application-mapper.md | gzip > keyd-application-mapper.1.gz
clean:
	-rm -rf bin
install:
	mkdir -p $(DESTDIR)/etc/keyd
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/keyd
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/keyd/examples

	@if [ -e $(DESTDIR)$(PREFIX)/lib/systemd/ ]; then \
		mkdir -p $(DESTDIR)$(PREFIX)/lib/systemd/system; \
		install -m644 keyd.service $(DESTDIR)$(PREFIX)/lib/systemd/system; \
	else \
		echo "NOTE: systemd not found, you will need to manually add keyd to your system's init process."; \
	fi

	@if [ -e $(DESTDIR)$(PREFIX)/share/libinput/ ]; then \
		install -m644 keyd.quirks $(DESTDIR)$(PREFIX)/share/libinput/30-keyd.quirks; \
	else \
		echo "WARNING: libinput not found, not installing keyd.quirks."; \
	fi


	-groupadd keyd
	install -m755 bin/keyd $(DESTDIR)$(PREFIX)/bin
	install -m755 scripts/keyd-application-mapper $(DESTDIR)$(PREFIX)/bin
	install -m644 keyd.1.gz $(DESTDIR)$(PREFIX)/share/man/man1
	install -m644 keyd-application-mapper.1.gz $(DESTDIR)$(PREFIX)/share/man/man1
	install -m644 DESIGN.md CHANGELOG.md README.md $(DESTDIR)$(PREFIX)/share/doc/keyd
	install -m644 examples/* $(DESTDIR)$(PREFIX)/share/doc/keyd/examples

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/share/libinput/30-keyd.quirks\
		$(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service\
		bin/keyd $(DESTDIR)$(PREFIX)/bin/keyd\
		$(DESTDIR)$(PREFIX)/bin/keyd-application-mapper\
		$(DESTDIR)$(PREFIX)/share/man/man1/keyd.1.gz\
		$(DESTDIR)$(PREFIX)/share/man/man1/keyd-application-mapper.1.gz

install-usb-gadget: install
	install -m644 src/vkbd/usb-gadget.service $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service
	install -m755 src/vkbd/usb-gadget.sh $(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh

uninstall-vkbd-usb-gadget: uninstall
	rm -f $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service\
		$(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh
test: all
	@cd t; \
	for f in *.sh; do \
		./$$f; \
	done
