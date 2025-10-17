# G2Chip - CHIP-8 Emulator

A modern, cross-platform CHIP-8 emulator implementation written in C with SDL2 support.

## Overview

G2Chip is a clean, well-documented CHIP-8 interpreter that provides a foundation for emulating classic CHIP-8 games and programs. The project features a modular design with configurable callback interfaces, making it easy to integrate into different frontends or extend for variants like SCHIP and XO-CHIP.

## Features

- ✅ **Complete CHIP-8 instruction set** - All 35 original CHIP-8 opcodes
- ✅ **SDL2 frontend** - Interactive emulator with display, keyboard, and audio
- ✅ **Modular architecture** - Clean separation between core emulator and platform code
- ✅ **Configurable callbacks** - Easy to port to different platforms or frameworks
- ✅ **Built-in font set** - Standard CHIP-8 hexadecimal font sprites
- ✅ **Timer support** - Delay and sound timers running at 60Hz
- ✅ **Debug logging** - Optional debug output for development
- 🚧 **SCHIP support** - Planned for future releases
- 🚧 **XO-CHIP support** - Planned for future releases

## Building

### Prerequisites

- CMake 3.10 or higher
- C compiler (GCC, Clang, or MSVC)
- SDL2 development libraries

### Ubuntu/Debian
```bash
sudo apt-get install cmake build-essential libsdl2-dev
```

### macOS (with Homebrew)
```bash
brew install cmake sdl2
```

### Windows
Download SDL2 development libraries from [libsdl.org](https://www.libsdl.org/)

### Compilation

```bash
git clone https://github.com/grzegorz-grzeda/g2chip.git
cd g2chip
mkdir build && cd build
cmake ..
make
```

## Usage

### Running Games

```bash
# Run the interactive SDL2 frontend
./examples/interactive/g2chip-interactive path/to/game.ch8

```

### Controls

The emulator maps CHIP-8's hexadecimal keypad to your keyboard:

```
CHIP-8 Keypad:    Your Keyboard:
+-+-+-+-+         +-+-+-+-+
|1|2|3|C|         |1|2|3|4|
+-+-+-+-+         +-+-+-+-+
|4|5|6|D|   -->   |Q|W|E|R|
+-+-+-+-+         +-+-+-+-+
|7|8|9|E|         |A|S|D|F|
+-+-+-+-+         +-+-+-+-+
|A|0|B|F|         |Z|X|C|V|
+-+-+-+-+         +-+-+-+-+
```

## Programming Interface

G2Chip provides a clean C API for integration into other projects:

```c
#include "g2chip.h"

// Configure callbacks for your platform
g2chip_config_t config = {
    .get_time_ms = your_timer_function,
    .display_clear = your_display_clear,
    .display_draw_pixel = your_pixel_draw,
    .display_refresh = your_display_refresh,
    .key_is_pressed = your_key_check,
    .sound_beep_start = your_sound_start,
    .sound_beep_stop = your_sound_stop,
    .debug_log = your_debug_function
};

// Create and run emulator
g2chip_t* chip = g2chip_create(&config);
g2chip_load_rom(chip, rom_data, rom_size);

// Main emulation loop
while (running) {
    g2chip_step(chip);  // Execute one instruction
}

g2chip_destroy(chip);
```

### API Reference

| Function | Description |
|----------|-------------|
| `g2chip_create()` | Create new emulator instance |
| `g2chip_destroy()` | Clean up emulator instance |
| `g2chip_load_rom()` | Load ROM data into memory |
| `g2chip_reset()` | Reset emulator to initial state |
| `g2chip_step()` | Execute one CPU instruction |

## CHIP-8 Specifications

- **Memory**: 4KB (4096 bytes)
- **Display**: 64×32 pixels, monochrome
- **Registers**: 16 8-bit general purpose (V0-VF)
- **Stack**: 16 levels of nesting
- **Timers**: 60Hz delay and sound timers
- **Input**: 16-key hexadecimal keypad
- **Font**: Built-in 4×5 pixel font for digits 0-F

## Project Structure

```
g2chip/
├── src/                  # Core emulator library
│   ├── g2chip.c         # Main implementation
│   └── g2chip.h         # Public API header
├── examples/
│   └── interactive/     # SDL2 frontend example
├── docs/                # Documentation
└── build/               # Build output directory
```

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

### Development Guidelines

- Follow existing code style and conventions
- Add tests for new features
- Update documentation as needed
- Ensure compatibility with standard CHIP-8 behavior

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## References

- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Wikipedia Article](https://en.wikipedia.org/wiki/CHIP-8)
- [Awesome CHIP-8](https://github.com/tobiasvl/awesome-chip-8)

## Author

**Grzegorz Grzęda** - [grzegorz-grzeda](https://github.com/grzegorz-grzeda)

---

*G2Chip - Simple, clean, and extensible CHIP-8 emulation.*
