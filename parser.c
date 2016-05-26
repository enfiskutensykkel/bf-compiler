#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "parser.h"


static struct token* create_token(enum symbol symbol)
{
    struct token* token = (struct token*) malloc(sizeof(struct token));
    
    if (token != NULL)
    {
        token->symbol = symbol;
        token->next = NULL;
    }

    return token;
}


static struct token* get_next_token(FILE* stream)
{
    int byte;
    struct token* token;

    while ((byte = fgetc(stream)) != -1)
    {
        switch (byte)
        {
            case INCR_CELL:
            case DECR_CELL:
            case INCR_DATA:
            case DECR_DATA:
            case LOOP_BEGIN:
            case LOOP_END:
            case WRITE_DATA:
            case READ_DATA:
                token = create_token((enum symbol) byte);
                if (token == NULL)
                {
                    fprintf(stderr, "Out of memory\n");
                    return NULL;
                }
                return token;

            default:
                // do nothing
                break;
        }
    }

    return NULL;
}


int tokenize_file(FILE* input_file, struct token** token_string)
{
    struct token* prev_token = NULL;
    struct token* curr_token = NULL;
    size_t num_tokens = 0;

    int loop_stack = 0;

    while ((curr_token = get_next_token(input_file)) != NULL)
    {
        if (prev_token == NULL)
        {
            *token_string = prev_token = curr_token;
        }
        else
        {
            prev_token->next = curr_token;
            prev_token = curr_token;
        }

        switch (curr_token->symbol)
        {
            case LOOP_BEGIN:
                if (++loop_stack >= MAX_NESTED_LOOPS)
                {
                    fprintf(stderr, "Excessive loop nesting\n");
                    return -1;
                }
                break;

            case LOOP_END:
                if (--loop_stack < 0)
                {
                    fprintf(stderr, "Loop token mismatch (open)\n");
                    return -2;
                }
                break;

            default:
                break;
        }

        ++num_tokens;
    }

    if (loop_stack != 0)
    {
        fprintf(stderr, "Loop token mismatch (close)\n");
        return -2;
    }
    else if (num_tokens == 0)
    {
        fprintf(stderr, "No tokens found\n");
        return -3;
    }

    return 0;
}
