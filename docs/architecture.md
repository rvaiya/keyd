# keyd Architecture

This document describes how keyd works internally. It is intended for
contributors and advanced users who want to understand the system.

## High-Level Overview

keyd is a system-wide key remapping daemon that operates by intercepting
keyboard input at the kernel level using `evdev`, then emitting remapped
keys through a `uinput` virtual keyboard device.

```
Physical Keyboard ──evdev──▶ keyd daemon ──uinput──▶ Virtual Keyboard ╧
                                                                    │
                                                           ──evdev──▶ User sees this
```

### The Daemon Loop

The daemon (`daemon.c`) operates in a single-threaded event loop
(`evloop.c`):

1. **Device Monitoring**: Uses `inotify` to detect when keyboards are
   plugged in or removed. Each keyboard is wrapped by `device.c`.

2. **Event Grab**: When a keyboard matches a config rule, keyd grabs the
   device via `EVIOCGRAB` so the raw events don't leak through.

3. **Event Processing**: Key events are passed to `keyboard.c` for
   resolution through the active layers.

4. **Output**: Resolved keys are emitted through the virtual keyboard
   backend (`vkbd/uinput.c`).

## Module Responsibilities

| File | Responsibility | Lines |
|---|---|---|
| `keyd.c` | CLI entry point, IPC client commands (`bind`, `input`, `do`, etc.) | ~260 |
| `daemon.c` | Daemon lifecycle, config loading, event dispatch, cleanup | ~650 |
| `device.c` | evdev device discovery, inotify monitoring, input grab/release | ~640 |
| `keyboard.c` | Core key processing: layer resolution, overloads, oneshots, macros, timeouts, chording | ~1310 |
| `config.c` | Config file parsing (INI-like format), include support | ~1170 |
| `keys.c` | Key name ↔ evdev code mappings (canonical + alternate names) | ~410 |
| `macro.c` | Macro expansion and sequencing | ~170 |
| `evloop.c` | Poll-based event loop (cross-platform) | ~160 |
| `ipc.c` | Unix socket IPC (client connect, server bind) | ~85 |
| `monitor.c` | `keyd monitor` subcommand — print raw key events | ~110 |
| `check.c` | `keyd check` subcommand — config validation/linting | ~55 |
| `unicode.c` | Unicode character → evdev sequence composition | ~21k* |
| `string.c` | String utility functions | ~110 |
| `log.c` | Structured logging | ~100 |
| `dbg.c` | Debug helpers (`dbg_print_evdev_details`) | ~70 |

\* `unicode.c` is large because it contains a generated unicode
composition table (~2MB).

### Virtual Keyboard Backends (`vkbd/`)

| File | Description |
|---|---|
| `uinput.c` | Default: uses Linux `uinput` to create a virtual keyboard |
| `stdout.c` | Debug: prints events to stdout instead |
| `usb-gadget.c` | SBC: uses USB HID gadget driver for USB OTG setups |

## Config Loading Flow

```
/etc/keyd/*.conf  ──config_parse()──▶  struct config
/etc/keyd/*.conf        │                              │
include common ────────┘    config_check_match() ───▶  matches keyboard?
                                          │
                                     new_keyboard() ──▶  struct keyboard
```

1. Each `.conf` file is parsed independently by `config_parse()`.
2. The parser handles `[ids]`, `[aliases]`, `[global]`, and layer sections.
3. `include` directives pull in additional files from `/etc/keyd/` or
   `/usr/share/keyd/` (non-recursive).
4. At runtime, each loaded config is checked against connected keyboards
   via `config_check_match()`.
5. Matching configs are instantiated as `struct keyboard` objects.

## Layer Resolution

Layers form an occlusion stack. When a key is pressed:

1. Walk the active layers in **activation order** (most recently
   activated first).
2. If a binding is found, execute it and stop.
3. If no binding is found but the layer has modifier tags, apply
   those modifiers and fall through to the next layer.
4. If no layer handles the key, emit it as-is.

The `[main]` layer is always active by default.

### Composite Layers

A composite layer (e.g. `[ctrl+alt]`) is activated when all its
constituents `ctrl` and `alt` are simultaneously active. It overrides
bindings from its constituent layers for keys it explicitly handles.

## Overload Resolution

`overload(layer, tap_action)` works as follows:

1. On key press: start a timer (default immediate).
2. If key is released before another key is pressed → execute `tap_action`.
3. If another key is pressed while this key is held → activate `layer`.

`overloadt(layer, action, timeout)` adds a hold-time threshold: the
key must be held for `<timeout>` ms before it is treated as a hold.

### Timeout-Related Structures in `keyboard.c`

```
pending_overload:   Tracks an overload waiting for tap-vs-hold resolution
pending_timeout:    Tracks a timeout() action waiting for key events
active_macro:       Tracks a running macro's state
timeouts[]:         General timeout table for nested timeouts
```

## IPC Protocol

keyd uses a Unix domain socket (`/var/run/keyd.socket`) for run-time
configuration changes. The protocol is simple binary messages:

```c
struct ipc_message {
    enum {
        IPC_SUCCESS, IPC_FAIL,
        IPC_BIND, IPC_INPUT, IPC_MACRO, IPC_RELOAD, IPC_LAYER_LISTEN,
    } type;
    uint32_t timeout;
    char data[MAX_IPC_MESSAGE_SIZE];  // 4096 bytes
    size_t sz;
};
```

The daemon drops privileges for the socket via `setgid("keyd")`, so
users in the `keyd` group can interact with it.

## Chord Resolution

Chords (e.g. `j+k = esc`) use a debouncing approach:

1. On key press, enter `CHORD_RESOLVING` state and start `chord_interkey_timeout`.
2. If additional keys are pressed within the window, accumulate them.
3. After the last key press, wait `chord_hold_timeout`.
4. If all keys in a defined chord are still held, activate the chord.
5. Otherwise, resolve each key normally.

## Test Harness Architecture

The test suite (`t/`) uses a Python-based harness that:

1. Creates a fake virtual keyboard via `uinput`.
2. Spawns the keyd `test-io` binary (compiled from `t/test-io.c`).
3. Sends synthetic key events at precise timings.
4. Compares collected output against expected results.

See [`t/README.md`](README.md) for test file format details.
