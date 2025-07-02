/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_int256.h
 */

#ifndef _S_INT256_H_
#define _S_INT256_H_

#include "s_uint256.h"

#define S__INT256_STR_LEN S__UINT256_STR_LEN
#define S__INT256_SET     S__UINT256_SET
#define s__int256         s__uint256
#define s__int256_init    s__uint256_init
#define s__int256_hex     s__uint256_hex
#define s__int256_is_zero s__uint256_is_zero
#define s__int256_shl_    s__uint256_shl_
#define s__int256_shl     s__uint256_shl
#define s__int256_set_bit s__uint256_set_bit
#define s__int256_clr_bit s__uint256_clr_bit
#define s__int256_get_bit s__uint256_get_bit
#define s__int256_not     s__uint256_not
#define s__int256_and     s__uint256_and
#define s__int256_xor     s__uint256_xor
#define s__int256_or      s__uint256_or

void s__int256_dec(const struct s__int256 *a, char *s);

int s__int256_cmp(const struct s__int256 *a_, const struct s__int256 *b);

int s__int256_neg(struct s__int256 *z, const struct s__int256 *a);

int s__int256_add(struct s__int256 *z,
		  const struct s__int256 *a,
		  const struct s__int256 *b);

int s__int256_sub(struct s__int256 *z,
		  const struct s__int256 *a,
		  const struct s__int256 *b);

int s__int256_mul(struct s__int256 *z,
		  const struct s__int256 *a,
		  const struct s__int256 *b);

int s__int256_div(struct s__int256 *z,
		  const struct s__int256 *a,
		  const struct s__int256 *b);

int s__int256_mod(struct s__int256 *z,
		  const struct s__int256 *a,
		  const struct s__int256 *b);

void s__int256_shr_(struct s__int256 *z, const struct s__int256 *a, int n);

int s__int256_bist(void);

S__INLINE int
s__int256_is_neg(const struct s__int256 *a)
{
	assert( a );

	return (a->hh & 0x8000000000000000) ? 1 : 0;
}

S__INLINE void
s__int256_shr(struct s__int256 *z,
	      const struct s__int256 *a,
	      const struct s__int256 *b)
{
	assert( z && a && b );

	s__int256_shr_(z, a, b->ll % 256);
}

#endif /* _S_INT256_H_ */
