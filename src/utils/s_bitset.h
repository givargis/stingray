/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_bitset.h
 */

#ifndef _S_BITSET_H_
#define _S_BITSET_H_

#include "../kernel/s_kernel.h"

typedef struct s__bitset *s__bitset_t;

s__bitset_t s__bitset_open(uint64_t capacity);

void s__bitset_close(s__bitset_t bitset);

uint64_t s__bitset_reserve(s__bitset_t bitset, uint64_t n);

uint64_t s__bitset_release(s__bitset_t bitset, uint64_t i);

uint64_t s__bitset_validate(s__bitset_t bitset, uint64_t i);

uint64_t s__bitset_utilized(s__bitset_t bitset);

uint64_t s__bitset_capacity(s__bitset_t bitset);

int s__bitset_bist(void);

#endif /* _S_BITSET_H_ */
