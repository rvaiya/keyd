% KEYD(1)
% Raheman Vaiya

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

\    **C** - Control\
\    **M** - Meta/Super\
\    **A** - Alt\
\    **S** - Shift\
\    **G** - AltGr

In addition to simple key mappings keyd can remap keys to actions which
can conditionally send keystrokes or transform the state of the keymap.

It is, for instance, possible to map a key to escape when tapped and control when held
by assigning it to `overload(C, esc)`. A complete list of available actions can be
found in *ACTIONS*.

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

Pressing capslock+f thus produces a tilde.

Any set of valid modifiers is also a valid layer. For example the layer `M-C`
corresponds to a layer which behaves like the modifiers meta and control. These
play nicely with other modifiers and preserve existing stacking semantics.

A layer may optionally have a parent from which mappings are drawn for keys
which are not explicitly mapped. By default layers do not have a parent, that
is, unmapped keys will have no effect. A parent is specified by appending
`:<parent>` to the layer name.

The *layout* is a special layer from which mappings are drawn if no other layers
are active.  The default layout is called *main* and is the one to which
mappings are assigned if no layer heading is present. By default all keys are
defined as themselves in the main layer.  Layers which intend to be used as
layouts will likely want to inherit from main. The default layout can be
changed by including layout(<layer>) at the top of the config file.

## The Modifier Layout

keyd distinguishes between the key layout and the modifier layout. This
allows the user to use a different letter arrangement for modifiers. It may,
for example, be desireable to use an alternative key layout like dvorak while
preserving standard qwerty modifier shortcuts. This can be achieved by passing
a second argument to the layout function like so: `layout(dvorak, main)`.

Note that this is different from simply defining a custom layer which reassigns
each key to a modified key sequence (e.g `s = C-s`) since it applies to all
modifiers and preserves expected stacking behaviour.

## Modifier Layers

In addition to standard layers, keyd introduces the concept of 'modifier
layers' to accomodate the common use case of remapping a subset of modifier keys. A
modifier layer will have identical behaviour to a set of modifiers unless a key is
explicitly defined within it. To define a modifier layer simply define a layer
which inherits from a valid modifier set.

E.G:

```
capslock = layer(custom_control)

[custom_control:C]

1 = C-A-f1
2 = C-A-f2
```

Will cause the capslock key to behave as control in all instances except when `C-1` is
pressed, in which case the key sequence C-A-f1 will be emitted. This is not
possible to achieve using standard layers without breaking expected behaviour
like modifier stacking and pointer combos.

## TLDR

1. Use [mylayer] if you want to define a custom shift layer (e.g [symbols]).
2. Use [mylayer:C] if you want a layer which behaves like a custom control key.
3. Use [mylayer:main] for defining custom key layouts (e.g dvorak).

## ACTIONS

**oneshot(\<layer\>)**: If tapped activate a layer for the next keypress. If this is a modifier layer then it will cause the key to behave as the corresponding modifiers while held.

**layer(\<layer\>)**: Activates the given layer while held.

**layert(\<layer\>)**: Toggle the state of the given layer. Note this is
intended for transient layers and is distinct from layout() which should
be used for letter layouts.

**overload(\<keyseq\>,\<layer\>)**: Activates the given layer while held and emits the given key sequence when tapped.

**layout(\<layer\>)**: Sets the current layout to the given layer. You will likely want
to ensure you have a way to switch layouts within the new one.

**macro(\<macro\>)**: Perform the key sequence described in `<macro>`

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

# Example

    # Set the default key layout to dvorak and the modifier layout to qwerty (main)
    layout(dvorak, main)

    # Holding escape activates the escape layer
    esc = layer(esc)

    shift = oneshot(S)
    rightshift = oneshot(S)

    [esc]

    # esc+q changes both the key and modifier layout to qwerty.
    q = layout(main)
    w = layout(dvorak, main)

    # Inherits all bindings from the main layer (including the defined escape/shift bindings),
    # and defines a custom letter arrangement.
    [dvorak:main]

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
