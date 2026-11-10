/* SPDX-License-Identifier: GPL-2.0 */
/* Bounded old STB2.4 CommitTrim policy for the ordinary Vpri/Bwd GUI path.
 * Fixed InitTmCtrl defaults are part of this explicit legacy profile.
 */
#include "trim.h"
#ifdef DV_STB24_METADATA_HOST_TEST
#include <string.h>
#else
#include <linux/string.h>
#endif

struct dvm24_l2_row {
	dvm24_u16 value[7];
};

static dvm24_u16 get16(const dvm24_u8 *p)
{
	return (dvm24_u16)(p[0] | (dvm24_u32)p[1] << 8);
}

static dvm24_u32 get32(const dvm24_u8 *p)
{
	return (dvm24_u32)p[0] | (dvm24_u32)p[1] << 8 |
	       (dvm24_u32)p[2] << 16 | (dvm24_u32)p[3] << 24;
}

static void put32(dvm24_u8 *p, dvm24_u32 n)
{
	p[0] = n; p[1] = n >> 8; p[2] = n >> 16; p[3] = n >> 24;
}

static int signed32(dvm24_u32 value)
{
	return value <= 0x7fffffffU ? (int)value : -1 - (int)~value;
}

static int asr15(dvm24_u32 value)
{
	return signed32((value >> 15) | ((0U - (value >> 31)) << 17));
}

static int clamp(int value, int low, int high)
{
	return value < low ? low : value > high ? high : value;
}

int dvm24_apply_graphics_trim(const dvm24_u8 *source, dvm24_u32 len,
			    dvm24_u8 control[160])
{
	struct dvm24_l2_row rows[17];
	dvm24_u16 coordinates[17], indices[17], tmp;
	dvm24_u8 updated[160];
	dvm24_u32 cursor = DVM24_METADATA_HEADER, count = 1;
	dvm24_u32 block, payload, level, i, j, pass, low, high, target;
	dvm24_u32 direction;
	int values[7] = {0}, bias, delta, quotient;
	int crop_offset = 0, swapped;

	if (!source || !control)
		return DVM24_TRIM_POINTER;
	direction = get32(control + 56);
	if (direction > 2)
		return DVM24_TRIM_RANGE;
	if (len < DVM24_METADATA_HEADER || len > DVM24_METADATA_MAX)
		return DVM24_TRIM_PACKET;
	if (source[60] != 12 || (source[61] != 0 && source[61] != 2) ||
	    source[62] > 2 || source[63] != 1)
		return DVM24_TRIM_UNSUPPORTED;
	/* InitMdsExt's neutral row is retained, then CommitTrim overwrites its
	 * target with source_max_PQ. This row participates in the same sort. */
	rows[0].value[0] = dvm24_get_be16(source + 66);
	if (rows[0].value[0] > 4095)
		return DVM24_TRIM_RANGE;
	for (j = 1; j < 7; ++j)
		rows[0].value[j] = 2048;
	for (block = 0; block < source[70]; ++block) {
		if (len - cursor < 5)
			return DVM24_TRIM_PACKET;
		payload = dvm24_get_be32(source + cursor);
		if (payload > len - cursor - 5)
			return DVM24_TRIM_PACKET;
		level = source[cursor + 4];
		if (level == 2) {
			if (payload != 14)
				return DVM24_TRIM_UNSUPPORTED;
			if (count >= 17)
				return DVM24_TRIM_COUNT;
			for (j = 0; j < 7; ++j)
				rows[count].value[j] = dvm24_get_be16(source + cursor + 5 + 2 * j);
			for (j = 0; j < 6; ++j)
				if (rows[count].value[j] > 4095)
					return DVM24_TRIM_RANGE;
			/* ParseMds 607c..609c: all-ones low13 bits means default MS. */
			if ((rows[count].value[6] & 0x1fff) == 0x1fff)
				rows[count].value[6] = 2048;
			else if (rows[count].value[6] > 8191)
				return DVM24_TRIM_RANGE;
			++count;
		} else if (level == 3) {
			/* The old parser requires its8B target+3value form. The
			 * current6B form fails that original parser and must make
			 * the complete legacy policy bundle fall back. Even valid
			 * old8B L3 is not consumed by this four-table generator. */
			if (payload != 8)
				return DVM24_TRIM_UNSUPPORTED;
		} else if (level == 5) {
			if (payload < 8)
				return DVM24_TRIM_PACKET;
			/* Match the original condition: right/top/bottom, not left.
			 * Repeated L5 blocks retain old ParseMds last-block behavior. */
			crop_offset = dvm24_get_be16(source + cursor + 7) ||
				dvm24_get_be16(source + cursor + 9) ||
				dvm24_get_be16(source + cursor + 11);
		}
		cursor += payload + 5;
	}
	if (cursor != len)
		return DVM24_TRIM_PACKET;
	/* The structural/legacy-format contract is checked in every direction.
	 * Only old Vpri effective backward mapping imports metadata L2. */
	if (direction != 1)
		return DVM24_TRIM_OK;
	for (i = 0; i < count; ++i) {
		coordinates[i] = rows[i].value[0];
		indices[i] = i;
	}
	/* Original bubble sort swaps equal coordinates too. Keep this tie rule
	 * rather than replacing it with a library sort or assuming uniqueness. */
	for (pass = count - 1; pass > 0; --pass) {
		swapped = 0;
		for (i = 0; i < pass; ++i) {
			if (coordinates[i + 1] <= coordinates[i]) {
				tmp = coordinates[i]; coordinates[i] = coordinates[i + 1]; coordinates[i + 1] = tmp;
				tmp = indices[i]; indices[i] = indices[i + 1]; indices[i + 1] = tmp;
				swapped = 1;
			}
		}
		if (!swapped)
			break;
	}
	/* 8a1c keeps GUI tcSource.maxPQ in w0 through the 8bac call. */
	target = get16(control + 2);
	if (target > 4095)
		return DVM24_TRIM_RANGE;
	if (target <= coordinates[0])
		low = high = 0;
	else if (target >= coordinates[count - 1])
		low = high = count - 1;
	else {
		for (high = 1; high < count && coordinates[high] <= target; ++high)
			;
		low = high - 1;
	}
	for (j = 1; j < 7; ++j) {
		values[j] = rows[indices[low]].value[j];
		if (low != high && coordinates[low] != coordinates[high]) {
			delta = (int)rows[indices[high]].value[j] - values[j];
			/* SDIV truncates toward zero; multiply then ASR rounds down.
			 * Qualified 12/13-bit fields keep all signed products bounded. */
			quotient = (delta * 32768) / (int)(coordinates[high] - coordinates[low]);
			values[j] += asr15((dvm24_u32)quotient * (target - coordinates[low]));
		}
	}
	memcpy(updated, control, sizeof(updated));
	for (j = 1; j <= 3; ++j)
		put32(updated + 116 + j * 4, (dvm24_u32)clamp(values[j] - 2048, -2048, 2047));
	bias = signed32((dvm24_u32)(values[4] - 2048) << 4);
	put32(updated + 132, clamp(bias, 0, 65535));
	put32(updated + 136, clamp(values[5] + 2048, 2048, 6143));
	put32(updated + 140, clamp(values[6] * 2, 0, 8191) - 4096);
	if (crop_offset && signed32(get32(updated + 124)) > 0)
		put32(updated + 124, 0);
	memcpy(control, updated, sizeof(updated));
	return DVM24_TRIM_OK;
}
