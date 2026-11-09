/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STD_IPT_H
#define DV_STD_IPT_H

/* Pure, fail-closed STDV parameter builder. No hardware or LUT access.
 * Inputs must be ORIGINAL settings and the matching frame's source/output
 * ETSI DM. The caller owns mode/epoch/VSYNC gating and atomic application.
 *
 * Video: At = row_normalize(inv(output M2) * source M2), Q15;
 *        Ct = inv(output M1), Q12, with constrained neutral row sums.
 * GUI:   Ag' = At * inv(native video A2B) * native GUI A2B.
 * At already includes source M2: another source-RPU correction is WRONG.
 * GUI rebase covers only the linear basis, not CVM/PMA/alpha qualification.
 * Both CVMs must be bypassed; retaining native GUI CVM is not equivalent.
 * GUI RGB is assumed <= 1000 nits (linear <= 0.1 of the PQ 10000-nit unit).
 * A <= 1+1/1024 GUI row gain retains native coefficient rounding, with ample
 * headroom at that peak. This is NOT a full-10000-nit GUI clipping guarantee.
 * Output metadata, source CSC, EOTF, PQ LUTs and normalization stay original.
 * This contract is NOT for LLDV/HDR10 or source RGB/P5.
 */
#ifdef DV_STD_IPT_HOST_TEST
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
#else
#include <linux/math64.h>
#include <linux/string.h>
#include <linux/types.h>
#endif

#define DV_STD_IPT_META_MAX 512
#define DV_STD_IPT_C1_WORDS 27
#define DV_STD_IPT_C2_WORDS 24
#define DV_STD_IPT_C3_WORDS 26
#define DV_STD_IPT_S64_MAX 0x7fffffffffffffffLL

enum dv_std_ipt_result {
	DV_STD_IPT_OK = 0,
	DV_STD_IPT_BAD_POINTER = -1,
	DV_STD_IPT_BAD_MODE = -2,
	DV_STD_IPT_BAD_SOURCE = -3,
	DV_STD_IPT_BAD_OUTPUT = -4,
	DV_STD_IPT_SOURCE_MISMATCH = -5,
	DV_STD_IPT_BAD_MATRIX = -6,
	DV_STD_IPT_BAD_GUI = -7
};

struct dv_std_ipt_input {
	const u8 *source_meta;
	u32 source_len;
	const u8 *output_meta;
	u32 output_len;
	const u32 *dm1;
	const u32 *dm2;
	const u32 *dm3;
	u32 core3_mode;
	bool low_latency;
};

struct dv_std_ipt_setting {
	u32 dm1[DV_STD_IPT_C1_WORDS];
	u32 dm2[DV_STD_IPT_C2_WORDS];
	u32 dm3[DV_STD_IPT_C3_WORDS];
	u32 core3_mode;
	bool bypass_csc1;
	bool bypass_cvm1;
	bool bypass_cvm2;
	bool gui_rebased; /* Geometric rebase only; NOT GUI display qualification. */
};

struct dv_std_ipt_matrix {
	s32 c[9];
	u32 shift;
};

static inline const char *dv_std_ipt_result_name(int result)
{
	switch (result) {
	case DV_STD_IPT_OK: return "ok";
	case DV_STD_IPT_BAD_POINTER: return "bad_pointer";
	case DV_STD_IPT_BAD_MODE: return "bad_mode";
	case DV_STD_IPT_BAD_SOURCE: return "bad_source";
	case DV_STD_IPT_BAD_OUTPUT: return "bad_output";
	case DV_STD_IPT_SOURCE_MISMATCH: return "source_mismatch";
	case DV_STD_IPT_BAD_MATRIX: return "bad_matrix";
	case DV_STD_IPT_BAD_GUI: return "bad_gui";
	default: return "unknown";
	}
}

static inline u64 dv_std_ipt_div(u64 n, u64 d)
{
#ifdef DV_STD_IPT_HOST_TEST
	return n / d;
#else
	return div64_u64(n, d);
#endif
}

static inline u64 dv_std_ipt_abs(s64 v)
{
	/* All callers exclude S64_MIN or use signed16-derived bounded values. */
	return v < 0 ? (u64)-v : (u64)v;
}

static inline bool dv_std_ipt_add(s64 a, s64 b, s64 *v)
{
	if ((b > 0 && a > DV_STD_IPT_S64_MAX - b) ||
	    (b < 0 && a < -DV_STD_IPT_S64_MAX - b))
		return false;
	*v = a + b;
	return true;
}

