/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_bist.c
 */

#include "s_index.h"
#include "s_index_bist.h"

#define N 1000000

#define UL(x) ( (unsigned long)(x) )

#define TEST(m,e)						\
	do {							\
		if ((e)) {					\
			t = s__time() - t;			\
			s__term_color(S__TERM_COLOR_RED);       \
			s__term_bold();				\
			printf("\t [FAIL] ");			\
			s__term_reset();			\
			printf("%20s %6.1fs\n", (m), 1e-6*t);   \
			t = s__time();				\
		}						\
		else {						\
			t = s__time() - t;			\
			s__term_color(S__TERM_COLOR_GREEN);     \
			s__term_bold();				\
			printf("\t [PASS] ");			\
			s__term_reset();			\
			printf("%20s %6.1fs\n", (m), 1e-6*t);   \
			t = s__time();				\
		}						\
	} while (0)

int
s__index_bist(void)
{
	uint64_t t, i, j, *record;
	char key[64], okey[64];
	s__index_t index;

	/* initialize */

	printf("---=== INDEX BIST ===---\n");
	t = s__time();
	if (!(index = s__index_open())) {
		S__TRACE(0);
		return -1;
	}

	/* zero item logic */

	s__index_truncate(index);
	if ((0 != s__index_items(index)) ||
	    (NULL != s__index_find(index, "K")) ||
	    (NULL != s__index_next(index, "K", okey)) ||
	    (NULL != s__index_next(index, NULL, okey)) ||
	    (NULL != s__index_prev(index, "K", okey)) ||
	    (NULL != s__index_prev(index, NULL, okey)) ||
	    s__index_compress(index) ||
	    (0 != s__index_items(index)) ||
	    (NULL != s__index_find(index, "K")) ||
	    (NULL != s__index_find(index, "K")) ||
	    (NULL != s__index_next(index, "K", okey)) ||
	    (NULL != s__index_next(index, NULL, okey)) ||
	    (NULL != s__index_prev(index, "K", okey)) ||
	    (NULL != s__index_prev(index, NULL, okey))) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("zero-item-logic", -1);
		return -1;
	}
	TEST("zero-item-logic", 0);

	/* single item logic */

	s__index_truncate(index);
	if ((0 != s__index_items(index)) ||
	    !(record = s__index_update(index, "B")) ||
	    (1 != s__index_items(index))) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("single-item-logic", -1);
		return -1;
	}
	(*record) = 123;
	if ((NULL != s__index_find(index, "A")) ||
	    (NULL != s__index_next(index, "B", okey)) ||
	    (NULL != s__index_prev(index, "A", okey)) ||
	    !(record = s__index_find(index, "B")) ||
	    (123 != (*record)) ||
	    (record != s__index_next(index, NULL, okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, NULL, okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_next(index, "", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, "", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_next(index, "A", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, "C", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    s__index_compress(index) ||
	    (1 != s__index_items(index)) ||
	    (NULL != s__index_find(index, "A")) ||
	    (NULL != s__index_next(index, "B", okey)) ||
	    (NULL != s__index_prev(index, "A", okey)) ||
	    !(record = s__index_find(index, "B")) ||
	    (123 != (*record)) ||
	    (record != s__index_next(index, NULL, okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, NULL, okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_next(index, "", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, "", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_next(index, "A", okey)) ||
	    (123 != (*record)) || strcmp("B", okey) ||
	    (record != s__index_prev(index, "C", okey)) ||
	    (123 != (*record)) || strcmp("B", okey)) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("single-item-logic", -1);
		return -1;
	}
	TEST("single-item-logic", 0);

	/* two item logic */

	s__index_truncate(index);
	if ((0 != s__index_items(index)) ||
	    !(record = s__index_update(index, "A")) ||
	    (1 != s__index_items(index))) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("two-item-logic", -1);
		return -1;
	}
	(*record) = 123;
	if ((1 != s__index_items(index)) ||
	    !(record = s__index_update(index, "C")) ||
	    (2 != s__index_items(index))) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("two-item-logic", -1);
		return -1;
	}
	(*record) = 321;
	if (!(record = s__index_find(index, "A")) ||
	    (123 != (*record)) ||
	    !(record = s__index_find(index, "C")) ||
	    (321 != (*record)) ||
	    (NULL != s__index_find(index, "B")) ||
	    s__index_compress(index) ||
	    (2 != s__index_items(index)) ||
	    !(record = s__index_next(index, NULL, okey)) ||
	    (123 != (*record)) || strcmp("A", okey) ||
	    !(record = s__index_prev(index, NULL, okey)) ||
	    (321 != (*record)) || strcmp("C", okey) ||
	    !(record = s__index_next(index, "", okey)) ||
	    (123 != (*record)) || strcmp("A", okey) ||
	    !(record = s__index_prev(index, "", okey)) ||
	    (321 != (*record)) || strcmp("C", okey) ||
	    !(record = s__index_next(index, "B", okey)) ||
	    (321 != (*record)) || strcmp("C", okey) ||
	    !(record = s__index_prev(index, "B", okey)) ||
	    (123 != (*record)) || strcmp("A", okey) ||
	    (NULL != s__index_find(index, "B"))) {
		s__index_close(index);
		S__TRACE(S__ERR_SOFTWARE);
		TEST("two-item-logic", -1);
		return -1;
	}
	TEST("two-item-logic", 0);

	/* sequential update */

	s__index_truncate(index);
	for (i=0; i<N; ++i) {
		s__sprintf(key, sizeof (key), "k:%012lu", UL(i));
		if (!(record = s__index_update(index, key)) ||
		    !((*record) = (i + 1)) ||
		    !(record = s__index_find(index, key)) ||
		    ((i + 1) != (*record)) ||
		    ((i + 1) != s__index_items(index))) {
			s__index_close(index);
			S__TRACE(0);
			TEST("sequential-update", -1);
			return -1;
		}
	}
	TEST("sequential-update", 0);

	/* sequential find */

	for (i=0; i<N; ++i) {
		s__sprintf(key, sizeof (key), "k:%012lu", UL(i));
		if (!(record = s__index_find(index, key)) ||
		    ((i + 1) != (*record))) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("sequential-find", -1);
			break;
		}
	}
	TEST("sequential-find", 0);

	/* random find */

	for (i=0; i<N; ++i) {
		j = (uint64_t)rand() % N;
		s__sprintf(key, sizeof (key), "k:%012lu", UL(j));
		if (!(record = s__index_find(index, key)) ||
		    ((j + 1) != (*record))) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("random-find", -1);
			break;
		}
	}
	TEST("random-find", 0);

	/* compress */

	if (s__index_compress(index) || (N != s__index_items(index))) {
		s__index_close(index);
		S__TRACE(0);
		TEST("compress", -1);
		return -1;
	}
	TEST("compress", 0);

	/* sequential find */

	for (i=0; i<N; ++i) {
		s__sprintf(key, sizeof (key), "k:%012lu", UL(i));
		if (!(record = s__index_find(index, key)) ||
		    ((i + 1) != (*record))) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("sequential-find", -1);
			break;
		}
	}
	TEST("sequential-find", 0);

	/* random find */

	for (i=0; i<N; ++i) {
		j = (uint64_t)rand() % N;
		s__sprintf(key, sizeof (key), "k:%012lu", UL(j));
		if (!(record = s__index_find(index, key)) ||
		    ((j + 1) != (*record))) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("random-find", -1);
			break;
		}
	}
	TEST("random-find", 0);

	/* next find */

	i = 0;
	key[0] = '\0';
	while ((record = s__index_next(index, key, okey))) {
		s__sprintf(key, sizeof (key), "k:%012lu", UL(i));
		if (((i + 1) != (*record)) || strcmp(key, okey)) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("next-find", -1);
			return -1;
		}
		s__sprintf(key, sizeof (key), "%s", okey);
		++i;
	}
	TEST("next-find", 0);

	/* prev find */

	i = N - 1;
	key[0] = '\0';
	while ((record = s__index_prev(index, key, okey))) {
		s__sprintf(key, sizeof (key), "k:%012lu", UL(i));
		if (((i + 1) != (*record)) || strcmp(key, okey)) {
			s__index_close(index);
			S__TRACE(S__ERR_SOFTWARE);
			TEST("prev-find", -1);
			return -1;
		}
		s__sprintf(key, sizeof (key), "%s", okey);
		--i;
	}
	TEST("prev-find", 0);

	/* done */

	s__index_close(index);
	printf("---=== INDEX BIST ===---\n\n");
	return 0;
}
