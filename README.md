A CHIP-8 interpreter/emulator written in C, using SDL2 for graphics and input.
Built as a learning project following [Ayman Bagabas's writeup](https://aymanbagabas.com/posts/chip-8-emulator/).

## Status

- [x] Fetch-decode-execute loop
- [x] Opcodes: 00E0, 1NNN, 6XNN, ANNN, DXYN
- [ ] Full opcode table
- [ ] Sound timer / beep
- [ ] Keypad input mapping

## Building

Requires SDL2 and CMake ≥ 3.16.

cmake -S . -B build
cmake --build build
./build/chip8 roms/INVADERS.ch8

## Controls

Original CHIP-8 keypad mapped to QWERTY:
\`\`\`
1 2 3 C        1 2 3 4
4 5 6 D   -->  Q W E R
7 8 9 E   -->  A S D F
A 0 B F        Z X C V
\`\`\`

## Notes / gotchas

- Memory starts at 0x200 (512), first 512 bytes reserved for interpreter/font data.
- Timers count down at 60Hz — decouple this from CPU cycle speed or games run too fast/slow.

## References

- [Writing a Chip-8 emulator](https://aymanbagabas.com/posts/chip-8-emulator/)
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
