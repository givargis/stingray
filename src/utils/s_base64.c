/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_base64.c
 */

#include "s_base64.h"

static const uint8_t ENCODE[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
	'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static const uint8_t DECODE[] = {
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 99, 99, 99,
	99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99,
	99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

void
s__base64_encode(const void *buf_, uint64_t len, char *s)
{
	const uint8_t *buf = (const uint8_t *)buf_;
	uint64_t q, r, i;
	uint8_t a, b, c;

	assert( buf && len && s );

	q = len / 3;
	r = len % 3;
	for (i=0; i<q; ++i) {
		a = (*(buf++));
		b = (*(buf++));
		c = (*(buf++));
		(*(s++)) = ENCODE[((0     )       ) | (a >> 2)]; /* 00aaaaaa */
		(*(s++)) = ENCODE[((a << 4) & 0x3f) | (b >> 4)]; /* 00aabbbb */
		(*(s++)) = ENCODE[((b << 2) & 0x3f) | (c >> 6)]; /* 00bbbbcc */
		(*(s++)) = ENCODE[((c     ) & 0x3f) | (0     )]; /* 00cccccc */
	}
	if (2 == r) {
		a = (*(buf++));
		b = (*(buf++));
		(*(s++)) = ENCODE[((0     )       ) | (a >> 2)]; /* 00aaaaaa */
		(*(s++)) = ENCODE[((a << 4) & 0x3f) | (b >> 4)]; /* 00aabbbb */
		(*(s++)) = ENCODE[((b << 2) & 0x3f) | (0     )]; /* 00bbbb00 */
		(*(s++)) = '=';
	}
	else if (1 == r) {
		a = (*(buf++));
		(*(s++)) = ENCODE[((0     )       ) | (a >> 2)]; /* 00aaaaaa */
		(*(s++)) = ENCODE[((a << 4) & 0x3f) | (0     )]; /* 00aa0000 */
		(*(s++)) = '=';
		(*(s++)) = '=';
	}
	(*s) = '\0';
}

int
s__base64_decode(void *buf_, uint64_t *len, const char *s)
{
	uint8_t *buf = (uint8_t *)buf_;
	uint64_t q, r, i, n;
	uint8_t a, b, c, d;

	assert( buf && len && s );

	n = s__strlen(s);
	if (0 != (n % 4)) {
		S__TRACE(S__ERR_CHECKSUM);
		return -1;
	}
	if ('=' == s[n - 1]) --n;
	if ('=' == s[n - 1]) --n;
	q = n / 4;
	r = n % 4;
	(*len) = q * 3;
	for (i=0; i<q; ++i) {
		if ((63 < (a = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (b = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (c = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (d = DECODE[(unsigned char)(*(s++))]))) {
			S__TRACE(S__ERR_CHECKSUM);
			return -1;
		}
		(*(buf++)) = (a << 2) | (b >> 4); /* aaaaaabb */
		(*(buf++)) = (b << 4) | (c >> 2); /* bbbbcccc */
		(*(buf++)) = (c << 6) | (d     ); /* ccdddddd */
	}
	if (3 == r) {
		(*len) += 2;
		if ((63 < (a = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (b = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (c = DECODE[(unsigned char)(*(s++))]))) {
			S__TRACE(S__ERR_CHECKSUM);
			return -1;
		}
		(*(buf++)) = (a << 2) | (b >> 4); /* aaaaaabb */
		(*(buf++)) = (b << 4) | (c >> 2); /* bbbbcccc */
	}
	else if (2 == r) {
		(*len) += 1;
		if ((63 < (a = DECODE[(unsigned char)(*(s++))])) ||
		    (63 < (b = DECODE[(unsigned char)(*(s++))]))) {
			S__TRACE(S__ERR_CHECKSUM);
			return -1;
		}
		(*(buf++)) = (a << 2) | (b >> 4); /* aaaaaabb */
	}
	else if (1 == r) {
		S__TRACE(S__ERR_CHECKSUM);
		return -1;
	}
	return 0;
}

int
s__base64_bist(void)
{
	uint64_t i, j, n, len, len_;
	char *s, *buf, *buf_;

	n = 10000;
	for (i=0; i<n; ++i) {
		s = NULL;
		buf = NULL;
		buf_ = NULL;
		len = rand() % n + 1;
		if (!(s = s__malloc(S__BASE64_ENCODE_LEN(len) + 1)) ||
		    !(buf = s__malloc(len)) ||
		    !(buf_ = s__malloc(len))) {
			S__FREE(s);
			S__FREE(buf);
			S__FREE(buf_);
			S__TRACE(0);
			return -1;
		}
		for (j=0; j<len; ++j) {
			buf[j] = buf_[j] = (char)(rand() % 256);
		}
		s__base64_encode(buf, len, s);
		if (s__base64_decode(buf_, &len_, s)) {
			S__FREE(s);
			S__FREE(buf);
			S__FREE(buf_);
			S__TRACE(0);
			return -1;
		}
		if ((len != len_) || memcmp(buf, buf_, len)) {
			S__FREE(s);
			S__FREE(buf);
			S__FREE(buf_);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
		S__FREE(s);
		S__FREE(buf);
		S__FREE(buf_);
	}
	return 0;
}
