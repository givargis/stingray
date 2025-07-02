/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_file.c
 */

#include "s_file.h"

const char *
s__file_read(const char *pathname)
{
	const uint64_t PAD = 8;
	char *content;
	FILE *file;
	long size;

	assert( s__strlen(pathname) );

	if (!(file = fopen(pathname, "r"))) {
		S__TRACE(S__ERR_FILE_OPEN);
		return NULL;
	}
	if (fseek(file, 0, SEEK_END) ||
	    (0 > (size = ftell(file))) ||
	    fseek(file, 0, SEEK_SET)) {
		fclose(file);
		S__TRACE(S__ERR_FILE_READ);
		return NULL;
	}
	if (!(content = s__malloc((uint64_t)size + PAD))) {
		fclose(file);
		S__TRACE(0);
		return NULL;
	}
	if (size && (1 != fread(content, (size_t)size, 1, file))) {
		fclose(file);
		S__FREE(content);
		S__TRACE(S__ERR_FILE_READ);
		return NULL;
	}
	fclose(file);
	memset(content + size, 0, PAD);
	return content;
}

int
s__file_write(const char *pathname, const char *content)
{
	uint64_t size;
	FILE *file;

	assert( s__strlen(pathname) );
	assert( content );

	size = s__strlen(content);
	if (!(file = fopen(pathname, "w"))) {
		S__TRACE(S__ERR_FILE_OPEN);
		return -1;
	}
	if (size && (1 != fwrite(content, (size_t)size, 1, file))) {
		fclose(file);
		s__unlink(pathname);
		S__TRACE(S__ERR_FILE_WRITE);
		return -1;
	}
	fclose(file);
	return 0;
}
