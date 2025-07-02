/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_uint256.c
 */

#include "s_uint256.h"

static int
hex2int(int c)
{
	c = tolower(c);
	if (('0' <= c) && ('9' >= c)) {
		return c - '0';
	}
	if (('a' <= c) && ('f' >= c)) {
		return 15 + c - 'f';
	}
	return -1;
}

static int
dec2int(int c)
{
	if (('0' <= c) && ('9' >= c)) {
		return c - '0';
	}
	return -1;
}

static int
add64(uint64_t *za, uint64_t b)
{
	if ((0xffffffffffffffff - (*za)) < b) {
		(*za) += b;
		return 1;
	}
	(*za) += b;
	return 0;
}

static int
sub64(uint64_t *za, uint64_t b)
{
	if ((*za) < b) {
		(*za) -= b;
		return 1;
	}
	(*za) -= b;
	return 0;
}

static int
add128(uint64_t *zah, uint64_t *zal, uint64_t bh, uint64_t bl)
{
	if (add64(zal, bl)) {
		return add64(zah, 1) + add64(zah, bh);
	}
	return add64(zah, bh);
}

static int
sub128(uint64_t *zah, uint64_t *zal, uint64_t bh, uint64_t bl)
{
	if (sub64(zal, bl)) {
		return sub64(zah, 1) + sub64(zah, bh);
	}
	return sub64(zah, bh);
}

static int
add256(struct s__uint256 *za, const struct s__uint256 *b)
{
	assert( za && b );

	if (add128(&za->lh, &za->ll, b->lh, b->ll)) {
		return add128(&za->hh, &za->hl, 0, 1) +
			add128(&za->hh, &za->hl, b->hh, b->hl);
	}
	return add128(&za->hh, &za->hl, b->hh, b->hl);
}

static int
sub256(struct s__uint256 *za, const struct s__uint256 *b)
{
	assert( za && b );

	if (sub128(&za->lh, &za->ll, b->lh, b->ll)) {
		return sub128(&za->hh, &za->hl, 0, 1) +
			sub128(&za->hh, &za->hl, b->hh, b->hl);
	}
	return sub128(&za->hh, &za->hl, b->hh, b->hl);
}

static void
add128_no_carry(uint64_t *zah, uint64_t *zal, uint64_t b)
{
	if (add64(zal, b)) {
		if (add64(zah, 1)) {
			S__HALT(S__ERR_SOFTWARE);
		}
	}
}

static void
add256_no_carry(struct s__uint256 *za, uint64_t bh, uint64_t bl)
{
	if (add128(&za->lh, &za->ll, bh, bl)) {
		if (add128(&za->hh, &za->hl, 0, 1)) {
			S__HALT(S__ERR_SOFTWARE);
		}
	}
}

static void
mul64(uint64_t *zh, uint64_t *zl, uint64_t a, uint64_t b)
{
#ifdef __SIZEOF_INT128__
	__uint128_t t;

	t = (__uint128_t)a * (__uint128_t)b;
	(*zh) = (uint64_t)(t >> 64);
	(*zl) = (uint64_t)(t      );
#else
	uint64_t ah, al, bh, bl, t1, t2, t3, t4;

	ah = a >> 32;
	al = a & 0xffffffff;
	bh = b >> 32;
	bl = b & 0xffffffff;
	t1 = bl * al;
	t2 = bl * ah;
	t3 = bh * al;
	t4 = bh * ah;
	t2 += t1 >> 32;
	if (add64(&t3, t2)) {
		t4 += (uint64_t)1 << 32;
	}
	t4 += t3 >> 32;
	(*zh) = t4;
	(*zl) = (t3 << 32) | (t1 & 0xffffffff);
#endif /* __SIZEOF_INT128__ */
}

