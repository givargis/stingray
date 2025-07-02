/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_base64.h
 */

#ifndef _S_BASE64_H_
#define _S_BASE64_H_

#include "../kernel/s_kernel.h"

#define S__BASE64_ENCODE_LEN(n) ( S__DUP((n), 3) * 4 )
#define S__BASE64_DECODE_LEN(n) ( S__DUP((n), 4) * 3 )

void s__base64_encode(const void *buf, uint64_t len, char *s);

int s__base64_decode(void *buf, uint64_t *len, const char *s);

int s__base64_bist(void);

#endif /* _S_BASE64_H_ */
