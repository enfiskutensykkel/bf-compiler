#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
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


static struct page* add_to_page(struct page* page, size_t page_size, size_t length, const char* code)
{
    struct page* curr_page = page;

    while (length > 0 && curr_page != NULL)
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


static uint32_t calculate_loop_offset(const struct token* token, uint32_t offset, size_t stack_pos)
{
    switch (token->symbol)
    {
        case INCR_CELL:
        case DECR_CELL:
            offset += 2;
            break;

        case INCR_DATA:
        case DECR_DATA:
            offset += 10;
            break;

        case WRITE_DATA:
            offset += 32;
            break;

        case READ_DATA:
            offset += 32;
            break;

        case LOOP_BEGIN:
            ++stack_pos;
            offset += 6 + 2 + 4;
            break;

        case LOOP_END:
            offset += 1 + 4; 
            if (--stack_pos == 0)
            {
                return offset;
            }
            break;
    }

    // Don't check for NULL here because the parser should have already matched all loops
    return calculate_loop_offset(token->next, offset, stack_pos);
}


int compile(const struct token* token_string, struct page** page_list, size_t page_size, uint64_t data_addr, uint64_t text_addr)
{
    struct page* curr_page = alloc_page(NULL, page_size);
    *page_list = curr_page;

    char byte_code[16];
    uint32_t addr = 0;
    uint32_t offset = 0;

    uint32_t addr_stack[MAX_NESTED_LOOPS];
    size_t stack_pos = 0;

    /* Save stack frame and point registers to data
     *
     *   al = working register
     *  rbx = data base address
     *  rdx = current cell
     *  rsi = store stuff temporarily
     *
     *  pushq 	%rbx
     *  pushq	%rbp
     *  pushq	%rsi
     *  pushq   %rdi
     *  movq	%rsp		    ,	%rbx
     *  xorq	%rax		    ,	%rax
     *  movq	<data address>  ,	%rbp
     *  movq	$0		        ,	%rdx
     */
    curr_page = add_to_page(curr_page, page_size, 12,"\x53\x55\x56\x57\x48\x89\xe3\x48\x31\xc0\x48\xbd");
    curr_page = add_to_page(curr_page, page_size, 8, (char*) &data_addr);
    curr_page = add_to_page(curr_page, page_size, 7, "\x48\xc7\xc2\x00\x00\x00\x00");
    data_addr += 12 + 8 + 7;

    while (token_string != NULL && curr_page != NULL)
    {
        switch (token_string->symbol)
        {
            case INCR_CELL:
                addr += 2;
                curr_page = add_to_page(curr_page, page_size, 2, "\xfe\xc2");
                break;

            case DECR_CELL:
                addr += 2;
                curr_page = add_to_page(curr_page, page_size, 2, "\xfe\xca");
                break;

            case INCR_DATA:
                addr += 10;
                curr_page = add_to_page(curr_page, page_size, 10, "\x8a\x44\x15\x00\xfe\xc0\x88\x44\x15\x00");
                break;

            case DECR_DATA:
                addr += 10;
                curr_page = add_to_page(curr_page, page_size, 10, "\x8a\x44\x15\x00\xfe\xc8\x88\x44\x15\x00");
                break;

            case LOOP_BEGIN:
                addr_stack[stack_pos++] = addr;
                addr += 12;

                offset = calculate_loop_offset(token_string, 0, 0) - 12;
                curr_page = add_to_page(curr_page, page_size, 6, "\x8a\x44\x15\x00\x3c\x00");

                byte_code[0] = 0x0f;
                byte_code[1] = 0x84;
                *((uint32_t*) (byte_code + 2)) = offset;

                curr_page = add_to_page(curr_page, page_size, 6, byte_code);
                break;

            case LOOP_END:
                offset = addr_stack[--stack_pos];

                byte_code[0] = 0xe9;
                *((uint32_t*) (byte_code + 1)) = offset - addr - 5;

                curr_page = add_to_page(curr_page, page_size, 5, byte_code);
                addr += 5;

                break;

            case WRITE_DATA:
                addr += 32;
                curr_page = add_to_page(curr_page, page_size, 32,
                        "\x48\xc7\xc0\x04\x00\x00\x02\x48\xc7\xc7\x01\x00\x00\x00\x48\x8d"
                        "\x74\x15\x00\x55\x52\x48\xc7\xc2\x01\x00\x00\x00\x0f\x05\x5a\x5d"
                        );
                break;

            case READ_DATA:
                addr += 32;
                curr_page = add_to_page(curr_page, page_size, 32,
                        "\x48\xc7\xc0\x03\x00\x00\x02\x48\xc7\xc7\x00\x00\x00\x00\x48\x8d"
                        "\x74\x15\x00\x55\x52\x48\xc7\xc2\x01\x00\x00\x00\x0f\x05\x5a\x5d"
                        );
                break;
        }

        token_string = token_string->next;
    }

    if (curr_page != NULL)
    {
        /* Extract return value from current cell and restore stack frame
         *
         *  movb	(%rbp, %rdx)	,	%al
         *  movq	%rbx		    ,	%rsp
         *  popq    %rdi
         *  popq	%rsi
         *  popq	%rbp
         *  popq	%rbx
         *  retq
         *
         */
        curr_page = add_to_page(curr_page, page_size, 12,
                "\x8a\x44\x15\x00\x48\x89\xdc\x5f\x5e\x5d\x5b\xc3");
    }
    else
    {
        return -ENOMEM;
    }

    return 0;
}
