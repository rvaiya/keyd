# Scripts

Utility scripts shipped with keyd. These are not compiled — they are interpreted
and can be inspected and modified freely.

## keyd-application-mapper

A Python script that detects window focus changes and dynamically applies
per-application key bindings via keyd's IPC mechanism. This is the main entry
point for application-specific remapping.

**Config:** `~/.config/keyd/app.conf`

**Usage:**
```sh
keyd-application-mapper -d    # run as daemon
keyd-application-mapper -v    # verbose (debug window detection)
```

Supported display servers: X (python-xlib), Sway/wlroots, Gnome (via GNOME extension), KDE/Plasma (via dbus).

For full details, see [`keyd-application-mapper(1)`](../docs/keyd-application-mapper.scdoc).

## generate_xcompose

Generates the X compose table (`data/keyd.compose`) from unicode data.
Run this script if you modify `data/unicode.txt`. Requires no external
dependencies.

**Usage:**
```sh
./scripts/generate_xcompose
```

## dump-xkb-config

Dumps your current XKB configuration (layout, options, symbols) for reference
or testing. Useful for comparing what your display server has configured vs
what keyd is doing.

**Usage:**
```sh
./scripts/dump-xkb-config
```
