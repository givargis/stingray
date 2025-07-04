/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_utils.c
 */

#include "s_utils.h"

#define TEST(f,m)						\
	do {							\
		uint64_t t = s__time();				\
		if (f()) {					\
			t = s__time() - t;			\
			s__term_color(S__TERM_COLOR_RED);       \
			s__term_bold();				\
			printf("\t [FAIL] ");			\
			s__term_reset();			\
			printf("%20s %6.1fs\n", (m), 1e-6*t);   \
		}						\
		else {						\
			t = s__time() - t;			\
			s__term_color(S__TERM_COLOR_GREEN);     \
			s__term_bold();				\
			printf("\t [PASS] ");			\
			s__term_reset();			\
			printf("%20s %6.1fs\n", (m), 1e-6*t);   \
		}						\
	} while (0)

void
s__utils_init(void)
{
	s__ec_init();
}

int
s__utils_bist(void)
{
	printf("---=== UTILS BIST ===---\n");
	TEST(s__ann_bist, "ann");
	TEST(s__avl_bist, "avl");
	TEST(s__base64_bist, "base64");
	TEST(s__bitset_bist, "bitset");
	TEST(s__buf_bist, "buf");
	TEST(s__ec_bist, "ec");
	TEST(s__fft_bist, "fft");
	TEST(s__hash_bist, "hash");
	TEST(s__int256_bist, "int256");
	TEST(s__json_bist, "json");
	TEST(s__sha3_bist, "sha3");
	TEST(s__string_bist, "string");
	TEST(s__uint256_bist, "uint256");
	printf("---=== UTILS BIST ===---\n\n");
	return 0;
}
