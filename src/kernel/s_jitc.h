/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_jitc.h
 */

#ifndef _S_JITC_H_
#define _S_JITC_H_

typedef struct s__jitc *s__jitc_t;

int s__jitc_compile(const char *input, const char *output);

s__jitc_t s__jitc_open(const char *pathname);

void s__jitc_close(s__jitc_t jitc);

long s__jitc_lookup(s__jitc_t jitc, const char *symbol);

#endif /* _S_JITC_H_ */
