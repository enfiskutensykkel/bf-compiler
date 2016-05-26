#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "token.h"
#include "compiler.h"


static struct page* alloc_page(struct page* prev, size_t page_size)
{
    struct page* curr = (struct page*) malloc(sizeof(struct page) - 1 + page_size);

    if (prev != NULL)
    {
        prev->next = curr;
    }

    if (curr != NULL)
    {
        curr->next = NULL;
        curr->size = 0;
    }

    return curr;
}


static struct page* add_to_page(struct page* page, size_t page_size, const char* code, size_t length)
{
    struct page* curr_page = page;

    while (length > 0)
    {
        if (curr_page->size == page_size)
        {
            curr_page = alloc_page(curr_page, page_size);
            if (curr_page == NULL)
            {
                fprintf(stderr, "Out of memory\n");
                return NULL;
            }
        }

        curr_page->data[curr_page->size++] = *code++;
        --length;
    }

    return curr_page;
}


int compile(const struct token* token_string, struct page** page_list, size_t page_size, uint64_t data_addr, uint64_t text_addr)
{
    struct page* curr_page = alloc_page(NULL, page_size);
    *page_list = curr_page;

    while (token_string != NULL && curr_page != NULL)
    {
        switch (token_string->symbol)
        {
            case INCR_CELL:
                curr_page = add_to_page(curr_page, page_size, "\xb8\x04\x00\x00\x00\xc3", 6);
                break;

            case DECR_CELL:
                break;

            case INCR_DATA:
                break;

            case DECR_DATA:
                break;

            case LOOP_BEGIN:
                break;

            case LOOP_END:
                break;

            case WRITE_DATA:
                break;

            case READ_DATA:
                break;
        }

        token_string = token_string->next;
    }

    if (curr_page == NULL)
    {
        return -ENOMEM;
    }

    return 0;
}
