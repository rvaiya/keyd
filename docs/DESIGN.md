This document contains a description of major design iterations.
It is not intended to be exhaustive.

# v2.3.0-rc

This is a major release which breaks backward compatibility for
non-trivial configs (we are still in beta after all :P).

In the absence of too much blowback, this will probably become the final
v2 design.

Much of this harkens back to v1, with some additional simplifications
and enhancements.

## Notable changes:

  - Introduced composite layers

  - Eliminated sequences in favour of macros (C-x is now just syntactic
    sugar for macro(C-x)).

  - Actions which previously accepted sequences as a second argument
    now accept macros of any kind.

  - General stability/speed/memory improvements

  - Made the man page less war and peacey

## Non backward-compatible changes

  - Replaced three arg overload() with a more flexible timeout() mechanism

  - Layers are now fully transparent. That is, bindings are drawn on the basis
    of activation order with [main] being active by default.

  - Modifiers now apply to all bindings with the exception of modifiers
    associated with the active layer.

    Rationale:

    The end result is more intuitive and in conjunction with transparency
    allows modifiers to be paired with layer entries without having
    to use layer nesting (#103).

    E.G

```
	capslock = layer(nav)

	[nav:C]

	h = left
	l = right
```

will cause `capslock+h` to produce `left` (rather than `C-left`), while
`control+capslock+h` will produce `C-left`, as one might intuit.

  - Abolished layer types. Notably, the concept of a 'layout' no longer exists.

    Rationale:

    This simplifies the lookup logic, elminates the need for dedicated layout
    actions, and makes it easier to define common bindings for letter layouts
    since the main layer can be used as a fallback during lookup.

    E.G

          [main]

          capslock = layer(capslock)

          [dvorak]

          a = a
          s = o
          ...

          [capslock]

          1 = toggle(dvorak)

  - Special characters within macros like ) and \ must be escaped with a backslash.

  - Modifier sequences (e.g `C-M`) are no longer valid layers by default.

    Rationale:

    This was legacy magic from v1 which added a bunch of cruft to the code and
    seemed to cause confusion by blurring the boundaries between layers and
    modifiers. Similar results can be achieved by explicitly defining an
    empty layer with the desired modifier tags:

          I.E

                  a = layer(M-C)
                  b = layer(M)

          becomes

                  a = layer(meta-control)
                  b = layer(meta)

                  [meta-control:M-C]

    Note that in the above sample "meta" does not need to be
    explicitly defined, since full modifier names are still
    mapped to their eponymously named layers.

  - Eliminated -d

    Rationale:

    Modern init systems are quite adept at daemonization, and the end user
    can always redirect/fork using a wrapper script if desired.

  - Eliminated reload on SIGUSR1

    Rationale:

    Startup time has been reduced making the cost of a full
    restart negligible (and probably more reliable).
