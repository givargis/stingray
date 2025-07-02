/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_thread.c
 */

#include <pthread.h>

#include "s_core.h"
#include "s_thread.h"

struct s__thread {
	int good;
	void *ctx;
	pthread_t pthread;
	s__thread_fnc_t fnc;
};

struct s__mutex {
	pthread_mutex_t mutex;
};

struct s__cond {
	s__mutex_t mutex;
	pthread_cond_t cond;
};

static void *
_thread_(void *ctx)
{
	struct s__thread *thread;

	assert( ctx );

	thread = (struct s__thread *)ctx;
	thread->fnc(thread->ctx);
	return NULL;
}

s__thread_t
s__thread_open(s__thread_fnc_t fnc, void *ctx)
{
	struct s__thread *thread;

	assert( fnc );

	if (!(thread = s__malloc(sizeof (struct s__thread)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(thread, 0, sizeof (struct s__thread));
	thread->ctx = ctx;
	thread->fnc = fnc;
	if (pthread_create(&thread->pthread, NULL, _thread_, thread)) {
		s__thread_close(thread);
		S__HALT(S__ERR_SYSTEM);
		return NULL;
	}
	thread->good = 1;
	return thread;
}

void
s__thread_close(s__thread_t thread)
{
	if (thread) {
		if (thread->good) {
			pthread_join(thread->pthread, NULL);
		}
		memset(thread, 0, sizeof (struct s__thread));
	}
	S__FREE(thread);
}

int
s__thread_good(s__thread_t thread)
{
	return thread && thread->good;
}

s__mutex_t
s__mutex_open(void)
{
	struct s__mutex *mutex;

	if (!(mutex = s__malloc(sizeof (struct s__mutex)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(mutex, 0, sizeof (struct s__mutex));
	if (pthread_mutex_init(&mutex->mutex, NULL)) {
		s__mutex_close(mutex);
		S__HALT(S__ERR_SYSTEM);
		return NULL;
	}
	return mutex;
}

void
s__mutex_close(s__mutex_t mutex)
{
	if (mutex) {
		pthread_mutex_destroy(&mutex->mutex);
		memset(mutex, 0, sizeof (struct s__mutex));
	}
	S__FREE(mutex);
}

void
s__mutex_lock(s__mutex_t mutex)
{
	assert( mutex );

	if (pthread_mutex_lock(&mutex->mutex)) {
		S__HALT(S__ERR_SYSTEM);
	}
}

void
s__mutex_unlock(s__mutex_t mutex)
{
	assert( mutex );

	if (pthread_mutex_unlock(&mutex->mutex)) {
		S__HALT(S__ERR_SYSTEM);
	}
}

s__cond_t
s__cond_open(s__mutex_t mutex)
{
	struct s__cond *cond;

	assert( mutex );

	if (!(cond = s__malloc(sizeof (struct s__cond)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(cond, 0, sizeof (struct s__cond));
	cond->mutex = mutex;
	if (pthread_cond_init(&cond->cond, NULL)) {
		s__cond_close(cond);
		S__HALT(S__ERR_SYSTEM);
		return NULL;
	}
	return cond;
}

void
s__cond_close(s__cond_t cond)
{
	if (cond) {
		pthread_cond_destroy(&cond->cond);
		memset(cond, 0, sizeof (struct s__cond));
	}
	S__FREE(cond);
}

void
s__cond_signal(s__cond_t cond)
{
	assert( cond );

	if (pthread_cond_signal(&cond->cond)) {
		S__HALT(S__ERR_SYSTEM);
	}
}

void
s__cond_wait(s__cond_t cond)
{
	assert( cond );

	if (pthread_cond_wait(&cond->cond, &cond->mutex->mutex)) {
		S__HALT(S__ERR_SYSTEM);
	}
}
