#ifndef __TOKEN_H__
#define __TOKEN_H__

/* Syntactic symbols of BF */
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


/* Structure to hold tokens in memory */
struct token
{
    enum symbol     symbol;
    struct token*   next;
};

#endif
