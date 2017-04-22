#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
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
    long page_size;

    // TODO: Rewrite main to support options and stopping at different stages
    
    page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0)
    {
        fprintf(stderr, "Failed to get system page size\n");
        return 2;
    }
    
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

    status = tokenize_file(stream, &token_string);
    if (status < 0)
    {
        fclose(stream);
        fprintf(stderr, "Invalid source file\n");
        return -status;
    }
    fclose(stream);

    status = parse(token_string);
    if (status < 0)
    {
        fprintf(stderr, "Syntax error\n");
        return -status;
    }

    // Compile tokens to bytecode
    status = compile(token_string, &page_list, page_size, DATA_ADDR);
    if (status < 0)
    {
        free_page_list(page_list);
        free_token_string(token_string);
        fprintf(stderr, "Failed to compile to byte code\n");
        return -status;
    }

    // Open output file for writing Mach-O executable
    if ((stream = fopen(argv[2], "w")) == NULL)
    {
        fprintf(stderr, "Could not open file for write: %s\n", argv[2]);
        return errno;
    }

    // Write Mach-O executable
    status = write_executable(stream, page_list, page_size, DATA_ADDR, TEXT_ADDR);
    if (status < 0)
    {
        fclose(stream);
        free_page_list(page_list);
        free_token_string(token_string);
        fprintf(stderr, "Could not write to executable\n");
        return -status;
    }

    fclose(stream);
    
    return 0;
}
