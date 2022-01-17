# v2.2.4-beta
  - Add support for application mapping by title
  - Fix macro timeouts
  - Forbid modifier keycodes as lone right hand values in favour of layers

# v2.2.3-beta

 - Enable hot swapping of depressed keybindings via -e
 - Improve support for application remapping

# v2.2.2-beta

 - Change panic sequence to `backspace+enter+escape` (#94)
 - Fix overload+modifer behaviour (#95)

# v2.2.1-beta

 - Move application bindings into ~/.config/keyd/app.conf.
 - Add -d to keyd-application-mapper.
 - Fix broken gnome support.

# v2.2.0-beta

 - Add a new IPC mechanism for dynamically altering the keymap (-e).
 - Add experimental support for app specific bindings.

# v2.1.0-beta

 NOTE: This might break some configs (see below)

 - Change default modifier mappings to their full names (i.e `control =
   layer(control)` instead of `control = layer(C)` in an unmodified key map)`.

 - Modifier names are now syntactic sugar for assigning both associated key
   codes.  E.G `control` corresponds to both `leftcontrol` and `rightcontrol`
   when assigned instead of just the former.  (Note: this also means it is no
   longer a valid right hand value)

 - Fixes v1 overload regression (#74)

# v2.0.1-beta

 - Add + syntax to macros to allow for simultaenously held keys.

# v2.0.0-beta

Major version update.

This breaks 1.x configs. The format may change slightly before leaving beta,
but once it does should remain backwards compatible for the foreseeable future.

A non exhaustive list of changes can be found below. It is best to forget
everything you know and read man page anew.

	- Eliminate layer inheritance in favour of simple types.
	(layouts are now defined with `:layout` instead of `:main`)
	- Macros are now repeatable.
	- Overload now accepts a hold threshold timeout.
	- Config files are now vendor/product id oriented.
	- SIGUSR1 now triggers a config reload.
	- Modifiers are layers by default and can be extended directly.
	- Config files now end in `.conf`.
	- `layert()` is now `toggle()`.
	- All layers are 'modifier layers' (terminological change)
	- Eliminate the dedicated modifer layout.
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

- Add layert() to facilitate semipermanent-activation of occluding layers.
- Resolve layer conflicts by picking the one which was most recently activated.

# v1.0.0

Major version update:

- Introduce support for modifier layers.
- Simplify the config format.
- Improve consistency/expected key behaviour.
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
