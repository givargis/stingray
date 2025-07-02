/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_dir.c
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "s_core.h"
#include "s_dir.h"

static int
_dir_(const char *pathname, s__dir_fnc_t fnc, void *ctx)
{
	struct dirent *dirent;
	struct stat stat_;
	char *pathname_;
	uint64_t n;
	DIR *dir;

	if (stat(pathname, &stat_) ||
	    (!S_ISDIR(stat_.st_mode) && !S_ISREG(stat_.st_mode))) {
		return 0;
	}
	if (S_ISREG(stat_.st_mode)) {
		if (fnc(ctx, pathname)) {
			S__TRACE(0);
			return -1;
		}
		return 0;
	}
	if (!(dir = opendir(pathname))) {
		S__TRACE(S__ERR_SYSTEM);
		return -1;
	}
	while ((dirent = readdir(dir))) {
		if (!strcmp(dirent->d_name, ".") ||
		    !strcmp(dirent->d_name, "..")) {
			continue;
		}
		n = s__strlen(pathname) + s__strlen(dirent->d_name) + 2;
		if (!(pathname_ = s__malloc(n))) {
			closedir(dir);
			S__TRACE(0);
			return -1;
		}
		s__sprintf(pathname_, n, "%s/%s", pathname, dirent->d_name);
		if (_dir_(pathname_, fnc, ctx)) {
			closedir(dir);
			S__FREE(pathname_);
			S__TRACE(0);
			return -1;
		}
		S__FREE(pathname_);
	}
	closedir(dir);
	return 0;
}

int
s__dir(const char *pathname, s__dir_fnc_t fnc, void *ctx)
{
	assert( s__strlen(pathname) );
	assert( fnc );

	if (_dir_(pathname, fnc, ctx)) {
		S__TRACE(0);
		return -1;
	}
	return 0;
}
