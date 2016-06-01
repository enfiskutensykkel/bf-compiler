Brainfuck compiler for Mac OS X
=====================================================================================================================
Compile Brainfuck programs to Mach-O executables for x86-64.


### Technical requirements ###
I personally use the following:
- LLVM version 7.0.2 (clang-700.1.81), which is Command Line Tools version 7.0.3
- Mac OS X 10.11.4 (x86\_64-apple-darwin15.5.0)

My guess is that it will work with older versions too, but you might have to tweak the version number for 
`/usr/lib/libSystem.B.dylib` in the Mach-O output.

What is Brainfuck? 
---------------------------------------------------------------------------------------------------------------------
[Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) is an extremely minimalistic, yet Turing-complete, programming
language (so-called _esoteric_ programming language).  The main idea is to manipulate an array of **cells**, using
simple commands. A cell is one byte, and the array consists of (at least) 30,000 cells.

On start up, each cell is initialised to zero and a **cell pointer** is set to the first (leftmost) cell. The program
is then executed sequentially, command by command, until the last command. Each command consists of one character,
and there are eight possible commands for moving the cell pointer, increasing/decreasing the current cell value,
I/O of the current cell and a simple loop structure. Non-command characters are just ignored.


| Command | Action                                                                             | Equivalent C code   |
|---------|------------------------------------------------------------------------------------|---------------------|
|   `>`   | Increment the cell pointer (move it to the right)                                  | `++ptr;`            |
|   `<`   | Decrement the cell pointer (move it to the left)                                   | `--ptr;`            |
|   `+`   | Increment the value of the current cell                                            | `++*ptr;`           |
|   `-`   | Decrement the value of the current cell                                            | `--*ptr;`           |
|   `.`   | Write the value of the current cell to screen                                      | `putchar(*ptr);`    |
|   `,`   | Read one byte of input and store the value in the current cell                     | `*ptr = getchar();` |
|   `[`   | If the cell value is zero, jump to the command following the matching `]` command  | `while (*ptr) {`    |
|   `]`   | If the cell value is **not** zero, jump back to the command after the matching `[` | `}`                 |


### Example program ###
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
 |   0 |   0 |   0 |   0 |...|   0 |         Initial state
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
 |   1 |   4 |   0 |   0 |...|   0 |    ]    Current cell is greater than 0, jump to loop start
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
 |   0 |   5 |   0 |   0 |...|   0 |    ]    Current cell is 0, break out of loop
 +-----+-----+-----+-----+~~~+-----+
    ^
   ptr
```


The compiler 
---------------------------------------------------------------------------------------------------------------------
The compiler consists of three parts, namely the **parser**, the **compiler** and the **Mach-O builder**.

The responsibilities of the parser is to first tokenise the file. 

tokeniser
parser
compiler
macho-builder


### Brainfuck to x86-64 assembly ###
TODO al, dx, rbp etc
TODO show table of mapping

#### Optimisations ####

addl
reduce load stores
reduce push and pops
skipping comment loops

### Creating a valid Mach-O executable ###
TODO: header + load commands + file offset
TODO: stricter


Executable image
---------------------------------------------------------------------------------------------------------------------
A compiled Brainfuck program will have the following layout when loaded into memory. The `__PAGEZERO` segment is used 
to catch null pointer exceptions; for our cause it's not really necessary, but as OS X has become stricter when 
evaluating Mach-O executables, it is expected. The protection level for this segment is set to no access. 

The `__DATA` segment and the `__data` section is empty on disk, but the load command for the section instructs the 
loader to reserve 2^16-1 bytes of memory for program data and to zero it out. There are two reasons for using a 
2^16-1 sized cell array (rather than "just" the 30,000). Reason one is that it is easier to align, as it is 
page-aligned. Reason two is that even though Brainfuck programs should not expect the array to be larger than 30,000
cells, there are many that ignore this. In order to ensure that most Brainfuck programs would compile and run, 
chose a size that was at least twice the minimum amount of cells.

The `__TEXT` segment and the corresponding `__text` section contains the actual opcodes that is ran. There is no 
restrictions on how large this section can be, except that my implementation uses JUMP opcodes that accept a four 
byte operand which means that it cannot be larger than 4 GB.

```
                 MEMORY  LAYOUT    
0x0000000000 +---------------------+
             |     __PAGEZERO      |
             |                     |
             |                     |  <-- 4 GB with inaccessible memory
             |                     |
             |                     |
0x1000000000 +---------------------+
             |    __DATA __data    |  <-- 2^16-1 bytes of zero filled data
0x1000010000 +---------------------+
             |    __TEXT __text    |
             |                     |
             |                     |  <-- The compiled code (max 4 GB)
             |                     |
             |                     |
             +---------------------+
```




- exit code
- no change on eof
- no dynamic bounds
- number of cells
- cell width

