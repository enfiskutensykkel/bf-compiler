#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdint.h>
#include "page.h"
#include "token.h"


/* Translate the parse tree to x86-64 byte code for UNIX/BSD/Mac OS X */
int compile(const struct token* token_string, struct page** page_list, size_t page_size, uint64_t data_addr);

#endif
