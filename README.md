# Chip 8 Emulator ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
> **CHIP-8** is an interpreted programming language, developed by Joseph Weisbecker. It was initially used on the COSMAC VIP and Telmac 1800 8-bit microcomputers in the mid-1970s. CHIP-8 programs are run on a CHIP-8 virtual machine. It was made to allow video games to be more easily programmed for these computer             
<br>--[Wikipedia](https://en.wikipedia.org/wiki/Chip-8)

This is a complete chip8 emulator :video_game: written in C++ and SDL2.

![Space Invaders](images/space_invaders.png?raw=true "Space Invaders")
<br>
*Space Invaders*

![Tank](images/tank.png?raw=true "Tank")
<br>
*Tank*


## Building

SDL2 is required for this project. You can install it on Ubuntu using 

```
$ sudo apt-get install libsdl2-dev
```

For compilation 

```
$ g++ src/main.cpp -Ofast -lSDL2main -lSDl2 -o chip8
```

## Running

```
./chip8 <ROM file>
```
20 (public domain) ROMs are included in the `roms` directory.

## Testing and Debugging

The roms that are prefixed with `test` are test roms.

There is a debugging mode which can be enabled by passing `-debug` flag after the rom file.

`./chip_8 <ROM file> -debug`

In debugging mode, each opcode is printed and execution is paused at each step.

   - &#8594; key : move to the next instruction.

   - &#8593; key : print all register values and the current instruction.

## Keypad

### Chip8 Keypad:
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | C |
| 4 | 5 | 6 | D |
| 7 | 8 | 9 | E |
| A | 0 | B | F |

### Emulator Keyboard Mapping:
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | 4 |
| Q | W | E | R |
| A | S | D | F |
| Z | X | C | V |