static inline bool dv_std_ipt_mul(s64 a, s64 b, s64 *v)
{
	u64 ua, ub;

	if (a == -DV_STD_IPT_S64_MAX - 1 || b == -DV_STD_IPT_S64_MAX - 1)
		return false;
	ua = dv_std_ipt_abs(a);
	ub = dv_std_ipt_abs(b);
	if (ub && ua > dv_std_ipt_div(DV_STD_IPT_S64_MAX, ub))
		return false;
	*v = a * b;
	return true;
}

static inline u32 dv_std_ipt_be32(const u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) |
		((u32)p[2] << 8) | p[3];
}

static inline s32 dv_std_ipt_be16(const u8 *p)
{
	return (s16)(((u16)p[0] << 8) | p[1]);
}

static inline bool dv_std_ipt_decode(const u32 *words,
				    struct dv_std_ipt_matrix *m)
{
	int i;

	m->shift = words[4] >> 16;
	if (m->shift > 15)
		return false;
	for (i = 0; i < 9; i++)
		m->c[i] = (s16)(words[i / 2] >> (16 * (i & 1)));
	return true;
}

static inline void dv_std_ipt_pack(const struct dv_std_ipt_matrix *m, u32 *w)
{
	int i;

	for (i = 0; i < 4; i++)
		w[i] = (u16)m->c[2 * i] | ((u32)(u16)m->c[2 * i + 1] << 16);
	w[4] = (m->shift << 16) | (u16)m->c[8];
}

/* Signed16 matrix: each adjugate entry <= 2^31, |det| < 2^48.
 * Infinity-norm conditioning test uses products < 2^50. Limit 64 is a
 * deliberately conservative support boundary, independent of sample errors.
 */
static inline bool dv_std_ipt_inverse_data(const struct dv_std_ipt_matrix *m,
					  s64 a[9], s64 *det)
{
	const s32 *c = m->c;
	u64 norm = 0, invnorm = 0;
	int r;

	for (r = 0; r < 9; r++)
		if (c[r] < -32768 || c[r] > 32767)
			return false;

	a[0] = (s64)c[4] * c[8] - (s64)c[5] * c[7];
	a[1] = (s64)c[2] * c[7] - (s64)c[1] * c[8];
	a[2] = (s64)c[1] * c[5] - (s64)c[2] * c[4];
	a[3] = (s64)c[5] * c[6] - (s64)c[3] * c[8];
	a[4] = (s64)c[0] * c[8] - (s64)c[2] * c[6];
	a[5] = (s64)c[2] * c[3] - (s64)c[0] * c[5];
	a[6] = (s64)c[3] * c[7] - (s64)c[4] * c[6];
	a[7] = (s64)c[1] * c[6] - (s64)c[0] * c[7];
	a[8] = (s64)c[0] * c[4] - (s64)c[1] * c[3];
	*det = (s64)c[0] * a[0] + (s64)c[1] * a[3] + (s64)c[2] * a[6];
	if (!*det)
		return false;
	for (r = 0; r < 3; r++) {
		u64 n = dv_std_ipt_abs(c[3 * r]) + dv_std_ipt_abs(c[3 * r + 1]) +
			dv_std_ipt_abs(c[3 * r + 2]);
		u64 an = dv_std_ipt_abs(a[3 * r]) + dv_std_ipt_abs(a[3 * r + 1]) +
			dv_std_ipt_abs(a[3 * r + 2]);

		if (n > norm)
			norm = n;
		if (an > invnorm)
			invnorm = an;
	}
	return norm * invnorm <= 64 * dv_std_ipt_abs(*det);
}

/* Round n/d * 2^shift, nearest/ties away, without forming n*2^shift.
 * error = rounded*|d_scaled| - exact_numerator in the positive-d denominator;
 * it permits EXACT constrained-nearest row adjustment without float/squares.
 */
