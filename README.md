# Brainfuck compiler for Mac OS X

Compile Brainfuck programs into Mach-O executables for x86-64.

-- version of Xcode, clang, and command-line-utils

## What is Brainfuck?

[Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) is an extremely minimalistic, yet Turing-complete, programming
language (so-called _esoteric_ programming language).  The main idea is to manipulate an array of **cells**, using
simple commands. A cell is one byte, and the array consists of (at least) 30,000 cells.

On start up, each cell is initialised to zero and a **cell pointer** is set to the first (leftmost) cell. The program
is then executed sequentially, command by command, until the last command. Each command consists of one character,
and there are eight possible commands for moving the cell pointer, increasing/decreasing the current cell value,
I/O of the current cell and a simple loop structure. Non-command characters are just ignored.


| Command | Action                                                                             | C equivalent        |
|---------|------------------------------------------------------------------------------------|---------------------|
|   `>`   | Increment the cell pointer (move it to the right)                                  | `++ptr;`            |
|   `<`   | Decrement the cell pointer (move it to the left)                                   | `--ptr;`            |
|   `+`   | Increment the value of the current cell                                            | `++*ptr;`           |
|   `-`   | Decrement the value of the current cell                                            | `--*ptr;`           |
|   `.`   | Write the value of the current cell to screen                                      | `putchar(*ptr);`    |
|   `,`   | Read one byte of input and store the value in the current cell                     | `*ptr = getchar();` |
|   `[`   | If the cell value is zero, jump to the command following the matching `]` command  | `while (*ptr) {`    |
|   `]`   | If the cell value is **not** zero, jump back to the command after the matching `[` | `}`                 |


### Example
The following Brainfuck code adds 2 and 3
```brainfuck
++       set cell 0 to 2
>        move cell pointer to the right
+++   	 set cell 1 to 3
<        move cell pointer back to cell 0
[        while current cell is greater than 0
  -        decrease cell 0 with 1
  >        move cell pointer to cell 1
  +        increase value of cell 1
  <        move cell pointer back to cell 0
]        end loop
```

Step by step, this will look something like this:
```
    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   0 |   0 |   0 |...|   0 |         Initial state, 30,000 cells initialised to 0 and the cell pointer pointing at cell 0.
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   0 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   0 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   0 |   0 |   0 |...|   0 |    >    Increment cell pointer
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   1 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   2 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   3 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   3 |   0 |   0 |...|   0 |    <    Decrement cell pointer
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   2 |   3 |   0 |   0 |...|   0 |    [    Current cell is greater than 0 so we enter the loop
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   3 |   0 |   0 |...|   0 |    -    Decrement current cell
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   3 |   0 |   0 |...|   0 |    >    Increment cell pointer
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   4 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   4 |   0 |   0 |...|   0 |    <    Decrement cell pointer
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   1 |   4 |   0 |   0 |...|   0 |    ]    Current cell is greater than 0 so we skip back to loop start
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   4 |   0 |   0 |...|   0 |    -    Decrement current cell
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   4 |   0 |   0 |...|   0 |    >    Increment cell pointer
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   5 |   0 |   0 |...|   0 |    +    Increment current cell
 +-----+-----+-----+-----+~~~+-----+
          ^
         ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   5 |   0 |   0 |...|   0 |    <    Decrement cell pointer
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr

    0     1     3     4        30k
 +-----+-----+-----+-----+~~~+-----+
 |   0 |   5 |   0 |   0 |...|   0 |    ]    Current cell is 0 so we skip the loop and terminate
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr
```


## Implementation details
The compiler will output an executable which, when loaded into memory, will have the following layout.

```
0x0000000000 +---------------------+
             |     __PAGEZERO      |
             |                     |
             |                     | <-- 4 GB with inaccessible memory
             |                     |
             |                     |
0x1000000000 +---------------------+
             |    __DATA __data    | <-- 2^16-1 bytes of zero filled data
0x1000010000 +---------------------+
             |    __TEXT __text    |
             |                     |
             |                     | <-- The compiled code
             |                     |
             |                     |
             +---------------------+
```

- exit code
- no change on eof
- no dynamic bounds
- number of cells
- cell width

## How it compiles
al, dx, rbp etc

## Compiler optimisations
addl
reduce load stores
reduce push and pops

## Macho

### Format


### Required load commands and stricter eval
