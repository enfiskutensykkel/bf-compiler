Brainfuck compiler for Mac OS X
=====================================================================================================================
Compile Brainfuck programs to x86-64 Mach-O executables for Mac OS X 10.11 and newer.


### Technical Requirements ###
I personally use the following:
- LLVM version 7.0.2 (clang-700.1.81), which is Command Line Tools version 7.0.3
- Mac OS X 10.11.4 (x86\_64-apple-darwin15.5.0)
- BSD Make

My guess is that it will work with older versions too, but you might have to tweak the version number for 
`/usr/lib/libSystem.B.dylib` in the Mach-O output. It will only work for OS X 10.9 and newer because of how Apple 
has deprecated the old `LC_UNIXTHREAD` load command and replaced it with the new `LC_MAIN` dynamic loader stuff.

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


### Example Program ###
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


Compiler Overview
---------------------------------------------------------------------------------------------------------------------
The compiler consists of three parts, namely the **parser**, the **compiler** and the **Mach-O builder**. The program
starts 

The responsibilities of the parser is to first tokenise the file. This is done by reading the source file character
by character and whenever a valid _token_ -- that is, a valid Brainfuck command character -- is encountered, it is
inserted into a linked list. After all the tokens are read, the list of tokens (which I call _token string_ in my
implementation) is passed to the parse function which does the following:
  - Match `[` and `]` together (and make sure that all of them matches, otherwise it is a syntax error).
  - Chain succeeding `+`, `-`, `<` and `>`commands and count them
  - Figure out where it is necessary to do memory stores and loads

After the parser has done its magic to the token string, it is passed to the _compiler_. The compiler is responsible
for converting the parsed token string into x86-64 machine code and writing it to page-sized buffers, allocated on 
demand. I use buffers that are aligned with pages because it is simple.

The buffers are then finally passed to the Macho-O builder, which essentially writes the Mach-O header to file along
with the necessary load commands and finally the compiled code.

### Brainfuck to x86-64 assembly ###
Translating Brainfuck to simple assembly is trivial. The following registers is used for the purposes listed:

| Register | Width   | Meaning                                                                               |
|----------|---------|---------------------------------------------------------------------------------------|
|     `al` | 1 byte  | Working register, used to increment and decrement cell value                          |
|    `rbp` | 8 bytes | Store the address of the beginning of the data segment (0x1000000000)                 |
|     `dx` | 2 bytes | Represents the current cell pointer. The current cell is retrieved with (`rbp`+`rdx`) |
|    `rbx` | 8 bytes | Holds the base stack pointer, because of calling convention (aka _not used_)          |
|    `rsi` | 8 bytes | Used for `write()` and `read()` syscalls                                              |
|    `rdi` | 8 bytes | Used for `write()` and `read()` syscalls                                              |

Then translating Brainfuck commands into assembly is just a matter of mapping commands to opcodes and operands.

| Command | GNU Assembly (AT&T)                                                                    |
|---------|----------------------------------------------------------------------------------------|
|   `<`   | `decw %dx`                                                                             |
|   `>`   | `incw %dx`                                                                             |
|   `+`   | `movb (%rbp, %rdx), %al`; `incb %al`; `movb %al, (%rbp, %rdx)`                         |
|   `-`   | `movb (%rbp, %rdx), %al`; `decb %al`; `movb %al, (%rbp, %rdx)`                         |
|   `[`   | `movb (%rbp, %rdx), %al`; `cmpb $0, %al`; `je <4-byte offset>`                         |
|   `]`   | `jmp <4-byte offset>`                                                                  |
|   `.`   | `movq $4, %rax`; `movq $1, %rdi`; `leaq (%rbp,%rdx), %rsi`; `pushq %rdx`; `movq $1, %rdx`; `syscall`; `popq %rdx` |
|   `,`   | `movq $3, %rax`; `movq $0, %rdi`; `leaq (%rbp,%rdx), %rsi`; `pushq %rdx`; `movq $1, %rdx`; `syscall`; `popq %rdx` |


#### Optimisations ####
I am currently in the process of attempting to make some optimisations in order to reduce the size of the
executable. Currently, I have chained together `+` and `-` in order to reduce number of loads and stores.
My next goal is to chain together succeeding increments and decrements and use `addq` and `subq` instead of
repeating `inc` and `dec`. I will also look into the possibility for skipping so-called _comment loops_ by 
checking if a cell _has_ to be zero.


### Creating a valid Mach-O executable ###
I had a hard time figuring out why my initial attempts at creating a valid Mach-O file did not work. The command 
line tools are extremely handy, but they do only minimal evaluation. When I first started, I was not familiar with the
Mach-O format, only with ELF. I started out by making a Mach-O parser and reading [this link](http://lowlevelbits.org/parse-mach-o-files/) about it. Basically, it is a very simple format with a 
**Mach header**, a series of variable sized **load commands** and then followed by data.

![Mach-O memory layout](http://lowlevelbits.org/images/parse-mach-o-files/macho_memory_layout.png)

Reading Mach-O files was fairly simple. Creating them, on the other hand, was not. I came across a blogpost about
making [minimal 64-bit executable](http://blog.softboysxp.com/post/7688131432/a-minimal-mach-o-x64-executable-for-os-x),
but I could not make it to work. After digging for a long time, I finally came across a [Stack overflow](http://stackoverflow.com/questions/32453849/minimal-mach-o-64-binary) post saying that the evaluation of
Mach-O files had become stricter because of some security issue with iOS.

This made me create a minimal C program instead, compile and link it and then use `otool` and my own Mach-O reader
to just copy load commands one by one until it worked. What I found out was that the following load commands are 
necessary for creating a valid executable:
  - `__PAGEZERO` segment to handle null pointer exceptions (I do not know why it is necessary, just that it is)
  - `__TEXT` segment (with corresponding `__text` segment)
  - an `LC_MAIN` command, telling the the dynamic loader (dyld) where in the file to look for `main()`
  - an `LC_LOAD_DYLINKER` command, telling the kernel which dynamic loader to use (in my case, `/usr/lib/dyld`)
  - an `LC_LOAD_DYLIB` command indicating where dyld can find libc (I guess it is necessary for some reason?)
  - an `LC_DYLD_INFO_ONLY` command, with all fields set to zero, so that dyld does not try to do anything "clever"


Executable Image Output
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

