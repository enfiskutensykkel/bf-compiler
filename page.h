#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

struct page
{
    struct page*    next;
    size_t          size;
    unsigned char   data[1];
};


#endif
