/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_HDR10_ENCODING_H
#define DV_HDR10_ENCODING_H

/* Output-basis conversion. Q20 matrix times signed Core3 B2A.
 * Return the original complete matrix on any unsupported scale/overflow.
 * Never mutate the algorithm's result or compound an already converted B2A.
 */
static bool dv_hdr10_convert_b2a(const u32 original[5], const s32 matrix[9],
			       u32 converted[5])
{
	s32 coeff[9], result[9];
	u32 scale = original[4] >> 16;
	int r, c, k, i;

	memcpy(converted, original, 5 * sizeof(*original));
	if (scale > 15)
		return false;
	for (i = 0; i < 9; i++) {
		if (matrix[i] < -(8 << 20) || matrix[i] > (8 << 20))
			return false;
		coeff[i] = (s16)(original[i / 2] >> (16 * (i & 1)));
	}
	for (r = 0; r < 3; r++) {
		for (c = 0; c < 3; c++) {
			s64 sum = 0;
			s64 rounded;

			for (k = 0; k < 3; k++)
				sum += (s64)matrix[r * 3 + k] * coeff[k * 3 + c];
			/* Round ties away from zero, avoiding signed right shifts. */
			rounded = sum >= 0 ? (sum + (1 << 19)) >> 20 :
				-((-sum + (1 << 19)) >> 20);
			if (rounded < -32768 || rounded > 32767)
				return false;
			result[r * 3 + c] = rounded;
		}
	}
	for (i = 0; i < 4; i++)
		converted[i] = (u16)result[2 * i] |
			((u32)(u16)result[2 * i + 1] << 16);
	converted[4] = (original[4] & 0xffff0000) | (u16)result[8];
	return true;
}

/* Decode actual algorithm VSVDB, not the user's override string. Coordinates
 * use Q12 (V0 native resolution; V1/V2 expand exact 8-bit coordinates).
 * Match STB's V1 short-block RY reconstruction, including all three low bits.
 * STB's LL output basis uses D65 even for V0's separately advertised white.
 */
static bool dv_hdr10_primaries(const u8 v[32], u32 size, s32 xy[6])
{
	u32 len, ver;
	int i;

	if (size < 6 || size > 32 || (v[0] >> 5) != 7 ||
	    v[1] != 1 || v[2] != 0x46 || v[3] != 0xd0 || v[4])
		return false;
	len = (v[0] & 31) + 1;
	if (len > size)
		return false;
	ver = v[5] >> 5;
	if (ver == 0 && len == 26) {
		for (i = 0; i < 3; i++) {
			u32 n = 6 + 3 * i;

			xy[2 * i] = (v[n + 1] << 4) | (v[n] >> 4);
			xy[2 * i + 1] = (v[n + 2] << 4) | (v[n] & 15);
		}
	} else if (ver == 1 && len == 15) {
		for (i = 0; i < 6; i++)
			xy[i] = v[9 + i] << 4;
	} else if (ver == 1 && len == 12) {
		xy[0] = (0xa0 | (v[11] >> 3)) << 4;
		xy[1] = (0x40 | (v[9] & 1) | ((v[10] & 1) << 1) |
			 ((v[11] & 7) << 2)) << 4;
		xy[2] = (v[9] >> 1) << 4;
		xy[3] = (0x80 | (v[10] >> 1)) << 4;
		xy[4] = (0x20 | (v[8] >> 5)) << 4;
		xy[5] = (0x08 | ((v[8] >> 2) & 7)) << 4;
	} else if (ver == 2 && len >= 12) {
		xy[0] = (0xa0 | (v[10] >> 3)) << 4;
		xy[1] = (0x40 | (v[11] >> 3)) << 4;
		xy[2] = (v[8] >> 1) << 4;
		xy[3] = (0x80 | (v[9] >> 1)) << 4;
		xy[4] = (0x20 | (v[10] & 7)) << 4;
		xy[5] = (0x08 | (v[11] & 7)) << 4;
	} else {
		return false;
	}
	for (i = 0; i < 3; i++)
		if (xy[2 * i] <= 0 || xy[2 * i + 1] <= 0 ||
		    xy[2 * i] + xy[2 * i + 1] > 4096)
			return false;
	return true;
}

