/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_bitmap.c
 */

#include "s_index_bitmap.h"

struct s__index_bitmap {
	uint64_t size;
	uint64_t *memory;
	uint32_t *popcount;
};

s__index_bitmap_t
s__index_bitmap_open(uint64_t size)
{
	struct s__index_bitmap *bitmap;
	uint64_t n1, n2;

	assert( 0 < size );

	if (!(bitmap = s__malloc(sizeof (struct s__index_bitmap)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(bitmap, 0, sizeof (struct s__index_bitmap));
	bitmap->size = S__DUP(size, 64);
	n1 = bitmap->size * sizeof (bitmap->memory[0]);
	n2 = bitmap->size * sizeof (bitmap->popcount[0]);
	if (!(bitmap->memory = s__malloc(n1)) ||
	    !(bitmap->popcount = s__malloc(n2))) {
		s__index_bitmap_close(bitmap);
		S__TRACE(0);
		return NULL;
	}
	memset(bitmap->memory, 0, n1);
	memset(bitmap->popcount, 0, n2);
	return bitmap;
}

void
s__index_bitmap_close(s__index_bitmap_t bitmap)
{
	if (bitmap) {
		S__FREE(bitmap->memory);
		S__FREE(bitmap->popcount);
		memset(bitmap, 0, sizeof (struct s__index_bitmap));
	}
	S__FREE(bitmap);
}

void
s__index_bitmap_prepare(s__index_bitmap_t bitmap)
{
	uint64_t i, popcount;

	assert( bitmap );

	popcount = 0;
	for (i=0; i<bitmap->size; ++i) {
		popcount += s__popcount(bitmap->memory[i]);
		bitmap->popcount[i] = (uint32_t)popcount;
	}
}

uint64_t
s__index_bitmap_rank(s__index_bitmap_t bitmap, uint64_t i)
{
	union { uint64_t *p64; uint32_t *p32; uint16_t *p16; } u;
	uint64_t q = i / 64;
	uint64_t r = i % 64;
	uint64_t popcount;

	assert( bitmap );
	assert( q < bitmap->size );

	u.p64 = &bitmap->memory[q];
	popcount = (q ? bitmap->popcount[q - 1] : 0);
	if (32 <= r) {
		popcount += s__popcount(*u.p32);
		u.p32 += 1;
		r -= 32;
	}
	if (16 <= r) {
		popcount += s__popcount(*u.p16);
		u.p16 += 1;
		r -= 16;
	}
	for (i=0; i<=r; ++i) {
		popcount += (*u.p16 & (1 << i)) ? 1 : 0;
	}
	return popcount;
}

void
s__index_bitmap_set(s__index_bitmap_t bitmap, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	assert( bitmap );
	assert( Q < bitmap->size );

	bitmap->memory[Q] |= ((uint64_t)1 << R);
}

void
s__index_bitmap_clr(s__index_bitmap_t bitmap, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	assert( bitmap );
	assert( Q < bitmap->size );

	bitmap->memory[Q] &= ~((uint64_t)1 << R);
}

int
s__index_bitmap_get(s__index_bitmap_t bitmap, uint64_t i)
{
	const uint64_t Q = i / 64;
	const uint64_t R = i % 64;

	assert( bitmap );
	assert( Q < bitmap->size );

	return (bitmap->memory[Q] & ((uint64_t)1 << R)) ? 1 : 0;
}
