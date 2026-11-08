# CHIP-8 emulator

A CHIP-8 emulator written in C, built as a learning project following [Tobias' Blogpost](https://tobiasvl.github.io/blog/write-a-chip-8-emulator) and [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM): the opcode list, memory map, and display format. 

It has a CPU core with no SDL and no file I/O, and an SDL2 frontend for the window, keyboard, and sound. It can run in CHIP-8, SUPER-CHIP, and XO-CHIP quirk modes, selected at startup.

The test suite runs unit tests and the Timendus CHIP-8 test ROMs. See [roms/README.md](roms/README.md) for the ROMs.

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

### Implemented

- All standard CHIP-8 opcodes, except `0nnn`, which is ignored.
- Keypad, delay timer, and sound timer.
- `Fx0A` waits for a key to be pressed and then released, as on the COSMAC VIP.
- Model presets for VF reset, memory increment, shift source, and jump with VX. These match the `5-quirks` expectations for `chip8`, `schip`, and `schip-legacy`.
- Test ROMs passing: `1-chip8-logo`, `2-ibm-logo`, `3-corax+` and `4-flags` (on `chip8` and `schip`), `6-keypad`, and `7-beep`.

### Missing SUPER-CHIP opcodes

- `00CN`: scroll down N rows.
- `00FB`: scroll right 4 pixels.
- `00FC`: scroll left 4 pixels.
- `00FD`: exit the interpreter.
- `00FE`: low resolution (64×32).
- `00FF`: high resolution (128×64).
- `DXY0`: draw a 16×16 sprite.
- `FX30`: point I at the large 8×10 font.
- `FX75` and `FX85`: save and load the RPL user flags.

High-resolution mode also needs a 128×64 display buffer, which doesn't exist yet. The display is 64×32.

### Missing quirk behaviour

- **Clipping:** `DXYN` always clips at the screen edges. XO-CHIP wraps sprites instead, so the `xochip` model is wrong here.
- **Display wait:** not implemented. The `chip8` model needs it on, `schip-legacy` needs it in low resolution only, and `schip` and `xochip` need it off.
- **XO-CHIP shifting:** the `xochip` preset shifts VX in place. The `5-quirks` test expects it to shift VY. This is not yet verified against the XO-CHIP spec, and I haven't changed it.
- **Expected screens:** the `5-quirks-schip-modern`, `5-quirks-schip-legacy`, and `5-quirks-xochip` screens come from Octo screenshots. Some rows in those screens differ from what the test ROM's own checks expect. This is still under review.

### Missing XO-CHIP features

- XO-CHIP opcodes beyond SUPER-CHIP, such as bitplane selection, long `I` loads, and audio pattern and pitch.
- The XO-CHIP memory size (64K) and multi-plane display.

### Not covered by tests

- The community ROMs in `roms/` (`dinorun`, `dodge`, `snake`, and `tank`) have no tests. `dodge` needs the SUPER-CHIP opcodes above.
- SDL key mapping and audio output have no automated tests.
- The `key_beep` and `index_overflow_sets_vf` flags exist, but every model leaves them off and no test uses them.

The four `5-quirks` tests fail for the reasons in the quirk list above.
