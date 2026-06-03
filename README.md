[![Kofi](https://badgen.net/badge/icon/kofi?icon=kofi&label)](https://ko-fi.com/rvaiya)
[![Packaging status](https://repology.org/badge/tiny-repos/keyd.svg)](https://repology.org/project/keyd/versions)

# keyd — A system-wide key remapping daemon for Linux

keyd remaps keys at the kernel level using `evdev` and `uinput`, providing
features like **layers**, **oneshot modifiers**, **macros**, and **tap/hold
overloads** — all with a simple config format that works across display
servers (X, Wayland, TTY).

## Why keyd?

Linux lacks a good key remapping solution. keyd fills this gap:

- **System-wide** — works in TTY, not tied to X11 or Wayland
- **Fast** — hand-tuned C that processes events in <<1ms
- **Simple** — intuitive INI-style config format
- **Powerful** — layers, chording, macros, per-app remapping

## Quick Start

```sh
git clone https://github.com/rvaiya/keyd && cd keyd
make && sudo make install
sudo systemctl enable --now keyd
```

Create `/etc/keyd/default.conf`:

```ini
[ids]
*

[main]
capslock = overload(control, esc)
esc = capslock
```

Reload: `sudo keyd reload`

🚨 **Locked yourself out?** Press `Backspace + Escape + Enter` to terminate keyd.

## Documentation

| Topic | Link |
|---|---|
| **Getting Started** | [docs/getting-started.md](docs/getting-started.md) |
| **Config Syntax** | [docs/config-syntax.md](docs/config-syntax.md) |
| **Actions Reference** | [docs/actions.md](docs/actions.md) |
| **Examples** | [examples/](examples/) |
| **Troubleshooting** | [docs/troubleshooting.md](docs/troubleshooting.md) |
| **Man Page** | `man keyd` (build with `make man`) |
| **Package List** | [docs/packages.md](docs/packages.md) |
| **Migration Guides** | [docs/migration.md](docs/migration.md) |
| **Architecture** | [docs/architecture.md](docs/architecture.md) |

## Features

- **Layers** — stackable keymaps with modifier support
- **Oneshot** — tap a modifier key to activate it for one use
- **Overload** — different behavior on tap vs. hold
- **Macros** — type sequences, execute shell commands
- **Chords** — remap simultaneous key combinations
- **Per-app remapping** — different bindings per application via `keyd-application-mapper`
- **Unicode** — compose international glyphs directly
- **SBC support** — USB gadget mode for single-board computers ([docs/usb-gadget.md](docs/usb-gadget.md))

## What keyd isn't

keyd is **not** a tool for programming individual key up/down events at arbitrary
timings. It is a layer-based remapper focused on keyboard ergonomics and
consistency.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

See [LICENSE](LICENSE). Written by Raheman Vaiya (2017–).

**IRC:** #keyd on OFTC
