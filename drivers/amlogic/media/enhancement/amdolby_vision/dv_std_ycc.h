/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STD_YCC_H
#define DV_STD_YCC_H

/* Default-off video diagnostic parameter builder: no hardware/LUT access.
 * Call with original, frame-matched algorithm settings and source ETSI DM.
 * This strict binary-unit/CSC-bypass path requests Core3 mode1, not mode5.
 * Its Core2 matrix covers only the linear part; GUI Y-black affine offset and
 * alpha are NOT qualified. Do not enable as a normal GUI/subtitle policy.
 */
#ifdef DV_STD_YCC_HOST_TEST
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
static inline u64 div64_u64(u64 n, u64 d) { return n / d; }
#else
#include <linux/math64.h>
#include <linux/string.h>
#include <linux/types.h>
#endif

#define DV_STD_YCC_META_MAX 512
#define DV_STD_YCC_C1_WORDS 27
#define DV_STD_YCC_C2_WORDS 24
#define DV_STD_YCC_C3_WORDS 26

enum dv_std_ycc_result {
	DV_STD_YCC_OK = 0,
	DV_STD_YCC_BAD_SOURCE = -1,
	DV_STD_YCC_BAD_MODE = -2,
	DV_STD_YCC_SOURCE_MISMATCH = -3,
	DV_STD_YCC_BAD_MATRIX = -4,
	DV_STD_YCC_BAD_POINTER = -5
};

struct dv_std_ycc_input {
	const u8 *source_meta;
	u32 source_len;
	const u32 *dm1; /* Original dm_reg_ipcore1, 27 words. */
	const u32 *dm2; /* Original dm_reg_ipcore2, 24 words. */
	const u32 *dm3; /* Original dm_reg_ipcore3, 26 words. */
	u32 core3_mode; /* Original native mode; this recipe accepts mode5 only. */
	bool low_latency;
	bool force_refresh;
};

struct dv_std_ycc_setting {
	u32 dm1[DV_STD_YCC_C1_WORDS];
	u32 dm2[DV_STD_YCC_C2_WORDS];
	u32 dm3[DV_STD_YCC_C3_WORDS];
	u8 metadata[DV_STD_YCC_META_MAX];
	u32 metadata_len;
	/* Caller must apply these together with all copied DM and metadata.
	 * They are explicit requests, not writes to undocumented DM fields. */
	bool bypass_csc1;
	bool bypass_cvm1;
	bool bypass_cvm2;
	u32 core3_mode;
};

struct dv_std_ycc_matrix {
	s32 c[9];
	u32 shift;
};

static u32 dv_std_ycc_be32(const u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) |
		((u32)p[2] << 8) | p[3];
}

static s32 dv_std_ycc_be16s(const u8 *p)
{
	return (s16)(((u16)p[0] << 8) | p[1]);
}

static bool dv_std_ycc_decode(const u32 words[5], struct dv_std_ycc_matrix *m)
{
	int i;

	m->shift = words[4] >> 16;
	if (m->shift > 15)
		return false;
	for (i = 0; i < 9; i++)
		m->c[i] = (s16)(words[i / 2] >> ((i & 1) * 16));
	return true;
}

static void dv_std_ycc_pack(const struct dv_std_ycc_matrix *m, u32 words[5])
{
	int i;

	for (i = 0; i < 4; i++)
		words[i] = (u16)m->c[2 * i] |
			((u32)(u16)m->c[2 * i + 1] << 16);
	words[4] = (m->shift << 16) | (u16)m->c[8];
}

/* adjugate <= 2^31, determinant < 2^48 for signed16 coefficients.
 * Products/sums below are signed64; no 128-bit arithmetic or division. */
static s64 dv_std_ycc_adjugate(const struct dv_std_ycc_matrix *m, s64 a[9])
{
	const s32 *c = m->c;

	a[0] = (s64)c[4] * c[8] - (s64)c[5] * c[7];
	a[1] = (s64)c[2] * c[7] - (s64)c[1] * c[8];
	a[2] = (s64)c[1] * c[5] - (s64)c[2] * c[4];
	a[3] = (s64)c[5] * c[6] - (s64)c[3] * c[8];
	a[4] = (s64)c[0] * c[8] - (s64)c[2] * c[6];
	a[5] = (s64)c[2] * c[3] - (s64)c[0] * c[5];
	a[6] = (s64)c[3] * c[7] - (s64)c[4] * c[6];
	a[7] = (s64)c[1] * c[6] - (s64)c[0] * c[7];
	a[8] = (s64)c[0] * c[4] - (s64)c[1] * c[3];
	return (s64)c[0] * a[0] + (s64)c[1] * a[3] +
		(s64)c[2] * a[6];
}

