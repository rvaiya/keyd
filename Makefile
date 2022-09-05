.PHONY: all clean install uninstall debug man compose test-harness
DESTDIR=
PREFIX=/usr

VERSION=2.4.2
COMMIT=$(shell git describe --no-match --always --abbrev=7 --dirty)
VKBD=uinput

CONFIG_DIR=/etc/keyd
SOCKET_PATH=/var/run/keyd.socket

CFLAGS:=-DVERSION=\"v$(VERSION)\ \($(COMMIT)\)\" \
	-I/usr/local/include \
	-L/usr/local/lib \
	-Wall \
	-Wextra \
	-Wno-unused \
	-std=c11 \
	-DSOCKET_PATH=\"$(SOCKET_PATH)\" \
	-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
	-D_DEFAULT_SOURCE \
	$(CFLAGS)

platform=$(shell uname -s)

ifeq ($(platform), Linux)
	COMPAT_FILES=
else
	LDFLAGS+=-linotify
	COMPAT_FILES=
endif

all:
	-mkdir bin
	cp scripts/keyd-application-mapper bin/
	$(CC) $(CFLAGS) -O3 $(COMPAT_FILES) src/*.c src/vkbd/$(VKBD).c -o bin/keyd -lpthread $(LDFLAGS)
debug:
	CFLAGS="-g -Wunused" $(MAKE)
compose:
	-mkdir data
	./scripts/generate_xcompose
man:
	for f in docs/*.scdoc; do \
		target=$${f%%.scdoc}.1.gz; \
		target=data/$${target##*/}; \
		scdoc < "$$f" | gzip > "$$target"; \
	done
install:
	@if [ -e $(DESTDIR)$(PREFIX)/lib/systemd/ ]; then \
		install -Dm644 keyd.service $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service; \
	else \
		echo "NOTE: systemd not found, you will need to manually add keyd to your system's init process."; \
	fi

	@if [ -e $(DESTDIR)$(PREFIX)/share/libinput/ ]; then \
		install -Dm644 keyd.quirks $(DESTDIR)$(PREFIX)/share/libinput/30-keyd.quirks; \
	else \
		echo "WARNING: libinput not found, not installing keyd.quirks."; \
	fi

	@if [ "$(VKBD)" = "usb-gadget" ]; then \
		install -Dm644 src/vkbd/usb-gadget.service $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service; \
		install -Dm755 src/vkbd/usb-gadget.sh $(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh; \
	fi

	mkdir -p $(DESTDIR)/etc/keyd
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	mkdir -p $(DESTDIR)$(PREFIX)/share/keyd/
	mkdir -p $(DESTDIR)$(PREFIX)/share/keyd/layouts/
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1/
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/keyd/
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/keyd/examples/

	-groupadd keyd
	install -m755 bin/* $(DESTDIR)$(PREFIX)/bin/
	install -m644 docs/*.md $(DESTDIR)$(PREFIX)/share/doc/keyd/
	install -m644 examples/* $(DESTDIR)$(PREFIX)/share/doc/keyd/examples/
	install -m644 layouts/* $(DESTDIR)$(PREFIX)/share/keyd/layouts
	install -m644 data/*.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/
	install -m644 data/keyd.compose $(DESTDIR)$(PREFIX)/share/keyd/

uninstall:
	rm -rf $(DESTDIR)$(PREFIX)/share/libinput/30-keyd.quirks \
		$(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service \
		$(DESTDIR)$(PREFIX)/bin/keyd \
		$(DESTDIR)$(PREFIX)/bin/keyd-application-mapper \
		$(DESTDIR)$(PREFIX)/share/doc/keyd/ \
		$(DESTDIR)$(PREFIX)/share/man/man1/keyd*.gz \
		$(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service \
		$(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh
clean:
	-rm -rf bin
test:
	@cd t; \
	for f in *.sh; do \
		./$$f; \
	done
