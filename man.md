% KEYD(1)

# NAME

**keyd** - A key remapping daemon.

# SYNOPSIS

**keyd** [options]

# OPTIONS

`-m, --monitor`
: Start keyd in monitor mode.  Mainly useful for discovering key codes and debugging.

`-e, --expression <expression> [<expression>...]`
: Modify bindings of the currently active keyboard. See *Expressions* for details.

`-l, --list`
: List all internal key names.

`-d, --daemonize`
: Start keyd as a daemon logging out all output to */var/log/keyd.log*.

`-v, --version`
: Print the current version and exit.

`-h, --help`
: Print help and exit.

# DESCRIPTION

keyd is a system wide key remapping daemon which supports features like
layering, oneshot modifiers, and macros. In its most basic form it can be used
to define a custom key layout that persists across display server boundaries
(e.g wayland/X/tty).

The program is intended to run as a systemd service but is capable of running
as a standalone daemon. The default behaviour is to run in the foreground and print
to stderr, unless **-d** is supplied, in which case in which case log output
will be stored in */var/log/keyd.log*.

**NOTE**

Because keyd modifies your primary input device it is possible to render your
machine unusable with a bad config file. If you find yourself in this
situation the sequence *\<backspace\>+\<backslash\>+\<enter\>* will force keyd
to terminate.

# CONFIGURATION

