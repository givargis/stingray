/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_fft.h
 */

#ifndef _S_FFT_H_
#define _S_FFT_H_

#include "../kernel/s_kernel.h"

void s__fft_forward(struct s__complex *signal, int n);

void s__fft_inverse(struct s__complex *signal, int n);

int s__fft_bist(void);

#endif /* _S_FFT_H_ */
