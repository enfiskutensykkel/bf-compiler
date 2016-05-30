Brainfuck compiler for Mac OS X
=======================================================================================================================

Compiler that translates Brainfuck programs into to x86-64 Mach-O executables.

Usage
-----------------------------------------------------------------------------------------------------------------------
```
$ ./bfc source-file.bf output-file
```

Requirements
-----------------------------------------------------------------------------------------------------------------------
- Mac OS X 10.11 (El Capitan) or newer
- LLVM/Clang (install Xcode version 7.0.3 or newer)

What is Brainfuck?
-----------------------------------------------------------------------------------------------------------------------

[Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) is an extremely minimalistic, Turing-complete, 
esoteric programming language. The main idea is to manipulate an array consisting of at _least_ 30,000 
cells, using simple commands for incrementing and decrementing the cell values, simple I/O for reading 
a value into a cell and writing cell's value and changing the current cell by moving around a _cell 
pointer_.

On start up, each cell is initialised to zero and the cell pointer is set to the first/left most cell.
The program is then run sequentially, command by command, until the last command. There are eight commands,
consisting of a single character.

| Command | Action                                                                             | C equivalent        |
|---------|------------------------------------------------------------------------------------|---------------------|
|   `>`   | Increment the cell pointer (move it to the right)                                  | `++ptr;`            |
|   `<`   | Decrement the cell pointer (move it to the left)                                   | `--ptr;`            |
|   `+`   | Increment the value of the cell pointed to                                         | `++*ptr;`           |
|   `-`   | Decrement the value of the cell pointed to                                         | `--*ptr;`           |
|   `.`   | Print the value of the cell pointed to                                             | `putchar(*ptr);`    |
|   `,`   | Read one byte of input and store the value in the cell pointed to                  | `*ptr = getchar();` |
|   `[`   | If the cell value is zero, jump to the command following the matching `]` command  | `while (*ptr) {`    |
|   `]`   | If the cell value is **not** zero, jump back to the command after the matching `[` | `}`                 |


#### Cell size
In my implementation, cell are 8-bits (one byte) and will wrap around on arithmetic overflow. I plan on
implementing cell size as a compiler option.

#### Array size
In my implementation, the cell array is 2^16 - 1 large. I also plan on implementing array size as a compiler option.

#### Array out-of-bounds behaviour
Currently I don't handle out-of-bounds behaviour for the compiler programs; it should be considered undefined behaviour.
At some point I will make dynamic resizing of the cell array and cell pointer wrap around as options to the compiler.

#### End-of-file behaviour
When EOF is encountered in the input stream, cell value is not changed. A possible extension to the compiler is to
support signed 16-bit values in cells and store -1.

