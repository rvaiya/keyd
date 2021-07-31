# Impetus

Linux lacks a good key remapping solution. In order to achieve satisfactory
results a medley of tools need to be employed (e.g xcape, xmodmap) with the end
result often being tethered to a specified environment (X11). keyd attempts to
solve this problem by providing a flexible system wide daemon which remaps keys
using kernel level input primitives (evdev, uinput).

# Features

keyd has several unique features many of which are traditionally only 
found in custom keyboard firmware like [QMK](https://github.com/qmk/qmk_firmware).
Some of the more interesting ones include:

- Layers.
- Key overloading (different behaviour on tap/hold).
- Per keyboard configuration.
- Instantaneous remapping (no flashing required).
- A simple and intuitive config format.
- Being display server agnostic (works on wayland and virtual console alike).

## Why would anyone want this?

### keyd is for people who:

 - Would like to experiment with [layers](https://beta.docs.qmk.fm/using-qmk/software-features/feature_layers) (i.e custom shift keys).
 - Want to have multiple keyboards with different logical layouts on the same machine.
 - Want to put the control and escape keys where God intended.
 - Would like the ability to easily generate keycodes in other languages.
 - Constantly fiddle with their key layout.
 - Want an inuitive keyboard config format which is simple to grok.
 - Wish to be able to switch to a VT to debug something without breaking their keymap.
 - Like tiny daemons that adhere to the Unix philosophy.

### What keyd isn't:

 - A tool for launching arbitrary system commands as root.
 - A tool for programming individual key up/down events.
 - A tool for generating unicode characters (which is best done higher up the input stack)

# Dependencies

 - Your favourite C compiler
 - libudev

# Installation

    sudo apt-get install libudev-dev # Debian specific, install the corresponding package on your distribution

    git clone https://github.com/rvaiya/keyd
    cd keyd
    make && sudo make DESTDIR=/usr install
    sudo systemctl enable keyd && sudo systemctl start keyd

# Quickstart

1. Install keyd

2. Put the following in `/etc/keyd/default.cfg`:

```
# Turns capslock into an escape key when pressed and a control key when held.
capslock = mods_on_hold(C, esc)

# Remaps the escape key to capslock
esc = capslock
```

3. Run `sudo systemctl restart keyd` to reload the config file.

4. See the [man page](man.md) for a comprehensive list of config options.

*Note*: It is possible to render your machine unusable with a bad config file.
Before proceeding ensure you have some way of killing keyd if things go wrong
(e.g ssh). It is recommended that you avoid experimenting in default.cfg (see
the man page for keyboard specific configuraiton) so you can plug in another
keyboard which is unaffected by the changes.

# Sample Config File

	# Maps escape to the escape layer when held and the escape key when pressed

	esc = layer_on_hold(escape_layer, esc)

	# Creates an escape layer which is activated by pressing the escape key.

	[escape_layer]

	# Esc+1 changes the letter layout to dvorak. 
	1 = layer_toggle(dvorak)

	# Esc+2 changes the letter layout back to the default. 
	2 = layer_toggle(default)

	# Creates a dvorak layer which inherits from the main layer (see the section on layer inheritance in the man page).

	[dvorak:default]

	q = apostrophe
	w = comma
	e = dot
	r = p
	t = y
	y = f
	u = g
	i = c
	o = r
	p = l

	a = a
	s = o
	d = e
	f = u
	g = i
	h = d
	j = h
	k = t
	l = n
	semicolon = s

	z = semicolon
	x = q
	c = j
	v = k
	b = x
	n = b
	m = m
	comma = w
	dot = v
	slash = z

# FAQS

## What about xmodmap/setxkbmap/*?

xmodmap and friends are display server level tools with limited functionality.
keyd is a system level solution which implements advanced features like
layering and
[oneshot](https://beta.docs.qmk.fm/using-qmk/software-features/one_shot_keys)
modifiers.  While some X tools offer similar functionality I am not aware of
anything that is as flexible as keyd.

## What about [kmonad](https://github.com/kmonad/kmonad)?

keyd was written several years ago to allow me to easily experiment with
different layouts on my growing keyboard collection. At the time kmonad did
not exist and custom keyboard firmware like [QMK](https://github.com/qmk/qmk_firmware) (which inspired keyd) was the
only way to get comparable features. I became aware of kmonad after having
published keyd. While kmonad is a fine project with similar goals, it takes
a different approach and has a different design philosophy.

Notably keyd was written entirely in C with performance and simplicitly in
mind and will likely never be as configurable as kmonad (which is extensible
in Haskell). Having said that, it supplies (in the author's opinion) the 
most valuable features in less than 2000 lines of C while providing
a simple language agnostic config format.

## Why doesn't keyd implement feature X?

If you feel something is missing or find a bug you are welcome to file an issue
on github. keyd has a minimalist (but sane) design philosophy which
intentionally omits certain features (e.g unicode/execing arbitrary executables
as root). Things which already exist in custom keyboard firmware like QMK are
good candidates for inclusion.
