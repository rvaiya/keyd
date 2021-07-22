% WARP(1)
% Raheman Vaiya

# OVERVIEW

A system-wide key remapping daemon.

# USAGE

keyd [-m] [-l]

# ARGS

 **-m**: Run in monitor mode. (ensure keyd is not running to see untranslated events).

 **-l**: List all valid key names.

# OVERVIEW

keyd is intended to be run as a system wide daemon via systemd. If run
directly it will run in the foreground and print diagnostic information to
stderr.

Control can be exercised using the standard systemd mechanisms. E.G:

 - journalctl -fu keyd # Print diagnostic information.
 - systemctl restart keyd # Restart the daemon to effect configuration reload.

# CONFIGURATION

All configuration files are stored in /etc/keyd. The name of each file should
correspond to the device name (see `-m`) to which it is to be applied followed
by .cfg (e.g /etc/keyd/Magic\ Keyboard.cfg). Configuration files are loaded
upon initialization, thus restarting the daemon is necessary for changes
to take effect.

If no configuration file exists for a given keyboard and default.cfg is present, it is used.

Each line in a configuration file consists of a mapping of the following form:

	<key> = <action>|<keyseq>

or else represents the beginning of a new layer. E.G:

	[<layer name>]

Where `<keyseq>` has the form: `[<modifier1>-[<modifier2>-]]<key>`

and each modifier is one of:

\    **C** - Control\
\    **M** - Meta/Super\
\    **A** - Alt\
\    **S** - Shift

In addition to simple key mappings keyd can remap keys to actions which
can conditionally send keystrokes or transform the state of the keymap. 

It is, for instance, possible to map a key to escape when tapped and control when held
by assigning it to mods_on_hold(C, esc). A complete list of available actions can be 
found in ACTIONS.

## LAYERS

Each configuration file consists of one or more layers. Each layer is a keymap
unto itself and can be activated by a key mapped to the appropriate
action (see ACTIONS).

By default layers do not have a parent, that is, unmapped keys will have no
effect. The default layer is called 'default' and is used for mappings which
are not explicitly assigned to a layer.

For example the following configuration creates a new layer called 'symbols' which
is activated while the caps lock key is depressed.

	capslock = layer(symbols)

	[symbols]

	f = S-grave
	d = slash

Pressing capslock+f thus produces a tilde.

By default unmapped keys inside of a layer do nothing, however
a layer may optionally have a parent from which mappings are
drawn for keys which are not explicitly mapped. A parent
is specified by appending `:<parent layer>` to the layer 
name. This is particularly useful for custom letter layouts
like dvorak which remap a subset of keys but otherwise
leave the default mappings in tact.

## ACTIONS

**oneshot(mods)**: If tapped activate a modifier sequence for the next keypress, otherwise act as a normal modifier key when held.

**mods_on_hold(mods, keyseq)**: Activates the given set of modifiers whilst held and emits keysequence when tapped.

**layer_on_hold(layer, keyseq)**: Activates the given layer whilst held and emits keysequence when tapped.

**layer_toggle(layer)**: Permanently activate a layer when tapped. *Note*: You will need to explicitly map a toggle in the destination layer if you wish to return.

**layer(layer)**: Activate the given layer while the key is held down.

**oneshot_layer(layer)**: If tapped activate a layer for the duration of the next keypress, otherwise act as a normal layer key when held.

### Legend:

 - `<mods>` = A set of modifiers of the form: `<mod1>[-<mod2>...]` (e.g C-M = control + meta).
 - `<keyseq>` = A key sequence consisting of zero or more control characters and a key (e.g C-a = control+a).
 - `<layer>` = The name of a layer.

## Examples

Example 1

	# Maps capslock to control when held and escape when tapped.
	capslock = mods_on_hold(C, esc)

	# Makes the shift key sticky for one keystroke.

	leftshift = oneshot(S)
	rightshift = oneshot(S)

Example 2

	# Maps escape to the escape layer when held and the escape key when pressed

	esc = layer_on_hold(escape_layer, esc)

	[escape_layer]

	1 = layer_toggle(dvorak)
	2 = layer_toggle(default)

	# Creates a dvorak layer which inherits from the default layer. Without
	# explicitly inheriting from another layer unmapped keys would be ignored.

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


# NOTES

- Because of the way keyd works it is possible to render your machine unusable with a bad
  config file. This can usually be resolved by plugging in a different keyboard, however
  if *default.cfg* has been misconfigured you will have to find an alternate way to kill 
  the daemon (e.g SSH).

# AUTHOR

 - Written by Raheman Vaiya (2017-).

# BUGS

Please file any bugs or feature requests at the following url:

https://github.com/rvaiya/keyd/issues
