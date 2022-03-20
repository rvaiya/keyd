keyd(1)

# NAME

*keyd* - A key remapping daemon.

# SYNOPSIS

*keyd* [options]

# OPTIONS

*-m, --monitor*
	Start keyd in monitor mode.  Useful for discovering keycodes and device ids.

*-e, --expression <expression> [<expression>...]*
	Modify bindings of the currently active keyboard. See _Expressions_ for details.

*-l, --list-keys*
	List valid key names.

*-v, --version*
	Print the current version and exit.

*-h, --help*
	Print help and exit.

# DESCRIPTION

keyd is a system wide key remapping daemon which supports features like
layering, oneshot modifiers, and macros. In its most basic form it can be used
to define a custom key layout that persists across display server boundaries
(e.g wayland/X/tty).

The program runs in the foreground, printing diagnostic information to the
standard output streams, and is intended to be run as a single instance managed
by the init system.

*NOTE:*

Because keyd modifies your primary input device, it is possible to render your
machine unusable with a bad config file. If you find yourself in this situation
the panic sequence *<backspace>+<escape>+<enter>* will force keyd to
terminate.

# CONFIGURATION

Configuration files are stored in _/etc/keyd/_ and are loaded upon initialization.
Changes to these files can be realized by restarting the daemon, which is usually
accomplished using your service manager.

E.G

	sudo systemctl restart keyd


A valid config file has the extension _.conf_ and *must* begin with an _[ids]_
section that has one of the following forms:

```
	[ids]

	<vendor id 1>:<product id 1>
	<vendor id 2>:<product id 2>
	...
```

or

```
	[ids]

	*
	-<vendor id 1>:<product id 1>
	-<vendor id 2>:<product id 2>
	...
```

The first form specifies a list of ids to be explicitly matched, while the
second matches any id which has not been explicitly excluded.

For example:

```
	[ids]
	*
	-0123:4567
```


Will match all devices which *do not* have the id _0123:4567_, while:

```
	[ids]
	0123:4567
```

will exclusively match any devices which do.

Each subsequent section of the file corresponds to a _layer_.

## Layers

A layer is a collection of _bindings_, each of which specifies the behaviour of
a particular key. Multiple layers may be active at any given time, forming a
stack of occluding keymaps consulted in activation order. The default layer is
called _main_ and is where common bindings should be defined.

For example, the following config snippet defines a layer called _nav_
and creates a toggle for it in the _main_ layer:

```
	[main]

	capslock = layer(nav)

	[nav]

	h = left
	k = up
	j = down
	l = right
```

When capslock is held, the _nav_ layer occludes the _main_ layer
causing _hjkl_ to function as the corresponding arrow keys.

Unlike most other remapping tools, keyd provides first class support for
modifiers. A layer name may optionally end with a ':' followed by a
set of modifiers to emulate in the absence of an explicit mapping.

These layers play nicely with other modifiers and preserve existing stacking
semantics.

Formally, each layer heading has the following form:

```
	"[" <layer name>[:<modifier set>] "]"
```

Where _<modifier_set>_ has the form:

	_<modifier1>[-<modifier2>]..._

and each modifier is one of:

	*C* - Control++
*M* - Meta/Super++
*A* - Alt++
*S* - Shift++
*G* - AltGr

Finally, each layer heading is followed by a set of bindings which take the form:

	<key> = <keycode>|<macro>|<action>

for a description of <action> and <macro> see _ACTIONS_ and _MACROS_.


For example:

```
	[main]

	capslock = layer(capslock)

	[capslock:C]

	j = down
```

will cause _capslock_ to behave as _control_, except in the case of _control+j_, which will
emit _down_. This makes it trivial to define custom modifiers which don't interfere with
one another.

By default, each key is bound to itself within the main layer. The exception to this
are the modifier keys, which are instead bound to eponymously named layers with the
corresponding modifiers.

For example, _meta_ is acutally bound to _layer(meta)_, where _meta_ is
internally defined as _meta:M_.

A consequence of this is that overriding modifier keys is a simple matter of
adding the desired bindings to appropriate pre-defined layer.

Thus

```
	[ids]
	*

	[control]
	j = down
```

is a completely valid config, which does what the benighted user might expect. Internally,
the full config actually looks something like this:

```
	[ids]
	*

	[main]
	leftcontrol = layer(control)
	rightcontrol = layer(control)

	[control:C]
	j = down
```



## Composite Layers

A special kind of layer called a *composite layer* can be defined by creating a
layer with a name consisting of existing layers delimited by _+_. The resultant
layer will be activated and given precedence when one of its constituents are
activated.

E.G

```
[main]
capslock = layer(capslock)

[capslock:C]
[capslock+shift]

h = left
```

Will cause the sequence _control+shift+h_ to produce _left_, while preserving
the expected functionality of _capslock_ and _shift_ in isolation.

