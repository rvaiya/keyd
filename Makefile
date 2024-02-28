.PHONY: all clean install uninstall debug man compose test-harness
VERSION=2.5.0
COMMIT=$(shell git describe --no-match --always --abbrev=7 --dirty)
VKBD=uinput
PREFIX?=/usr/local

CONFIG_DIR?=/etc/keyd
SOCKET_PATH=/var/run/keyd.socket

SYSTEMD_SYSTEM_DIR = /usr/lib/systemd/system

CFLAGS:=-DVERSION=\"v$(VERSION)\ \($(COMMIT)\)\" \
	-I/usr/local/include \
	-L/usr/local/lib \
	-Wall \
	-Wextra \
	-Wno-unused \
	-std=c11 \
	-DSOCKET_PATH=\"$(SOCKET_PATH)\" \
	-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
	-DDATA_DIR=\"$(PREFIX)/share/keyd\" \
	-D_FORTIFY_SOURCE=2 \
	-D_DEFAULT_SOURCE \
	-Werror=format-security \
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
	$(CC) $(CFLAGS) -O3 $(COMPAT_FILES) src/*.c src/vkbd/$(VKBD).c -lpthread -o bin/keyd $(LDFLAGS)
debug:
	CFLAGS="-g -fsanitize=address -Wunused" $(MAKE)
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

	@if [ -e /run/systemd/system -o "$(FORCE_SYSTEMD)" ]; then \
		sed -e 's#@PREFIX@#$(PREFIX)#' keyd.service.in > keyd.service; \
		mkdir -p '$(DESTDIR)$(SYSTEMD_SYSTEM_DIR)'; \
		install -Dm644 keyd.service '$(DESTDIR)$(SYSTEMD_SYSTEM_DIR)/keyd.service'; \
		mkdir -p $(DESTDIR)$(PREFIX)/lib/sysusers.d/; \
		install -m644 data/sysusers.d $(DESTDIR)$(PREFIX)/lib/sysusers.d/keyd.conf; \
	else \
		echo "NOTE: systemd not found, you will need to manually add keyd to your system's init process."; \
	fi

	@if [ "$(VKBD)" = "usb-gadget" ]; then \
		sed -e 's#@PREFIX@#$(PREFIX)#' src/vkbd/usb-gadget.service.in > src/vkbd/usb-gadget.service; \
		install -Dm644 src/vkbd/usb-gadget.service '$(DESTDIR)$(SYSTEMD_SYSTEM_DIR)/keyd-usb-gadget.service'; \
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
		'$(DESTDIR)$(SYSTEMD_SYSTEM_DIR)/keyd-usb-gadget.service' \
		$(DESTDIR)$(PREFIX)/bin/keyd-usb-gadget.sh \
		'$(DESTDIR)$(SYSTEMD_SYSTEM_DIR)/keyd.service' \
		$(DESTDIR)$(PREFIX)/share/doc/keyd \
		$(DESTDIR)$(PREFIX)/lib/sysusers.d/keyd.conf
clean:
	-rm -rf bin keyd.service src/vkbd/usb-gadget.service
test:
	@cd t; \
	for f in *.sh; do \
		./$$f; \
	done
test-io:
	-mkdir bin
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
