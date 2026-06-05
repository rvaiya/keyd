# Migration Guides

## v1 → v2

The config format changed significantly between v1 and v2. Key changes:

| v1 | v2 |
|---|---|
| `mods_on_hold(C, esc)` | `overload(control, esc)` |
| `layer_on_hold(layer, esc)` | `overload(layer, esc)` |
| `layer_toggle(layer)` | `toggle(layer)` |
| `layert(layer)` | `toggle(layer)` |
| `[dvorak:default]` | `[dvorak:layout]` |
| `.cfg` extension | `.conf` extension |

### Migration steps:

1. Rename files from `.cfg` to `.conf`
2. Replace `layert` with `toggle`
3. Replace `mods_on_hold` with `overload`
4. Replace `layer_on_hold` with `overload`
5. Replace `:default` with `:layout` for layout sections
6. Add `[ids]` and `[main]` sections if missing:

```ini
[ids]
*

[main]
# your mappings here
```

### Other v2 changes:

- Modifier names are now full names (`control` not `C`)
- Config files are vendor/product ID oriented
- `SIGUSR1` triggers reload (v2.0+)
- Modifiers are layers by default

## v2.2 → v2.3 (major — breaks non-trivial configs)

v2.3.0 introduced breaking changes. See
[DESIGN.md](DESIGN.md) for full details.

### Key changes:

- **Composite layers** are now available (`[layer1+layer2]`)
- Sequences replaced by macros (`C-x` is now `macro(C-x)`)
- **All layers are fully transparent** — bindings drawn by activation order
- **Modifiers apply to all bindings** except the active layer's own modifiers
- **Layout type still exists** but is simpler
- `toggle()` now activates on key *down* instead of key *up*

### Non-compatible changes:

- Three-argument `overload()` replaced by `timeout()`
- Layer types simplified (no more `:main` type distinction)
- `-d` flag eliminated (use systemd/wrapper scripts)
- Special chars in macros (`)`, `\`) must be escaped with backslash

## Current Version

See [CHANGELOG.md](CHANGELOG.md) for all version history. For the latest
version and breaking changes, check the release tags on GitHub:
<https://github.com/rvaiya/keyd/tags>
