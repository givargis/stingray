/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_table.h
 */

#ifndef _S_TABLE_H_
#define _S_TABLE_H_

#include "../kernel/s_kernel.h"

typedef struct s__table *s__table_t;

s__table_t s__table_open(uint64_t rows, uint64_t cols);

void s__table_close(s__table_t table);

int s__table_insert(s__table_t table,
		    uint64_t row,
		    uint64_t col,
		    const char *val);

void s__table_print(s__table_t table);

int s__table_bist(void);

#endif /* _S_TABLE_H_ */
