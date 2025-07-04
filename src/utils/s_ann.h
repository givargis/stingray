/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_ann.h
 */

#ifndef _S_ANN_H_
#define _S_ANN_H_

#include "../kernel/s_kernel.h"

typedef struct s__ann *s__ann_t;

s__ann_t s__ann_open(int input, int output, int hidden, int layers);

void s__ann_close(s__ann_t ann);

const s__real *s__ann_activate(s__ann_t ann, const s__real *x);

s__real s__ann_train(s__ann_t ann,
		     const s__real *x,
		     const s__real *y,
		     s__real eta,
		     int k);

int s__ann_bist(void);

#endif /* _S_ANN_H_ */
