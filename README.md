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

# Dependencies

 - make
 - gcc
 - libudev

# Installation

    git clone https://github.com/rvaiya/keyd
    cd keyd
    sudo apt-get install libudev-dev # Debian specific install the corresponding package on your distribution
    make && sudo make install
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
