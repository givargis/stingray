/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_map.c
 */

#include "s_lang_map.h"

#define SIZE 128

struct s__lang_map {
	struct {
		int op;
		const char *key;
	} *maps;
};

s__lang_map_t
s__lang_map_open(void)
{
	struct s__lang_map *map;

	if (!(map = s__malloc(sizeof (struct s__lang_map)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(map, 0, sizeof (struct s__lang_map));
	map->maps = s__malloc(SIZE * sizeof (map->maps[0]));
	if (!map->maps) {
		s__lang_map_close(map);
		S__TRACE(0);
		return NULL;
	}
	memset(map->maps, 0, SIZE * sizeof (map->maps[0]));
	return map;
}

void
s__lang_map_close(s__lang_map_t map)
{
	if (map) {
		S__FREE(map->maps);
		memset(map, 0, sizeof (struct s__lang_map));
	}
	S__FREE(map);
}

void
s__lang_map_insert(s__lang_map_t map, const char *key, int op)
{
	uint64_t i, j;

	assert( s__strlen(key) );
	assert( op );

	j = s__hash(key, s__strlen(key));
	for (i=0; i<SIZE; ++i) {
		j = (j + 1) % SIZE;
		if (!map->maps[j].key) { /* insert */
			map->maps[j].key = key;
			map->maps[j].op = op;
			return;
		}
		if (!strcmp(map->maps[j].key, key)) { /* update */
			break;
		}
	}
	S__HALT(S__ERR_SOFTWARE);
}

int
s__lang_map_lookup(s__lang_map_t map, const char *b, const char *e)
{
	uint64_t i, j;

	assert( map );
	assert( b && (b < e) );

	j = s__hash(b, e - b);
	for (i=0; i<SIZE; ++i) {
		j = (j + 1) % SIZE;
		if (!map->maps[j].key) {
			break;
		}
		if (!strncmp(map->maps[j].key, b, e - b)) {
			return map->maps[j].op;
		}
	}
	return 0;
}
