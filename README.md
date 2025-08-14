[![Kofi](https://badgen.net/badge/icon/kofi?icon=kofi&label)](https://ko-fi.com/rvaiya)

# Impetus

[![Packaging status](https://repology.org/badge/tiny-repos/keyd.svg)](https://repology.org/project/keyd/versions)

Linux lacks a good key remapping solution. In order to achieve satisfactory
results a medley of tools need to be employed (e.g xcape, xmodmap) with the end
result often being tethered to a specified environment (X11). keyd attempts to
solve this problem by providing a flexible system wide daemon which remaps keys
using kernel level input primitives (evdev, uinput).

# Note on v2

The config format has undergone several iterations since the first
release. For those migrating their configs from v1 it is best
to reread the [man page](https://raw.githubusercontent.com/rvaiya/keyd/refs/heads/master/docs/keyd.scdoc) (`man keyd`).

See also: [changelog](docs/CHANGELOG.md).

# Goals

  - Speed       (a hand tuned input loop written in C that takes <<1ms)
  - Simplicity  (a [config format](#sample-config) that is intuitive)
  - Consistency (modifiers that [play nicely with layers](https://github.com/rvaiya/keyd/blob/6dc2d5c4ea76802fd192b143bdd53b1787fd6deb/docs/keyd.scdoc#L128) by default)
  - Modularity  (a UNIXy core extensible through the use of an [IPC](https://github.com/rvaiya/keyd/blob/90973686723522c2e44d8e90bb3508a6da625a20/docs/keyd.scdoc#L391) mechanism)

# Features

keyd has several unique features many of which are traditionally only
found in custom keyboard firmware like [QMK](https://github.com/qmk/qmk_firmware)
as well as some which are unique to keyd.

Some of the more interesting ones include:

- Layers (with support for [hybrid modifiers](https://github.com/rvaiya/keyd/blob/6dc2d5c4ea76802fd192b143bdd53b1787fd6deb/docs/keyd.scdoc#L128)).
- Key overloading (different behaviour on tap/hold).
- Keyboard specific configuration.
- Instantaneous remapping (no more flashing :)).
- A client-server model that facilitates scripting and display server agnostic application remapping. (Currently ships with support for X, sway, and gnome (wayland)).
- System wide config (works in a VT).
- First class support for modifier overloading.
- Unicode support.

### keyd is for people who:

 - Would like to experiment with custom [layers](https://docs.qmk.fm/feature_layers) (i.e custom shift keys)
   and oneshot modifiers.
 - Want to have multiple keyboards with different layouts on the same machine.
 - Want to be able to remap `C-1` without breaking modifier semantics.
 - Want a keyboard config format which is easy to grok.
 - Like tiny daemons that adhere to the Unix philosophy.
 - Want to put the control and escape keys where God intended.
 - Wish to be able to switch to a VT to debug something without breaking their keymap.

### What keyd isn't:

 - A tool for programming individual key up/down events.

# Dependencies

 - Your favourite C compiler
 - Linux kernel headers (already present on most systems)

## Optional

 - python      (for application specific remapping)
 - python-xlib (only for X support)
 - dbus-python (only for KDE support)

# Installation

*Note:* master serves as the development branch, things may occasionally break
between releases. Releases are [tagged](https://github.com/rvaiya/keyd/tags), and should be considered stable.

## From Source

    git clone https://github.com/rvaiya/keyd
    cd keyd
    make && sudo make install
    sudo systemctl enable --now keyd

# Quickstart

1. Install and start keyd (e.g `sudo systemctl enable keyd --now`)

2. Put the following in `/etc/keyd/default.conf`:

```
[ids]

*

[main]

# Maps capslock to escape when pressed and control when held.
capslock = overload(control, esc)

# Remaps the escape key to capslock
esc = capslock
```

Key names can be obtained by using the `keyd monitor` command. Note that while keyd is running, the output of this
command will correspond to keyd's output. The original input events can be seen by first stopping keyd and then
running the command. See the man page for more details.

3. Run `sudo keyd reload` to reload the config set.

4. See the man page (`man keyd`) for a more comprehensive description.

Config errors will appear in the log output and can be accessed in the usual
way using your system's service manager (e.g `sudo journalctl -eu keyd`).

*Note*: It is possible to render your machine unusable with a bad config file.
Should you find yourself in this position, the special key sequence
`backspace+escape+enter` should cause keyd to terminate.

Some mice (e.g the Logitech MX Master) are capable of emitting keys and
are consequently matched by the wildcard id. It may be necessary to
explicitly blacklist these.

## Application Specific Remapping (experimental)

- Add yourself to the keyd group:

	`usermod -aG keyd <user>`

- Populate `~/.config/keyd/app.conf`:

E.G

	[alacritty]

	alt.] = macro(C-g n)
	alt.[ = macro(C-g p)

	[chromium]

	alt.[ = C-S-tab
	alt.] = macro(C-tab)

- Run:

	`keyd-application-mapper`

You will probably want to put `keyd-application-mapper -d` somewhere in your 
display server initialization logic (e.g ~/.xinitrc) unless you are running Gnome.

See the man page for more details.

## SBC support

Experimental support for single board computers (SBCs) via usb-gadget
has been added courtesy of Giorgi Chavchanidze.

See [usb-gadget.md](src/vkbd/usb-gadget.md) for details.

## Packages

Third-party packages exist for some distributions. If you wish to add
yours to the list please file a PR. These are kindly maintained by community
members, no personal responsibility is taken for them.

### Alpine Linux

[keyd](https://pkgs.alpinelinux.org/packages?name=keyd) package maintained by [@jirutka](https://github.com/jirutka).

### Arch

[Arch Linux](https://archlinux.org/packages/extra/x86_64/keyd/) package maintained by Arch packagers.

### Debian

A keyd package is available in Debian 13 ("trixie") and later.  To install:

```shell
sudo apt install keyd
```

### Fedora

[COPR](https://copr.fedorainfracloud.org/coprs/alternateved/keyd/) package maintained by [@alternateved](https://github.com/alternateved).

### Gentoo

[GURU](https://gitweb.gentoo.org/repo/proj/guru.git/tree/app-misc/keyd) package maintained by [jack@pngu.org](mailto:jack@pngu.org).

### openSUSE
[opensuse](https://software.opensuse.org//download.html?project=hardware&package=keyd) package maintained by [@bubbleguuum](https://github.com/bubbleguuum).

Easy install with `sudo zypper in keyd`.

### Ubuntu

A keyd package is available in Ubuntu 25.04 ("plucky") and later.  To install:

```shell
sudo apt install keyd
```

In addition, the latest Debian package backported to various Ubuntu releases can
be found in the [`ppa:keyd-team/ppa`
archive](https://launchpad.net/~keyd-team/+archive/ubuntu/ppa).

### Void Linux

[xbps](https://github.com/void-linux/void-packages/tree/master/srcpkgs/keyd) package maintained by [@Barbaross](https://gitlab.com/Barbaross).

Easy install with `sudo xbps-install -Su keyd`.

# Example 1

	[ids]

	*
	
	[main]

	leftshift = oneshot(shift)
	capslock = overload(symbols, esc)

	[symbols]

	d = ~
	f = /
	...

# Example 2

This overrides specific alt combinations macOS users might
be more familiar with, while keeping the rest intact.

	[ids]
	*

	[alt]

	x = C-x
	c = C-c
	v = C-v

	a = C-a
	f = C-f
	r = C-r
	z = C-z

# Recommended config

Many users will probably not be interested in taking full advantage of keyd.
For those who seek simple quality of life improvements I can recommend the
following config:

	[ids]

	*

	[main]

	shift = oneshot(shift)
	meta = oneshot(meta)
	control = oneshot(control)

	leftalt = oneshot(alt)
	rightalt = oneshot(altgr)

	capslock = overload(control, esc)
	insert = S-insert

This overloads the capslock key to function as both escape (when tapped) and
control (when held) and remaps all modifiers to 'oneshot' keys. Thus to produce
the letter A you can now simply tap shift and then a instead of having to hold
it. Finally it remaps insert to S-insert (paste on X11).

# FAQS

## Why is my trackpad is interfering with input after enabling keyd?

libinput, a higher level input component used by most wayland and X11 setups,
includes a feature called 'disable-while-typing' that disables the trackpad
when typing.

In order to achieve this, it needs to distinguish between internal and external
keyboards, which it does by hard coding a rules for specific hardware
('quirks'). Since keyd creates a virtual device which subsumes both external
and integrated keyboards, you will need to instruct libinput to regard the keyd
virtual device as internal.

This can be achieved by adding the following to `/etc/libinput/local-overrides.quirks` (which may need to be created):

```
[Serial Keyboards]

MatchUdevType=keyboard
MatchName=keyd*keyboard
AttrKeyboardIntegration=internal
```

Credit to @mark-herbert42 and @canadaduane for the original solution.

## What about xmodmap/setxkbmap/*?

xmodmap and friends are display server level tools with limited functionality.
keyd is a system level solution which implements advanced features like
layering and [oneshot](https://docs.qmk.fm/#/one_shot_keys)
modifiers.  While some X tools offer similar functionality I am not aware of
anything that is as flexible as keyd.

## What about [kmonad](https://github.com/kmonad/kmonad)?

keyd was written several years ago to allow me to easily experiment with
different layouts on my growing keyboard collection. At the time kmonad did not
exist and custom keyboard firmware like
[QMK](https://github.com/qmk/qmk_firmware) (which inspired keyd) was the only
way to get comparable features. I became aware of kmonad after having published
keyd. While kmonad is a fine project with similar goals, it takes a different
approach and has a different design philosophy.

Notably keyd was written entirely in C with performance and simplicitly in
mind and will likely never be as configurable as kmonad (which is extensible
in Haskell). Having said that, it supplies (in the author's opinion) the
most valuable features in less than 2000 lines of C while providing
a simple language agnostic config format.

## Why doesn't keyd implement feature X?

If you feel something is missing or find a bug you are welcome to file an issue
on github. keyd has a minimalist (but sane) design philosophy which
intentionally omits certain features (e.g execing arbitrary executables
as root). Things which already exist in custom keyboard firmware like QMK are
good candidates for inclusion.

# Contributing

See [CONTRIBUTING](CONTRIBUTING.md).
IRC Channel: #keyd on oftc
