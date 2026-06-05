# Actions Reference

Actions are special functions that define key behavior. Each action takes
zero or more arguments.

## Layer Actions

### `layer(name)`
Activate `name` while the key is held.

```ini
capslock = layer(nav)
```

### `oneshot(name[, name2, name3])`
Activate the layer(s) for the next keypress only. Up to 3 layers:

```ini
leftshift = oneshot(shift)
capslock = oneshot(ctrl, shift)
```

### `toggle(name)`
Permanently toggle a layer (toggles off on repeat):

```ini
esc = toggle(dvorak)
```

### `layerm(name, macro)`
Like `layer()`, but executes a macro on activation.

### `oneshotm(name, macro)`
Like `oneshot()`, but executes a macro on activation.

### `oneshotk(name, key)`
Like `oneshot()`, but also fires `key` while held:

```ini
# Tap: one-shot shift. Hold: fire 'a'.
leftshift = oneshotk(shift, a)
```

### `swap(name [, macro])`
Swap the active layer with `name`. Complex — see [man page](https://www.mankier.com/1/keyd) for details.

### `clear()`
Clear all toggled and oneshot layers.

### `clearm(macro)`
Like `clear()`, but executes a macro first.

## Key Overloading

### `overload(layer, tap_action)`
Hold → activate `layer`. Tap → fire `tap_action`.

```ini
capslock = overload(control, esc)
```

### `overloadt(layer, action, timeout_ms)`
Only activate `layer` if held for `timeout_ms`. Otherwise fire `action`.
Ideal for home-row modifiers (letter keys):

```ini
a = overloadt(control, a, 200)
```

### `overloadt2(layer, action, timeout_ms)`
Like `overloadt()`, but also resolves as a hold on any intervening key tap.

### `overloadi(action1, action2, idle_timeout_ms)`
Fire `action1` if a non-action key was struck within `idle_timeout_ms`;
otherwise fire `action2`. Use with `overloadt2` for homerow mods:

```ini
a = overloadi(a, overloadt2(control, a, 200), 150)
```

### `lettermod(layer, key, idle_timeout, hold_timeout)`
Convenience macro for the pattern above:

```ini
a = lettermod(control, a, 150, 200)
```

### `timeout(action1, timeout_ms, action2)`
If no keys are pressed within `timeout_ms`, fire `action1`; otherwise fire `action2`. Useful for advanced patterns:

```ini
# Hold for 500ms → control. Tap → 'a'.
key = timeout(a, 500, layer(control))
```

## Macro Actions

### `macro(tokens)`
Execute a sequence of key events:

```ini
key = macro(C-g n)          # Ctrl+G then N
key = macro(Hello world)    # Type "Hello world"
key = macro(C-t 100ms google.com enter)
```

### `macro2(timeout, repeat_timeout, macro)`
Macro with custom timeouts (0 disables repeat):

```ini
key = macro2(400, 50, macro(Hello world))
```

## Misc Actions

### `repeat()`
Repeat the last key or macro.

### `setlayout(name)`
Change the active layout at runtime.

### `command(cmd)`
Execute a shell command as the daemon user (usually root). Use judiciously:

```ini
key = command(brightness down)
```

### `noop`
Do nothing. Useful for disabling keys:

```ini
esc = noop
```

## Chording

Define simultaneous key combinations:

```ini
j+k = esc          # pressing j and k together → Escape
```

The default inter-key timeout is 50ms. Adjust per-layer with `chord_timeout`
in the `[global]` section.
