/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_succinct.c
 */

#include "s_index_bitmap.h"
#include "s_index_succinct.h"

#define CHAR2INT(c) ( (int)((unsigned char)(c)) )

struct s__index_succinct {
	char *keys;
	uint64_t *records;
	/*-*/
	uint64_t size;
	uint64_t items;
	s__index_bitmap_t nodes;
	s__index_bitmap_t valids;
};

static uint64_t
get_node(const struct s__index_succinct *succinct, uint64_t i)
{
	if (s__index_bitmap_get(succinct->nodes, i)) {
		return (3 * s__index_bitmap_rank(succinct->nodes, i));
	}
	return 0;
}

static uint64_t
find(const struct s__index_succinct *succinct, const char *key)
{
	uint64_t root;
	int d;

	root = 3;
	while (root) {
		d = CHAR2INT(*key) - CHAR2INT(succinct->keys[root / 3]);
		if (!d) {
			if ('\0' == (*(++key))) {
				break;
			}
			root = get_node(succinct, root + 1);
		}
		else if (0 > d) {
			root = get_node(succinct, root + 0);
		}
		else {
			root = get_node(succinct, root + 2);
		}
	}
	if (root) {
		if (s__index_bitmap_get(succinct->valids, root / 3)) {
			return s__index_bitmap_rank(succinct->valids,
						    root / 3);
		}
	}
	return 0;
}

static uint64_t
min(const struct s__index_succinct *succinct, uint64_t root, char *okey)
{
	uint64_t i, node;

	i = 0;
	while (root) {
		if (!(node = get_node(succinct, root + 0))) {
			okey[i++] = succinct->keys[root / 3];
			okey[i] = '\0';
			if (s__index_bitmap_get(succinct->valids, root / 3)) {
				return s__index_bitmap_rank(succinct->valids,
							    root / 3);
			}
			node = get_node(succinct, root + 1);
		}
		root = node;
	}
	return 0;
}

static uint64_t
max(const struct s__index_succinct *succinct, uint64_t root, char *okey)
{
	uint64_t i, node;

	i = 0;
	while (root) {
		if (!(node = get_node(succinct, root + 2))) {
			okey[i++] = succinct->keys[root / 3];
			okey[i] = '\0';
			if (s__index_bitmap_get(succinct->valids, root / 3)) {
				return s__index_bitmap_rank(succinct->valids,
							    root / 3);
			}
			node = get_node(succinct, root + 1);
		}
		root = node;
	}
	return 0;
}

static uint64_t
next(const struct s__index_succinct *succinct, const char *key, char *okey)
{
	uint64_t i, up, node, root, hold;
	int d, flag;

	i = 0;
	up = 0;
	root = 3;
	hold = 0;
	flag = 0;
	while (root) {
		if (0 > (d = (int)(*key) - (int)succinct->keys[root / 3])) {
			if (s__index_bitmap_get(succinct->valids, root / 3) ||
			    get_node(succinct, root + 1)) {
				up = root;
				hold = i;
				flag = 1;
			}
			else if ((node = get_node(succinct, root + 2))) {
				up = node;
				hold = i;
				flag = 0;
			}
			root = get_node(succinct, root + 0);
		}
		else if (!d) {
			if ((node = get_node(succinct, root + 2))) {
				up = node;
				hold = i;
				flag = 0;
			}
			root = get_node(succinct, root + 1);
			okey[i++] = (*key);
			okey[i] = '\0';
			if ('\0' == (*(++key))) {
				break;
			}
		}
		else {
			root = get_node(succinct, root + 2);
		}
	}
	if (root) {
		return min(succinct, root, okey + i);
	}
	if (!up) {
		return 0;
	}
	i = hold;
	if (flag) {
		if (s__index_bitmap_get(succinct->valids, up / 3)) {
			okey[i++] = succinct->keys[up / 3];
			okey[i] = '\0';
			return s__index_bitmap_rank(succinct->valids, up / 3);
		}
		if ((root = get_node(succinct, up + 1))) {
			okey[i++] = succinct->keys[up / 3];
			okey[i] = '\0';
			return min(succinct, root, okey + i);
		}
		if ((root = get_node(succinct, up + 2))) {
			return min(succinct, root, okey + i);
		}
	}
	else {
		return min(succinct, up, okey + i);
	}
	return 0;
}

