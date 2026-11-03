/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_HDR10_ENCODING_H
#define DV_HDR10_ENCODING_H

/* Diagnostic output-basis conversion. Q20 matrix times signed Core3 B2A.
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

#endif
