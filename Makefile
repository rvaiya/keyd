.PHONY: all clean install uninstall debug man compose test-harness test test-io help
VERSION=2.6.0
COMMIT=$(shell git describe --no-match --always --abbrev=7 --dirty)
VKBD=uinput
PREFIX?=/usr/local

CONFIG_DIR?=/etc/keyd
SOCKET_PATH=/var/run/keyd.socket

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

platform=$(shell uname -s)

ifeq ($(platform), Linux)
	COMPAT_FILES=
else
	LDFLAGS+=-linotify
	COMPAT_FILES=
endif

all:
	mkdir -p bin
	cp scripts/keyd-application-mapper bin/
	$(CC) $(CFLAGS) -O3 $(COMPAT_FILES) src/*.c src/vkbd/$(VKBD).c -lpthread -o bin/keyd $(LDFLAGS)
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
			src/keys.c    \
			src/unicode.c && \
	./bin/test-io t/test.conf t/*.t

help:
	@echo "Usage: make <target> [VAR=value ...]"
	@echo ""
	@echo "Targets:"
	@echo "  all       Compile keyd binary (default VKBD=uinput)"
	@echo "  debug     Compile with ASAN + debug symbols"
	@echo "  man       Build man pages from .scdoc sources (requires scdoc)"
	@echo "  compose   Generate X compose table (data/keyd.compose)"
	@echo "  test      Run integration test suite (requires root)"
	@echo "  test-io   Run unit tests (keyboard logic, no root needed)"
	@echo "  install   Install to PREFIX [=/usr/local]"
	@echo "  uninstall Remove installed files"
	@echo "  clean     Remove build artifacts (bin/, keyd.service)"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX       Install prefix (default: /usr/local)"
	@echo "  CONFIG_DIR   Config directory (default: /etc/keyd)"
	@echo "  VKBD         Virtual keyboard backend (default: uinput)"
	@echo "               Alternatives: stdout, usb-gadget"
	@echo "  SOCKET_PATH  IPC socket path (default: /var/run/keyd.socket)"
	@echo ""
	@echo "Examples:"
	@echo "  make VKBD=usb-gadget && sudo make install VKBD=usb-gadget"
	@echo "  make test-io > /dev/null 2>&1; echo $?"

# End of file
