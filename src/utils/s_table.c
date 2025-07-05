/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_table.c
 */

#include "s_table.h"

struct s__table {
	uint64_t rows;
	uint64_t cols;
	uint64_t *widths;
	const char **cells;
};

static const char *
cell(const struct s__table *table, uint64_t row, uint64_t col)
{
	const char *val;

	val = table->cells[row * table->cols + col];
	return val ? val : "";
}

s__table_t
s__table_open(uint64_t rows, uint64_t cols)
{
	struct s__table *table;
	uint64_t n;

	assert( rows );
	assert( cols );

	/* initialize */

	if (!(table = s__malloc(sizeof (struct s__table)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(table, 0, sizeof (struct s__table));
	table->rows = rows;
	table->cols = cols;

	/* initialize */

	n = table->cols * sizeof (table->widths[0]);
	if (!(table->widths = s__malloc(n))) {
		s__table_close(table);
		S__TRACE(0);
		return NULL;
	}
	memset(table->widths, 0, n);

	/* initialize */

	n = table->rows * table->cols * sizeof (table->cells[0]);
	if (!(table->cells = s__malloc(n))) {
		s__table_close(table);
		S__TRACE(0);
		return NULL;
	}
	memset(table->cells, 0, n);
	return table;
}

void
s__table_close(s__table_t table)
{
	if (table) {
		S__FREE(table->cells);
		S__FREE(table->widths);
		memset(table, 0, sizeof (struct s__table));
	}
	S__FREE(table);
}

void
s__table_assign(s__table_t table, uint64_t row, uint64_t col, const char *val)
{
	assert( table );
	assert( row < table->rows );
	assert( col < table->cols );

	table->cells[row * table->cols + col] = val;
}

void
s__table_print(s__table_t table)
{
	uint64_t i, j, n;
	char format[64];

	assert( table );

	for (j=0; j<table->cols; ++j) {
		for (i=0; i<table->rows; ++i) {
			table->widths[j] =
				S__MAX(table->widths[j],
				       s__strlen(cell(table, i, j)));
		}
	}
	s__term_color(S__TERM_COLOR_MAGENTA);
	s__term_bold();
	printf("+");
	for (j=0; j<table->cols; ++j) {
		n = table->widths[j] + 2;
		for (i=0; i<n; ++i) {
			printf("-");
		}
		printf("+");
	}
	printf("\n|");
	for (j=0; j<table->cols; ++j) {
		s__term_color(S__TERM_COLOR_CYAN);
		sprintf(format, " %%%ds ", (int)table->widths[j]);
		printf(format, cell(table, 0, j) ? cell(table, 0, j) : "");
		s__term_color(S__TERM_COLOR_MAGENTA);
		s__term_bold();
		printf("|");
	}
	printf("\n+");
	for (j=0; j<table->cols; ++j) {
		n = table->widths[j] + 2;
		for (i=0; i<n; ++i) {
			printf("-");
		}
		printf("+");
	}
	for (i=1; i<table->rows; ++i) {
		printf("\n|");
		for (j=0; j<table->cols; ++j) {
			s__term_reset();
			sprintf(format, " %%%ds ", (int)table->widths[j]);
			printf(format,
			       cell(table, i, j) ?
			       cell(table, i, j) : "");
			s__term_color(S__TERM_COLOR_MAGENTA);
			s__term_bold();
			printf("|");
		}
	}
	printf("\n+");
	for (j=0; j<table->cols; ++j) {
		n = table->widths[j] + 2;
		for (i=0; i<n; ++i) {
			printf("-");
		}
		printf("+");
	}
	printf("\n");
	s__term_reset();
}

int
s__table_bist(void)
{
	return 0;
}
