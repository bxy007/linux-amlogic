/* Local compatibility for the audited integer-only STB 2.4 curve port. */
#ifndef DV_STB24_TYPES_H
#define DV_STB24_TYPES_H
#ifdef DV_STB24_HOST_TEST
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef int64_t s64;
#else
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/math64.h>
#endif
#ifndef UINT64_C
#define UINT64_C(v) v##ULL
#endif
#ifndef INT64_C
#define INT64_C(v) v##LL
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffffU
#endif
#ifndef UINT64_MAX
#define UINT64_MAX 0xffffffffffffffffULL
#endif
#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif
#ifndef INT32_MIN
#define INT32_MIN (-INT32_MAX - 1)
#endif

static inline u64 dv_stb24_udiv64(u64 numerator, u64 denominator)
{
	if (!denominator)
		return 0;
#ifdef DV_STB24_HOST_TEST
	return numerator / denominator;
#else
	return div64_u64(numerator, denominator);
#endif
}

/* Raw AArch64 SDIV semantics, including zero and INT64_MIN / -1. */
static inline u64 dv_stb24_raw_div64(u64 numerator, u64 denominator)
{
	u64 a = numerator >> 63 ? 0ULL - numerator : numerator;
	u64 b = denominator >> 63 ? 0ULL - denominator : denominator;
	u64 q = dv_stb24_udiv64(a, b);

	return (numerator ^ denominator) >> 63 ? 0ULL - q : q;
}
#endif
