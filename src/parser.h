#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include "token.h"


/* Pass through the input file and create a string of tokens */
int tokenize_file(FILE* input_file, struct token** token_string);


/* Pass through the string of tokens and build the parse tree */
int parse(struct token* token_string);

#endif
