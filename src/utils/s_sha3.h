/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_sha3.h
 */

#ifndef _S_SHA3_H_
#define _S_SHA3_H_

#include "../kernel/s_kernel.h"

#define S__SHA3_LEN 32

void s__sha3(const void *buf, uint64_t len, void *out);

int s__sha3_bist(void);

#endif /* _S_SHA3_H_ */
