# Configuration Examples

Practical examples demonstrating keyd features, organized by use case.

All examples are standalone `.conf` files that can be copied directly to
`/etc/keyd/`.

## Quick Wins

### [capslock-esc-basic.conf](capslock-esc-basic.conf)
CapsLock → Escape on tap, Control on hold. The classic first step.

### [capslock-escape-with-vim-mode.conf](capslock-escape-with-vim-mode.conf)
CapsLock as Escape/Control with vim navigation (hjkl) on the hold layer.

### [shift-bar.conf](shift-bar.conf)
One-shot shift activated by pressing both shift keys simultaneously.

## Layers & Navigation

### [nav-layer.conf](nav-layer.conf)
Full vim-style navigation layer (hjkl → arrows, C-hjkl → C-arrows) with
composite layers. Well-documented with inline comments.

### [layer-carousel.conf](layer-carousel.conf)
Cycle through multiple layers using a single key.

### [extend-layer.conf](extend-layer.conf)
Extend the built-in Control layer with additional bindings.

### [simlayer.conf](simlayer.conf)
Sim-layer example for simultaneous modifier + key combos.

## Advanced Key Behaviors

### [home-row-mods.conf](home-row-mods.conf)
Home-row modifiers using `overloadt()`. Best for combos, not for typing.
Comes with detailed explanation of the mechanics.

### [macos.conf](macos.conf)
Override Alt shortcuts to match macOS conventions (Alt+x → Ctrl+x, etc.).

## Alternative Layouts

### [half-qwerty.conf](half-qwerty.conf)
Split keyboard layout for 40% keyboards (typematic style).

### [international-glyphs.conf](international-glyphs.conf)
International characters via the compose key mechanism.

## Platform-Specific

### [apple-magic-keyboard-touch-id.conf](apple-magic-keyboard-touch-id.conf)
Fix the Apple Magic Keyboard Touch ID key mapping.

### [chromebook-linux.conf](chromebook-linux.conf)
Chromebook keyboard on Linux — remaps special keys.

## Using Examples

1. Pick an example and copy it to `/etc/keyd/`:
   ```sh
   cp examples/nav-layer.conf /etc/keyd/
   ```

2. Reload:
   ```sh
   sudo keyd reload
   ```

3. Check for errors:
   ```sh
   sudo journalctl -eu keyd
   ```

For a full syntax reference, see [docs/config-syntax.md](../docs/config-syntax.md).
