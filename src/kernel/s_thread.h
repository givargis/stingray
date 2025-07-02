/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_thread.h
 */

#ifndef _S_THREAD_H_
#define _S_THREAD_H_

typedef struct s__thread *s__thread_t;
typedef struct s__mutex *s__mutex_t;
typedef struct s__cond *s__cond_t;

typedef void (*s__thread_fnc_t)(void *ctx);

s__thread_t s__thread_open(s__thread_fnc_t fnc, void *ctx);

void s__thread_close(s__thread_t thread);

int s__thread_good(s__thread_t thread);

s__mutex_t s__mutex_open(void);

void s__mutex_close(s__mutex_t mutex);

void s__mutex_lock(s__mutex_t mutex);

void s__mutex_unlock(s__mutex_t mutex);

s__cond_t s__cond_open(s__mutex_t mutex);

void s__cond_close(s__cond_t cond);

void s__cond_signal(s__cond_t cond);

void s__cond_wait(s__cond_t cond);

#endif /* _S_THREAD_H_ */
