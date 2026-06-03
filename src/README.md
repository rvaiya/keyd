# Source Code

This directory contains the keyd core, written in C11. See
[../docs/architecture.md](../docs/architecture.md) for the full system overview
and cross-reference this module table.

## Core Modules

| File | Size | Responsibility |
|---|---|---|
| [`keyd.c`](keyd.c) | 266L | CLI entry point, IPC client commands (`bind`, `input`, `do`, `monitor`, `check`, `listen`) |
| [`daemon.c`](daemon.c) | 651L | Daemon lifecycle, config loading, event dispatch, IPC server |
| [`keyboard.c`](keyboard.c) | 1,315L | **Core engine**: layer resolution, overloads, oneshots, macros, timeouts, chording |
| [`config.c`](config.c) | 1,167L | Config file parsing: `[ids]`, layers, `[global]`, `[aliases]`, `include` directives |
| [`device.c`](device.c) | 643L | evdev device monitoring, inotify-based device discovery, input grab/release |
| [`keys.c`](keys.c) | 413L | Key name ↔ evdev code lookup tables (canonical + alternate names) |
| [`macro.c`](macro.c) | 173L | Macro expansion and sequencing logic |
| [`evloop.c`](evloop.c) | 164L | Poll-based event loop (Linux/BSD compatible) |
| [`ipc.c`](ipc.c) | 87L | Unix domain socket IPC (client connect, server bind, lock file) |
| [`monitor.c`](monitor.c) | 111L | `keyd monitor` subcommand — print real-time key events |
| [`check.c`](check.c) | 55L | `keyd check` subcommand — config validation |
| [`unicode.c`](unicode.c) | ~21kL | Unicode character → evdev sequence composition (generated table) |
| [`string.c`](string.c) | 108L | String utility functions (concat, split, etc.) |
| [`log.c`](log.c) | 102L | Structured logging (`log` function with levels) |
| [`dbg.c`](dbg.c) | 71L | Debug helpers (`dbg_print_evdev_details`) |

### Headers

| Header | Purpose |
|---|---|
| `keyd.h` | Master include (all common headers + core types: `event`, `ipc_message`) |
| `config.h` | Config structures: `descriptor`, `chord`, `layer`, `config` |
| `keyboard.h` | Keyboard state machine: `keyboard`, `key_event`, `cache_entry` |
| `keys.h` | Key code structures |
| `macro.h` | Macro structures |
| `device.h` | Device structures |
| `vkbd.h` | Virtual keyboard backend interface |
| `unicode.h` | Unicode composition interface |
| `string.h` | String utility declarations |
| `log.h` | Logging declarations |

## Virtual Keyboard Backends (`vkbd/`)

| File | Description |
|---|---|
| `uinput.c` | **Default** — Linux uinput virtual keyboard |
| `stdout.c` | Debug backend — prints events to stdout |
| `usb-gadget.c` | SBC USB gadget — HID reports via USB OTG |

## External Dependencies (`ext/`)

| File | Source |
|---|---|
| `evdev-input-codes.h` | Linux kernel evdev input event definitions |

## Build

All `.c` files in `src/` (except those in subdirs) are compiled with:

```sh
make                     # standard build (VKBD=uinput)
make VKBD=usb-gadget     # SBC USB gadget build
make debug               # ASAN + debug symbols
```

## Code Organization Notes

- **Single-threaded design**: The event loop processes all input synchronously.
  This avoids race conditions and keeps latency <<1ms.
- **Layer resolution** is the most complex path. See `keyboard.c` comments
  for the state machines handling overloads, timeouts, and chording.
- **Config parsing** supports includes, aliases, and composite layers.
  All of this logic is in `config.c`.
