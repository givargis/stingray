/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_avl.h
 */

#ifndef _S_AVL_H_
#define _S_AVL_H_

#include "../kernel/s_kernel.h"

typedef struct s__avl *s__avl_t;

typedef int (*s__avl_fnc_t)(void *ctx, const char *key, void *val);

s__avl_t s__avl_open(void);

void s__avl_close(s__avl_t avl);

void s__avl_empty(s__avl_t avl);

void s__avl_remove(s__avl_t avl, const char *key);

int s__avl_update(s__avl_t avl, const char *key, const void *val);

void *s__avl_lookup(s__avl_t avl, const char *key);

int s__avl_iterate(s__avl_t avl, s__avl_fnc_t fnc, void *ctx);

uint64_t s__avl_size(s__avl_t avl);

int s__avl_bist(void);

#endif /* _S_AVL_H_ */
