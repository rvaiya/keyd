AM_DISTCHECK_CONFIGURE_FLAGS =
bin_PROGRAMS =
bin_SCRIPTS =
pkgdata_DATA =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = \
	.autom4te.cfg \
	CONTRIBUTING.md \
	LICENSE \
	README.md \
	autogen \
	scripts/dump-xkb-config \
	src/vkbd/usb-gadget.md

AM_CPPFLAGS = \
	-D'CONFIG_DIR="$(sysconfdir)/keyd"' \
	-D'DATA_DIR="$(pkgdatadir)"' \
	-D'SOCKET_PATH="$(runstatedir)/keyd.socket"'

bin_PROGRAMS += keyd
keyd_SOURCES = \
	src/config.c \
	src/config.h \
	src/daemon.c \
	src/device.c \
	src/device.h \
	src/evloop.c \
	src/ini.c \
	src/ini.h \
	src/ipc.c \
	src/keyboard.c \
	src/keyboard.h \
	src/keyd.c \
	src/keyd.h \
	src/keys.c \
	src/keys.h \
	src/log.c \
	src/log.h \
	src/macro.c \
	src/macro.h \
	src/monitor.c \
	src/strutil.c \
	src/strutil.h \
	src/unicode.h \
	src/util.c \
	src/vkbd.h
nodist_keyd_SOURCES = \
	src/unicode.c
BUILT_SOURCES += $(nodist_keyd_SOURCES)
CLEANFILES += $(nodist_keyd_SOURCES)
keyd_CPPFLAGS = $(AM_CPPFLAGS) -Isrc -I$(srcdir)/src

if USB_GADGET
nodist_keyd_SOURCES += src/vkbd/usb-gadget.h
bin_SCRIPTS += keyd-usb-gadget.sh
CLEANFILES += keyd-usb-gadget.sh
keyd-usb-gadget.sh: src/vkbd/usb-gadget.sh
	sed -e '1s;^#!.*;#!$(BASH);' $< >$@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@
endif USB_GADGET
nodist_keyd_SOURCES += src/vkbd/$(VKBD).c
EXTRA_DIST += \
	src/vkbd/stdout.c \
	src/vkbd/uinput.c \
	src/vkbd/usb-gadget.c \
	src/vkbd/usb-gadget.h \
	src/vkbd/usb-gadget.sh

systemdsystemunit_DATA =
if SYSTEMD_SYSTEM_UNIT
systemdsystemunit_DATA += keyd.service
if USB_GADGET
systemdsystemunit_DATA += src/vkbd/usb-gadget.service
endif USB_GADGET
# `make distcheck` simulates installation by an unprivileged user by setting $prefix to a temporary
# directory and $DESTDIR to the empty string, so absolute paths like /lib/systemd/system cause `make
# distcheck` to fail.  Fool it by pretending the systemd unit files can be installed in
# ${prefix}/lib/systemd/system.
AM_DISTCHECK_CONFIGURE_FLAGS += --with-systemd-system-unit='$${prefix}$(systemdsystemunitdir)'
else !SYSTEMD_SYSTEM_UNIT
AM_DISTCHECK_CONFIGURE_FLAGS += --without-systemd-system-unit
endif !SYSTEMD_SYSTEM_UNIT
$(systemdsystemunit_DATA): %.service: %.service.in
	sed -e 's,[@]PREFIX[@],$(prefix),g' <$< >$@.tmp
	mv $@.tmp $@
EXTRA_DIST += keyd.service.in src/vkbd/usb-gadget.service.in
CLEANFILES += $(systemdsystemunit_DATA)

bin_SCRIPTS += keyd-application-mapper
CLEANFILES += keyd-application-mapper
EXTRA_DIST += scripts/keyd-application-mapper
keyd-application-mapper: scripts/keyd-application-mapper
	sed -e '1s;^#!.*;#!$(PYTHON);' $< >$@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

dist_doc_DATA = \
	docs/CHANGELOG.md \
	docs/DESIGN.md
examplesdir = $(docdir)/examples
dist_examples_DATA = \
	examples/capslock-esc-basic.conf \
	examples/capslock-escape-with-vim-mode.conf \
	examples/international-glyphs.conf \
	examples/macos.conf \
	examples/nav-layer.conf

pkgdata_DATA += keyd.compose
EXTRA_DIST += \
	data/unicode.txt \
	scripts/generate_xcompose
CLEANFILES += compose.stamp keyd.compose
compose.stamp: data/unicode.txt scripts/generate_xcompose
	@rm -f $@.tmp
	@touch $@.tmp
	@mkdir -p src
	$(PYTHON) $(srcdir)/scripts/generate_xcompose $< keyd.compose src/unicode.c
	@mv -f $@.tmp $@
src/unicode.c keyd.compose: compose.stamp
	@test -f $@ || rm -f $<
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $<

gnomeexdir = $(pkgdatadir)/gnome-extension
dist_gnomeex_DATA = \
	data/gnome-extension/extension.js \
	data/gnome-extension/metadata.json
gnomeex45dir = $(pkgdatadir)/gnome-extension-45
dist_gnomeex45_DATA = \
	data/gnome-extension-45/extension.js \
	data/gnome-extension-45/metadata.json

