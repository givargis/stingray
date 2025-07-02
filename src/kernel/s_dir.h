/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_dir.h
 */

#ifndef _S_DIR_H_
#define _S_DIR_H_

typedef int (*s__dir_fnc_t)(void *ctx, const char *pathname);

int s__dir(const char *pathname, s__dir_fnc_t fnc, void *ctx);

#endif /* _S_DIR_H_ */
