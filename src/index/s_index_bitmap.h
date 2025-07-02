/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_bitmap.h
 */

#ifndef _S_INDEX_BITMAP_H_
#define _S_INDEX_BITMAP_H_

#include "../utils/s_utils.h"

typedef struct s__index_bitmap *s__index_bitmap_t;

s__index_bitmap_t s__index_bitmap_open(uint64_t size);

void s__index_bitmap_close(s__index_bitmap_t bitmap);

void s__index_bitmap_prepare(s__index_bitmap_t bitmap);

uint64_t s__index_bitmap_rank(s__index_bitmap_t bitmap, uint64_t i);

void s__index_bitmap_set(s__index_bitmap_t bitmap, uint64_t i);

void s__index_bitmap_clr(s__index_bitmap_t bitmap, uint64_t i);

int s__index_bitmap_get(s__index_bitmap_t bitmap, uint64_t i);

#endif /* _S_INDEX_BITMAP_H_ */
