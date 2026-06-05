# Troubleshooting

## My machine is locked up after a bad config

**Panic sequence:** Press `Backspace + Escape + Enter` simultaneously.
This forces keyd to terminate and restores your original keyboard.

## Config errors

Errors appear in keyd's journal. View them with:

```sh
sudo journalctl -eu keyd
```

Validate configs without applying them:

```sh
keyd check
keyd check /etc/keyd/mykeyboard.conf
```

## Trackpad disabled while typing

libinput's "disable-while-typing" feature misidentifies keyd's virtual
keyboard as external. Fix by creating `/etc/libinput/local-overrides.quirks`:

```ini
[Serial Keyboards]
MatchUdevType=keyboard
MatchName=keyd*keyboard
AttrKeyboardIntegration=internal
```

## setxkbmap / xset settings lost after restart

keyd creates a virtual input device, which may bypass display server
settings. Reapply your settings after (re)starting keyd, or define the
remappings directly in keyd configs instead.

See: <https://github.com/rvaiya/keyd/issues/183>

## Unicode / compose glyphs not working

1. Link the compose file:
   ```sh
   ln -s /usr/share/keyd/keyd.compose ~/.XCompose
   ```

2. Ensure your display server is set to the **US** layout:
   ```sh
   setxkbmap -option compose:menu
   ```

3. Use keyd's built-in layouts for non-English keyboards instead of
   display server layouts to avoid conflicts.

**Note:** GTK4 has a known bug crashing with large XCompose files.

## Mice misbehaving with wildcard `[ids]`

Some mice (e.g. Logitech MX Master) emit keyboard events and are
matched by the wildcard. Explicitly blacklist them:

```ini
[ids]
*
-046d:b01d    # your mouse's vendor:product ID
```

## Wayland compositors interfering with LED indicators

Some compositors (e.g. Sway) aggressively reset LED state. The
`layer_indicator` option in `[global]` may not work under these.

## Application remapping not working

Prerequisites:

1. User is in the `keyd` group: `usermod -aG keyd $USER` (relogin required)
2. keyd is running and you can access `/var/run/keyd.socket`
3. `keyd-application-mapper` is started (see [man page](https://www.mankier.com/1/keyd-application-mapper))

Debug window detection:

```sh
keyd-application-mapper -v
```

Or check the log when running as a daemon:

```sh
tail -f ~/.config/keyd/app.log
```

## Debugging key events

Use `keyd monitor` to discover key names:

```sh
sudo keyd monitor          # shows keyd's output
sudo systemctl stop keyd   # stop keyd
sudo keyd monitor          # now shows raw input
```

## keyd fails to start

Common issues:

- **Permission denied on socket:** Ensure the `keyd` group exists.
  `make install` creates it automatically.
- **uinput not available:** Ensure `/dev/uinput` exists and you have
  permissions (`sudo usermod -aG input root` or use udev rules).
- **Device not found:** Run `keyd monitor` to see detected devices and
  their IDs.
