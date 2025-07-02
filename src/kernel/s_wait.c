/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_wait.c
 */

#include "s_core.h"

static int _sigint_;

static void
_signal_(int signum)
{
	if (SIGINT == signum) {
		__sync_fetch_and_add(&_sigint_, 1);
		printf("\r  \r");
	}
}

void
s__wait(void)
{
	_sigint_ = 0;
	if ((SIG_ERR == signal(SIGHUP, _signal_)) ||
	    (SIG_ERR == signal(SIGPIPE, _signal_)) ||
	    (SIG_ERR == signal(SIGINT, _signal_))) {
		S__HALT(S__ERR_SYSTEM);
	}
	while (!__sync_fetch_and_add(&_sigint_, 0)) {
		s__usleep(500000);
	}
	if ((SIG_ERR == signal(SIGHUP, SIG_DFL)) ||
	    (SIG_ERR == signal(SIGPIPE, SIG_DFL)) ||
	    (SIG_ERR == signal(SIGINT, SIG_DFL))) {
		S__HALT(S__ERR_SYSTEM);
	}
}
