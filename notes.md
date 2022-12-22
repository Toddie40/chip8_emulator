# Chip 8 notes

## Links 
- Following the documentation [here]('https://tobiasvl.github.io/blog/write-a-chip-8-emulator/')
- [Link to the wikipedia for chip 8]('https://en.wikipedia.org/wiki/CHIP-8')

## Programming Languge Choice

Alex. You have decided you will write this in C++, not becuase it's easy, but because it's hard. You don't understand computers as well as you'd like to think and your knowledge of C++ is conceptual only. (i.e. object oriented and low-ish level language) This is not going to be easy. It will take time. It will be confusing and difficult often. But, when you get through it to the other side, you will have learned so much and will have made yourself instantly more employable as a software engineer should you wish to take that path. Maybe afterwards you could emulate something more exciting? 

Please follow through with this. 

Please.

## Chip 8 Architecture

The `Chip8` system comprises of:

- 4 kilo _bytes_ of [memory](#Memory)
- A 64 x 32 pixel _monochrome_ [display](#Display)
- A [program counter](#ProgramCounter)
- A single index register (16 bit - 2 byte) called the [I Register](#I-Register)
- A [stack](#Stack) for 16-bit addresses
- An 8-bit [DelayTimer](#DelayTimer) which is decremented at a rate of 60 Hz
- An 8-bit [Sound Timer](#SoundTimer) which makes a beeping noise when non-zero. Decrements the same way as the `delay timer` does
- And finally, 16 general purpose 8-bit [variable registers](#VariableRegisters) numbered 0 to F (in hex) and called V0 to VF (VF is often used as the flag register)

## Plan Of Attack:

You're going to go through the list below and tackle each component of this device one at a time. Learning how each bit works and how it interacts with the other components. Good luck.

## Memory
## Display
## ProgramCounter
## I-Register
## Stack
## DelayTimer
## SoundTimer
## VariableRegisters