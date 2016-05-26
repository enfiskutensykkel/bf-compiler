#ifndef __TOKEN_H__
#define __TOKEN_H__

/* Syntactic symbols of BF */
enum symbol
{
   INCR_CELL    = '>', 
   DECR_CELL    = '<',
   INCR_DATA    = '+',
   DECR_DATA    = '-',
   LOOP_BEGIN   = '[',
   LOOP_END     = ']',
   WRITE_DATA   = '.',
   READ_DATA    = ',',
};


/* Structure to hold tokens in memory */
struct token
{
    enum symbol     symbol;
    struct token*   next;
};

#endif
