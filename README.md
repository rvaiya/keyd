# Impetus

Linux lacks a good key remapping solution. In order to achieve satisfactory
results a medley of tools need to be employed (e.g. xcape, xmodmap) with the end
result often being tethered to a specified environment (X11). keyd attempts to
solve this problem by providing a flexible system wide daemon which remaps keys
using kernel level input primitives (evdev, uinput).

# UPDATE

*Version 1.0.0 has just been released and includes some breaking changes. Please see the [changelog](CHANGELOG.md) for details.*

# Features

keyd has several unique features many of which are traditionally only
found in custom keyboard firmware like [QMK](https://github.com/qmk/qmk_firmware).
Some of the more interesting ones include:

- Layers.
- Key overloading (different behaviour on tap/hold).
- Per keyboard configuration.
- Instantaneous remapping (no flashing required).
- A simple and intuitive config format.
- Being display server agnostic (works on wayland and virtual console alike).

## Why would anyone want this?

### keyd is for people who:

 - Would like to experiment with [layers](https://beta.docs.qmk.fm/using-qmk/software-features/feature_layers) (i.e custom shift keys).
 - Want to have multiple keyboards with different logical layouts on the same machine.
 - Want to put the control and escape keys where God intended.
 - Would like the ability to easily generate keycodes in other languages.
 - Constantly fiddle with their key layout.
 - Want an intuitive keyboard config format which is simple to grok.
 - Wish to be able to switch to a VT to debug something without breaking their keymap.
 - Like tiny daemons that adhere to the Unix philosophy.

### What keyd isn't:

 - A tool for launching arbitrary system commands as root.
 - A tool for programming individual key up/down events.
 - A tool for generating unicode characters (though macros can be used to send the requisite keystrokes)

# Dependencies

 - Your favourite C compiler
 - libudev

# Installation

## From Source

    sudo apt-get install libudev-dev # Debian specific, install the corresponding package on your distribution

    git clone https://github.com/rvaiya/keyd
    cd keyd
    make && sudo make install
    sudo systemctl enable keyd && sudo systemctl start keyd

## Packages

Third party packages for the some distributions also exist. If you wish to add
yours to the list please file a PR. These are kindly maintained by community
members, no personal responsibility is taken for them.

### Arch

[AUR](https://aur.archlinux.org/packages/keyd-git/) package maintained by eNV25.

# Quickstart

1. Install keyd

2. Put the following in `/etc/keyd/default.cfg`:

```
# Turns capslock into an escape key when pressed and a control key when held.
capslock = overload(C, esc)

# Remaps the escape key to capslock
esc = capslock
```

3. Run `sudo systemctl restart keyd` to reload the config file.

4. See the [man page](man.md) for a comprehensive list of config options.

*Note*: It is possible to render your machine unusable with a bad config file.
In the event of a rogue configuration the key sequence `backspace+slash+enter`
should terminate keyd. It is recommended that you avoid experimenting in
default.cfg (see the man page for keyboard specific configuration) so you can
plug in another keyboard which is unaffected by the changes.

# Sample Config

    leftshift = oneshot(S)
    capslock = overload(symbols, esc)

    [symbols]

    d = ~
    f = /
    ...

# Recommended config

Many users will probably not be interested in taking full advantage of keyd.
For those who seek simple quality of life improvements I can recommend the
following config:

    leftshift = oneshot(S)
    leftalt = oneshot(A)
    rightalt = oneshot(G)
    rightshift = oneshot(A)
    leftmeta = oneshot(M)
    rightmeta = oneshot(M)

    capslock = overload(C, esc)
    insert = S-insert

This remaps all modifiers to 'oneshot' keys and overloads the capslock key to
function as both escape (when tapped) and control (when held). Thus to produce
the letter A you can now simply tap shift and then a instead of having to hold
it. Finally it remaps insert to S-insert (paste on X11).

# FAQS

## What about xmodmap/setxkbmap/*?

xmodmap and friends are display server level tools with limited functionality.
keyd is a system level solution which implements advanced features like
layering and
[oneshot](https://beta.docs.qmk.fm/using-qmk/software-features/one_shot_keys)
modifiers.  While some X tools offer similar functionality I am not aware of
anything that is as flexible as keyd.

## What about [kmonad](https://github.com/kmonad/kmonad)?

keyd was written several years ago to allow me to easily experiment with
different layouts on my growing keyboard collection. At the time kmonad did not
exist and custom keyboard firmware like
[QMK](https://github.com/qmk/qmk_firmware) (which inspired keyd) was the only
way to get comparable features. I became aware of kmonad after having published
keyd. While kmonad is a fine project with similar goals, it takes a different
approach and has a different design philosophy.

Notably keyd was written entirely in C with performance and simplicity in
mind and will likely never be as configurable as kmonad (which is extensible
in Haskell). Having said that, it supplies (in the author's opinion) the
most valuable features in less than 2000 lines of C while providing
a simple language agnostic config format.

## Why doesn't keyd implement feature X?

If you feel something is missing or find a bug you are welcome to file an issue
on github. keyd has a minimalist (but sane) design philosophy which
intentionally omits certain features (e.g. unicode/executing arbitrary executables
as root). Things which already exist in custom keyboard firmware like QMK are
good candidates for inclusion.

# Contributing

See [CONTRIBUTING](CONTRIBUTING.md).
