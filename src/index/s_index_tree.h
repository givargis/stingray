/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_tree.h
 */

#ifndef _S_INDEX_TREE_H_
#define _S_INDEX_TREE_H_

#include "../utils/s_utils.h"

#define S__INDEX_TREE_MAX_KEY_LEN 32767 /* including '\0' */

typedef struct s__index_tree *s__index_tree_t;

typedef int (*s__index_tree_fnc_t)(void *ctx,
				   const char *key,
				   uint64_t record);

int s__index_tree_iterate(s__index_tree_t tree,
			  s__index_tree_fnc_t fnc,
			  void *ctx);

s__index_tree_t s__index_tree_open(void);

void s__index_tree_close(s__index_tree_t tree);

void s__index_tree_truncate(s__index_tree_t tree);

uint64_t *s__index_tree_update(s__index_tree_t tree, const char *key);

uint64_t *s__index_tree_find(s__index_tree_t tree, const char *key);

uint64_t *s__index_tree_next(s__index_tree_t tree,
			     const char *key,
			     char *okey);

uint64_t *s__index_tree_prev(s__index_tree_t tree,
			     const char *key,
			     char *okey);

uint64_t s__index_tree_items(s__index_tree_t tree);

#endif /* _S_INDEX_TREE_H_ */
