/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_node.h
 */

#ifndef _S_LANG_NODE_H_
#define _S_LANG_NODE_H_

#include "s_lang_lexer.h"

enum {
	S__LANG_NODE_,
	S__LANG_NODE_EXPR_,
	S__LANG_NODE_EXPR_LIST,      /* cond, right */
	S__LANG_NODE_EXPR_LITERAL,   /* val.? */
	S__LANG_NODE_EXPR_VARIABLE,  /* val.s */
	S__LANG_NODE_EXPR_FUNCTION,  /* val.s, cond */
	S__LANG_NODE_EXPR_NEG,       /* - right */
	S__LANG_NODE_EXPR_NOT,       /* ~ right */
	S__LANG_NODE_EXPR_LOGIC_NOT, /* ! right */
	S__LANG_NODE_EXPR_MUL,       /* left *  right */
	S__LANG_NODE_EXPR_DIV,       /* left /  right */
	S__LANG_NODE_EXPR_MOD,       /* left %  right */
	S__LANG_NODE_EXPR_ADD,       /* left +  right */
	S__LANG_NODE_EXPR_SUB,       /* left -  right */
	S__LANG_NODE_EXPR_SHL,       /* left << right */
	S__LANG_NODE_EXPR_SHR,       /* left >> right */
	S__LANG_NODE_EXPR_LT,        /* left <  right */
	S__LANG_NODE_EXPR_GT,        /* left >  right */
	S__LANG_NODE_EXPR_LE,        /* left <= right */
	S__LANG_NODE_EXPR_GE,        /* left >= right */
	S__LANG_NODE_EXPR_EQ,        /* left == right */
	S__LANG_NODE_EXPR_NE,        /* left != right */
	S__LANG_NODE_EXPR_AND,       /* left &  right */
	S__LANG_NODE_EXPR_XOR,       /* left ^  right */
	S__LANG_NODE_EXPR_OR,        /* left |  right */
	S__LANG_NODE_EXPR_LOGIC_AND, /* left && right */
	S__LANG_NODE_EXPR_LOGIC_OR,  /* left || right */
	S__LANG_NODE_EXPR_COND,      /* cond ? left : right */
	/*-*/
	S__LANG_NODE_END
};

enum {
	S__LANG_NODE_TYPE_,
	S__LANG_NODE_TYPE_INT,
	S__LANG_NODE_TYPE_UINT,
	S__LANG_NODE_TYPE_REAL,
	S__LANG_NODE_TYPE_BOOL
};

struct s__lang_node {
	int id;
	int op;
	int type;
	struct s__lang_node *cond;
	struct s__lang_node *left;
	struct s__lang_node *right;
	const struct s__lang_lexer_token *token;
};

typedef struct s__lang_node_ *s__lang_node_t;

s__lang_node_t s__lang_node_open(void);

void s__lang_node_close(s__lang_node_t node);

struct s__lang_node *s__lang_node_allocate(s__lang_node_t node);

extern const char * const S__LANG_NODE_STR[];
extern const char * const S__LANG_NODE_TYPE_STR[];

#endif /* _S_LANG_NODE_H_ */
