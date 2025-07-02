/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_hash.c
 */

#include "s_hash.h"

#define MIX(a,b,c,d)							\
	do {								\
		(c) = ((c) << 50)|((c) >> 14); (c) += (d); (a) ^= (c);	\
		(d) = ((d) << 52)|((d) >> 12); (d) += (a); (b) ^= (d);	\
		(a) = ((a) << 30)|((a) >> 34); (a) += (b); (c) ^= (a);	\
		(b) = ((b) << 41)|((b) >> 23); (b) += (c); (d) ^= (b);	\
		(c) = ((c) << 54)|((c) >> 10); (c) += (d); (a) ^= (c);	\
		(d) = ((d) << 48)|((d) >> 16); (d) += (a); (b) ^= (d);	\
		(a) = ((a) << 38)|((a) >> 26); (a) += (b); (c) ^= (a);	\
		(b) = ((b) << 37)|((b) >> 27); (b) += (c); (d) ^= (b);	\
		(c) = ((c) << 62)|((c) >>  2); (c) += (d); (a) ^= (c);	\
		(d) = ((d) << 34)|((d) >> 30); (d) += (a); (b) ^= (d);	\
		(a) = ((a) <<  5)|((a) >> 59); (a) += (b); (c) ^= (a);	\
		(b) = ((b) << 36)|((b) >> 28); (b) += (c); (d) ^= (b);	\
	} while (0)

uint64_t
s__hash(const void *buf, uint64_t len)
{
	const uint64_t MAGIC = 0xde79d3747be356ce;
	uint64_t a, b, c, d;
	uint64_t n, i, q;
	uint64_t u64[4];
	uint32_t u32[1];
	uint8_t u8[4];
	const char *p;

	assert( !len || buf );

	/* initialize */

	n = len;
	p = (const char *)buf;
	a = b = c = d = n + MAGIC;

	/* quads */

	q = n / 32;
	n = n % 32;
	for (i=0; i<q; ++i) {
		memcpy(&u64[0], p, sizeof (u64[0])); p += sizeof (u64[0]);
		memcpy(&u64[1], p, sizeof (u64[1])); p += sizeof (u64[1]);
		memcpy(&u64[2], p, sizeof (u64[2])); p += sizeof (u64[2]);
		memcpy(&u64[3], p, sizeof (u64[3])); p += sizeof (u64[3]);
		a += u64[0];
		b += u64[1];
		c += u64[2];
		d += u64[3];
		MIX(a, b, c, d);
	}

	/* words */

	q = n / 16;
	n = n % 16;
	for (i=0; i<q; ++i) {
		memcpy(&u64[0], p, sizeof (u64[0])); p += sizeof (u64[0]);
		memcpy(&u64[1], p, sizeof (u64[1])); p += sizeof (u64[1]);
		a += u64[0];
		b += u64[1];
		MIX(a, b, c, d);
	}

	/* bytes */

	if (12 <= n) {
		memcpy(&u64[0], p, sizeof (u64[0])); p += sizeof (u64[0]);
		memcpy(&u32[0], p, sizeof (u32[0])); p += sizeof (u32[0]);
		memcpy(u8, p, n - 12);
		memset(u8 + n - 12, 0, 15 - n);
		c += u64[0];
		d += u32[0];
		d += (uint64_t)u8[0] << 32;
		d += (uint64_t)u8[1] << 40;
		d += (uint64_t)u8[2] << 48;
	}
	else if (8 <= n) {
		memcpy(&u64[0], p, sizeof (u64[0])); p += sizeof (u64[0]);
		memcpy(u8, p, n - 8);
		memset(u8 + n - 8, 0, 11 - n);
		c += u64[0];
		d += (uint64_t)u8[0];
		d += (uint64_t)u8[1] <<  8;
		d += (uint64_t)u8[2] << 16;
	}
	else if (4 <= n) {
		memcpy(&u32[0], p, sizeof (u32[0])); p += sizeof (u32[0]);
		memcpy(u8, p, n - 4);
		memset(u8 + n - 4, 0, 7 - n);
		c += u32[0];
		c += (uint64_t)u8[0] << 32;
		c += (uint64_t)u8[1] << 40;
		c += (uint64_t)u8[2] << 48;
	}
	else {
		memcpy(u8, p, n);
		memset(u8 + n, 0, 3 - n);
		c += (uint64_t)u8[0];
		c += (uint64_t)u8[1] <<  8;
		c += (uint64_t)u8[2] << 16;
		c += MAGIC;
		d += MAGIC;
	}

	/* output */

	MIX(a, b, c, d);
	return a + b + c + d;
}

int
s__hash_bist(void)
{
	uint64_t i, j, n, len, hash;
	double stats[64];
	char buf[128];

	n = 10000;
	memset(stats, 0, sizeof (stats));
	for (i=0; i<n; ++i) {
		for (j=0; j<(sizeof (buf)); ++j) {
			buf[j] = (char)rand();
		}
		len = (uint64_t)rand() % (sizeof (buf));
		hash = s__hash(buf, len);
		for (j=0; j<64; ++j) {
			if (hash & ((uint64_t)1 << j)) {
				stats[j] += 1.0;
			}
		}
	}
	for (i=0; i<64; ++i) {
		stats[i] /= n;
		if ((0.52 < stats[i]) || (0.48 > stats[i])) {
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	return 0;
}
