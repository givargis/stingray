/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_jitc.c
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>
#include <dlfcn.h>

#include "s_core.h"
#include "s_jitc.h"

struct s__jitc {
	void *handle;
};

int
s__jitc_compile(const char *input, const char *output)
{
	extern char **environ;
	const char *argv[] = {
		"<cc>",
		"-O3",
		"-fPIC",
		"-shared",
		"-o",
		"<output>",
		"<input>",
		NULL
	};
	pid_t pid, pid_;
	int status;

	assert( s__strlen(input) );
	assert( s__strlen(output) );

	argv[0] = "/usr/bin/gcc";
	argv[S__ARRAY_SIZE(argv) - 2] = input;
	argv[S__ARRAY_SIZE(argv) - 3] = output;
	if (posix_spawn(&pid,
			argv[0],
			NULL,
			NULL,
			(char * const *)argv,
			environ)) {
		S__TRACE(S__ERR_SYSTEM);
		return -1;
	}
	for (;;) {
		errno = 0;
		pid_ = waitpid(pid, &status, 0);
		if ((-1 == pid_) && (EINTR == errno)) {
			continue;
		}
		if ((-1 == pid_) || (pid_ != pid) || !WIFEXITED(status)) {
			S__HALT(S__ERR_SOFTWARE);
			return -1;
		}
		if (WEXITSTATUS(status)) {
			S__TRACE(S__ERR_SYSTEM);
			return -1;
		}
		break;
	}
	return 0;
}

s__jitc_t
s__jitc_open(const char *pathname)
{
	struct s__jitc *jitc;

	assert( s__strlen(pathname) );

	if (!(jitc = s__malloc(sizeof (struct s__jitc)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(jitc, 0, sizeof (struct s__jitc));
	if (!(jitc->handle = dlopen(pathname, RTLD_LAZY | RTLD_LOCAL))) {
		s__jitc_close(jitc);
		S__TRACE(S__ERR_SYSTEM);
		return NULL;
	}
	return jitc;
}

void
s__jitc_close(s__jitc_t jitc)
{
	int errno_;

	if (jitc) {
		if (jitc->handle) {
			errno_ = errno;
			dlclose(jitc->handle);
			errno = errno_;
		}
		memset(jitc, 0, sizeof (struct s__jitc));
	}
	S__FREE(jitc);
}

long
s__jitc_lookup(s__jitc_t jitc, const char *symbol)
{
	assert( jitc );
	assert( jitc->handle );
	assert( s__strlen(symbol) );

	return (long)dlsym(jitc->handle, symbol);
}
