/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_spinlock.c
 */

#include <sched.h>

#include "s_core.h"
#include "s_spinlock.h"

void
s__spinlock_lock(volatile s__spinlock_t *lock)
{
	assert( lock );

	while (!__sync_bool_compare_and_swap(lock, 0, 1)) {
		sched_yield();
	}
}

void
s__spinlock_unlock(volatile s__spinlock_t *lock)
{
	assert( lock );

	while (!__sync_bool_compare_and_swap(lock, 1, 0)) {
		sched_yield();
	}
}