static void
mul128(struct s__uint256 *z,
       uint64_t ah,
       uint64_t al,
       uint64_t bh,
       uint64_t bl)
{
	uint64_t t1h, t1l, t2h, t2l, t3h, t3l, t4h, t4l;

	mul64(&t1h, &t1l, bl, al);
	mul64(&t2h, &t2l, bl, ah);
	mul64(&t3h, &t3l, bh, al);
	mul64(&t4h, &t4l, bh, ah);
	add128_no_carry(&t2h, &t2l, t1h);
	if (add128(&t3h, &t3l, t2h, t2l)) {
		if (add64(&t4h, 1)) {
			S__HALT(S__ERR_SOFTWARE);
		}
	}
	add128_no_carry(&t4h, &t4l, t3h);
	z->hh = t4h;
	z->hl = t4l;
	z->lh = t3l;
	z->ll = t1l;
}

static void
shl128(uint64_t *zh, uint64_t *zl, uint64_t ah, uint64_t al, int n)
{
	int m;

	while (n) {
		m  = (63 < n) ? 63 : n;
		n -= m;
		ah = ah << m;
		ah = ah | (al >> (64 - m));
		al <<= m;
	}
	(*zh) = ah;
	(*zl) = al;
}

static void
shr128(uint64_t *zh, uint64_t *zl, uint64_t ah, uint64_t al, int n)
{
	int m;

	while (n) {
		m  = (63 < n) ? 63 : n;
		n -= m;
		al = al >> m;
		al = al | (ah << (64 - m));
		ah >>= m;
	}
	(*zh) = ah;
	(*zl) = al;
}

int
s__uint256_init(struct s__uint256 *z, const char *s)
{
	struct s__uint256 m, a;
	int (*p2v)(int);
	int v;

	assert( z );

	p2v = dec2int;
	S__UINT256_SET(z, 0);
	S__UINT256_SET(&m, 10);
	if (s__strlen(s)) {
		if (('0' == s[0]) && (('x' == s[1]) || ('X' == s[1]))) {
			if (!s[2]) {
				S__TRACE(S__ERR_ARGUMENT);
				return -1;
			}
			s += 2;
			S__UINT256_SET(&m, 16);
			p2v = hex2int;
		}
		while (*s) {
			if (0 > (v = p2v((unsigned char)(*s++)))) {
				S__TRACE(S__ERR_ARGUMENT);
				return -1;
			}
			S__UINT256_SET(&a, v);
			if (s__uint256_mul(z, z, &m) ||
			    s__uint256_add(z, z, &a)) {
				S__TRACE(0);
				return -1;
			}
		}
	}
	return 0;
}

void
s__uint256_hex(const struct s__uint256 *a, char *s)
{
	assert( a && s );

	s__sprintf(s,
		   S__UINT256_STR_LEN,
		   "0x%016lx%016lx%016lx%016lx",
		   (unsigned long)a->hh,
		   (unsigned long)a->hl,
		   (unsigned long)a->lh,
		   (unsigned long)a->ll);
}

void
s__uint256_dec(const struct s__uint256 *a, char *s)
{
	struct s__uint256 q, r, m;
	uint64_t i, n, stack[10];

	assert( a && s );

	i = 0;
	q = (*a);
	S__UINT256_SET(&m, 1000000000);
	while (!s__uint256_is_zero(&q)) {
		if (s__uint256_divmod(&q, &r, &q, &m)) {
			S__HALT(S__ERR_SOFTWARE);
		}
		stack[i++] = r.ll;
		assert( i < S__ARRAY_SIZE(stack) );
	}
	s__sprintf(s, S__UINT256_STR_LEN, "0");
	if (i) {
		s__sprintf(s,
			   S__UINT256_STR_LEN, "%lu",
			   (unsigned long)stack[--i]);
		while (i) {
			n = s__strlen(s);
			s__sprintf(s + n,
				   S__UINT256_STR_LEN - n,
				   "%09lu",
				   (unsigned long)stack[--i]);
		}
	}
}

void
s__uint256_add_(struct s__uint256 *z,
		const struct s__uint256 *a,
		const struct s__uint256 *b)
{
	assert( z && a && b );

	(*z) = (*a);
	add256(z, b);
}

int
s__uint256_add(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	(*z) = (*a);
	if (add256(z, b)) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	return 0;
}