All configuration files are stored in */etc/keyd/* and are loaded upon
initialization. A reload can be triggered by restarting the daemon or by
sending SIGUSR1 to the process (e.g sudo pkill -usr1 keyd).

A valid config file has the extension .conf and must begin with an *ids* section that has the following form:

	[ids]

	<id 1>
	<id 2>
	...

Where each \<id\> is one of:

	- A device id of the form <vendor id>:<product id> (obtained with -m).
	- The wildcard "*".

A wildcard indicates that the file should apply to all keyboards
which are not explicitly listed in another configuration file and
may optionally be followed by one or more lines of the form:

	-<vendor id>:<product id>

representing a device to be excluded from the matching policy. Thus the following
config will match all devices except 0123:4567:

	[ids]
	*
	-0123:4567


The monitor flag (**-m**) can be used to interactively obtain device ids and key names like so:

	> sudo systemctl stop keyd # Avoid loopback.
	> sudo keyd -m

	Magic Keyboard	0ade:0fac	capslock down
	Magic Keyboard	0ade:0fac	capslock up
	...

Every subsequent section of the file corresponds to a layer and has the form:

	[<name>[:<type>]]

Where `<type>` is either a valid modifier set (see *MODIFIERS*) or "layout".

Each line within a layer is a mapping of the form:

	<key> = <action>|<keyseq>

Where `<keyseq>` has the form: `[<modifier1>[-<modifier2>...]-<key>`

and each modifier is one of:

\ **C** - Control\
\ **M** - Meta/Super\
\ **A** - Alt\
\ **S** - Shift\
\ **G** - AltGr

In addition to key sequences, keyd can remap keys to actions which
conditionally send keystrokes or transform the state of the keymap.

It is, for instance, possible to map a key to escape when tapped and control
when held by assigning it to `overload(C, esc)`. A complete list of available
actions can be found in *ACTIONS*.

## Layers

Each layer is a keymap unto itself and can be transiently activated by a key
mapped to the corresponding *layer()* action.

For example, the following configuration creates a new layer called 'symbols' which
is activated by holding the capslock key.

	[ids]
	*

	[main]
	capslock = layer(symbols)

	[symbols]
	f = ~
	d = /
	...

Pressing `capslock+f` thus produces a tilde.

Key sequences within a layer are fully descriptive and completely self
contained. That is, the sequence 'A-b' corresponds exactly to the combination
`<alt>+<b>`. If any additional modifiers are active they will be deactivated
for the duration of the corresponding key stroke.

## Layouts

The *layout* is a special kind of layer from which mappings are drawn if no
other layers are active. By default all keys are mapped to themselves within a
layout. Every config has at least one layout called *main*, but additional
layouts may be defined and subsequently activated using the `layout()` action.

Layouts also have the additional property of being affected by the active modifier
set. That is, unlike layers, key sequences mapped within them are not
interpreted literally.

If you wish to use an alternative letter arrangement this is the appropriate
place to define it.

E.G

	[main]

	rightshift = layout(dvorak)

	[dvorak:layout]

	rightshift = layout(main)
	s = o
	d = e
	...

## Modifiers

Unlike most other remapping tools keyd provides first class support for
modifiers. A valid modifier set may optionally be used as a layer type,
causing the layer to behave as the modifier set in all instances except
where an explicit mapping overrides the default behaviour.

These layers play nicely with other modifiers and preserve existing stacking
semantics.

For example:

	[main]

	leftalt = layer(myalt)
	rightalt = layer(myalt)
	
	[myalt:A]

	1 = C-A-f1

Will cause the leftalt key to behave as alt in all instances except when
alt+1 is pressed, in which case the key sequence `C-A-f1` will be emitted.

By default each modifier key is mapped to an eponymously named modifier layer.

Thus the above config can be shortened to:

	[alt]

	1 = C-A-f1

since leftalt and rightalt are already assigned to `layer(alt)`.

Additionally, left hand values which are modifier names are expanded to both
associated keycodes.

E.G
	control = esc

is the equivalent of

	rightcontrol = esc
	leftcontrol = esc

Finally any set of valid modifiers is also a valid layer. For example, the
layer `M-C` corresponds to a layer which behaves like the modifiers meta and
control, which means the following:

	capslock = layer(M-A)

will cause capslock to behave as meta and alt when held.

### Lookup Rules

In order to achieve this (un)holy union, the following lookup rules are used:

	if len(active_layers) > 0:
		if key in most_recent_layer:
			action = most_recent_layer[key]
		else if has_modifiers(most_recent_layer):
			for layer in active_layers:
				activate_modifiers(layer)

			action = layout[key]
	else:
		action = layout[key]

The upshot of all this is that things should mostly just work™. The
majority of users needn't be explicitly conscious of the lookup rules
unless they are doing something unorthodox (e.g nesting hybrid layers).

## IPC

Unlike other remapping tools keyd employs a client-server model. This 
makes the keymap a 'living' entity that can be modified at runtime.

In addition to allowing the user to try new bindings on the fly, this 
enables the user to fully leverage keyd's expressive power from other programs
without incurring a performance penalty.

For instance, the user may use this functionality to write a script which
alters the keymap when they switch between different tmux sessions.  

The application remapping tool (keyd-application-mapper) which ships with keyd
is a good example of this. It is a small python script which performs
event detection for the various platforms (e.g X/sway/gnome, etc) and feeds the
desired mappings to the core using `-e`.

### Expressions

The `-e` flag accepts one or more *expressions*, each of which must have one of the following forms:

	[<layer>.]<key> = <action>|<key sequence>
	reset

Where `<layer>` is the name of an (existing) layer in which the key is to be bound. 

As a special case an expression may be the string 'reset' in which case the
current keymap will revert to its original state (all dynamically applied
bindings will be dropped).

Examples:

	$ keyd -e '1 = a' # Map
	$ keyd -e 'dvorak.1 = a'
	$ keyd -e 'control.1 = macro(Hello space World)'

By default expressions apply to the most recently active keyboard.

### Application Support

keyd ships with a python script called `keyd-application-mapper` which
reads a file called *~/.keyd-mappings* and applies the supplied mappings 
whenever a window of the relevant class comes into focus. 

The file has the following form:

	[<application class>]

	<expression 1>
	<expression 2...>

Where each expression is a valid argument to `-e`.

For example:

	[Alacritty]

	rightshift = layer(mylayer)
	control.1 = macro(Inside space alacritty)

	mylayer.a = C-c
	insert = S-insert

	[chromium]

	control.1 = macro(Inside space chrome)

Will remap `C-1` to the the string 'Inside alacritty' when a window with class
'Alacritty' is active and 'Inside chrome' when a window with class 'chromium'
is active.

Application classes can be obtained by running `keyd-application-mapper` with
the `-m` flag. At the moment X, sway and gnome are supported.

In order for this to work the user must have access to */var/run/keyd.socket*
(i.e be a member of the *keyd* group)

### A note on security

Any user which can interact with programs that directly control input devices
should be assumed to have full access to the entire system.

