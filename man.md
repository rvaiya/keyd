% KEYD(1)

# NAME

**keyd** - A key remapping daemon.

# SYNOPSIS

**keyd** [**-m**] [**-l**] [**-d**]

# DESCRIPTION

keyd is a system wide key remapping daemon which supports features like
layering, oneshot modifier, and macros. In its most basic form it can be used
to define a custom key layout that persists accross display server boundaries
(e.g wayland/X/tty).

keyd is intended to run as a systemd service but is capable of running
independently. The default behaviour is to run the forground and print to
stderr, unless **-d** is supplied, in which case in which case log output will
be stored in */var/log/keyd.log*.

**NOTE**

Becuase keyd modifies your primary input device it is possible to render your
machine unusuable with a bad config file. If you find yourself in this
situation the sequence *\<backspace\>+\<backslash\>+\<enter\>* will force keyd to
terminate. If you are experimenting with the available options it is advisable
to do so in a keyboard specific file (see CONFIGURATION) instead of directly in
`default.cfg`, so it remains possible to plug in another keyboard unaffected by the
changes.

# OPTIONS

**-m**: Run in monitor mode. (ensure keyd is not running to see untranslated events).

**-l**: List all valid key names.

**-d**: Fork and run in the background.

# CONFIGURATION

All configuration files are stored in */etc/keyd/*. The name of each file
should correspond to the device name to which it is to be applied followed by
.cfg (e.g "/etc/keyd/Magic Keyboard.cfg"). Configuration files are loaded upon
initialization and can be reified by reloading keyd (e.g sudo systemctl restart
keyd).

A list of valid key names can be produced with **-l**. The monitor flag (**-m**) can
also be used to obtain device and key names like so:

	> sudo systemctl stop keyd
	> sudo keyd -m

	Magic Keyboard: capslock down
	Magic Keyboard: capslock up
	...

If no configuration file exists for a given keyboard *default.cfg* is used as a
fallback (if present).

Each line in a configuration file consists of a mapping of the following form:

	<key> = <action>|<keyseq>

or else represents the beginning of a new layer. E.G:

	[<layer>]

Where `<keyseq>` has the form: `[<modifier1>[-<modifier2>...]-<key>`

and each modifier is one of:

\ **C** - Control\
\ **M** - Meta/Super\
\ **A** - Alt\
\ **S** - Shift\
\ **G** - AltGr

Lines can be commented out by prepending # but inline comments are not supported.

In addition to simple key mappings keyd can remap keys to actions which
can conditionally send keystrokes or transform the state of the keymap.

It is, for instance, possible to map a key to escape when tapped and control
when held by assigning it to `overload(C, esc)`. A complete list of available
actions can be found in *ACTIONS*.

As a special case \<key\> may be 'noop' which causes it to be
ignored. This can be used to simulate a modifier sequence with no
attendant termination key:

E.G 

`C-A-noop` will simulate the simultaneous depression and release
of the control and alt keys.


## Layers

Each configuration file consists of one or more layers. Each layer is a keymap
unto itself and can be transiently activated by a key mapped to the *layer*
action.

For example the following configuration creates a new layer called 'symbols' which
is activated by holding the capslock key.

	capslock = layer(symbols)

	[symbols]

	f = ~
	d = /

Pressing `capslock+f` thus produces a tilde.

Any set of valid modifiers is also a valid layer. For example the layer `M-C`
corresponds to a layer which behaves like the modifiers meta and control. These
play nicely with other modifiers and preserve existing stacking semantics.

A layer may optionally have a parent from which mappings are drawn for keys
which are not explicitly mapped. By default layers do not have a parent, that
is, unmapped keys will have no effect. A parent is specified by appending
`:<parent>` to the layer name.

The *layout* is a special layer from which mappings are drawn if no other layers
are active.  The default layout is called **main** and is the one to which
mappings are assigned if no layer heading is present. By default all keys are
defined as themselves in the main layer. Layouts should inherit from main to 
avoid having to explicitly define each key. The default layout can be
changed by including `layout(<layer>)` at the top of the config file.

## The Modifier Layout

keyd distinguishes between the key layout and the modifier layout. This
allows the user to use a different letter arrangement for modifiers. It may,
for example, be desireable to use an alternative key layout like dvorak while
preserving standard qwerty modifier shortcuts. This can be achieved by passing
a second argument to the layout function like so: `layout(dvorak, main)`. The
default behaviour is to assign the modifier layout to the key layout if one
is not explicitly specified.

Note that this is different from simply defining a custom layer which reassigns
each key to a modified key sequence (e.g `s = C-s`) since it applies to all
modifiers and preserves expected stacking behaviour.

## Modifier Layers

In addition to standard layers, keyd introduces the concept of 'modifier
layers' to accomodate the common use case of remapping a subset of modifier
keys. A modifier layer will behave as a set of modifiers in all instances
except when a key is explicitly mapped within it and can be defined
by creating a layer which inherits from a valid modifier set.

E.G:

	capslock = layer(custom_control)
	
	[custom_control:C]
	
	1 = C-A-f1
	2 = C-A-f2

Will cause the capslock key to behave as control in all instances except when
`C-1` is pressed, in which case the key sequence `C-A-f1` will be emitted. This
is not possible to achieve using standard layers without breaking expected
behaviour like modifier stacking and pointer combos.

## Summary

1. Use [mylayer] if you want to define a custom shift layer (e.g [symbols]).
2. Use [mylayer:C] if you want a layer which behaves like a custom control key.
3. Use [mylayer:main] for defining custom key layouts (e.g dvorak).

## ACTIONS

**oneshot(\<layer\>)**

: If tapped, activate the supplied layer for the duration of the next keypress.
If `<layer>` is a modifier layer then it will cause the key to behave as the
corresponding modifiers while held.

**layer(\<layer\>)**

: Activates the given layer while held.

**layert(\<layer\>)**

: Toggles the state of the given layer. Note this is intended for transient
layers and is distinct from `layout()` which should be used for letter layouts.

**overload(\<layer\>,\<keyseq\>,)**

: Activates the given layer while held and emits the given key sequence when tapped.

**layout(\<layer\>[, \<modifier layer\>])**

: Sets the current layout to the given layer. You will likely want to ensure
you have a way to switch layouts within the new one. A second layer may
optionally be supplied and is used as the modifier layer if present.

**macro(\<macro\>)**

: Perform the key sequence described in `<macro>`

**swap(\<layer\>[, \<keyseq\>])**

: Swap the currently active layer with the supplied one. The supplied layer is
active for the duration of the depression of the current layer's activation
key. A key sequence may optionally be supplied to be performed before the layer
change.

Where `<macro>` has the form `<token1> [<token2>...]` where each token is one of:

- a valid key sequence.
- a contiguous group of characters each of which is a valid key sequence.
- a timeout of the form `<time>ms` (where `<time>` < 1024).

Examples:

	# Sends alt+p, waits 100ms (allowing the launcher time to start) and then sends 'chromium' before sending enter.
	macro(A-p 100ms chromium enter)

	# Types 'Hello World'
	macro(h e l l o space w o r ld)

	# Identical to the above
	macro(Hello space World)

# EXAMPLES

## Example 1 

Set the default key layout to dvorak and the modifier layout to qwerty (main).

	layout(dvorak, main)

	[dvorak:main]

	q = apostrophe
	w = comma
	e = dot
	# etc...

## Example 2

Make `esc+q/w/e` set the letter layout.

	# ...
	esc = layer(esc)

	[esc]

	q = layout(main)
	w = layout(dvorak, main)
	e = layout(dvorak)

## Example 3

Invert the behaviour of the shift key without breaking modifier behaviour.

	leftshift = layer(shift)
	rightshift = layer(shift)

	1 = !
	2 = @
	3 = #
	4 = $
	5 = %
	6 = ^
	7 = &
	8 = *
	9 = (
	0 = )

	[shift:S]

	0 = 0
	1 = 1
	2 = 2
	3 = 3
	4 = 4
	5 = 5
	6 = 6
	7 = 7
	8 = 8
	9 = 9


## Example 4

Tapping control once causes it to apply to the next key, tapping it twice
activates it until it is pressed again, and holding it produces expected
behaviour.

	control = oneshot(control)

	[control:C]

	layert(control)

# Example 5

Meta behaves as normal except when tab is pressed after which the alt_tab layer
is activated for the duration of the leftmeta keypress. Subsequent actuations
of \` will thus produce A-S-tab instead of M-\`.

	leftmeta = layer(meta)

	[meta:M]

	tab = swap(alt_tab)

	[alt_tab:A]

	tab = A-tab
	` = A-S-tab


# AUTHOR

Written by Raheman Vaiya (2017-).

# BUGS

Please file any bugs or feature requests at the following url:

<https://github.com/rvaiya/keyd/issues>
