# CHIP-8 Instruction Set and Memory Layout — Manual

## Table of contents
- Introduction
- High-level architecture
- Memory layout
- Registers and stack
- Display, input and timers
- Fontset and ROMs
- Instruction encoding
- Opcode reference (concise)
- Implementation notes & common quirks
- Quick emulator checklist

---

## Introduction
CHIP‑8 is a simple interpreted language used historically on 1970s/80s microcomputers. It is commonly used as an emulator tutorial target. This manual describes memory layout, core state (registers, display, input, timers), and the canonical instruction set and semantics you need to implement a compatible emulator.

---

## High-level architecture
- 4K address space: 0x000–0xFFF
- 16 general-purpose 8-bit registers: V0–VF (VF often used as flag)
- 16-bit index register: I
- Program counter (PC): 16-bit, starts at 0x200 for most ROMs
- Stack: used for subroutine calls (commonly depth 16)
- Timers: Delay timer and Sound timer, both 8-bit, decrement at 60 Hz
- Display: monochrome 64×32 pixels (original) or 128×64 in SCHIP extensions
- Input: hexadecimal keypad (16 keys 0x0–0xF)

---

## Memory layout
Typical conventions:
- 0x000–0x1FF: Interpreter / fontset (reserved)  
- 0x050–0x0A0: Common place to store 4×5 font (some implementations store at 0x000)
- 0x200–0xFFF: Program (ROM) and working memory

Example:
- Start PC = 0x200
- First opcode at 0x200, second at 0x202, etc.
- I often points into 0x000–0xFFF for sprites or memory operations.

---

## Registers and stack
- V0–VF: 8-bit registers
    - VF = 0xF commonly used as carry/borrow/collision flag (set to 0 or 1)
- I: 16-bit index register used for memory addressing
- PC: 16-bit program counter; advances by 2 per instruction unless changed
- SP/stack: push PC then set PC to target for CALL; RET pops PC
- Delay timer: 8-bit; when nonzero decremented at 60 Hz
- Sound timer: 8-bit; when nonzero, beeps; decremented at 60 Hz

---

## Display, input and timers
- Display: framebuffer of 64×32 bits (coordinate origin top-left)
- Drawing: XOR sprites; set VF = 1 if any pixel goes from 1→0 (collision), else 0
- Keypad: 16 keys (map free); instructions for conditional skip based on key state
- Timers tick at 60 Hz; ensure consistent timing (host system-dependent)

---

## Fontset
Common 4×5 pixel sprites for hex digits 0x0–0xF stored sequentially (5 bytes per glyph). Many implementations place them at 0x050; other interpreters keep them at 0x000.

---

## Instruction encoding
- Opcodes are 2 bytes (16 bits), big-endian
- Notation:
    - NNN = 12-bit address
    - NN = 8-bit immediate
    - N = 4-bit nibble
    - X, Y = 4-bit register identifiers (Vx, Vy)

Fetch:
- opcode = memory[PC] << 8 | memory[PC + 1]
- PC += 2 (unless instruction modifies it)

---

## Opcode reference (concise)
Note: pseudocode uses V[], I, PC, stack[], SP, delay, sound, display[], keys[].

0x0000 group
- 00E0 — CLS
    - Clear display: zero framebuffer; PC += 2
- 00EE — RET
    - PC = stack.pop(); SP--; PC += 2 (or set PC to popped value)

0x1NNN
- 1NNN — JP addr
    - PC = NNN

0x2NNN
- 2NNN — CALL addr
    - stack.push(PC); PC = NNN

0x3XNN
- 3XNN — SE Vx, byte
    - if V[X] == NN: PC += 4 else PC += 2

0x4XNN
- 4XNN — SNE Vx, byte
    - if V[X] != NN: PC += 4 else PC += 2

0x5XY0
- 5XY0 — SE Vx, Vy
    - if V[X] == V[Y]: PC += 4 else PC += 2

0x6XNN
- 6XNN — LD Vx, byte
    - V[X] = NN

0x7XNN
- 7XNN — ADD Vx, byte
    - V[X] = (V[X] + NN) & 0xFF

