# Virtual Keyboard Backends

Virtual keyboard backend implementations. Selected at build time via `make VKBD=<name>`.

| Backend | Description | Build flag |
|---|---|---|
| `uinput.c` | **Default** — Linux uinput virtual keyboard | `make` or `make VKBD=uinput` |
| `stdout.c` | Debug — prints events to stdout | `make VKBD=stdout` |
| `usb-gadget.c` | SBC USB gadget — HID reports over USB OTG | `make VKBD=usb-gadget` |

See [docs/usb-gadget.md](../docs/usb-gadget.md) for USB gadget setup instructions.
