# v1.3.1

- Fixed overload behaviour (see #34)

# v1.3.0

- Adds the ability to swap layers mid-key (#48).
- Adds a panic key sequence (backspace + backslash + enter).
- Clears oneshot layers when undefined keys are pressed.
- Removes duplicate keycode names in `keyd -l`.
- Fixes accidental layer toggle bug (#44).
- Improves logging.
- Fixes support for keyboards with built in mice.
- Misc bugfixes.

# v1.1.1

- Make layert behaviour more intuitive when combined with oneshot() and layer().

# v1.1.0

- Added layert() to facilitate semipermanent-activation of occluding layers.
- Now resolve layer conflicts by picking the one which was most recently activated.

# v1.0.0

Major version update:

- Introduces support for modifier layers.
- Simplifies the config format.
- Improves consistency/expected key behaviour.
- Symbols can now be assigned directly place of their names (e.g `&` instead of `S-7`).
- Macro support.

*This breaks existing configs*. Moving forward the config format is expected to
remain backwards compatible.

Main Config Changes:

- Modifiers are now just treated as layers
- The default layer is now called main
- The modifier layout is distinct from the key layout

Config migration map:

```
mods_on_hold(C, esc)        = overload(C, esc)
layer_on_hold(layer, esc)   = overload(layer, esc)
layer_toggle(layer)         = layout(layer)
layer(layer)                = layer(layer)
oneshot(mods)               = oneshot(mods)
oneshot_layer(layer)        = oneshot(layer)
[dvorak:default]            = [dvorak:main]
```

See the [manpage](man.md) for details.