static inline bool dv_std_ipt_scaled(s64 n, s64 d, int shift, u32 limit,
				    s32 *value, s64 *error, s64 *denom)
{
	u64 un, ud, q, rem;
	bool negative, up;
	int i;

	if (!d || n == -DV_STD_IPT_S64_MAX - 1 ||
	    d == -DV_STD_IPT_S64_MAX - 1 || shift < -31 || shift > 46)
		return false;
	negative = (n < 0) != (d < 0);
	un = dv_std_ipt_abs(n);
	ud = dv_std_ipt_abs(d);
	if (shift < 0) {
		if (ud > ((u64)DV_STD_IPT_S64_MAX >> -shift))
			return false;
		ud <<= -shift;
		shift = 0;
	}
	/* Row-error updates need 3*d within signed64. */
	if (ud > (u64)DV_STD_IPT_S64_MAX / 4)
		return false;
	q = dv_std_ipt_div(un, ud);
	rem = un - q * ud;
	if (q > limit)
		return false;
	for (i = 0; i < shift; i++) {
		if (q > limit / 2)
			return false;
		q *= 2;
		if (rem >= ud - rem) {
			rem -= ud - rem;
			q++;
		} else {
			rem += rem;
		}
		if (q > limit)
			return false;
	}
	up = rem >= ud / 2 + (ud & 1);
	if (up)
		q++;
	if (q > limit)
		return false;
	*value = negative ? -(s32)q : (s32)q;
	*error = up ? (s64)(ud - rem) : -(s64)rem;
	if (negative)
		*error = -*error;
	*denom = ud;
	return true;
}

/* All three fractions have one denominator and binary scale. Greedy minimum
 * squared-error adjustment, stable lower-column tie break, matches mark14.
 * Constrains row sums without changing the requested matrix's white gain.
 */
static inline bool dv_std_ipt_quant_row(const s64 n[3], s64 d, int shift,
				       s32 target, bool positive, s32 out[3])
{
	s64 error[3], denom = 0, sum = 0;
	s32 diff;
	int i, steps, best, direction;

	for (i = 0; i < 3; i++) {
		if (!dv_std_ipt_scaled(n[i], d, shift, 32768, &out[i],
				       &error[i], &denom) || out[i] > 32767 ||
		    (positive && out[i] < 0))
			return false;
		sum += out[i];
	}
	diff = target - sum;
	/* Three independently nearest coefficients need at most two unit fixes. */
	if (diff < -2 || diff > 2)
		return false;
	direction = diff < 0 ? -1 : 1;
	steps = diff < 0 ? -diff : diff;
	while (steps--) {
		best = -1;
		for (i = 0; i < 3; i++) {
			if (out[i] + direction > 32767 ||
			    out[i] + direction < (positive ? 0 : -32768))
				continue;
			if (best < 0 || (direction > 0 ? error[i] < error[best] :
						       error[i] > error[best]))
				best = i;
		}
		if (best < 0)
			return false;
		out[best] += direction;
		error[best] += direction * denom;
	}
	return true;
}

static inline bool dv_std_ipt_metadata(const u8 *md, u32 len, bool output,
				      s32 offsets[3])
{
	u32 cursor = 71, block, count, i;

	if (len < 71 || len > DV_STD_IPT_META_MAX || md[0] || md[1] > 1 ||
	    md[50] != 255 || md[51] != 255 || md[60] != 12 ||
	    md[61] != (output ? 2 : 0) || md[62] > 2 || md[63] != 1 ||
	    (output && md[62] != 1))
		return false;
	for (i = 52; i < 60; i++)
		if (md[i])
			return false;
	if ((((u32)md[64] << 8) | md[65]) >
	    (((u32)md[66] << 8) | md[67]) || md[66] > 15)
		return false;
	for (i = 0; i < 3; i++) {
		u32 offset = dv_std_ipt_be32(md + 20 + 4 * i);

		if ((offset & 65535) || (offset >> 16) > 4095)
			return false;
		offsets[i] = offset >> 16;
	}
	if (output && (offsets[0] || offsets[1] != 2048 || offsets[2] != 2048))
		return false;
	count = md[70];
	for (i = 0; i < count; i++) {
		if (len - cursor < 5)
			return false;
		block = dv_std_ipt_be32(md + cursor);
		if (block > len - cursor - 5)
			return false;
		cursor += 5 + block;
	}
	return cursor == len;
}

static inline void dv_std_ipt_read_matrix(const u8 *md, int off, u32 shift,
					 struct dv_std_ipt_matrix *m)
{
	int i;

	for (i = 0; i < 9; i++)
		m->c[i] = dv_std_ipt_be16(md + off + 2 * i);
	m->shift = shift;
}

static inline bool dv_std_ipt_video(const struct dv_std_ipt_matrix *source,
				   const struct dv_std_ipt_matrix *o2,
				   struct dv_std_ipt_matrix *out)
{
	s64 a[9], det, num[9], row;
	int r, c, k;

