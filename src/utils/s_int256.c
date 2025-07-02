/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_int256.c
 */

#include "s_int256.h"

void
s__int256_dec(const struct s__int256 *a, char *s)
{
	static const struct s__uint256 ONE = { 0, 0, 0, 1 };
	struct s__int256 z;

	assert( a && s );

	if (s__int256_is_neg(a)) {
		(*s++) = '-';
		s__uint256_not(&z, a);
		s__uint256_add_(&z, &z, &ONE);
		s__uint256_dec(&z, s);
	}
	else {
		s__uint256_dec(a, s);
	}
}

int
s__int256_cmp(const struct s__int256 *a_, const struct s__int256 *b_)
{
	struct s__uint256 a, b, o;
	int sa, sb;

	assert( a_ && b_ );

	a = (*a_);
	b = (*b_);
	sa = sb = 0;
	if (s__int256_is_neg(a_)) {
		sa = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&a, a_);
		s__uint256_add_(&a, &a, &o);
	}
	if (s__int256_is_neg(b_)) {
		sb = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&b, b_);
		s__uint256_add_(&b, &b, &o);
	}
	if (sa && sb) {
		return s__uint256_cmp(&a, &b) * -1;
	}
	if (!sa && !sb) {
		return s__uint256_cmp(&a, &b);
	}
	return sa ? -1 : +1;
}

int
s__int256_neg(struct s__int256 *z, const struct s__int256 *a)
{
	static const struct s__uint256 ONE = { 0, 0, 0, 1 };
	int sa;

	assert( z && a );

	sa = s__int256_is_neg(a);
	s__uint256_not(z, a);
	s__uint256_add_(z, z, &ONE);
	if (sa && s__int256_is_neg(z)) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	return 0;
}

int
s__int256_add(struct s__int256 *z,
	      const struct s__int256 *a,
	      const struct s__int256 *b)
{
	int sa, sb;

	assert( z && a && b );

	sa = s__int256_is_neg(a);
	sb = s__int256_is_neg(b);
	s__uint256_add_(z, a, b);
	if ((sa == sb) && (sa != s__int256_is_neg(z))) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	return 0;
}

int
s__int256_sub(struct s__int256 *z,
	      const struct s__int256 *a,
	      const struct s__int256 *b)
{
	struct s__uint256 t;

	assert( z && a && b );

	return s__int256_neg(&t, b) || s__int256_add(z, a, &t);
}

int
s__int256_mul(struct s__int256 *z,
	      const struct s__int256 *a_,
	      const struct s__int256 *b_)
{
	struct s__uint256 a, b, o;
	int sa, sb;

	assert( z && a_ && b_ );

	a = (*a_);
	b = (*b_);
	sa = sb = 0;
	if (s__int256_is_neg(a_)) {
		sa = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&a, a_);
		s__uint256_add_(&a, &a, &o);
	}
	if (s__int256_is_neg(b_)) {
		sb = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&b, b_);
		s__uint256_add_(&b, &b, &o);
	}
	if (s__uint256_mul(z, &a, &b)) {
		S__TRACE(0);
		return -1;
	}
	if (sa ^ sb) {
		s__uint256_not(z, z);
		s__uint256_add_(z, z, &o);
	}
	return 0;
}

int
s__int256_div(struct s__int256 *z,
	      const struct s__int256 *a_,
	      const struct s__int256 *b_)
{
	struct s__uint256 a, b, o;
	int sa, sb;

	assert( z && a_ && b_ );

	a = (*a_);
	b = (*b_);
	sa = sb = 0;
	if (s__int256_is_neg(a_)) {
		sa = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&a, a_);
		s__uint256_add_(&a, &a, &o);
	}
	if (s__int256_is_neg(b_)) {
		sb = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&b, b_);
		s__uint256_add_(&b, &b, &o);
	}
	if (s__uint256_div(z, &a, &b)) {
		S__TRACE(0);
		return -1;
	}
	if (sa ^ sb) {
		s__uint256_not(z, z);
		s__uint256_add_(z, z, &o);
	}
	return 0;
}

int
s__int256_mod(struct s__int256 *z,
	      const struct s__int256 *a_,
	      const struct s__int256 *b_)
{
	struct s__uint256 a, b, o;
	int sa, sb;

	assert( z && a_ && b_ );

	a = (*a_);
	b = (*b_);
	sa = sb = 0;
	if (s__int256_is_neg(a_)) {
		sa = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&a, a_);
		s__uint256_add_(&a, &a, &o);
	}
	if (s__int256_is_neg(b_)) {
		sb = 1;
		S__UINT256_SET(&o, 1);
		s__uint256_not(&b, b_);
		s__uint256_add_(&b, &b, &o);
	}
	if (s__uint256_mod(z, &a, &b)) {
		S__TRACE(0);
		return -1;
	}
	if (sa) {
		s__uint256_not(z, z);
		s__uint256_add_(z, z, &o);
	}
	return 0;
}

void
s__int256_shr_(struct s__int256 *z, const struct s__int256 *a, int n)
{
	int i, sa;

	assert( z && a );
	assert( (0 <= n) && (255 >= n) );

	sa = s__int256_is_neg(a);
	s__uint256_shr_(z, a, n);
	if (sa) {
		for (i=0; i<n; ++i) {
			s__int256_set_bit(z, 255 - i);
		}
	}
}

int
s__int256_bist(void)
{
	struct s__int256 a, b, q, r;
	char buf[S__INT256_STR_LEN];
	uint64_t i, n;

	/* initialize */

	n = 654321;

	/* basic */

	if (s__int256_init(&a, 0) ||
	    s__int256_is_neg(&a) ||
	    s__int256_neg(&b, &a) ||
	    s__int256_is_neg(&b)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&a, buf);
	if (strcmp("0", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&b, buf);
	if (strcmp("0", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* basic */

	if (s__int256_init(&a,
			   "0x"
			   "ffffffffffffffff"
			   "ffffffffffffffff"
			   "ffffffffffffffff"
			   "ffffffffffffffff") ||
	    !s__int256_is_neg(&a) ||
	    s__int256_neg(&b, &a) ||
	    s__int256_is_neg(&b)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&a, buf);
	if (strcmp("-1", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&b, buf);
	if (strcmp("1", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* basic */

	if (s__int256_init(&a, "1") ||
	    s__int256_is_neg(&a) ||
	    s__int256_neg(&b, &a) ||
	    !s__int256_is_neg(&b)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&a, buf);
	if (strcmp("1", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__int256_dec(&b, buf);
	if (strcmp("-1", buf)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* a == (a / b) * b + (a % b) */

	for (i=0; i<n; ++i) {
		a.hh = ((uint64_t)rand() << 32) + rand();
		a.hl = ((uint64_t)rand() << 32) + rand();
		a.lh = ((uint64_t)rand() << 32) + rand();
		a.ll = ((uint64_t)rand() << 32) + rand();
		b.hh = ((uint64_t)rand() << 32) + rand();
		b.hl = ((uint64_t)rand() << 32) + rand();
		b.lh = ((uint64_t)rand() << 32) + rand();
		b.ll = ((uint64_t)rand() << 32) + rand();
		b.ll = b.ll ? b.ll : 1; /* unlikely */
		if (s__int256_div(&q, &a, &b) ||
		    s__int256_mod(&r, &a, &b) ||
		    s__int256_mul(&q, &q, &b) ||
		    s__int256_add(&b, &q, &r) ||
		    s__int256_cmp(&a, &b)) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	return 0;
}
