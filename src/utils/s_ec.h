/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_ec.h
 */

#ifndef _S_EC_H_
#define _S_EC_H_

#include "../kernel/s_kernel.h"

void s__ec_init(void);

void s__ec_encode_pq(void *buf, uint64_t k, uint64_t n);

void s__ec_encode_p(void *buf, uint64_t k, uint64_t n);

void s__ec_encode_q(void *buf, uint64_t k, uint64_t n);

void s__ec_encode_dp(void *buf, uint64_t k, uint64_t n, uint64_t x);

void s__ec_encode_dq(void *buf, uint64_t k, uint64_t n, uint64_t x);

void s__ec_encode_dd(void *buf,
		     uint64_t k,
		     uint64_t n,
		     uint64_t x,
		     uint64_t y);

int s__ec_bist(void);

#endif /* _S_EC_H_ */
