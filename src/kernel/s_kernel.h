/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_kernel.h
 */

#ifndef _S_KERNEL_H_
#define _S_KERNEL_H_

#include "s_core.h"
#include "s_dir.h"
#include "s_file.h"
#include "s_jitc.h"
#include "s_network.h"
#include "s_spinlock.h"
#include "s_term.h"
#include "s_thread.h"
#include "s_wait.h"

void s__kernel_init(int notrace, int nocolor);

#endif /* _S_KERNEL_H_ */
