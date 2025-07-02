/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_map.h
 */

#ifndef _S_LANG_MAP_H_
#define _S_LANG_MAP_H_

#include "../utils/s_utils.h"

typedef struct s__lang_map *s__lang_map_t;

s__lang_map_t s__lang_map_open(void);

void s__lang_map_close(s__lang_map_t map);

void s__lang_map_insert(s__lang_map_t map, const char *key, int op);

int s__lang_map_lookup(s__lang_map_t map, const char *b, const char *e);

#endif /* _S_LANG_MAP_H_ */