	if (!dv_std_ipt_inverse_data(source, a, &det) ||
	    !dv_std_ipt_inverse_data(o2, a, &det))
		return false;
	for (r = 0; r < 3; r++) {
		row = 0;
		for (c = 0; c < 3; c++) {
			s64 n = 0;

			/* Each product <= 2^46; three-term sum < 2^48. */
			for (k = 0; k < 3; k++)
				n += a[3 * r + k] * source->c[3 * k + c];
			num[3 * r + c] = n;
			if ((det > 0 && n < 0) || (det < 0 && n > 0))
				return false;
			row += n;
		}
		/* Allow at most 1/1024 relative white-gain approximation. */
		if (!row || dv_std_ipt_abs(row - det) * 1024 > dv_std_ipt_abs(det) ||
		    !dv_std_ipt_quant_row(num + 3 * r, row, 15, 32768, true,
					out->c + 3 * r))
			return false;
	}
	out->shift = 15;
	return true;
}

static inline bool dv_std_ipt_target(const struct dv_std_ipt_matrix *o1,
				    struct dv_std_ipt_matrix *out)
{
	s64 a[9], det, sum, error, denom;
	s32 target;
	int r;

	/* Equal positive first column means neutral PQ triplets have no chroma.
 * Gain >= 1 keeps target white <= full scale; deviation bound is 1/1024.
 */
	if (o1->c[0] < 8192 || o1->c[0] > 8200 ||
	    o1->c[0] != o1->c[3] || o1->c[0] != o1->c[6] ||
	    !dv_std_ipt_inverse_data(o1, a, &det))
		return false;
	for (r = 0; r < 3; r++) {
		sum = a[3 * r] + a[3 * r + 1] + a[3 * r + 2];
		if (!dv_std_ipt_scaled(sum, det, 25, 98304, &target, &error, &denom) ||
		    (r && target) || (!r && (target <= 0 || target > 4096)) ||
		    !dv_std_ipt_quant_row(a + 3 * r, det, 25, target, false,
					out->c + 3 * r))
			return false;
	}
	out->shift = 12;
	return true;
}

static inline bool dv_std_ipt_gui(const struct dv_std_ipt_matrix *target,
				 const struct dv_std_ipt_matrix *video,
				 const struct dv_std_ipt_matrix *gui,
				 struct dv_std_ipt_matrix *out)
{
	s64 a[9], det, middle[9], num[9], sum, v, row, error, denom;
	s32 row_target;
	int r, c, k, shift, exponent;
	bool fits;

	if (!dv_std_ipt_inverse_data(gui, a, &det) ||
	    !dv_std_ipt_inverse_data(video, a, &det))
		return false;
	for (r = 0; r < 3; r++)
		for (c = 0; c < 3; c++) {
			sum = 0;
			for (k = 0; k < 3; k++)
				sum += a[3 * r + k] * gui->c[3 * k + c];
			middle[3 * r + c] = sum;
		}
	for (r = 0; r < 3; r++)
		for (c = 0; c < 3; c++) {
			sum = 0;
			/* Unlike a two-matrix s16 product, a third matrix MAY overflow.
			 * Check every product and addition, never silently truncate. */
			for (k = 0; k < 3; k++) {
				if (!dv_std_ipt_mul(target->c[3 * r + k], middle[3 * k + c], &v) ||
				    !dv_std_ipt_add(sum, v, &sum))
					return false;
			}
			if ((det > 0 && sum < 0) || (det < 0 && sum > 0))
				return false;
			num[3 * r + c] = sum;
		}
	for (shift = 15; shift >= 12; shift--) {
		fits = true;
		exponent = (int)video->shift - (int)gui->shift - (int)target->shift + shift;
		for (r = 0; r < 3; r++) {
			if (!dv_std_ipt_add(num[3 * r], num[3 * r + 1], &row) ||
			    !dv_std_ipt_add(row, num[3 * r + 2], &row) ||
			    !dv_std_ipt_scaled(row, det, exponent, 32768,
					       &row_target, &error, &denom) ||
			    row_target <= 0 || row_target > (1 << shift) + (1 << (shift - 10)) ||
			    /* Preserve native gain within the declared 1/1024 budget;
			     * compare the exact rational, not only its rounded value. */
			    (row_target == (1 << shift) + (1 << (shift - 10)) && error < 0) ||
			    !dv_std_ipt_quant_row(num + 3 * r, det, exponent,
						row_target, true, out->c + 3 * r)) {
				fits = false;
				break;
			}
		}
		if (fits) {
			out->shift = shift;
			return true;
		}
	}
	return false;
}

