# Getting Started

This guide walks you through installing keyd and setting up your first
configuration.

## Installation

### From a Package

See [packages.md](packages.md) for your distribution.

### From Source

```sh
git clone https://github.com/rvaiya/keyd
cd keyd
make && sudo make install
sudo systemctl enable --now keyd
```

**Dependencies:** C compiler + Linux kernel headers (usually pre-installed).

Optional dependencies for application-specific remapping:
- `python` (for `keyd-application-mapper`)
- `python-xlib` (X11 support)
- `dbus-python` (KDE support)

**Note:** The `master` branch is the development branch. For stable releases,
checkout a tagged version.

## Your First Config

1. Create `/etc/keyd/default.conf`:

```ini
[ids]
*

[main]
capslock = overload(control, esc)
esc = capslock
```

This remaps CapsLock to Escape (tap) / Control (hold), and swaps Escape to
CapsLock. A classic quality-of-life improvement.

2. Reload the config:
```sh
sudo keyd reload
```

3. Check for errors:
```sh
sudo journalctl -eu keyd
```

## Discovering Key Names

```sh
sudo systemctl stop keyd
sudo keyd monitor
# press keys to see their names
sudo systemctl start keyd
```

Note: With keyd running, `keyd monitor` shows the *remapped* output.
Stop keyd first to see raw input names.

### Finding Device IDs

`keyd monitor` also shows device IDs in the format `vendor:product`.
Use these to target specific keyboards:

```ini
[ids]
046d:c534    # match only Logitech K380
```

## Recommended Quick-Start Config

A config that many users find immediately useful:

```ini
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
```

This converts all modifiers to "one-shot" keys (tap to activate for the next
keypress) and overloads CapsLock.

## Exploring Further

- **[Config syntax](config-syntax.md)** — full reference for `.conf` files
- **[Actions reference](actions.md)** — all available actions
- **[Examples](../examples/)** — curated configuration examples
- **[Troubleshooting](troubleshooting.md)** — common issues and solutions
- **[Architecture](architecture.md)** — how keyd works internally

## Application-Specific Remapping

See the [`keyd-application-mapper` man page](https://www.mankier.com/1/keyd-application-mapper)
for application-level remapping. Briefly:

1. Add yourself to the keyd group:
   ```sh
   usermod -aG keyd $USER
   ```

2. Configure `~/.config/keyd/app.conf`:
   ```ini
   [alacritty]
   alt.] = macro(C-g n)
   alt.[ = macro(C-g p)
   ```

3. Start the mapper:
   ```sh
   keyd-application-mapper -d
   ```

## Safety Net

If you lock yourself out with a bad config, press
`Backspace + Escape + Enter` simultaneously to terminate keyd.