int
s__uint256_sub(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	assert( z && a && b );

	(*z) = (*a);
	if (sub256(z, b)) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	return 0;
}

void
s__uint256_mulf(struct s__uint256 *zh,
		struct s__uint256 *zl,
		const struct s__uint256 *a,
		const struct s__uint256 *b)
{
	struct s__uint256 t1, t2, t3, t4;

	assert( zh && zl && a && b );

	mul128(&t1, b->lh, b->ll, a->lh, a->ll);
	mul128(&t2, b->lh, b->ll, a->hh, a->hl);
	mul128(&t3, b->hh, b->hl, a->lh, a->ll);
	mul128(&t4, b->hh, b->hl, a->hh, a->hl);
	add256_no_carry(&t2, t1.hh, t1.hl);
	if (add256(&t3, &t2)) {
		if (add128(&t4.hh, &t4.hl, 0, 1)) {
			S__HALT(S__ERR_SOFTWARE);
		}
	}
	add256_no_carry(&t4, t3.hh, t3.hl);
	zh->hh = t4.hh;
	zh->hl = t4.hl;
	zh->lh = t4.lh;
	zh->ll = t4.ll;
	zl->hh = t3.lh;
	zl->hl = t3.ll;
	zl->lh = t1.lh;
	zl->ll = t1.ll;
}

int
s__uint256_mul(struct s__uint256 *z,
	       const struct s__uint256 *a,
	       const struct s__uint256 *b)
{
	struct s__uint256 t1, t2, t3;

	assert( z && a && b );

	mul128(&t1, b->lh, b->ll, a->lh, a->ll);
	mul128(&t2, b->lh, b->ll, a->hh, a->hl);
	mul128(&t3, b->hh, b->hl, a->lh, a->ll);
	add256_no_carry(&t2, t1.hh, t1.hl);
	if (add256(&t3, &t2)) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	z->hh = t3.lh;
	z->hl = t3.ll;
	z->lh = t1.lh;
	z->ll = t1.ll;
	return 0;
}

int
s__uint256_divmod(struct s__uint256 *zq,
		  struct s__uint256 *zr,
		  const struct s__uint256 *a_,
		  const struct s__uint256 *b_)
{
	struct s__uint256 a, b;
	int i;

	assert( zq && zr && a_ && b_ );

	a = (*a_);
	b = (*b_);
	S__UINT256_SET(zq, 0);
	S__UINT256_SET(zr, 0);
	if (s__uint256_is_zero(b_)) {
		S__TRACE(S__ERR_ARITHMETIC);
		return -1;
	}
	for (i=255; i>=0; --i) {
		s__uint256_shl_(zr, zr, 1);
		if (s__uint256_get_bit(&a, i)) {
			s__uint256_set_bit(zr, 0);
		}
		if (0 <= s__uint256_cmp(zr, &b)) {
			if (s__uint256_sub(zr, zr, &b)) {
				S__HALT(S__ERR_SOFTWARE);
				return -1;
			}
			s__uint256_set_bit(zq, i);
		}
	}
	return 0;
}

void
s__uint256_shl_(struct s__uint256 *z, const struct s__uint256 *a, int n)
{
	uint64_t h, l;
	int m;

	assert( z && a );
	assert( (0 <= n) && (255 >= n) );

	(*z) = (*a);
	while (n) {
		m  = (127 < n) ? 127 : n;
		n -= m;
		shl128(&z->hh, &z->hl, z->hh, z->hl, m);
		shr128(&h, &l, z->lh, z->ll, 128 - m);
		z->hh |= h;
		z->hl |= l;
		shl128(&z->lh, &z->ll, z->lh, z->ll, m);
	}
}

void
s__uint256_shr_(struct s__uint256 *z, const struct s__uint256 *a, int n)
{
	uint64_t h, l;
	int m;

	assert( z && a );
	assert( (0 <= n) && (255 >= n) );

	(*z) = (*a);
	while (n) {
		m  = (127 < n) ? 127 : n;
		n -= m;
		shr128(&z->lh, &z->ll, z->lh, z->ll, m);
		shl128(&h, &l, z->hh, z->hl, 128 - m);
		z->lh |= h;
		z->ll |= l;
		shr128(&z->hh, &z->hl, z->hh, z->hl, m);
	}
}

