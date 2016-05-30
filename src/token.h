#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdint.h>

/* Syntactic symbols (commands) of Brainfuck */
enum symbol
{
   INCR_CELL    = '>',  // increment cell pointer
   DECR_CELL    = '<',  // decrement cell pointer
   INCR_DATA    = '+',  // increment cell value
   DECR_DATA    = '-',  // decrement cell value
   LOOP_BEGIN   = '[',  // begin loop ( while cell value > 0 )
   LOOP_END     = ']',  // end loop
   WRITE_DATA   = '.',  // print cell value to stdout
   READ_DATA    = ',',  // read cell value from stdin
};


/* Structure to hold tokens in memory 
 *
 * This structure is like a super-class in OOP terminology,
 * all sub-classes of this will have the members {symbol, next, size}
 */
struct token
{
    enum symbol    symbol; // which symbol this token represents
    struct token*  next;   // pointer to the following symbol
    size_t         size;   // total size of the token structure
};


/* Modify cell value token
 *
 * Sub-class of struct token. Used to represent the '+' and '-' commands.
 */
struct modify_data
{
    enum symbol    symbol; // either '+' or '-'
    struct token*  next;   // pointer to succeeding token
    size_t         size;   // sizeof(struct modify_data)
    int            load;   // indicates that a memory load must preceed the data modification
    int            store;  // indicates that a memory store must succeed the data modification
};


/* Loop token
 *
 * Sub-class of struct token. Used to represent the '[' command.
 */
struct loop
{
    enum symbol     symbol; // '['
    struct token*   next;   // pointer to succeeding token
    size_t          size;   // sizeof(struct loop)
    struct token*   match;  // pointer to the matching ']' token
};

#endif
