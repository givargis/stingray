/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_json.h
 */

#ifndef _S_JSON_H_
#define _S_JSON_H_

#include "../kernel/s_kernel.h"

struct s__json_node {
	enum s__json_node_op {
		S__JSON_NODE_OP_,
		S__JSON_NODE_OP_NULL,
		S__JSON_NODE_OP_BOOL,
		S__JSON_NODE_OP_ARRAY,
		S__JSON_NODE_OP_OBJECT,
		S__JSON_NODE_OP_NUMBER,
		S__JSON_NODE_OP_STRING
	} op;
	union {
		int bool;
		double number;
		const char *string;
		struct s__json_node_array {
			struct s__json_node *node;
			struct s__json_node *link;
		} array;
		struct s__json_node_object {
			const char *key;
			struct s__json_node *node;
			struct s__json_node *link;
		} object;
	} u;
};

typedef struct s__json *s__json_t;

s__json_t s__json_open(const char *s);

void s__json_close(s__json_t json);

const struct s__json_node *s__json_root(s__json_t json);

int s__json_bist(void);

#endif /* _S_JSON_H_ */
