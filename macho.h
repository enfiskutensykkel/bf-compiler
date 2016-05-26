#ifndef __MACHO_H__
#define __MACHO_H__

#include <stdio.h>
#include "page.h"

int write_executable(FILE* output_file, struct page* page_list, size_t page_size, uint64_t data_addr, uint64_t text_addr);

#endif
