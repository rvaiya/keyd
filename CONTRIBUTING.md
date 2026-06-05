# Contributing to keyd

keyd welcomes contributions in the form of bug reports, feature requests,
documentation, and code patches. Please read this guide before submitting
anything.

## 1. Reporting Bugs & Feature Requests

- Open an issue on [GitHub](https://github.com/rvaiya/keyd/issues).
- Include your OS, keyd version (`keyd -v`), and a minimal config reproducing the issue.
- For feature requests, explain the use case and any existing alternatives.

## 2. Building

### Standard build:
```sh
git clone https://github.com/rvaiya/keyd && cd keyd
make && sudo make install
```

### Debug build (with AddressSanitizer):
```sh
make debug
```

### Build with USB gadget backend (for SBCs):
```sh
make VKBD=usb-gadget
```

### Build man pages:
```sh
make man          # requires scdoc
```

### Cross-reference the architecture:
See [docs/architecture.md](docs/architecture.md) for system internals and module responsibilities.

## 3. Testing

### Unit tests (no root required):
```sh
make test-io                        # run all unit tests
```

### Integration tests (requires root):
```sh
sudo make test                     # run all integration tests
sudo python3 t/runner.py -v t/layer.t   # verbose, single test
sudo python3 t/runner.py -e t/layer.t   # exit on first failure
```

### Writing new tests:
See [t/README.md](t/README.md) for the `.t` file format and test harness details.

## 4. Code Style

- **Language:** C11 (`-std=c11`).
- **Indentation:** Tabs (8-space equivalent). No spaces for indentation.
- **Braces:** Opening brace on the same line.
- **Headers:** Include `keyd.h` first, then specific headers as needed.
- **Comments:** Use `/* */` for multi-line, `//` is not used.
- **Prototypes:** Functions should declare `static` when not externally visible.
- **Types:** Use fixed-width types (`uint8_t`, `size_t`) preferentially.
- **No trailing whitespace.** Run `sed -i 's/[[:space:]]*$//' file.c` before committing.

### Header files:
- Use include guards (`#ifndef NAME_H`).
- Group includes: system headers first, then project headers, then local.
- Each `.c` should have a corresponding `.h` unless it's intentionally standalone.

## 5. Adding New Features

The key processing logic in `keyboard.c` is complex and has subtle interactions
between layers, overloads, macros, and timeouts. **Please file an issue before
sending patches that modify core functionality.**

When adding a new config action:

1. Add the `OP_NEW_ACTION` enum to `config.h`.
2. Parse it in `config.c` (look for existing action parsers as reference).
3. Implement the resolution in `keyboard.c` (look for similar actions).
4. Add test cases in `t/` covering:
   - Basic usage
   - Interaction with overloads
   - Interaction with modifiers
   - Edge cases (nested, multiple layers, etc.)
5. Document the action in `docs/actions.md` and update the man page in `docs/keyd.scdoc`.

## 6. Adding Layouts

1. Create a new file in `layouts/` with the `[layoutname:layout]` header.
2. Define key mappings in standard format (`key = mapped_key`).
3. Include a shift layer for non-US layouts.
4. Add a reference to [layouts/README.md](layouts/README.md) if it's a new category.

## 7. Adding Examples

1. Create a `.conf` file in `examples/`.
2. Include detailed comments explaining the purpose and mechanics.
3. Update [examples/README.md](examples/README.md) with a one-line description.
4. Ensure the example is self-contained and works when copied to `/etc/keyd/`.

## 8. Documentation Updates

### Man pages (`docs/*.scdoc`)
Build with `make man`. The `.scdoc` format uses indentation-based markup.

### Adding to docs
New documentation files go in `docs/`. Update the README to link to new docs.

## 9. Commit Messages

- **Subject line:** `module: short description` (e.g. `keyboard: fix overload timeout regression`)
- **Body:** Explain *why* the change was made, not just *what* changed.
- Reference issue numbers where applicable.

## 10. Pull Requests

- Keep PRs focused. One feature/fix per PR.
- Include test cases for behavior changes.
- Mention if the change is backward-incompatible.
- All tests must pass (`make test && make test-io`).
