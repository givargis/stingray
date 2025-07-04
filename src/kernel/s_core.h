/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_core.h
 */

#ifndef _S_CORE_H_
#define _S_CORE_H_

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#define S__INLINE static __inline__

#define S__PI ( (s__real)3.14159265358979323846264338327950288 )

#define S__MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define S__MAX(a,b) ( ((a) > (b)) ? (a) : (b) )
#define S__DUP(a,b) ( (0 == ((a) % (b))) ? ((a) / (b)) : ((a) / (b) + 1) )

#define S__ARRAY_SIZE(a) ( sizeof ((a)) / sizeof ((a)[0]) )

#define S__UNUSED(x)				\
	do {					\
		(void)(x);			\
	} while (0)

#define S__TRACE(e)				\
	do {					\
		s__log("trace: %s:%d: %s",	\
		       __FILE__,		\
		       __LINE__,		\
		       s__errstr((e)));		\
		if ((e)) {			\
			errno = (e);		\
		}				\
	} while(0)

#define S__HALT(e)				\
	do {					\
		S__TRACE((e));			\
		s__log("error: ** HALT **");	\
		exit(-1);			\
	} while (0)

#define S__FREE(m)				\
	do {					\
		if ((m)) {			\
			free((void *)(m));	\
			(m) = NULL;		\
		}				\
	}					\
	while (0)

enum {
	S__ERR_SYSTEM = -100,
	S__ERR_MEMORY,
	S__ERR_SYNTAX,
	S__ERR_SOFTWARE,
	S__ERR_ARGUMENT,
	S__ERR_CHECKSUM,
	S__ERR_ARITHMETIC,
	S__ERR_ARCHITECTURE,
	S__ERR_FILE_OPEN,
	S__ERR_FILE_READ,
	S__ERR_FILE_WRITE,
	S__ERR_NETWORK_READ,
	S__ERR_NETWORK_WRITE,
	S__ERR_NETWORK_ADDRESS,
	S__ERR_NETWORK_CONNECT,
	S__ERR_NETWORK_INTERFACE
};

typedef double s__real;

struct s__complex {
	s__real r;
	s__real i;
};

void s__core_init(int notrace);

void s__sprintf(char *buf, uint64_t len, const char *format, ...);

void s__log(const char *format, ...);

void s__usleep(uint64_t us);

void s__unlink(const char *pathname);

void *s__malloc(uint64_t n);

void *s__realloc(void *m, uint64_t n);

char *s__strdup(const char *s);

uint64_t s__time(void);

uint64_t s__cores(void);

int s__endian(void); /* 0 -> little, 1 -> big */

const char *s__errstr(int e);

S__INLINE uint64_t
s__strlen(const char *s)
{
	return s ? (uint64_t)strlen(s) : 0;
}

S__INLINE uint64_t
s__popcount(uint64_t x)
{
	return __builtin_popcountll(x);
}

S__INLINE void
s__atomic_add(volatile uint64_t *a, int64_t b)
{
	__sync_add_and_fetch(a, b);
}

S__INLINE void *
s__align(void *p, uint64_t n)
{
	uint64_t r;

	if ((r = (uint64_t)p % n)) {
		r = n - r;
	}
	return (void *)((char *)p + r);
}

S__INLINE int
s__is_zero(const void *buf, uint64_t len)
{
	assert( buf && len );

	if (*((const char *)buf)) {
		return 0;
	}
	return !memcmp(buf, ((const char *)buf) + 1, len - 1);
}

#endif /* _S_CORE_H_ */