static uint64_t
prev(const struct s__index_succinct *succinct, const char *key, char *okey)
{
	uint64_t i, up, node, root, hold;
	int d, flag;

	i = 0;
	up = 0;
	root = 3;
	hold = 0;
	flag = 0;
	while (root) {
		if (0 < (d = (int)(*key) - (int)succinct->keys[root / 3])) {
			if (s__index_bitmap_get(succinct->valids, root / 3) ||
			    get_node(succinct, root + 1)) {
				up = root;
				hold = i;
				flag = 1;
			}
			else if ((node = get_node(succinct, root + 0))) {
				up = node;
				hold = i;
				flag = 0;
			}
			root = get_node(succinct, root + 2);
		}
		else if (!d) {
			if ((node = get_node(succinct, root + 0))) {
				up = node;
				hold = i;
				flag = 0;
			}
			root = get_node(succinct, root + 1);
			okey[i++] = (*key);
			okey[i] = '\0';
			if ('\0' == (*(++key))) {
				break;
			}
		}
		else {
			root = get_node(succinct, root + 0);
		}
	}
	if (root) {
		return max(succinct, root, okey + i);
	}
	if (!up) {
		return 0;
	}
	i = hold;
	if (flag) {
		if (s__index_bitmap_get(succinct->valids, up / 3)) {
			okey[i++] = succinct->keys[up / 3];
			okey[i] = '\0';
			return s__index_bitmap_rank(succinct->valids, up / 3);
		}
		if ((root = get_node(succinct, up + 1))) {
			okey[i++] = succinct->keys[up / 3];
			okey[i] = '\0';
			return max(succinct, root, okey + i);
		}
		if ((root = get_node(succinct, up + 0))) {
			return max(succinct, root, okey + i);
		}
	}
	else {
		return max(succinct, up, okey + i);
	}
	return 0;
}

static void
_encode_(void *ctx,
	 char key,
	 int left,
	 int center,
	 int right,
	 uint64_t *record)
{
	struct s__index_succinct *succinct;

	succinct = (struct s__index_succinct *)ctx;
	if (left) {
		s__index_bitmap_set(succinct->nodes, succinct->size * 3 + 0);
	}
	if (center) {
		s__index_bitmap_set(succinct->nodes, succinct->size * 3 + 1);
	}
	if (right) {
		s__index_bitmap_set(succinct->nodes, succinct->size * 3 + 2);
	}
	if (record) {
		s__index_bitmap_set(succinct->valids, succinct->size);
		succinct->records[succinct->items++] = (*record);
	}
	succinct->keys[succinct->size++] = key;
}

s__index_succinct_t
s__index_succinct_open(s__index_ternary_t ternary)
{
	struct s__index_succinct *succinct;
	uint64_t size, items, n1, n2;

	assert( ternary );

	if (!(succinct = s__malloc(sizeof (struct s__index_succinct)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(succinct, 0, sizeof (struct s__index_succinct));
	if (s__index_ternary_items(ternary)) {
		size = s__index_ternary_nodes(ternary) + 1;
		items = s__index_ternary_items(ternary) + 1;
		if (!(succinct->nodes = s__index_bitmap_open(size * 3)) ||
		    !(succinct->valids = s__index_bitmap_open(size * 1))) {
			s__index_succinct_close(succinct);
			S__TRACE(0);
			return NULL;
		}
		n1 = size * sizeof (succinct->keys[0]);
		n2 = items * sizeof (succinct->records[0]);
		if (!(succinct->keys = s__malloc(n1)) ||
		    !(succinct->records = s__malloc(n2))) {
			s__index_succinct_close(succinct);
			S__TRACE(0);
			return NULL;
		}
		memset(succinct->keys, 0, n1);
		memset(succinct->records, 0, n2);
		succinct->size = 1;
		succinct->items = 1;
		s__index_bitmap_set(succinct->nodes, 1);
		if (s__index_ternary_iterate(ternary, _encode_, succinct)) {
			s__index_succinct_close(succinct);
			S__TRACE(0);
			return NULL;
		}
		s__index_bitmap_prepare(succinct->nodes);
		s__index_bitmap_prepare(succinct->valids);
		assert( size == succinct->size );
		assert( items == succinct->items );
	}
	return succinct;
}

void
s__index_succinct_close(s__index_succinct_t succinct)
{
	if (succinct) {
		s__index_bitmap_close(succinct->nodes);
		s__index_bitmap_close(succinct->valids);
		S__FREE(succinct->keys);
		S__FREE(succinct->records);
		memset(succinct, 0, sizeof (struct s__index_succinct));
	}
	S__FREE(succinct);
}

uint64_t *
s__index_succinct_find(s__index_succinct_t succinct, const char *key)
{
	uint64_t i;

	assert( succinct );
	assert( key );

	i = 0;
	if (succinct->items) {
		i = find(succinct, key);
	}
	return i ? &succinct->records[i] : NULL;
}

uint64_t *
s__index_succinct_next(s__index_succinct_t succinct,
		       const char *key,
		       char *okey)
{
	uint64_t i;

	assert( succinct );
	assert( okey );

	i = 0;
	if (succinct->items) {
		if (s__strlen(key)) {
			i = next(succinct, key, okey);
		}
		else {
			i = min(succinct, 3, okey);
		}
	}
	return i ? &succinct->records[i] : NULL;
}

uint64_t *
s__index_succinct_prev(s__index_succinct_t succinct,
		       const char *key,
		       char *okey)
{
	uint64_t i;

	assert( succinct );
	assert( okey );

	i = 0;
	if (succinct->items) {
		if (s__strlen(key)) {
			i = prev(succinct, key, okey);
		}
		else {
			i = max(succinct, 3, okey);
		}
	}
	return i ? &succinct->records[i] : NULL;
}

uint64_t
s__index_succinct_items(s__index_succinct_t succinct)
{
	assert( succinct );

	return succinct->items ? (succinct->items - 1) : 0;
}
