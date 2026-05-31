.PHONY: all clean install uninstall debug man compose test-harness
VERSION=2.6.0
COMMIT=$(shell git describe --no-match --always --abbrev=7 --dirty)
PREFIX?=/usr/local

CONFIG_DIR?=/etc/keyd
platform=$(shell uname -s)

# Platform-specific socket path: /var/run requires root on macOS, /tmp is
# world-writable and sufficient for single-user desktop use.
ifeq ($(platform), Darwin)
	SOCKET_PATH=/tmp/keyd.socket
else
	SOCKET_PATH=/var/run/keyd.socket
endif

# CFLAGS is a simply-expanded variable (:=) so $(SOCKET_PATH) is captured now,
# after the platform override above.
CFLAGS:=-DVERSION=\"v$(VERSION)\ \($(COMMIT)\)\" \
	-I/usr/local/include \
	-L/usr/local/lib \
	-Wall \
	-Wextra \
	-Wstrict-prototypes \
	-Wno-unused \
	-std=c11 \
	-DSOCKET_PATH=\"$(SOCKET_PATH)\" \
	-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
	-DDATA_DIR=\"$(PREFIX)/share/keyd\" \
	-D_FORTIFY_SOURCE=2 \
	-D_DEFAULT_SOURCE \
	-Werror=format-security \
	$(CFLAGS)

# Core sources shared across all platforms (no Linux/macOS-specific I/O).
CORE_SRCS = src/keyd.c src/daemon.c src/keyboard.c src/config.c src/keys.c \
            src/ipc.c src/macro.c src/unicode.c src/log.c src/string.c \
            src/ini.c src/check.c src/monitor.c src/util.c src/dbg.c

ifeq ($(platform), Darwin)
	VKBD=macos
	PLATFORM_SRCS = src/macos/input.c src/macos/keycodes.c
	PLATFORM_LDFLAGS = -framework CoreGraphics -framework ApplicationServices
	CFLAGS += -DPLATFORM_MACOS
else ifeq ($(platform), Linux)
	VKBD=uinput
	PLATFORM_SRCS = src/device.c src/evloop.c
	PLATFORM_LDFLAGS =
else
	# FreeBSD or other — keep the old behaviour
	VKBD=uinput
	PLATFORM_SRCS = src/device.c src/evloop.c
	PLATFORM_LDFLAGS = -linotify
endif

all:
	mkdir -p bin
	cp scripts/keyd-application-mapper bin/
	$(CC) $(CFLAGS) -O3 $(CORE_SRCS) $(PLATFORM_SRCS) src/vkbd/$(VKBD).c \
	    -lpthread -o bin/keyd $(PLATFORM_LDFLAGS) $(LDFLAGS)
debug:
	CFLAGS="-g -fsanitize=address -Wunused" $(MAKE)
compose:
	mkdir -p data
	./scripts/generate_xcompose
man:
	for f in docs/*.scdoc; do \
		target=$${f%%.scdoc}.1.gz; \
		target=data/$${target##*/}; \
		scdoc < "$$f" | gzip > "$$target"; \
	done
install:

	@if [ -e /run/systemd/system -o "$(FORCE_SYSTEMD)" ]; then \
		sed -e 's#@PREFIX@#$(PREFIX)#' keyd.service.in > keyd.service; \
		mkdir -p $(DESTDIR)$(PREFIX)/lib/systemd/system/; \
		install -Dm644 keyd.service $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service; \
		mkdir -p $(DESTDIR)$(PREFIX)/lib/sysusers.d/; \
		install -m644 data/sysusers.d $(DESTDIR)$(PREFIX)/lib/sysusers.d/keyd.conf; \
	else \
		echo "NOTE: systemd not found, you will need to manually add keyd to your system's init process."; \
	fi

	@if [ "$(VKBD)" = "usb-gadget" ]; then \
		sed -e 's#@PREFIX@#$(PREFIX)#' src/vkbd/usb-gadget.service.in > src/vkbd/usb-gadget.service; \
		install -Dm644 src/vkbd/usb-gadget.service $(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service; \
		install -Dm755 src/vkbd/usb-gadget.sh $(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh; \
	fi

	mkdir -p $(DESTDIR)$(CONFIG_DIR)
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
	cp -r data/gnome-* $(DESTDIR)$(PREFIX)/share/keyd
	install -m644 data/*.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/
	install -m644 data/keyd.compose $(DESTDIR)$(PREFIX)/share/keyd/
	install -m644 README.md $(DESTDIR)$(PREFIX)/share/doc/keyd

uninstall:
	-groupdel keyd
	rm -rf $(DESTDIR)$(PREFIX)/bin/keyd \
		$(DESTDIR)$(PREFIX)/bin/keyd-application-mapper \
		$(DESTDIR)$(PREFIX)/share/doc/keyd/ \
		$(DESTDIR)$(PREFIX)/share/man/man1/keyd*.gz \
		$(DESTDIR)$(PREFIX)/share/keyd/ \
		$(DESTDIR)$(PREFIX)/lib/systemd/system/keyd-usb-gadget.service \
		$(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh \
		$(DESTDIR)$(PREFIX)/lib/systemd/system/keyd.service \
		$(DESTDIR)$(PREFIX)/lib/sysusers.d/keyd.conf
clean:
	rm -rf bin keyd.service src/vkbd/usb-gadget.service
test:
	@cd t; \
	for f in *.sh; do \
		./$$f; \
	done
test-io:
	mkdir -p bin
	$(CC) \
	-DDATA_DIR=\"\" \
	-o bin/test-io \
		t/test-io.c \
		src/keyboard.c \
		src/string.c \
		src/macro.c \
		src/config.c \
		src/log.c \
		src/ini.c \
		src/keys.c  \
		src/unicode.c && \
	./bin/test-io t/test.conf t/*.t