static inline int dv_std_ipt_build(const struct dv_std_ipt_input *in,
				    struct dv_std_ipt_setting *out)
{
	struct dv_std_ipt_setting next;
	struct dv_std_ipt_matrix h, source, o1, o2, native, gui, at, ct, ag, cv, cg;
	s32 offsets[3], target_offsets[3];
	s64 post, adj[9], det;
	int i, r, c;

	if (!in || !out || !in->source_meta || !in->output_meta ||
	    !in->dm1 || !in->dm2 || !in->dm3)
		return DV_STD_IPT_BAD_POINTER;
	if (in->low_latency || in->core3_mode != 5 || in->dm1[0] != 4095 ||
	    in->dm1[1] != 0x10010 || in->dm1[13] != 1 || in->dm1[24] ||
	    in->dm2[11] != 1 || in->dm2[22] || in->dm3[10] ||
	    in->dm3[11] != 0x9c400000)
		return DV_STD_IPT_BAD_MODE;
	if (!dv_std_ipt_metadata(in->source_meta, in->source_len, false, offsets))
		return DV_STD_IPT_BAD_SOURCE;
	if (!dv_std_ipt_metadata(in->output_meta, in->output_len, true, target_offsets))
		return DV_STD_IPT_BAD_OUTPUT;
	if (!dv_std_ipt_decode(in->dm1 + 5, &h) ||
	    !dv_std_ipt_decode(in->dm1 + 14, &native) ||
	    !dv_std_ipt_decode(in->dm2 + 12, &gui) ||
	    !dv_std_ipt_decode(in->dm1 + 19, &cv) ||
	    !dv_std_ipt_decode(in->dm2 + 17, &cg) ||
	    !dv_std_ipt_inverse_data(&h, adj, &det))
		return DV_STD_IPT_BAD_MATRIX;
	/* Both original planes must actually use the same post-PQ basis. */
	for (i = 0; i < 9; i++)
		if ((s64)cv.c[i] * (1U << cg.shift) !=
		    (s64)cg.c[i] * (1U << cv.shift))
			return DV_STD_IPT_BAD_GUI;
	for (i = 0; i < 9; i++)
		if ((s64)h.c[i] * 8192 !=
		    (s64)dv_std_ipt_be16(in->source_meta + 2 + 2 * i) * (1U << h.shift))
			return DV_STD_IPT_SOURCE_MISMATCH;
	for (r = 0; r < 3; r++) {
		post = 0;
		for (c = 0; c < 3; c++)
			post += (s64)h.c[3 * r + c] * offsets[c];
		if (post * 8192 != (s64)(s32)in->dm1[10 + r] * (1U << h.shift))
			return DV_STD_IPT_SOURCE_MISMATCH;
	}
	dv_std_ipt_read_matrix(in->source_meta, 32, 14, &source);
	dv_std_ipt_read_matrix(in->output_meta, 2, 13, &o1);
	dv_std_ipt_read_matrix(in->output_meta, 32, 14, &o2);
	if (!dv_std_ipt_video(&source, &o2, &at) || !dv_std_ipt_target(&o1, &ct))
		return DV_STD_IPT_BAD_MATRIX;
	if (!dv_std_ipt_gui(&at, &native, &gui, &ag))
		return DV_STD_IPT_BAD_GUI;
	memset(&next, 0, sizeof(next));
	memcpy(next.dm1, in->dm1, sizeof(next.dm1));
	memcpy(next.dm2, in->dm2, sizeof(next.dm2));
	memcpy(next.dm3, in->dm3, sizeof(next.dm3));
	dv_std_ipt_pack(&at, next.dm1 + 14);
	dv_std_ipt_pack(&ct, next.dm1 + 19);
	dv_std_ipt_pack(&ag, next.dm2 + 12);
	dv_std_ipt_pack(&ct, next.dm2 + 17);
	next.dm3[12] = 4096;
	next.dm3[13] = 0;
	next.dm3[14] = 2048;
	next.dm3[15] = 2048;
	next.core3_mode = 1;
	next.bypass_csc1 = false;
	next.bypass_cvm1 = true;
	next.bypass_cvm2 = true;
	next.gui_rebased = true;
	*out = next;
	return DV_STD_IPT_OK;
}

#endif /* DV_STD_IPT_H */
