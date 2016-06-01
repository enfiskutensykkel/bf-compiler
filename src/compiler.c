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


static const struct token* find_chain_end(const struct token* current_token, uint32_t* offset, uint32_t max)
{
    const struct token* ptr = current_token;
    uint32_t count = 1;

    while (ptr->next != NULL && ptr->next->symbol == current_token->symbol)
    {
        ptr = ptr->next;
        if (++count == max)
        {
            break;
        }
    }
    
    if (offset != NULL)
    {
        *offset = count;
    }

    return ptr;
}


static uint32_t calculate_loop_offset(const struct token* begin, const struct token* end, uint32_t offset)
{
    const struct token* token_chain;

    switch (begin->symbol)
    {
        case INCR_CELL:
        case DECR_CELL:
            token_chain = find_chain_end(begin, NULL, 0x7f);
            offset += 4;
            begin = token_chain;
            break;

        case INCR_DATA:
        case DECR_DATA:
            token_chain = find_chain_end(begin, NULL, 0x7f);
            offset += 5;
            begin = token_chain;
            break;

        case WRITE_DATA:
            offset += 30;
            break;

        case READ_DATA:
            offset += 30;
            break;

        case LOOP_BEGIN:
            offset += 6 + 2 + 4;
            break;

        case LOOP_END:
            offset += 1 + 4; 
            break;
    }

    if (begin == end)
    {
        return offset;
    }

    // Don't check for NULL here because the parser should have already matched all loops
    return calculate_loop_offset(begin->next, end, offset);
}


int compile(const struct token* token_string, struct page** page_list, size_t page_size, uint64_t data_addr)
{
    struct page* curr_page = alloc_page(NULL, page_size);
    *page_list = curr_page;
    const struct token* last_token;

    char byte_code[8];
    uint32_t addr = 0;
    uint32_t offset = 0;

    /* Save stack frame and point registers to data
     *
     *   al = working register
     *  rbp = data base address
     *   dx = current cell offset
     *
     *  pushq 	%rbx
     *  pushq	%rbp
     *  pushq	%rsi
     *  pushq   %rdi
     *  movq	%rsp		    ,	%rbx
     *  xorq	%rax		    ,	%rax
     *  movq	<data address>  ,	%rbp
     *  xorq	%rdx		    ,	%rdx
     */
    curr_page = add_to_page(curr_page, page_size, 12,"\x53\x55\x56\x57\x48\x89\xe3\x48\x31\xc0\x48\xbd");
    curr_page = add_to_page(curr_page, page_size, 8, (char*) &data_addr);
    curr_page = add_to_page(curr_page, page_size, 3, "\x48\x31\xd2");

    while (token_string != NULL && curr_page != NULL)
    {
        switch (token_string->symbol)
        {
            case INCR_CELL:
                /*
                 *  addw    <chain count>   ,   %dx
                 */
                last_token = find_chain_end(token_string, &offset, 0x7f);

                byte_code[0] = 0x66;
                byte_code[1] = 0x83;
                byte_code[2] = 0xc2;
                byte_code[3] = (uint8_t) offset;

                curr_page = add_to_page(curr_page, page_size, 4, byte_code);

                addr += 4;
                token_string = last_token;
                break;

            case DECR_CELL:
                /*
                 *  subw    <chain count>   ,   %dx
                 */
                last_token = find_chain_end(token_string, &offset, 0x7f);

                byte_code[0] = 0x66;
                byte_code[1] = 0x83;
                byte_code[2] = 0xea;
                byte_code[3] = (uint8_t) offset;

                curr_page = add_to_page(curr_page, page_size, 4, byte_code);

                addr += 4;
                token_string = last_token;
                break;

            case INCR_DATA:
                /*
                 *  addb    <value>      ,   (%rbp, %rdx)
                 */
                last_token = find_chain_end(token_string, &offset, 0x7f);

                byte_code[0] = 0x80;
                byte_code[1] = 0x44;
                byte_code[2] = 0x15;
                byte_code[3] = 0x00;
                byte_code[4] = (uint8_t) offset;

                curr_page = add_to_page(curr_page, page_size, 5, byte_code);

                addr += 5;
                token_string = last_token;
                break;

            case DECR_DATA:
                /*
                 *  subb    <value>      ,   (%rbp, %rdx)
                 */
                last_token = find_chain_end(token_string, &offset, 0x7f);

                byte_code[0] = 0x80;
                byte_code[1] = 0x6c;
                byte_code[2] = 0x15;
                byte_code[3] = 0x00;
                byte_code[4] = (uint8_t) offset;

                curr_page = add_to_page(curr_page, page_size, 5, byte_code);

                addr += 5;
                token_string = last_token;
                break;

            case LOOP_BEGIN:
                /*
                 *  movb    (%rbp, %rdx) ,  %al
                 *  cmpb    $0           ,  %al
                 *  je      <calculated address>
                 *
                 */
                offset = calculate_loop_offset(token_string->next, ((const struct loop*) token_string)->match, 0);

                byte_code[0] = 0x0f;
                byte_code[1] = 0x84;
                *((uint32_t*) (byte_code + 2)) = offset;

                addr += 12;
                curr_page = add_to_page(curr_page, page_size, 6, "\x8a\x44\x15\x00\x3c\x00");
                curr_page = add_to_page(curr_page, page_size, 6, byte_code);
                break;

            case LOOP_END:
                /*
                 *  jump    <address of comparison>
                 */
                offset = calculate_loop_offset(((const struct loop*) token_string)->match->next, token_string, 0);

                byte_code[0] = 0xe9;
                *((uint32_t*) (byte_code + 1)) = -offset - 12;

                curr_page = add_to_page(curr_page, page_size, 5, byte_code);
                addr += 5;

                break;

            case WRITE_DATA:
                /*
                 *  movq    $0x2000004   ,  %rax    # 4 = syscall write, 2000000 = UNIX/BSD mask
                 *  movq    $1           ,  %rdi    # file number 1 = stdout
                 *  leaq    (%rbp, %rdx) ,  %rsi    # move address of cell we're going to print
                 *  pushq   %rdx                    # save register
                 *  movq    $1           ,  %rdx    # how many bytes
                 *  syscall
                 *  popq    %rdx
                 */
                addr += 30;
                curr_page = add_to_page(curr_page, page_size, 30,
                        "\x48\xc7\xc0\x04\x00\x00\x02\x48\xc7\xc7\x01\x00\x00\x00\x48\x8d"
                        "\x74\x15\x00\x52\x48\xc7\xc2\x01\x00\x00\x00\x0f\x05\x5a"
                        );
                break;

            case READ_DATA:
                /*
                 *  movq    $0x2000003   ,  %rax    # 3 = syscall read, 2000000 = UNIX/BSD mask
                 *  movq    $0           ,  %rdi    # file number 0 = stdin
                 *  leaq    (%rbp, %rdx) ,  %rsi    # move address of cell we're going to print
                 *  pushq   %rdx                    # save register
                 *  movq    $1           ,  %rdx    # how many bytes
                 *  syscall
                 *  popq    %rdx
                 */
                addr += 30;
                curr_page = add_to_page(curr_page, page_size, 30,
                        "\x48\xc7\xc0\x03\x00\x00\x02\x48\xc7\xc7\x00\x00\x00\x00\x48\x8d"
                        "\x74\x15\x00\x52\x48\xc7\xc2\x01\x00\x00\x00\x0f\x05\x5a"
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
