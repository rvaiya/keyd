# Test Suite

keyd uses a hybrid test approach: **unit tests** (C-based, no kernel interaction)
and **integration tests** (Python harness with uinput).

## Test Format

Each `.t` file contains two sections separated by a **blank line**:

### Section 1: Input (key events)

```
capslock down
h down
h up
capslock up
```

Each line is `key down` or `key up`. Timed waits are specified as `Nms`:

```
100ms
```

### Section 2: Expected Output

```
left down
left up
```

The same format as input — these are the events keyd should produce.

## Running Tests

### Unit tests (keyboard logic, no root needed):
```sh
make test-io                     # compile and run all tests
make test-io > /dev/null 2>&1   # silence output, check exit code
```

The test harness compiles `t/test-io.c` with the core keyboard processing code
and runs each `.t` file through it.

### Integration tests (requires root):
```sh
sudo make test                   # run all integration tests via run.sh
```

Or using the Python runner directly:
```sh
sudo python3 t/runner.py -v t/layer.t    # verbose, single test
sudo python3 t/runner.py t/*.t           # all tests
sudo python3 t/runner.py -e t/layer.t   # exit on first failure
```

Options:
- `-v` — verbose output with diffs on failure
- `-e` — exit on first failure
- `files` — specific `.t` files to run (default: all)

## Writing Tests

1. Pick a descriptive name: `my-feature.t` (not `layer99.t`)
2. Write input events (top) and expected output (bottom), separated by blank line
3. Run the test:
    ```sh
   sudo python3 t/runner.py -v t/my-feature.t
    ```

### Test File Structure

```
# Top: input events sent to the virtual keyboard
capslock down
h down;h up
capslock up

# Bottom: expected events from keyd
left down;left up
```

## How It Works

The Python runner (`runner.py`):

1. Creates a fake **virtual keyboard** via `/dev/uinput` with vendor:product `2fac:2ade`
2. Runs the keyd binary with a `testio` config pointing at the virtual keyboard
3. Sends synthetic key events from the input section
4. Collects output from keyd's virtual device
5. Compares against the expected output section

See [../../docs/architecture.md](../../docs/architecture.md) for the full system architecture.

## Test Categories (by name prefix)

| Prefix | Feature |
|---|---|
| `layer*.t` | Layer activation, switching, nesting |
| `oneshot*.t` | One-shot modifier behavior |
| `overload*.t` | Tap/hold overload behavior |
| `macro*.t` | Macro expansion and sequencing |
| `swap*.t` | Layer swapping |
| `chord*.t` | Simultaneous key chords |
| `toggle*.t` | Toggle behavior |
| `timeout*.t` | Timeout-based actions |
| `composite*.t` | Composite layer behavior |
| `disarm*.t` | Disarming oneshots/overloads |
| `layout*.t` | Layout switching |
| `mod*.t` | Modifier handling |
| `repeat*.t` | Key repeat behavior |