*Note:* composite layers *must* always be defined _after_ the layers of which they
are comprised.

That is:

```
[layer1]
[layer2]
[layer1+layer2]
```

and not

```
[layer1+layer2]
[layer1]
[layer2]
```

# MACROS

Various keyd actions accept macro expressions.

A valid macro expression has one of the following forms:

	. macro(<exp>)
	. [<modifier 1>[-<modifier 2>...]-<key>

Where _<exp>_ has the form _<token1> [<token2>...]_ and each token is one of:

	- a valid key code.
	- a type 2. macro.
	- a contiguous group of characters, each of which is a valid key code.
	- a group of key codes delimited by + to be depressed as a unit.
	- a timeout of the form _<time>ms_ (where _<time>_ < 1024).

The following are all valid macro expressions:

	- C-a
	- macro(C-a)
	- macro(control+a)
	- A-M-x
	- macro(Hello space World)
	- macro(h e l l o space w o r ld)    (identical to the above)
	- macro(C-t 100ms google.com enter)


# ACTIONS

A key may optionally be bound to an _action_ which accepts zero or more
arguments.

*oneshot(<layer>)*
	If tapped, activate the supplied layer for the duration of the next keypress.
	If _<layer>_ is a modifier layer then it will cause the key to behave as the
	corresponding modifiers while held.

*layer(<layer>)*
	Activates the given layer for the duration of the keypress.

*toggle(<layer>)*
	Permanently toggle the state of the given layer.

*overload(<layer>, <macro>)*
	Activates the given layer while held and executes the provided macro when tapped.

*timeout(<action 1>, <timeout>, <action 2>)*
	If the key is held in isolation for more than _<timeout> ms_, activate the first
	action, if the key is held for less than _<timeout> ms_ or another key is struck
	before <timeout> ms expires, execute the first action.

	E.G

	timeout(a, 500, layer(control))

	Will cause the assigned key to behave as _control_ if it is held for more than
	500 ms.

*swap(<layer>[, <macro>])*
	Swap the currently active layer with the supplied one. The supplied layer is
	active for the duration of the depression of the current layer's activation
	key. A macro may optionally be supplied to be performed before the layer
	change.

```
	[control]

	x = swap(xlayer)

	[xlayer]

	s = C-s
	b = S-insert
```

*noop*
	Do nothing.

# IPC

To facilitate extensibility, keyd employs a client-server model accessible
through the use of *-e*. The keymap can thus be conceived of as a
living entity that can be modified at run time.

In addition to allowing the user to try new bindings on the fly, this
enables the user to fully leverage keyd's expressive power from other programs
without incurring a performance penalty.

For instance, the user may use this functionality to write a script which
alters the keymap when they switch between different tmux sessions.

The application remapping tool (*keyd-application-mapper(1)*) which ships with keyd
is a good example of this. It is a small python script which performs event
detection for the various display servers (e.g X/sway/gnome, etc) and feeds the
desired mappings to the core using _-e_.

## Expressions

The _-e_ flag accepts one or more _expressions_, each of which must have the following form:

	\[<layer>.\]<key> = <key>|<macro>|<action>

Where _<layer>_ is the name of an (existing) layer in which the key is to be bound.

As a special case, an expression may be the string *reset*, in which case the
current keymap will revert to its original state (all dynamically applied
bindings will be dropped).

Examples:

```
	# keyd -e '- = C-c'
	# keyd -e reset     # Reset the state of the keymap so it reflects /etc/keyd/.
```

By default expressions apply to the most recently active keyboard.

# EXAMPLES

## Example 1

Make _esc+q_ toggle the dvorak letter layout.

```
	[ids]
	*

	[main]
	esc = layer(esc)

	[dvorak]

	a = a
	s = o
	...

	[esc]

	q = toggle(dvorak)
```

## Example 2

Invert the behaviour of the shift key without breaking modifier behaviour.

```
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
```

## Example 3

Tapping control once causes it to apply to the next key, tapping it twice
activates it until it is pressed again, and holding it produces expected
behaviour.

```
	[main]

	control = oneshot(control)

	[control]

	toggle(control)
```

## Example 4

Meta behaves as normal except when \` is pressed, after which the alt_tab layer
is activated for the duration of the leftmeta keypress. Subsequent actuations
of _will thus produce A-tab instead of M-\\_.

```
	[meta]

	` = swap(alt_tab, A-tab)

	[alt_tab:A]

	tab = A-S-tab
	` = A-tab
```

## Example 5

```
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
```

## Example 6

```
	# Tapping both shift keys will activate capslock.

	[shift]

	leftshift = capslock
	rightshift = capslock
```

# AUTHOR

Written by Raheman Vaiya (2017-).

# BUGS

Please file any bugs or feature requests at the following url:

<https://github.com/rvaiya/keyd/issues>