0x8XY_ (arithmetic / bit ops)
- 8XY0 — LD Vx, Vy: V[X] = V[Y]
- 8XY1 — OR Vx, Vy: V[X] |= V[Y]
- 8XY2 — AND Vx, Vy
- 8XY3 — XOR Vx, Vy
- 8XY4 — ADD Vx, Vy: VF = carry; V[X] += V[Y]
- 8XY5 — SUB Vx, Vy: VF = NOT borrow; V[X] -= V[Y]
- 8XY6 — SHR Vx {, Vy}: VF = LSB of Vx; V[X] >>= 1
    - Note: some interpreters set Vx = Vy before shift; choose one behavior and document it.
- 8XY7 — SUBN Vx, Vy: V[X] = V[Y] - V[X]; VF = NOT borrow
- 8XYE — SHL Vx {, Vy}: VF = MSB of Vx; V[X] <<= 1
    - Behavior variations exist (see quirks)

0x9XY0
- 9XY0 — SNE Vx, Vy
    - if V[X] != V[Y]: PC += 4

0xANNN
- ANNN — LD I, addr
    - I = NNN

0xBNNN
- BNNN — JP V0, addr
    - PC = NNN + V[0]  (wrap to 12 bits often)

0xCXNN
- CXNN — RND Vx, byte
    - V[X] = random_byte() & NN

0xDXYN
- DXYN — DRW Vx, Vy, nibble
    - Draw N-byte sprite at (Vx, Vy) from memory[I]
    - For each sprite byte: XOR into 8-pixel row; if any 1→0, VF = 1 else 0
    - Wrap or clip per implementation (originals wrap)

0xEX__
- EX9E — SKP Vx
    - if key[V[X]] pressed: PC += 4
- EXA1 — SKNP Vx
    - if key[V[X]] not pressed: PC += 4

0xFX__
- FX07 — LD Vx, DT
    - V[X] = delay
- FX0A — LD Vx, K
    - Wait for key press, store key in V[X] (blocking)
- FX15 — LD DT, Vx
    - delay = V[X]
- FX18 — LD ST, Vx
    - sound = V[X]
- FX1E — ADD I, Vx
    - I += V[X]; VF undefined in original; some set VF on overflow
- FX29 — LD F, Vx
    - I = location of sprite for digit V[X] (usually 5×bytes per digit)
- FX33 — LD B, Vx
    - Store BCD of V[X] into memory[I], I+1, I+2
- FX55 — LD [I], Vx
    - Store V0..Vx into memory starting at I; some interpreters set I += X+1
- FX65 — LD Vx, [I]
    - Read memory into V0..Vx; some interpreters set I += X+1

---

## Implementation notes & common quirks
- Endianness: opcodes are big-endian; read two bytes then decode.
- PC increments by 2 normally; branching/calls/returns set PC explicitly.
- Behavior variations: SHR/SHL, FX55/FX65 I modification, location of fontset, sprite wrapping vs clipping — document chosen behavior and optionally provide compatibility modes (XO/Schip).
- Collision detection: for DXYN, set VF to 1 if any pixel changed from 1 to 0.
- Timing: timers tick at 60 Hz. Instruction speed is host dependent; many emulators run at a fixed cycles-per-frame.
- Keyboard mapping: map CHIP‑8 hex keys to host keys (e.g., numpad or keyboard cluster).
- Memory safety: ensure I + N does not overflow 0xFFF; wrap or clamp per chosen policy.

---

## Testing & debugging tips
- Use known test ROMs (blinking, font, draw tests) to verify drawing and timers.
- Log opcodes in hex along with PC to track flow.
- Implement single-step and break-on-key to debug blocking FX0A.
- Visualize memory and V registers while running.

---

## Quick emulator checklist
- [ ] Memory 4K, PC starts at 0x200
- [ ] V0–VF, I, stack, SP
- [ ] Display 64×32 bits, clear and draw with XOR
- [ ] Timers (60 Hz)
- [ ] Keypad input and blocking key-read
- [ ] All opcodes implemented (including FX and 8XY variants)
- [ ] Handle common quirks and document choices

---

This manual provides the core facts and concise opcode semantics needed to build a CHIP‑8 emulator. For extended behavior (Super‑CHIP, SCHIP 1.1) consult those specific extension docs and test ROMs; handle variants via compatibility modes.
