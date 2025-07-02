/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_string.c
 */

#include "s_string.h"

void
s__string_trim(char *s)
{
	const char *b, *e;
	uint64_t n;

	assert( s );

	b = s;
	e = s + s__strlen(s) - 1;
	while ((b <= e) && isspace((unsigned char)(*b))) {
		++b;
	}
	while ((e >= s) && isspace((unsigned char)(*e))) {
		--e;
	}
	n = (b <= e) ? (e - b + 1) : 0;
	memmove(s, b, n);
	s[n] = '\0';
}

void
s__string_lower(char *s)
{
	assert( s );

	while (*s) {
		(*s) = tolower(*s);
		++s;
	}
}

void
s__string_upper(char *s)
{
	assert( s );

	while (*s) {
		(*s) = toupper(*s);
		++s;
	}
}

void
s__string_unspace(char *s)
{
	assert( s );

	while (*s) {
		if (isspace((unsigned char)(*s))) {
			memmove(s, s + 1, s__strlen(s + 1) + 1);
		}
		else {
		      	++s;
		}
	}
}

int
s__string_bist(void)
{
	char buf[1024];
	int i;

	/* trim */

	s__sprintf(buf, sizeof (buf), "test");
	s__string_trim(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " test");
	s__string_trim(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "test ");
	s__string_trim(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " test ");
	s__string_trim(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "te st");
	s__string_trim(buf);
	if (strcmp(buf, "te st")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " te st");
	s__string_trim(buf);
	if (strcmp(buf, "te st")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "te st ");
	s__string_trim(buf);
	if (strcmp(buf, "te st")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " te st ");
	s__string_trim(buf);
	if (strcmp(buf, "te st")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "t");
	s__string_trim(buf);
	if (strcmp(buf, "t")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "  t");
	s__string_trim(buf);
	if (strcmp(buf, "t")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "t  ");
	s__string_trim(buf);
	if (strcmp(buf, "t")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "  t  ");
	s__string_trim(buf);
	if (strcmp(buf, "t")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* lower */

	memset(buf, 0, sizeof (buf));
	for (i=0; i<26; ++i) {
		if (0 == (rand() % 2)) {
			buf[i] = 'a' + i;
		}
		else {
			buf[i] = 'A' + i;
		}
	}
	s__string_lower(buf);
	for (i=0; i<26; ++i) {
		if (('a' + i) != buf[i]) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}

	/* upper */

	memset(buf, 0, sizeof (buf));
	for (i=0; i<26; ++i) {
		if (0 == (rand() % 2)) {
			buf[i] = 'a' + i;
		}
		else {
			buf[i] = 'A' + i;
		}
	}
	s__string_upper(buf);
	for (i=0; i<26; ++i) {
		if (('A' + i) != buf[i]) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}

	/* unspace */

	s__sprintf(buf, sizeof (buf), "te st");
	s__string_unspace(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " te st");
	s__string_unspace(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), "te st ");
	s__string_unspace(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__sprintf(buf, sizeof (buf), " te st ");
	s__string_unspace(buf);
	if (strcmp(buf, "test")) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	return 0;
}
