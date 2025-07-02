/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_kernel.c
 */

#include "s_kernel.h"

void
s__kernel_init(int notrace, int nocolor)
{
	s__core_init(notrace);
	s__term_init(nocolor);
}
