# CHIP-8 emulator

A CHIP-8 emulator written in C, built as a learning project. It is designed so the CPU core can later be reused for a GBA emulator.

It has a CPU core with no SDL and no file I/O, and an SDL2 frontend for the window, keyboard, and sound. It can run in CHIP-8, SUPER-CHIP, and XO-CHIP quirk modes, selected at startup.

The test suite runs unit tests and the [Timendus CHIP-8 test ROMs](https://github.com/Timendus/chip8-test-suite), and checks the screens against reference images. See [roms/README.md](roms/README.md) for the ROMs.

## Requirements

- Linux (other platforms may work but are untested)
- A C17 compiler, such as GCC or Clang
- CMake 3.16 or newer
- SDL2 development files

## Building

```sh
cmake -S . -B build
cmake --build build
```

This builds two programs in `build/`:

- `chip8`, the emulator.
- `chip8_tests`, the test runner.

To build with AddressSanitizer and UBSan, use a separate build directory:

```sh
cmake -S . -B build-san -DCHIP8_SANITIZE=ON
cmake --build build-san
```

## Running

```sh
./build/chip8 <rom.ch8> [--model chip8|schip|schip-legacy|xochip]
```

The model defaults to `chip8`. It sets the quirk behaviour for the run:

| Model         | Behaviour                                          |
|---------------|----------------------------------------------------|
| `chip8`       | Original COSMAC VIP behaviour                      |
| `schip`       | SUPER-CHIP (modern) behaviour                      |
| `schip-legacy`| SUPER-CHIP (legacy) behaviour                      |
| `xochip`      | XO-CHIP behaviour                                  |

For example:

```sh
./build/chip8 roms/1-chip8-logo.ch8 --model chip8
```

### Keyboard

The CHIP-8 keypad is mapped onto the left side of a QWERTY keyboard:

```
CHIP-8     Keyboard
1 2 3 C    1 2 3 4
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
```

Close the window to quit.

## Testing

```sh
./build/chip8_tests
```

To list every test:

```sh
./build/chip8_tests --list
```

To run only tests whose area or name contains a text string, pass it as a filter. The match is case-insensitive:

```sh
./build/chip8_tests 7-beep
```

## Project layout

```
include/   Public headers for the CPU, ROM loader, keypad, and display
src/       Emulator source: CPU core, opcodes, keypad, ROM loader, SDL frontend
tests/     Test runner, unit tests, and ROM tests with expected screens
roms/      Test ROMs (see roms/README.md)
```

## Status

Implemented:

- All standard CHIP-8 opcodes.
- Quirk behaviour for the four models.
- Keypad, sound timer, and delay timer.
- The `1-chip8-logo`, `2-ibm-logo`, `3-corax+`, `4-flags`, `6-keypad`, and `7-beep` test ROMs pass.

Not yet implemented:

- SUPER-CHIP `00xx` opcodes and high-resolution mode.
- Display wait and sprite clipping as quirk options.
- XO-CHIP features beyond the basics.

The four `5-quirks` tests fail until these are done. This is expected.
