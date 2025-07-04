/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_fft.c
 */

#include "s_fft.h"

static void
fft(struct s__complex *v, struct s__complex *t, int n)
{
	struct s__complex z, w, *vo, *ve;
	int i, k;

	if ((k = n / 2)) {
		ve = t;
		vo = t + k;
		for(i=0; i<k; i++) {
			ve[i] = v[2 * i];
			vo[i] = v[2 * i + 1];
		}
		fft(ve, v, k);
		fft(vo, v, k);
		for(i=0; i<k; i++) {
			w.r = cos(2 * S__PI * i / (s__real)n);
			w.i = -sin(2 * S__PI * i / (s__real)n);
			z.r = w.r * vo[i].r - w.i * vo[i].i;
			z.i = w.r * vo[i].i + w.i * vo[i].r;
			v[i].r = ve[i].r + z.r;
			v[i].i = ve[i].i + z.i;
			v[i + k].r = ve[i].r - z.r;
			v[i + k].i = ve[i].i - z.i;
		}
	}
}

static void
ifft(struct s__complex *v, struct s__complex *t, int n)
{
	struct s__complex z, w, *vo, *ve;
	int i, k;

	if ((k = n / 2)) {
		k = n / 2;
		ve = t;
		vo = t + k;
		for(i=0; i<k; i++) {
			ve[i] = v[2 * i];
			vo[i] = v[2 * i + 1];
		}
		ifft(ve, v, k);
		ifft(vo, v, k);
		for(i=0; i<k; i++) {
			w.r = cos(2 * S__PI * i / (s__real)n);
			w.i = sin(2 * S__PI * i / (s__real)n);
			z.r = w.r * vo[i].r - w.i * vo[i].i;
			z.i = w.r * vo[i].i + w.i * vo[i].r;
			v[i].r = ve[i].r + z.r;
			v[i].i = ve[i].i + z.i;
			v[i + k].r = ve[i].r - z.r;
			v[i + k].i = ve[i].i - z.i;
		}
	}
}

void
s__fft_forward(struct s__complex *signal, int n)
{
	assert( signal );
	assert( n && (0 == (n % 2)) );

	fft(signal, signal + n, n);
}

void
s__fft_inverse(struct s__complex *signal, int n)
{
	int i;

	assert( signal );
	assert( n && (0 == (n % 2)) );

	for (i=0; i<n; ++i) {
		signal[i].r /= n;
		signal[i].i /= n;
	}
	ifft(signal, signal + n, n);
}

int
s__fft_bist(void)
{
	struct s__complex signal[8192 * 2], signal_[8192 * 2];
	int i, n;

	n = S__ARRAY_SIZE(signal) / 2;
	for (i=0; i<n; ++i) {
		signal[i].r = .5 - (rand() / (s__real)RAND_MAX) * 1.0;
		signal[i].i = 0.0;
		signal_[i] = signal[i];
	}
	s__fft_forward(signal, n);
	s__fft_inverse(signal, n);
	for (i=0; i<n; ++i) {
		if ((0.001 < fabs(signal[i].r - signal_[i].r)) ||
		    (0.001 < fabs(signal[i].i - signal_[i].i))) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	return 0;
}