static s64 dv_hdr10_det(const s32 m[9])
{
	return (s64)m[0] * ((s64)m[4] * m[8] - (s64)m[5] * m[7]) -
	       (s64)m[1] * ((s64)m[3] * m[8] - (s64)m[5] * m[6]) +
	       (s64)m[2] * ((s64)m[3] * m[7] - (s64)m[4] * m[6]);
}

static s64 dv_hdr10_round_div(s64 num, s64 den)
{
	/* Callers bound all intermediates away from S64_MIN/MAX. */
	if (den < 0) {
		den = -den;
		num = -num;
	}
	return num >= 0 ? div64_s64(num + den / 2, den) :
		-div64_s64(-num + den / 2, den);
}

static bool dv_hdr10_matrix(const u8 v[32], u32 size, s32 result[9])
{
	/* inverse(BT.2020/D65 RGB->XYZ), Q20; only standard constants. */
	static const s32 xyz_to_2020[9] = {
		1800039, -372948, -265674,
		-699069, 1695003, 16535,
		18497, -44848, 987867
	};
	static const s32 white[3] = {3127, 3290, 3583};
	s32 xy[6], p[9], replaced[9], xyz[9], out[9];
	s64 det, weight, sum, value;
	int row, col, k;

	if (!dv_hdr10_primaries(v, size, xy))
		return false;
	for (col = 0; col < 3; col++) {
		p[col] = xy[2 * col];
		p[3 + col] = xy[2 * col + 1];
		p[6 + col] = 4096 - p[col] - p[3 + col];
	}
	det = dv_hdr10_det(p);
	/* Reject nearly collinear primaries rather than amplify quantization. */
	if (det > -(1 << 20) && det < (1 << 20))
		return false;
	for (col = 0; col < 3; col++) {
		memcpy(replaced, p, sizeof(p));
		for (row = 0; row < 3; row++)
			replaced[3 * row + col] = white[row];
		/* Q12 coordinates bound determinant*2^20 below 2^61. */
		weight = dv_hdr10_round_div(dv_hdr10_det(replaced) * (1LL << 20), det);
		/* D65 must be strictly inside the advertised RGB triangle. */
		if (weight <= 0 || weight > (3 << 20))
			return false;
		for (row = 0; row < 3; row++)
			xyz[row * 3 + col] = dv_hdr10_round_div(
				(s64)p[row * 3 + col] * weight, white[1]);
	}
	for (row = 0; row < 3; row++) {
		s32 neutral = 0;

		for (col = 0; col < 3; col++) {
			sum = 0;
			for (k = 0; k < 3; k++)
				sum += (s64)xyz_to_2020[row * 3 + k] * xyz[k * 3 + col];
			value = dv_hdr10_round_div(sum, 1 << 20);
			if (value < -(8 << 20) || value > (8 << 20))
				return false;
			out[row * 3 + col] = value;
			neutral += value;
		}
		/* Keep neutral RGB exact despite fixed-point roundoff. */
		if (neutral < (1 << 20) - 32 || neutral > (1 << 20) + 32)
			return false;
		out[row * 3 + row] += (1 << 20) - neutral;
	}
	memcpy(result, out, sizeof(out));
	return true;
}

struct dv_hdr10_encoding_cache {
	u8 target[32];
	u32 size;
	u32 generation;
	bool initialized;
	bool valid;
	s32 matrix[9];
};

static int dv_hdr10_update(struct dv_hdr10_encoding_cache *cache,
			  u32 dm[26], bool enabled, bool hdr10_ll,
			  const u8 vsvdb[32], u32 size)
{
	u32 converted[5];

	if (!enabled || !hdr10_ll)
		return 0;
	if (!cache->initialized || cache->size != size ||
	    memcmp(cache->target, vsvdb, sizeof(cache->target))) {
		memcpy(cache->target, vsvdb, sizeof(cache->target));
		cache->size = size;
		cache->initialized = true;
		cache->generation++;
		memset(cache->matrix, 0, sizeof(cache->matrix));
		cache->valid = dv_hdr10_matrix(vsvdb, size, cache->matrix);
	}
	if (!cache->valid)
		return -1;
	if (!dv_hdr10_convert_b2a(dm + 5, cache->matrix, converted))
		return -2;
	memcpy(dm + 5, converted, sizeof(converted));
	return 1;
}

#endif