/* Round n/d*2^shift, ties away from zero. Scale the quotient/remainder
 * rather than multiplying n by 2^shift, so even dense s16 inversions cannot
 * overflow merely because the requested coefficient has many fraction bits.
 */
static bool dv_std_ycc_scaled(s64 n, s64 d, int shift, s32 *value)
{
	u64 un, ud, q, rem, limit;
	bool negative;
	int i;

	if (!d || n == (-0x7fffffffffffffffLL - 1) ||
	    d == (-0x7fffffffffffffffLL - 1) || shift < -31 || shift > 46)
		return false;
	negative = (n < 0) != (d < 0);
	un = n < 0 ? -n : n;
	ud = d < 0 ? -d : d;
	limit = negative ? 32768 : 32767;
	if (shift < 0) {
		if (ud > (~0ULL >> -shift))
			return false;
		ud <<= -shift;
		shift = 0;
	}
	q = div64_u64(un, ud);
	rem = un - q * ud;
	if (q > limit)
		return false;
	for (i = 0; i < shift; i++) {
		if (q > limit / 2)
			return false;
		q *= 2;
		/* This doubles the remainder without overflowing at large ud. */
		if (rem >= ud - rem) {
			rem -= ud - rem;
			q++;
		} else {
			rem += rem;
		}
		if (q > limit)
			return false;
	}
	if (rem >= ud / 2 + (ud & 1))
		q++;
	if (q > limit)
		return false;
	*value = negative ? -(s32)q : (s32)q;
	return true;
}

/* row_shift controls powers of two on the LEFT of the inverse.
 * Q12 minimum is deliberate: fail closed instead of silently returning an
 * ill-conditioned inverse with only a few useful fractional bits. */
static bool dv_std_ycc_inverse(const struct dv_std_ycc_matrix *m,
			       const int row_shift[3], struct dv_std_ycc_matrix *out)
{
	s64 a[9], det = dv_std_ycc_adjugate(m, a);
	int shift, i;
	bool fits;

	if (!det)
		return false;
	for (shift = 15; shift >= 12; shift--) {
		fits = true;
		for (i = 0; i < 9; i++)
			if (!dv_std_ycc_scaled(a[i], det,
				m->shift + shift + row_shift[i / 3], &out->c[i])) {
				fits = false;
				break;
			}
		if (fits) {
			out->shift = shift;
			return true;
		}
	}
	return false;
}

static bool dv_std_ycc_rebase(const struct dv_std_ycc_matrix *video,
			     const struct dv_std_ycc_matrix *gui,
			     struct dv_std_ycc_matrix *out)
{
	s64 a[9], num[9], det = dv_std_ycc_adjugate(video, a);
	int r, c, k, shift, i;
	bool fits;

	if (!det)
		return false;
	for (r = 0; r < 3; r++)
		for (c = 0; c < 3; c++) {
			s64 sum = 0;

			/* Three products, each <= 2^46, sum < 2^48. */
			for (k = 0; k < 3; k++)
				sum += a[3 * r + k] * gui->c[3 * k + c];
			num[3 * r + c] = sum;
		}
	for (shift = 15; shift >= 12; shift--) {
		fits = true;
		for (i = 0; i < 9; i++)
			if (!dv_std_ycc_scaled(num[i], det,
				(int)video->shift - (int)gui->shift + shift, &out->c[i])) {
				fits = false;
				break;
			}
		if (fits) {
			out->shift = shift;
			return true;
		}
	}
	return false;
}

static bool dv_std_ycc_source(const u8 *md, u32 len, s32 offsets[3])
{
	u32 cursor = 71, block, count, i;

	if (len < 71 || len > DV_STD_YCC_META_MAX || md[0] || md[1] > 1 ||
	    md[50] != 255 || md[51] != 255 || md[60] != 12 ||
	    md[61] != 0 || md[62] > 2 || md[63] != 1)
		return false;
	for (i = 52; i < 60; i++)
		if (md[i])
			return false;
	if ((((u32)md[64] << 8) | md[65]) >
	    (((u32)md[66] << 8) | md[67]) || md[66] > 15)
		return false;
	for (i = 0; i < 3; i++) {
		u32 offset = dv_std_ycc_be32(md + 20 + 4 * i);

		if ((offset & 65535) || (offset >> 16) > 4095)
			return false;
		offsets[i] = offset >> 16;
	}
	count = md[70];
	for (i = 0; i < count; i++) {
		if (len - cursor < 5)
			return false;
		block = dv_std_ycc_be32(md + cursor);
		if (block > len - cursor - 5)
			return false;
		cursor += 5 + block;
	}
	return cursor == len;
}

