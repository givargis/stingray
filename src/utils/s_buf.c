/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_buf.c
 */

#define _DEFAULT_SOURCE

#include "s_buf.h"

struct s__buf {
	char *buf;
	uint64_t len;
};

static int
grow(struct s__buf *buf, uint64_t len)
{
	char *buf_;

	if (len > buf->len) {
		if (!(buf_ = s__realloc(buf->buf, len))) {
			S__TRACE(0);
			return -1;
		}
		buf_[buf->len] = '\0';
		buf->len = len;
		buf->buf = buf_;
	}
	return 0;
}

s__buf_t
s__buf_open(void)
{
	struct s__buf *buf;

	if (!(buf = s__malloc(sizeof (struct s__buf)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(buf, 0, sizeof (struct s__buf));
	grow(buf, 1);
	return buf;
}

void
s__buf_close(s__buf_t buf, const char **buf_)
{
	if (buf) {
		if (buf_) {
			(*buf_) = buf->buf;
			buf->buf = NULL;
		}
		S__FREE(buf->buf);
		memset(buf, 0, sizeof (struct s__buf));
	}
	S__FREE(buf);
}

int
s__buf_append(s__buf_t buf, const char *format, ...)
{
	uint64_t len;
	va_list ap;
	int n;

	assert( buf );
	assert( format );

	va_start(ap, format);
	len = s__strlen(buf->buf);
	if (buf->len <= (uint64_t)(n = vsnprintf(buf->buf + len,
						 buf->len - len,
						 format,
						 ap)) + len) {
		va_end(ap);
		if (grow(buf, len + (2 * (uint64_t)n + 1))) {
			S__TRACE(0);
			return -1;
		}
		va_start(ap, format);
		if (buf->len <= (uint64_t)vsnprintf(buf->buf + len,
						    buf->len - len,
						    format,
						    ap)) {
			va_end(ap);
			S__HALT(S__ERR_SOFTWARE);
			return -1;
		}
	}
	va_end(ap);
	return 0;
}

const char *
s__buf_buf(s__buf_t buf)
{
	assert( buf );

	return buf->buf;
}

int
s__buf_bist(void)
{
	s__buf_t buf;

	if (!(buf = s__buf_open())) {
		S__TRACE(0);
		return -1;
	}
	if (s__buf_append(buf, "")) {
		s__buf_close(buf, NULL);
		S__TRACE(0);
		return -1;
	}
	if ((NULL == s__buf_buf(buf)) || strcmp("", s__buf_buf(buf))) {
		s__buf_close(buf, NULL);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (s__buf_append(buf, "%d", 12345678)) {
		s__buf_close(buf, NULL);
		S__TRACE(0);
		return -1;
	}
	if (strcmp("12345678", s__buf_buf(buf))) {
		s__buf_close(buf, NULL);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (s__buf_append(buf, "%s", "struct s__buf")) {
		s__buf_close(buf, NULL);
		S__TRACE(0);
		return -1;
	}
	if (strcmp("12345678struct s__buf", s__buf_buf(buf))) {
		s__buf_close(buf, NULL);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__buf_close(buf, NULL);
	return 0;
}
