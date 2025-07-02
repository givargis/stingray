/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_file.h
 */

#ifndef _S_FILE_H_
#define _S_FILE_H_

#include "../kernel/s_kernel.h"

const char *s__file_read(const char *pathname);

int s__file_write(const char *pathname, const char *content);

#endif /* _S_FILE_H_ */
