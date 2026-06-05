# Config Syntax Reference

This document describes keyd's configuration file format in detail.
For the authoritative reference, see the [man page](https://www.mankier.com/1/keyd)
or build it locally with `make man && man docs/keyd.1`.

## Overview

Config files are stored in `/etc/keyd/` and have a `.conf` extension.
They loosely follow an INI-style format with `[section]` headers and key=value bindings.

Lines beginning with `#` are comments.

## Basic Structure

A valid config file *must* start with an `[ids]` section:

```ini
[ids]
*

[main]
capslock = overload(control, esc)
esc = capslock
```

## The `[ids]` Section

Specifies which keyboards the config applies to. Device IDs are obtained
via `keyd monitor`.

### Explicit match:
```ini
[ids]
046d:c534    # match only this device
```

### Wildcard (match all, with exclusions):
```ini
[ids]
*
-046d:c534   # match everything except this device
```

### Type prefixes:
- `k:046d:c534` — match only keyboards with this ID
- `m:046d:c534` — match only mice with this ID

**Note:** The wildcard `*` only matches keyboards. Mice must be explicitly listed.

## Layers

Each `[section]` after `[ids]` defines a layer:

```ini
[main]
capslock = layer(nav)

[nav]
h = left
j = down
k = up
l = right
```

### Layer Modifiers

Layers can emulate modifiers for unbound keys:

```ini
[control:C]       # emulate Control when no binding matches
j = down
```

Modifier tags: `C` (Control), `M` (Super), `A` (Alt), `S` (Shift), `G` (AltGr).

### Composite Layers

Activated when all constituent layers are active:

```ini
[control+shift]
h = C-left        # fires when Control+Shift+h is pressed
```

**Must be defined *after* all constituent layers.**

## Layouts

Layouts modify alpha keys. Only one layout may be active at a time:

```ini
[mylayout:layout]
a = b
b = c
# ...
```

Ship layouts are in `/usr/share/keyd/layouts/` and can be included:

```ini
include layouts/dvorak

[global]
default_layout = dvorak
```

## File Inclusion

Include shared config snippets (`include` supports no recursion):

```ini
[ids]
*

include common        # from /etc/keyd/common or /usr/share/keyd/common

[main]
capslock = overload(nav, esc)
```

**Limitations:**
- Includes must appear after `[ids]`
- Included files must not have an `[ids]` section
- Included files must not end in `.conf`
- Inclusion is non-recursive

## Aliases

Rename keys for config convenience:

```ini
[aliases]
leftmeta = meta
rightmeta = meta
```

Now `meta = oneshot(meta)` applies to both meta keys.

## `[global]` Section

```ini
[global]
macro_timeout = 600                 # ms between macro repeat iterations
macro_repeat_timeout = 50           # ms between keys within a macro
macro_sequence_timeout = ?          # microseconds between macro key emissions
layer_indicator = 0                 # use CapsLock LED for layer indicator
chord_timeout = 50                  # ms between keys in a chord
chord_hold_timeout = 0              # ms a chord must be held
oneshot_timeout = 0                 # timeout oneshot after ms (0 = disabled)
disable_modifier_guard = 0          # don't guard against phantom modifier keypresses
overload_tap_timeout = 0           # ignore tap if key held > ms
default_layout = dvorak
```

## Config Validation

Use `keyd check` to validate configs without applying them:

```sh
keyd check                           # check all configs in /etc/keyd/
keyd check /etc/keyd/mykeyboard.conf # check a specific file
```
