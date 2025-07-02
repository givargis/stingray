/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_succinct.h
 */

#ifndef _S_INDEX_SUCCINCT_H_
#define _S_INDEX_SUCCINCT_H_

#include "s_index_ternary.h"

typedef struct s__index_succinct *s__index_succinct_t;

s__index_succinct_t s__index_succinct_open(s__index_ternary_t ternary);

void s__index_succinct_close(s__index_succinct_t succinct);

uint64_t *s__index_succinct_find(s__index_succinct_t succinct,
				 const char *key);

uint64_t *s__index_succinct_next(s__index_succinct_t succinct,
				 const char *key,
				 char *okey);

uint64_t *s__index_succinct_prev(s__index_succinct_t succinct,
				 const char *key,
				 char *okey);

uint64_t s__index_succinct_items(s__index_succinct_t succinct);

#endif /* _S_INDEX_SUCCINCT_H_ */