While keyd is slightly better at providing some degree of isolation than other
remappers (by dint of mediating access through an IPC mechanism rather than
granting users blanket access to /dev/input/* and /dev/uinput), it still
provides the opportunity for abuse and should be treated with due deference.

Specifically, access to */var/run/keyd.socket* should only be granted to
trusted users and the group `keyd` should be regarded with the same reverence
as `wheel`.

## ACTIONS

**oneshot(\<layer\>)**

: If tapped, activate the supplied layer for the duration of the next keypress.
If `<layer>` is a modifier layer then it will cause the key to behave as the
corresponding modifiers while held.

**layer(\<layer\>)**

: Activates the given layer while held.

**toggle(\<layer\>)**

: Toggles the state of the given layer. Note this is intended for layers and is
distinct from `layout()` which should be used for letter layouts.

**overload(\<layer\>,\<keyseq\>[,\<timeout\>])**

: Activates the given layer while held and emits the given key sequence when
tapped. A timeout in milliseconds may optionally be supplied to disambiguate
a tap and a hold.

	If a timeout is present depression of the corresponding key is only interpreted
as a layer activation in the event that it is sustained for more than
\<timeout\> a milliseconds. This is useful if the overloaded key is frequently
used on its own (e.g space) and only occasionally treated as a modifier (the opposite
of the default assumption).

**layout(\<layer\>)**

: Sets the current layout to the given layer. You will likely want to ensure
you have a way to switch layouts within the newly activated one.

**swap(\<layer\>[, \<keyseq\>])**

: Swap the currently active layer with the supplied one. The supplied layer is
active for the duration of the depression of the current layer's activation
key. A key sequence may optionally be supplied to be performed before the layer
change.

**macro(\<macro\>)**

: Perform the key sequence described in `<macro>`

Where `<macro>` has the form `<token1> [<token2>...]` where each token is one of:

- a valid key sequence.
- a contiguous group of characters each of which is a valid key sequence.
- a group of key codes delimited by + to be depressed as a unit.
- a timeout of the form `<time>ms` (where `<time>` < 1024).

Examples:

	# Sends alt+p, waits 100ms (allowing the launcher time to start) and then sends 'chromium' before sending enter.
	macro(A-p 100ms chromium enter)

	# Types 'Hello World'
	macro(h e l l o space w o r ld)

	# Identical to the above
	macro(Hello space World)

	# Produces control + b
	macro(control + b)

	# Produces the sequence <control down> <b down> <control up> <b up>
	macro(control+b)

	# Produces the sequence <control down> <control up> <b down> <b up>
	macro(control b)

# EXAMPLES

## Example 1

Make `esc+q/w/e` set the letter layout.

	[ids]
	*

	[main]
	esc = layer(esc)

	[dvorak:layout]
	s = o
	d = e
	...

	[esc]
	q = layout(main)
	w = layout(dvorak)

## Example 3

Invert the behaviour of the shift key without breaking modifier behaviour.

	[ids]
	*

	[main]
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

	[shift]
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


## Example 3

Tapping control once causes it to apply to the next key, tapping it twice
activates it until it is pressed again, and holding it produces expected
behaviour.

	[main]

	control = oneshot(control)

	[control]

	toggle(control)

# Example 4

Meta behaves as normal except when \` is pressed, after which the alt_tab layer
is activated for the duration of the leftmeta keypress. Subsequent actuations
of \` will thus produce A-tab instead of M-\`.

	[meta]

	` = swap(alt_tab, A-tab)

	[alt_tab:A]

	tab = A-S-tab
	` = A-tab

# Example 5

	# Uses the compose key functionality of the display server to generate
	# international glyphs.  # For this to work 'setxkbmap -option
	# compose:menu' must # be run after keyd has started.

	# A list of sequences can be found in /usr/share/X11/locale/en_US.UTF-8/Compose 
	# on most systems.


	[main]

	rightalt = layer(dia)

	[dia]

	# Map o to ö
	o = macro(compose o ")

	# Map e to €
	e = macro(compose c =)

# AUTHOR

Written by Raheman Vaiya (2017-).

# BUGS

Please file any bugs or feature requests at the following url:

<https://github.com/rvaiya/keyd/issues>
