/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_string.h
 */

#ifndef _S_STRING_H_
#define _S_STRING_H_

#include "../kernel/s_kernel.h"

void s__string_trim(char *s);

void s__string_lower(char *s);

void s__string_upper(char *s);

void s__string_unspace(char *s);

int s__string_bist(void);

#endif /* _S_STRING_H_ */
