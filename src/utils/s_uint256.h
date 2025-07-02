/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_uint256.h
 */

#ifndef _S_UINT256_H_
#define _S_UINT256_H_

#include "../kernel/s_kernel.h"

#define S__UINT256_STR_LEN 81

#define S__UINT256_SET(z,v)				\
	do {						\
		memset(z,				\
		       0,				\
		       sizeof (struct s__uint256));	\
		(z)->ll = (uint64_t)(v);		\
	}						\
	while (0)

struct s__uint256 {
	uint64_t hh;
	uint64_t hl;
	uint64_t lh;
	uint64_t ll;
};

int s__uint256_init(struct s__uint256 *z, const char *s);

void s__uint256_hex(const struct s__uint256 *a, char *s);

void s__uint256_dec(const struct s__uint256 *a, char *s);

void s__uint256_add_(struct s__uint256 *z,
		     const struct s__uint256 *a,
		     const struct s__uint256 *b);

int s__uint256_add(struct s__uint256 *z,
		   const struct s__uint256 *a,
		   const struct s__uint256 *b);

int s__uint256_sub(struct s__uint256 *z,
		   const struct s__uint256 *a,
		   const struct s__uint256 *b);

void s__uint256_mulf(struct s__uint256 *zh,
		     struct s__uint256 *zl,
		     const struct s__uint256 *a,
		     const struct s__uint256 *b);

int s__uint256_mul(struct s__uint256 *z,
		   const struct s__uint256 *a,
		   const struct s__uint256 *b);

int s__uint256_divmod(struct s__uint256 *zq,
		      struct s__uint256 *zr,
		      const struct s__uint256 *a,
		      const struct s__uint256 *b);

void s__uint256_shl_(struct s__uint256 *z,
		     const struct s__uint256 *a,
		     int n);

void s__uint256_shr_(struct s__uint256 *z,
		     const struct s__uint256 *a,
		     int n);

int s__uint256_bist(void);

S__INLINE int
s__uint256_is_zero(const struct s__uint256 *a)
{
	assert( a );

	return s__is_zero(a, sizeof (struct s__uint256));
}

S__INLINE int
s__uint256_cmp(const struct s__uint256 *a, const struct s__uint256 *b)
{
	assert( a && b );

	if (a->hh > b->hh) return +1;
	if (a->hh < b->hh) return -1;
	if (a->hl > b->hl) return +1;
	if (a->hl < b->hl) return -1;
	if (a->lh > b->lh) return +1;
	if (a->lh < b->lh) return -1;
	if (a->ll > b->ll) return +1;
	if (a->ll < b->ll) return -1;
	return 0;
}

S__INLINE int
s__uint256_div(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	struct s__uint256 r;

	assert( z && a && b );

	return s__uint256_divmod(z, &r, a, b);
}

S__INLINE int
s__uint256_mod(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	struct s__uint256 q;

	assert( z && a && b );

	return s__uint256_divmod(&q, z, a, b);
}

S__INLINE void
s__uint256_shl(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	s__uint256_shl_(z, a, b->ll & 0xff);
}

S__INLINE void
s__uint256_shr(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	s__uint256_shr_(z, a, b->ll & 0xff);
}

S__INLINE void
s__uint256_set_bit(struct s__uint256 *a, int i)
{
	assert( a );
	assert( (0 <= i) && (255 >= i) );

	if      ( 64 > i) a->ll |= ((uint64_t)1 <<  i       );
	else if (128 > i) a->lh |= ((uint64_t)1 << (i -  64));
	else if (192 > i) a->hl |= ((uint64_t)1 << (i - 128));
	else /*--------*/ a->hh |= ((uint64_t)1 << (i - 192));
}

S__INLINE void
s__uint256_clr_bit(struct s__uint256 *a, int i)
{
	assert( a );
	assert( (0 <= i) && (255 >= i) );

	if      ( 64 > i) a->ll &= ~((uint64_t)1 <<  i       );
	else if (128 > i) a->lh &= ~((uint64_t)1 << (i -  64));
	else if (192 > i) a->hl &= ~((uint64_t)1 << (i - 128));
	else /*--------*/ a->hh &= ~((uint64_t)1 << (i - 192));
}

S__INLINE int
s__uint256_get_bit(const struct s__uint256 *a, int i)
{
	assert( a );
	assert( (0 <= i) && (255 >= i) );

	if      ( 64 > i) return (a->ll & ((uint64_t)1 <<  i       )) ? 1 : 0;
	else if (128 > i) return (a->lh & ((uint64_t)1 << (i -  64))) ? 1 : 0;
	else if (192 > i) return (a->hl & ((uint64_t)1 << (i - 128))) ? 1 : 0;
	else /*--------*/ return (a->hh & ((uint64_t)1 << (i - 192))) ? 1 : 0;
}

S__INLINE void
s__uint256_not(struct s__uint256 *z, const struct s__uint256 *a)
{
	assert( z && a );

	z->hh = ~a->hh;
	z->hl = ~a->hl;
	z->lh = ~a->lh;
	z->ll = ~a->ll;
}

S__INLINE void
s__uint256_and(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	z->hh = a->hh & b->hh;
	z->hl = a->hl & b->hl;
	z->lh = a->lh & b->lh;
	z->ll = a->ll & b->ll;
}

S__INLINE void
s__uint256_xor(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	z->hh = a->hh ^ b->hh;
	z->hl = a->hl ^ b->hl;
	z->lh = a->lh ^ b->lh;
	z->ll = a->ll ^ b->ll;
}

S__INLINE void
s__uint256_or(struct s__uint256 *z,
	      const struct s__uint256 *a,
	      const struct s__uint256 *b)
{
	assert( z && a && b );

	z->hh = a->hh | b->hh;
	z->hl = a->hl | b->hl;
	z->lh = a->lh | b->lh;
	z->ll = a->ll | b->ll;
}

#endif /* _S_UINT256_H_ */
