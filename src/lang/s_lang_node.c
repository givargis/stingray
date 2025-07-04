/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_node.c
 */

#include "s_lang_node.h"

struct s__lang_node_ {
	void *chunk;
	uint64_t size;
};

s__lang_node_t
s__lang_node_open(void)
{
	struct s__lang_node_ *node;

	if (!(node = s__malloc(sizeof (struct s__lang_node_)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(node, 0, sizeof (struct s__lang_node_));
	return node;
}

void
s__lang_node_close(s__lang_node_t node)
{
	void *chunk;

	if (node) {
		while ((chunk = node->chunk)) {
			node->chunk = (*((void **)chunk));
			S__FREE(chunk);
		}
		memset(node, 0, sizeof (struct s__lang_node_));
	}
	S__FREE(node);
}

struct s__lang_node *
s__lang_node_allocate(s__lang_node_t node_)
{
	const uint64_t CHUNK_SIZE = 8192;
	struct s__lang_node *node;
	uint64_t size;
	void *chunk;

	assert( node_ );

	size = sizeof (struct s__lang_node);
	if (!node_->chunk || (CHUNK_SIZE < (node_->size + size))) {
		if (!(chunk = s__malloc(CHUNK_SIZE))) {
			S__TRACE(0);
			return NULL;
		}
		(*((void **)chunk)) = node_->chunk; /* link */
		node_->size = sizeof (struct s__lang_node *);
		node_->chunk = chunk;
	}
	node = (struct s__lang_node *)((char *)node_->chunk + node_->size);
	node_->size += size;
	memset(node, 0, size);
	return node;
}

const char * const S__LANG_NODE_STR[] = {
	"",
	"EXPR_",
	"EXPR_LIST",
	"EXPR_LITERAL",
	"EXPR_VARIABLE",
	"EXPR_FUNCTION",
	"EXPR_NEG",
	"EXPR_NOT",
	"EXPR_LOGIC_NOT",
	"EXPR_MUL",
	"EXPR_DIV",
	"EXPR_MOD",
	"EXPR_ADD",
	"EXPR_SUB",
	"EXPR_SHL",
	"EXPR_SHR",
	"EXPR_LT",
	"EXPR_GT",
	"EXPR_LE",
	"EXPR_GE",
	"EXPR_EQ",
	"EXPR_NE",
	"EXPR_AND",
	"EXPR_XOR",
	"EXPR_OR",
	"EXPR_LOGIC_AND",
	"EXPR_LOGIC_OR",
	"EXPR_COND",
	/*-*/
	"END"
};
