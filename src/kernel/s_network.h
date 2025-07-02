/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_network.h
 */

#ifndef _S_NETWORK_H_
#define _S_NETWORK_H_

#include "s_core.h"

#define S__NETWORK_WRITEV_MAX_N 8

typedef struct s__network *s__network_t;

typedef void (*s__network_fnc_t)(void *ctx, s__network_t network);

s__network_t s__network_listen(const char *hostname,
			       const char *servname,
			       s__network_fnc_t fnc,
			       void *ctx);

s__network_t s__network_connect(const char *hostname, const char *servname);

void s__network_close(s__network_t network);

int s__network_read(s__network_t network, void *buf, uint64_t len);

int s__network_write(s__network_t network, const void *buf, uint64_t len);

int s__network_writev(s__network_t network, int n, ...);

#endif /* _S_NETWORK_H_ */
