/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index.c
 */

#include "s_index_succinct.h"
#include "s_index.h"

struct s__index {
	s__index_tree_t tree;
	s__index_succinct_t succinct;
};

s__index_t
s__index_open(void)
{
	struct s__index *index;

	assert( S__INDEX_MAX_KEY_LEN == S__INDEX_TREE_MAX_KEY_LEN );

	if (!(index = s__malloc(sizeof (struct s__index)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(index, 0, sizeof (struct s__index));
	if (!(index->tree = s__index_tree_open())) {
		s__index_close(index);
		S__TRACE(0);
		return NULL;
	}
	return index;
}

void
s__index_close(s__index_t index)
{
	if (index) {
		s__index_tree_close(index->tree);
		s__index_succinct_close(index->succinct);
		memset(index, 0, sizeof (struct s__index));
	}
	S__FREE(index);
}

void
s__index_truncate(s__index_t index)
{
	assert( index );

	s__index_tree_truncate(index->tree);
	s__index_succinct_close(index->succinct);
	index->succinct = NULL;
}

int
s__index_compress(s__index_t index)
{
	s__index_ternary_t ternary;

	assert( index );
	assert( !index->succinct );

	if (!(ternary = s__index_ternary_open(index->tree))) {
		S__TRACE(0);
		return -1;
	}
	s__index_tree_truncate(index->tree);
	if (!(index->succinct = s__index_succinct_open(ternary))) {
		S__TRACE(0);
		return -1;
	}
	s__index_ternary_close(ternary);
	return 0;
}

uint64_t *
s__index_update(s__index_t index, const char *key)
{
	uint64_t *record;

	assert( index );
	assert( !index->succinct );
	assert( s__strlen(key) );
	assert( S__INDEX_MAX_KEY_LEN > s__strlen(key) );

	if (!(record = s__index_tree_update(index->tree, key))) {
		S__TRACE(0);
		return NULL;
	}
	return record;
}

uint64_t *
s__index_find(s__index_t index, const char *key)
{
	assert( index );
	assert( s__strlen(key) );

	if (index->succinct) {
		return s__index_succinct_find(index->succinct, key);
	}
	return s__index_tree_find(index->tree, key);
}

uint64_t *
s__index_next(s__index_t index, const char *key, char *okey)
{
	assert( index );
	assert( okey );

	if (index->succinct) {
		return s__index_succinct_next(index->succinct, key, okey);
	}
	return s__index_tree_next(index->tree, key, okey);
}

uint64_t *
s__index_prev(s__index_t index, const char *key, char *okey)
{
	assert( index );
	assert( okey );

	if (index->succinct) {
		return s__index_succinct_prev(index->succinct, key, okey);
	}
	return s__index_tree_prev(index->tree, key, okey);
}

uint64_t
s__index_items(s__index_t index)
{
	assert( index );

	if (index->succinct) {
		return s__index_succinct_items(index->succinct);
	}
	return s__index_tree_items(index->tree);
}