static int dv_std_ycc_build(const struct dv_std_ycc_input *in,
			    struct dv_std_ycc_setting *out)
{
	static const int inverse_rows[3] = {0, 0, 0};
	struct dv_std_ycc_setting next;
	struct dv_std_ycc_matrix h, source_lms, a1, a2, n, p;
	s32 offsets[3];
	s64 adj[9];
	int r, c, i;

	if (!in || !out || !in->source_meta || !in->dm1 || !in->dm2 || !in->dm3)
		return DV_STD_YCC_BAD_POINTER;
	if (in->low_latency || in->core3_mode != 5 || in->dm1[13] != 1 ||
	    in->dm1[0] != 4095 || in->dm1[1] != 0x10010 || in->dm1[24] ||
	    in->dm2[11] != 1 || in->dm2[22] || in->dm3[10] ||
	    in->dm3[11] != 0x9c400000)
		return DV_STD_YCC_BAD_MODE;
	if (!dv_std_ycc_source(in->source_meta, in->source_len, offsets))
		return DV_STD_YCC_BAD_SOURCE;
	/* This probe contract centres chroma at 2048 before the half-amplitude
	 * port and Core3 mode1 restores it around 2048. Other source centres need
	 * an unproven GUI affine offset; leave those sources native. Y offset
	 * remains described by the source DM and is not subtracted from video. */
	if (offsets[1] != 2048 || offsets[2] != 2048)
		return DV_STD_YCC_BAD_SOURCE;
	if (!dv_std_ycc_decode(in->dm1 + 5, &h) ||
	    !dv_std_ycc_decode(in->dm1 + 14, &a1) ||
	    !dv_std_ycc_decode(in->dm2 + 12, &a2))
		return DV_STD_YCC_BAD_MATRIX;
	for (i = 0; i < 9; i++) {
		s32 source = dv_std_ycc_be16s(in->source_meta + 2 + 2 * i);

		if ((s64)h.c[i] * 8192 != (s64)source * (1U << h.shift))
			return DV_STD_YCC_SOURCE_MISMATCH;
		source_lms.c[i] = dv_std_ycc_be16s(in->source_meta + 32 + 2 * i);
	}
	source_lms.shift = 14;
	if (!dv_std_ycc_adjugate(&source_lms, adj))
		return DV_STD_YCC_BAD_SOURCE;
	if (!dv_std_ycc_adjugate(&a2, adj))
		return DV_STD_YCC_BAD_MATRIX;
	for (r = 0; r < 3; r++) {
		s64 post = 0;

		for (c = 0; c < 3; c++)
			post += (s64)h.c[3 * r + c] * offsets[c];
		if (post != (s32)in->dm1[10 + r])
			return DV_STD_YCC_SOURCE_MISMATCH;
	}
	if (!dv_std_ycc_inverse(&h, inverse_rows, &n) ||
	    !dv_std_ycc_rebase(&a1, &a2, &p))
		return DV_STD_YCC_BAD_MATRIX;
	memset(&next, 0, sizeof(next));
	memcpy(next.dm1, in->dm1, sizeof(next.dm1));
	memcpy(next.dm2, in->dm2, sizeof(next.dm2));
	memcpy(next.dm3, in->dm3, sizeof(next.dm3));
	/* Already measured strict unit bypass: A=[Y,(Cb+2048)/2,(Cr+2048)/2]
	 * with integer port rounding. Keep Composer and all CSC/PQ parameters;
	 * the caller bypasses CSC and CVM, so they do not process video. */
	next.dm1[2] = 0x100000;
	next.dm1[3] = 0x08000000;
	next.dm1[4] = 2048;
	dv_std_ycc_pack(&p, next.dm2 + 12);
	dv_std_ycc_pack(&n, next.dm2 + 17);
	/* Core2 C2D=H^-1, NOT diag(1,2,2)*H^-1: this half-chroma domain
	 * differs from the previous mode5 recipe. With source [oy,2048,2048],
	 * the linear result still lacks +oy in GUI Y. c2d_off stays original0;
	 * its affine layout is not proven. GUI remains explicitly unqualified. */
	next.dm3[12] = 4096;
	next.dm3[13] = 0;
	next.dm3[14] = 2048;
	next.dm3[15] = 2048;
	memcpy(next.metadata, in->source_meta, in->source_len);
	next.metadata[61] = 0;
	next.metadata[62] = 1;
	if (in->force_refresh)
		next.metadata[1] = 1;
	next.metadata_len = in->source_len;
	next.bypass_csc1 = true;
	next.bypass_cvm1 = true;
	next.bypass_cvm2 = true;
	next.core3_mode = 1;
	*out = next;
	return DV_STD_YCC_OK;
}

#endif
