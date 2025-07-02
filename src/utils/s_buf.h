/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_buf.h
 */

#ifndef _S_BUF_H_
#define _S_BUF_H_

#include "../kernel/s_kernel.h"

typedef struct s__buf *s__buf_t;

s__buf_t s__buf_open(void);

void s__buf_close(s__buf_t buf, const char **buf_);

int s__buf_append(s__buf_t buf, const char *format, ...);

const char *s__buf_buf(s__buf_t buf);

int s__buf_bist(void);

#endif /* _S_BUF_H_ */