int
s__uint256_bist(void)
{
	struct s__uint256 a, b, q, r;
	char buf[S__UINT256_STR_LEN];
	uint64_t i, j, k, n;
	int set[256];

	/* initialize */

	n = 123456;

	/* basic */

	if (s__uint256_init(&a, 0) ||
	    !s__uint256_is_zero(&a) ||
	    s__uint256_init(&a, "") ||
	    !s__uint256_is_zero(&a) ||
	    s__uint256_init(&a, "0") ||
	    !s__uint256_is_zero(&a) ||
	    s__uint256_init(&a, "0x0") ||
	    !s__uint256_is_zero(&a) ||
	    s__uint256_init(&a,
			    "11579208923731619542357098"
			    "50086879078532699846656405"
			    "64039457584007913129639935") ||
	    s__uint256_init(&b,
			    "0x"
			    "ffffffffffffffff"
			    "ffffffffffffffff"
			    "ffffffffffffffff"
			    "ffffffffffffffff") ||
	    s__uint256_cmp(&a, &b)) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* basic */

	for (i=0; i<n; ++i) {
		if (0 == i) {
			memset(&a, 0x00, sizeof (struct s__uint256));
		}
		else if (1 == i) {
			memset(&a, 0xff, sizeof (struct s__uint256));
		}
		else {
			a.hh = ((uint64_t)rand() << 32) + rand();
			a.hl = ((uint64_t)rand() << 32) + rand();
			a.lh = ((uint64_t)rand() << 32) + rand();
			a.ll = ((uint64_t)rand() << 32) + rand();
		}
		s__uint256_hex(&a, buf);
		if (s__uint256_init(&b, buf) || s__uint256_cmp(&a, &b)) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
		s__uint256_dec(&a, buf);
		if (s__uint256_init(&b, buf) || s__uint256_cmp(&a, &b)) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
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
		if (s__uint256_div(&q, &a, &b) ||
		    s__uint256_mod(&r, &a, &b) ||
		    s__uint256_mul(&q, &q, &b) ||
		    s__uint256_add(&b, &q, &r) ||
		    s__uint256_cmp(&a, &b)) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}

	/* shl */

	for (i=0; i<n; ++i) {
		S__UINT256_SET(&a, 0);
		for (j=0; j<256; ++j) {
			set[j] = rand() % 2;
			switch (set[j]) {
			case 1: s__uint256_set_bit(&a, j); break;
			case 0: s__uint256_clr_bit(&a, j); break;
			}
		}
		k = rand() % 256;
		s__uint256_shl_(&a, &a, k);
		for (j=0; j<256; ++j) {
			if (j < k) {
				if (s__uint256_get_bit(&a, j)) {
					S__TRACE(S__ERR_SOFTWARE);
					return -1;
				}
			}
			else if (set[j - k] && !s__uint256_get_bit(&a, j)) {
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
			else if (!set[j - k] && s__uint256_get_bit(&a, j)) {
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
		}
	}

	/* shr */

	for (i=0; i<n; ++i) {
		S__UINT256_SET(&a, 0);
		for (j=0; j<256; ++j) {
			set[j] = rand() % 2;
			switch (set[j]) {
			case 1: s__uint256_set_bit(&a, 255 - j); break;
			case 0: s__uint256_clr_bit(&a, 255 - j); break;
			}
		}
		k = rand() % 256;
		s__uint256_shr_(&a, &a, k);
		for (j=0; j<256; ++j) {
			if (j < k) {
				if (s__uint256_get_bit(&a, 255 - j)) {
					S__TRACE(S__ERR_SOFTWARE);
					return -1;
				}
			}
			else if (set[j - k] &&
				 !s__uint256_get_bit(&a, 255 - j)) {
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
			else if (!set[j - k] &&
				 s__uint256_get_bit(&a, 255 - j)) {
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
		}
	}
	return 0;
}