layouts = \
	af \
	al \
	am \
	ara \
	at \
	au \
	az \
	ba \
	bd \
	be \
	bg \
	br \
	brai \
	bt \
	bw \
	by \
	ca \
	cd \
	ch \
	cm \
	cn \
	colemak \
	cz \
	de \
	dk \
	dvorak \
	dz \
	ee \
	epo \
	es \
	et \
	fi \
	fo \
	fr \
	gb \
	ge \
	gh \
	gn \
	gr \
	hr \
	hu \
	id \
	ie \
	il \
	in \
	iq \
	ir \
	is \
	it \
	jp \
	jv \
	ke \
	kg \
	kh \
	kr \
	kz \
	la \
	latam \
	lk \
	lt \
	lv \
	ma \
	mao \
	md \
	me \
	mk \
	ml \
	mm \
	mn \
	mt \
	mv \
	my \
	ng \
	nl \
	no \
	np \
	ph \
	pk \
	pl \
	pt \
	ro \
	rs \
	ru \
	se \
	si \
	sk \
	sn \
	sy \
	tg \
	th \
	tj \
	tm \
	tr \
	tw \
	tz \
	ua \
	uz \
	vn \
	workman \
	za
layoutsdir = $(pkgdatadir)/layouts
dist_layouts_DATA = $(layouts:%=layouts/%)

man1_MANS = keyd.1 keyd-application-mapper.1
CLEANFILES += $(man1_MANS)
EXTRA_DIST += $(man1_MANS:%.1=docs/%.scdoc)
$(man1_MANS): %.1: docs/%.scdoc
	$(SCDOC) <$< >$@.tmp
	mv $@.tmp $@

EXTRA_DIST += build-aux/git-version-gen
dist-hook:
	printf %s\\n '$(VERSION)' >$(distdir)/.dist-version

test: check
check_PROGRAMS = test-io
test_io_SOURCES = \
	src/config.c \
	src/ini.c \
	src/keyboard.c \
	src/keys.c \
	src/keys.h \
	src/log.c \
	src/macro.c \
	src/strutil.c \
	t/test-io.c
nodist_test_io_SOURCES = \
	src/unicode.c
test_io_CPPFLAGS = $(AM_CPPFLAGS) -Isrc -I$(srcdir)/src
TEST_EXTENSIONS = .t
T_LOG_COMPILER = ./test-io '$(srcdir)'/t/test.conf
handwritten_tests = \
	t/altgr-modifier-guard.t \
	t/chord-disambiguate.t \
	t/chord-double.t \
	t/chord-hold.t \
	t/chord.t \
	t/chord2.t \
	t/chord3.t \
	t/clear.t \
	t/composite.t \
	t/composite2.t \
	t/composite3.t \
	t/composite4.t \
	t/control.t \
	t/disarm.t \
	t/disarm2.t \
	t/disarm3.t \
	t/layer.t \
	t/layer1.t \
	t/layer2.t \
	t/layer3.t \
	t/layer4.t \
	t/layerm.t \
	t/layout-mods.t \
	t/layout-mods2.t \
	t/layout-seq.t \
	t/layout.t \
	t/layout2.t \
	t/macro-disarm.t \
	t/macro-hold-1.t \
	t/macro-hold-2.t \
	t/macro-hold.t \
	t/macro-nested.t \
	t/macro-unicode-2.t \
	t/macro-unicode.t \
	t/macro.t \
	t/meta.t \
	t/mod.t \
	t/mod2.t \
	t/mod3.t \
	t/oneshot+overload.t \
	t/oneshot-single-key.t \
	t/oneshot.t \
	t/oneshot10.t \
	t/oneshot11.t \
	t/oneshot12.t \
	t/oneshot14.t \
	t/oneshot2.t \
	t/oneshot3.t \
	t/oneshot4.t \
	t/oneshot5.t \
	t/oneshot6.t \
	t/oneshot9.t \
	t/oneshotm.t \
	t/oneshotn.t \
	t/oneshotn3.t \
	t/overload-expire.t \
	t/overload-expire2.t \
	t/overload-nested.t \
	t/overload-nested2.t \
	t/overload-oneshot.t \
	t/overload-same-key.t \
	t/overload-swap.t \
	t/overload.t \
	t/overload1.t \
	t/overload2.t \
	t/overload3.t \
	t/overload4.t \
	t/overload5.t \
	t/overload_2-2.t \
	t/overload_2.t \
	t/overload_3.t \
	t/swap-oneshot.t \
	t/swap-toggle.t \
	t/swap.t \
	t/swap2.t \
	t/swap3.t \
	t/swap4.t \
	t/swap5.t \
	t/swap6.t \
	t/swap8.t \
	t/swap9.t \
	t/timeout1.t \
	t/timeout2.t \
	t/toggle-2.t \
	t/toggle.t \
	t/toggle2.t
generated_tests =
TESTS = $(handwritten_tests) $(generated_tests)
EXTRA_DIST += $(handwritten_tests) \
	t/keys.py \
	t/run.sh \
	t/runner.py \
	t/test.conf
