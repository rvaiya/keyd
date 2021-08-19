# 0.1.0

Major version update:

- Introduces support for modifier layers.
- Simplifies config format.
- Improves consistency/expected key behaviour.

This breaks existing configs. Moving forward the config format is expected to
remain backwards compatible.

Main Changes:

- Modifiers are now just treated as layers
- The default layer is now called main
- The modifier layout is distinct from the key layout
- Symbols can be used in place of their names (e.g ';' instead of 'semicolon')
- Shifted symbols can now be specified directly (e.g & instead of S-7)

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

See the manpage for details.
