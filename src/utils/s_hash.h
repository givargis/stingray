/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_hash.h
 */

#ifndef _S_HASH_H_
#define _S_HASH_H_

#include "../kernel/s_kernel.h"

uint64_t s__hash(const void *buf, uint64_t len);

int s__hash_bist(void);

#endif /* _S_HASH_H_ */
