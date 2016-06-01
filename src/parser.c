#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "token.h"
#include "parser.h"


static struct token* create_token(enum symbol command, size_t size)
{
    struct token* token = (struct token*) malloc(size);

    if (token == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    memset(token, 0, size);
    token->symbol = command;
    token->next = NULL;
    token->size = size;

    return token;
}


static struct token* get_next_token(FILE* stream)
{
    int byte;

    while ((byte = fgetc(stream)) != -1)
    {
        switch (byte)
        {
            case LOOP_BEGIN:
            case LOOP_END:
                return create_token((enum symbol) byte, sizeof(struct loop));

            case INCR_DATA:
            case DECR_DATA:
            case INCR_CELL:
            case DECR_CELL:
            case WRITE_DATA:
            case READ_DATA:
                return create_token((enum symbol) byte, sizeof(struct token));

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
    }

    if (prev_token == NULL)
    {
        fprintf(stderr, "No tokens found\n");
        return -1;
    }

    return 0;
}


static struct token* find_loop_end(struct token* token, size_t loop_stack)
{
    while (token != NULL)
    {
        switch (token->symbol)
        {
            case LOOP_BEGIN:
                ++loop_stack;
                break;

            case LOOP_END:
                if (--loop_stack == 0)
                {
                    return token;
                }
                break;

            default:
                // do nothing
                break;
        }

        token = token->next;
    }

    return NULL;
}


int parse(struct token* token_string)
{
    struct token* curr_token = token_string;
    int64_t nest_count = 0;
    
    while (curr_token != NULL)
    {
        switch (curr_token->symbol)
        {
            case LOOP_BEGIN:
                if (++nest_count >= INT64_MAX)
                {
                    fprintf(stderr, "Excessive loop nesting\n");
                    return -1;
                }

                ((struct loop*) curr_token)->match = find_loop_end(curr_token, 0);

                if (((struct loop*) curr_token)->match == NULL)
                {
                    fprintf(stderr, "Matching ']' not found, searched past end of file\n");
                    return -2;
                }

                ((struct loop*) ((struct loop*) curr_token)->match)->match = curr_token;
                break;

            case LOOP_END:
                if (--nest_count < 0 || ((struct loop*) curr_token)->match == NULL)
                {
                    fprintf(stderr, "Rogue ']'\n");
                    return -3;
                }
                break;

            default:
                break;
        }

        curr_token = curr_token->next;
    }

    return 0;
}
