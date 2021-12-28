# v2.0.1-beta

 - Added + syntax to macros to allow for simultaenously held keys.

# v2.0.0-beta

Major version update. 

This breaks 1.x configs. The format may change slightly before leaving beta,
but once it does should remain backwards compatible for the foreseeable future.

A non exhaustive list of changes can be found below. It is best to forget
everything you know and read man page anew.

- Eliminated layer inheritance in favour of simple types.
(layouts are now defined with `:layout` instead of `:main`)
- Macros are now repeatable.
- Overload now accepts a hold threshold timeout.
- Config files are now vendor/product id oriented.
- SIGUSR1 now triggers a config reload.
- Modifiers are layers by default and can be extended directly.
- Config files now end in `.conf`.
- `layert()` is now `toggle()`.
- All layers are 'modifier layers' (terminological change)
- Eliminated the dedicated modifer layout.
- Modifiers no longer apply to key sequences defined within a layer.
  (Layer entries are now always executed verbatim.)

  The old behaviour was unintuitive and can be emulated using nested
  layers if necessary.

For most old configs transitioning should be a simple matter of changing
the file extension from `.cfg` to `.conf`, replacing `layert` with
`toggle`, changing `:main` to `:layout` and adding

```
[ids]

*

[main]
```

to the top of the file. 

More involved configs may need additional changes, but should be possible
to replicate using the new rules. If not, please file an issue on
[github](https://github.com/rvaiya/keyd/issues).

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
