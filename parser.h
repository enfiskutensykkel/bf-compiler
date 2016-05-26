#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include "token.h"


/* Pass through the input file and create a string of tokens */
size_t tokenize_file(FILE* input_file, struct token** token_string);

#endif
