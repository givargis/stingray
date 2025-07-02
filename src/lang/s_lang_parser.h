/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_parser.h
 */

#ifndef _S_LANG_PARSER_H_
#define _S_LANG_PARSER_H_

#include "s_lang_node.h"
#include "s_lang_lexer.h"

typedef struct s__lang_parser *s__lang_parser_t;

s__lang_parser_t s__lang_parser_open(s__lang_lexer_t lexer);

void s__lang_parser_close(s__lang_parser_t parser);

struct s__lang_node *s__lang_parser_root(s__lang_parser_t parser);

const char *s__lang_parser_pathname(s__lang_parser_t parser);

#endif /* _S_LANG_PARSER_H_ */
