/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_term.c
 */

#include "s_core.h"
#include "s_term.h"

static int _nocolor_;

void
s__term_init(int nocolor)
{
	_nocolor_ = nocolor ? 1 : 0;
}

void
s__term_color(int color)
{
	assert( (0 <= color) && (7 >= color) );

	if (!_nocolor_) {
		printf("\033[%dm", 30 + color);
		fflush(stdout);
	}
}

void
s__term_bold(void)
{
	if (!_nocolor_) {
		printf("\033[1m");
		fflush(stdout);
	}
}

void
s__term_reset(void)
{
	if (!_nocolor_) {
		printf("\033[?25h");
		printf("\033[0m");
		fflush(stdout);
	}
}
