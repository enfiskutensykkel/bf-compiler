#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdint.h>
#include "page.h"
#include "token.h"

int compile(const struct token* token_string, struct page** page_list, size_t page_size, uint64_t data_addr, uint64_t text_addr);

#endif
