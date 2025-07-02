/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_spinlock.h
 */

#ifndef _S_SPINLOCK_H_
#define _S_SPINLOCK_H_

typedef int s__spinlock_t;

void s__spinlock_lock(volatile s__spinlock_t *lock);

void s__spinlock_unlock(volatile s__spinlock_t *lock);

#endif /* _S_SPINLOCK_H_ */
