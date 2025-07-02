/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_ternary.h
 */

#ifndef _S_INDEX_TERNARY_H_
#define _S_INDEX_TERNARY_H_

#include "s_index_tree.h"

typedef void (*s__index_ternary_fnc_t)(void *ctx,
				       char key,
				       int left,
				       int center,
				       int right,
				       uint64_t *record);

typedef struct s__index_ternary *s__index_ternary_t;

s__index_ternary_t s__index_ternary_open(s__index_tree_t tree);

void s__index_ternary_close(s__index_ternary_t ternary);

int s__index_ternary_iterate(s__index_ternary_t ternary,
			     s__index_ternary_fnc_t fnc,
			     void *ctx);

uint64_t s__index_ternary_items(s__index_ternary_t ternary);

uint64_t s__index_ternary_nodes(s__index_ternary_t ternary);

#endif /* _S_INDEX_TERNARY_H_ */
