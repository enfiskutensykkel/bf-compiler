#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "parser.h"
#include "compiler.h"
#include "macho.h"


static void free_token_string(struct token* token_string)
{
    while (token_string != NULL)
    {
        struct token* next = token_string->next;
        free(token_string);
        token_string = next;
    }
}


static void free_page_list(struct page* page_list)
{
    while (page_list != NULL)
    {
        struct page* next = page_list->next;
        free(page_list);
        page_list = next;
    }
}


int main(int argc, char** argv)
{
    int status;
    struct token* token_string = NULL;
    struct page* page_list = NULL;

    // TODO: Rewrite main to support options and stopping at different stages
    
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source file> <executable>\n", argv[0]);
        return 1;
    }

    FILE* stream;

    // Read input file and create token string
    if ((stream = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Could not open file for read: %s\n", argv[1]);
        return errno;
    }
    size_t num_tokens = tokenize_file(stream, &token_string); // TODO rewrite so it returns status instead
    fclose(stream);

    // Check if there's actually anything to compile
    if (num_tokens == 0)
    {
        fprintf(stderr, "Empty source file: %s\n", argv[1]);
        return 3;
    }

    // Compile tokens to bytecode
    status = compile(token_string, &page_list, PAGE_SIZE, DATA_ADDR, TEXT_ADDR);
    if (status < 0)
    {
        free_page_list(page_list);
        free_token_string(token_string);
        fprintf(stderr, "Failed to compile to byte code\n");
        return status;
    }

    // Open output file for writing Mach-O executable
    if ((stream = fopen(argv[2], "w")) == NULL)
    {
        fprintf(stderr, "Could not open file for write: %s\n", argv[2]);
        return errno;
    }

    // Write Mach-O executable
    status = write_executable(stream, page_list, PAGE_SIZE, DATA_ADDR, TEXT_ADDR);
    if (status < 0)
    {
        fclose(stream);
        free_page_list(page_list);
        free_token_string(token_string);
        fprintf(stderr, "Could not write to executable\n");
        return status;
    }

    fclose(stream);
    
    return 0;
}
