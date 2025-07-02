/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_bitset.c
 */

#include "s_bitset.h"

#define NW64(n) S__DUP(n, 64)

struct s__bitset {
	uint64_t ii;
	uint64_t capacity;
	uint64_t *memory[2];
};

static void
set(struct s__bitset *bitset, uint64_t s, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	bitset->memory[s][Q] |= ((uint64_t)1 << R);
}

static void
clr(struct s__bitset *bitset, uint64_t s, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	bitset->memory[s][Q] &= ~((uint64_t)1 << R);
}

static int
get(const struct s__bitset *bitset, uint64_t s, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	if (i < bitset->capacity) {
		if ( (bitset->memory[s][Q] & ((uint64_t)1 << R)) ) {
			return 1;
		}
	}
	return 0;
}

s__bitset_t
s__bitset_open(uint64_t capacity)
{
	struct s__bitset *bitset;

	assert( capacity );

	if (!(bitset = s__malloc(sizeof (struct s__bitset)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(bitset, 0, sizeof (struct s__bitset));
	bitset->capacity = capacity;
	if (!(bitset->memory[0] = s__malloc(NW64(capacity) * 8)) ||
	    !(bitset->memory[1] = s__malloc(NW64(capacity) * 8))) {
		s__bitset_close(bitset);
	}
	memset(bitset->memory[0], 0, NW64(capacity) * 8);
	memset(bitset->memory[1], 0, NW64(capacity) * 8);
	set(bitset, 0, 0);
	return bitset;
}

void
s__bitset_close(s__bitset_t bitset)
{
	if (bitset) {
		S__FREE(bitset->memory[0]);
		S__FREE(bitset->memory[1]);
		memset(bitset, 0, sizeof (struct s__bitset));
	}
	S__FREE(bitset);
}

uint64_t
s__bitset_reserve(s__bitset_t bitset, uint64_t n)
{
	uint64_t i, j, j_, k;

	assert( bitset );
	assert( n );

	k = n;
	i = bitset->ii;
	for (j=0; j<bitset->capacity; ++j) {
		j_ = (bitset->ii + j) % bitset->capacity;
		if (get(bitset, 0, j_)) {
			i = j_ + 1;
			k = n;
			continue;
		}
		if (!--k) {
			for (j=0; j<n; ++j) {
				set(bitset, 0, i + j);
				set(bitset, 1, i + j);
			}
			clr(bitset, 1, i);
			bitset->ii = i + n - 1;
			return i;
		}
	}
	return 0;
}

uint64_t
s__bitset_release(s__bitset_t bitset, uint64_t i)
{
	uint64_t n;

	assert( bitset );
	assert( i );

	n = 0;
	if (get(bitset, 0, i) && !get(bitset, 1, i)) {
		do {
			clr(bitset, 0, i);
			clr(bitset, 1, i);
			++n;
			++i;
		}
		while (get(bitset, 1, i));
	}
	return n;
}

uint64_t
s__bitset_validate(s__bitset_t bitset, uint64_t i)
{
	uint64_t n;

	assert( bitset );

	n = 0;
	if (get(bitset, 0, i) && !get(bitset, 1, i)) {
		do {
			++n;
			++i;
		}
		while (get(bitset, 1, i));
	}
	return n;
}

uint64_t
s__bitset_utilized(s__bitset_t bitset)
{
	uint64_t i, n;

	assert( bitset );

	n = 0;
	for (i=0; i<NW64(bitset->capacity); ++i) {
		n += s__popcount(bitset->memory[0][i]);
	}
	return n;
}

uint64_t
s__bitset_capacity(s__bitset_t bitset)
{
	assert( bitset );

	return bitset->capacity;
}

int
s__bitset_bist(void)
{
	uint64_t seg_n[1000];
	uint64_t seg_i[1000];
	s__bitset_t bitset;
	uint64_t utilized;
	uint64_t i, n, m;

	/* capacity 1 */

	n = 1;
	if (!(bitset = s__bitset_open(n)) ||
	    (1 != s__bitset_utilized(bitset)) ||
	    (n != s__bitset_capacity(bitset)) ||
	    (0 != s__bitset_reserve(bitset, 1)) ||
	    (1 != s__bitset_validate(bitset, 0)) ||
	    (0 != s__bitset_validate(bitset, 1))) {
		s__bitset_close(bitset);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__bitset_close(bitset);

	/* capacity 63 */

	n = 63;
	if (!(bitset = s__bitset_open(n)) ||
	    (1 != s__bitset_utilized(bitset)) ||
	    (n != s__bitset_capacity(bitset)) ||
	    (1 != s__bitset_reserve(bitset, 1)) ||
	    (2 != s__bitset_reserve(bitset, 2)) ||
	    (4 != s__bitset_reserve(bitset, n - 4)) ||
	    (1 != s__bitset_validate(bitset, 0)) ||
	    (1 != s__bitset_validate(bitset, 1)) ||
	    (2 != s__bitset_validate(bitset, 2)) ||
	    ((n - 4) != s__bitset_validate(bitset, 4)) ||
	    (2 != s__bitset_release(bitset, 2)) ||
	    (2 != s__bitset_reserve(bitset, 1)) ||
	    (3 != s__bitset_reserve(bitset, 1)) ||
	    (1 != s__bitset_validate(bitset, 0)) ||
	    (1 != s__bitset_validate(bitset, 1)) ||
	    (1 != s__bitset_validate(bitset, 2)) ||
	    (1 != s__bitset_validate(bitset, 3)) ||
	    ((n - 4) != s__bitset_validate(bitset, 4))) {
		s__bitset_close(bitset);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__bitset_close(bitset);

	/* big test */

	n = 10000000;
	memset(seg_n, 0, sizeof (seg_n));
	memset(seg_i, 0, sizeof (seg_i));
	if (!(bitset = s__bitset_open(n)) ||
	    (1 != s__bitset_utilized(bitset)) ||
	    (n != s__bitset_capacity(bitset))) {
		s__bitset_close(bitset);
		S__TRACE(0);
		return -1;
	}
	utilized = 1;
	m = S__ARRAY_SIZE(seg_n);
	for (i=0; i<m; ++i) {
		seg_n[i] = (uint64_t)rand() % (n / m);
		seg_i[i] = s__bitset_reserve(bitset, seg_n[i]);
		utilized += seg_n[i];
		if (!seg_i[i] ||
		    (utilized != s__bitset_utilized(bitset)) ||
		    (seg_n[i] != s__bitset_validate(bitset, seg_i[i]))) {
			s__bitset_close(bitset);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	for (i=0; i<m; i+=2) {
		utilized -= seg_n[i];
		if ((seg_n[i] != s__bitset_release(bitset, seg_i[i])) ||
		    (utilized != s__bitset_utilized(bitset)) ||
		    (0 != s__bitset_validate(bitset, seg_i[i]))) {
			s__bitset_close(bitset);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
		seg_n[i] = (uint64_t)rand() % (n / m);
		seg_i[i] = s__bitset_reserve(bitset, seg_n[i]);
		utilized += seg_n[i];
		if (!seg_i[i] ||
		    (utilized != s__bitset_utilized(bitset)) ||
		    (seg_n[i] != s__bitset_validate(bitset, seg_i[i]))) {
			s__bitset_close(bitset);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	s__bitset_close(bitset);
	return 0;
}
