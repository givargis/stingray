/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_core.c
 */

#define _GNU_SOURCE

#include <sys/time.h>
#include <unistd.h>

#include "s_spinlock.h"
#include "s_term.h"
#include "s_core.h"

static int _notrace_;

void
s__core_init(int notrace)
{
	_notrace_ = notrace ? 1 : 0;
	if ((1 != sizeof (char)) ||
	    (2 != sizeof (short)) ||
	    (4 != sizeof (int)) ||
	    (8 != sizeof (long)) ||
	    (4 != sizeof (float)) ||
	    (8 != sizeof (double)) ||
	    (8 != sizeof (size_t)) ||
	    (8 != sizeof (void *)) ||
	    (8 != sizeof (void (*)(void)))) {
		S__HALT(S__ERR_ARCHITECTURE);
	}
}

void
s__sprintf(char *buf, uint64_t len, const char *format, ...)
{
	va_list ap;

	assert( (!len || buf) && format );

	va_start(ap, format);
	if ((int)len <= vsnprintf(buf, len, format, ap)) {
		S__HALT(S__ERR_SOFTWARE);
	}
	va_end(ap);
}

void
s__log(const char *format, ...)
{
	static s__spinlock_t lock;
	struct tm *tm;
	va_list ap;
	time_t t;

	assert( format );

	/* M.E. -> */ s__spinlock_lock(&lock);

	t = time(NULL);
	tm = gmtime(&t);
	if (strncmp(format, "trace:", 6) || !_notrace_) {
		s__term_bold();
		printf("[%02d-%02d-%d %02d:%02d:%02d]: ",
		       tm->tm_mon + 1,
		       tm->tm_mday,
		       tm->tm_year + 1900,
		       tm->tm_hour,
		       tm->tm_min,
		       tm->tm_sec);
		if (!strncmp(format, "trace:", 6)) {
			format += 6;
			s__term_color(S__TERM_COLOR_CYAN);
			printf("trace:");
		}
		else if (!strncmp(format, "error:", 6)) {
			format += 6;
			s__term_color(S__TERM_COLOR_RED);
			printf("error:");
		}
		else if (!strncmp(format, "warning:", 8)) {
			format += 8;
			s__term_color(S__TERM_COLOR_YELLOW);
			printf("warning:");
		}
		else if (!strncmp(format, "info:", 5)) {
			format += 5;
			s__term_color(S__TERM_COLOR_BLUE);
			printf("info:");
		}
		s__term_reset();
		s__term_bold();
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
		s__term_reset();
		printf("\n");
		fflush(stdout);
	}

	/* M.E. <- */ s__spinlock_unlock(&lock);
}

void
s__unlink(const char *pathname)
{
	assert( s__strlen(pathname) );

	if (unlink(pathname)) {
		/* ignore */
	}
}

void
s__usleep(uint64_t us)
{
	struct timespec in, out;

	in.tv_sec = (time_t)(us / 1000000);
	in.tv_nsec = (long)(us % 1000000) * 1000;
	while (nanosleep(&in, &out)) {
		in = out;
	}
}

void *
s__malloc(uint64_t n)
{
	void *p;

	assert( n );

	if (!(p = malloc(n))) {
		S__HALT(S__ERR_MEMORY);
		return NULL;
	}
	return p;
}

void *
s__realloc(void *m, uint64_t n)
{
	void *p;

	assert( n );

	if (!(p = realloc(m, n))) {
		S__TRACE(S__ERR_MEMORY);
		return NULL;
	}
	return p;
}

char *
s__strdup(const char *s)
{
	uint64_t n;
	char *p;

	n = s__strlen(s);
	if (!(p = s__malloc(n + 1))) {
		S__TRACE(0);
		return NULL;
	}
	memcpy(p, s, n);
	p[n] = '\0';
	return p;
}

uint64_t
s__time(void)
{
	struct timeval timeval;

	if (gettimeofday(&timeval, 0)) {
		S__HALT(S__ERR_SYSTEM);
		return 0;
	}
	return (uint64_t)timeval.tv_sec * 1000000 + (uint64_t)timeval.tv_usec;
}

uint64_t
s__cores(void)
{
	long cores;

	if (0 >= (cores = sysconf(_SC_NPROCESSORS_ONLN))) {
		S__HALT(S__ERR_SYSTEM);
		return 0;
	}
	return (uint64_t)cores;
}

int
s__endian(void)
{
	uint32_t sample;
	uint8_t *probe;

	sample = 0x87654321;
	probe = (uint8_t *)&sample;
	if ((0x21 == probe[0])) {
		if ((0x43 != probe[1]) ||
		    (0x65 != probe[2]) ||
		    (0x87 != probe[3])) {
			S__HALT(S__ERR_ARCHITECTURE);
		}
		return 0; /* little */
	}
	return 1; /* big */
}

const char *
s__errstr(int e)
{
	switch (e) {
	case 0: return "^";
	case S__ERR_SYSTEM: return "system";
	case S__ERR_MEMORY: return "memory";
	case S__ERR_SYNTAX: return "syntax";
	case S__ERR_SOFTWARE: return "software";
	case S__ERR_ARGUMENT: return "argument";
	case S__ERR_CHECKSUM: return "checksum";
	case S__ERR_ARITHMETIC: return "arithmetic";
	case S__ERR_ARCHITECTURE: return "architecture";
	case S__ERR_FILE_OPEN: return "file open";
	case S__ERR_FILE_READ: return "file read";
	case S__ERR_FILE_WRITE: return "file write";
	case S__ERR_NETWORK_READ: return "network read";
	case S__ERR_NETWORK_WRITE: return "network write";
	case S__ERR_NETWORK_ADDRESS: return "network address";
	case S__ERR_NETWORK_CONNECT: return "network connect";
	case S__ERR_NETWORK_INTERFACE: return "network interface";
	}
	return "?";
}
