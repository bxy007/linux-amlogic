/* SPDX-License-Identifier: GPL-2.0 */
#include "metadata-policy.h"
#ifdef DV_STB24_METADATA_HOST_TEST
#include <string.h>
#else
#include <linux/string.h>
#endif

static dvm24_u16 get_le16(const dvm24_u8 *p)
{
	return p[0] | ((dvm24_u16)p[1] << 8);
}

static void put_be16(dvm24_u8 *p, dvm24_u16 n)
{
	p[0] = n >> 8;
	p[1] = n;
}

int dvm24_sink_version(const dvm24_u8 *v, dvm24_u32 len,
			 dvm24_u32 *version)
{
	dvm24_u32 declared, vsvdb_version;

	if (!v || !version || len < 6 || len > 32)
		return -1;
	declared = (v[0] & 31U) + 1U;
	if ((v[0] & 0xe0) != 0xe0 || declared != len ||
	    v[1] != 1 || v[2] != 0x46 || v[3] != 0xd0 || v[4])
		return -1;
	/* vsvdb_to_target_param1b28 writes byte5>>5 to+20;1b34 writes
	 * the distinct byte5 bits4:2 below to public target_param+24.
	 */
	vsvdb_version = v[5] >> 5;
	if (vsvdb_version > 2 ||
	    (vsvdb_version == 0 && len != 26) ||
	    (vsvdb_version == 1 && len != 12 && len != 15) ||
	    (vsvdb_version == 2 && len < 12))
		return -1;
	*version = (v[5] >> 2) & 7;
	return 0;
}

int dvm24_output_policy(const dvm24_u8 *source, dvm24_u32 source_len,
		       const dvm24_u8 *native, dvm24_u32 native_len,
		       const dvm24_u8 control[160], dvm24_u32 sink_dm_version,
		       dvm24_u8 output[512], dvm24_u32 *out_len,
		       struct dvm24_output_info *info)
{
	struct dvm24_scene_metadata src, dst;
	struct dvm24_output_info result = {0};
	dvm24_u8 packet[512];
	dvm24_u32 cursor, i, n, l2_count = 0, native_l2_count = 0;
	dvm24_u32 source_l2[32];
	dvm24_u16 minimum, maximum, average;
	bool cm4 = false, cropped = false, seen_254 = false;
	bool native_cm4 = false, native_seen_254 = false;

	if (!source || !native || !control || !output || !out_len || !info ||
	    output == source || output == native || sink_dm_version > 7)
		return -1;
	if (dvm24_read_scene_metadata(source, source_len, false, &src) ||
	    dvm24_read_scene_metadata(native, native_len, true, &dst))
		return -2;
	if (get_le16(control + 28) != src.source_min_pq ||
	    get_le16(control + 30) != src.source_max_pq ||
	    get_le16(control + 32) != src.source_diagonal)
		return -3;
	minimum = get_le16(control + 48);
	maximum = get_le16(control + 50);
	/* Original writer edits the original source dm_metadata_t. The native
	 *2.6 packet here is only an immutable encoding/tail template.
	 */
	average = src.scene_avg_pq;
	if (minimum > maximum || maximum > 4095)
		return -3;
	result.input_l1[0] = src.scene_min_pq;
	result.input_l1[1] = src.scene_max_pq;
	result.input_l1[2] = src.scene_avg_pq;
	/* All boundaries have already been checked by the immutable reader. */
	cursor = 71;
	for (i = 0; i < source[70]; i++) {
		n = dvm24_get_be32(source + cursor);
		if (source[cursor + 4] == 254) {
			if (seen_254 || n < 2)
				return -4;
			seen_254 = true;
			cm4 = source[cursor + 6] >= 2;
		}
		if (source[cursor + 4] == 5) {
			if (n < 8)
				return -4;
			/* Match old ParseMds/CommitMds: last source L5 wins;
			 * right/top/bottom are tested, left alone is not.
			 */
			cropped = dvm24_get_be16(source + cursor + 7) ||
				  dvm24_get_be16(source + cursor + 9) ||
				  dvm24_get_be16(source + cursor + 11);
		}
		if (source[cursor + 4] == 2) {
			if (n < 14 || l2_count == 32)
				return -4;
			source_l2[l2_count++] = cursor;
		}
		/* Legacy2.4 L3 is4*u16. Standard CM4's6B L3 makes the
		 * original ParseMds fail; never silently apply a partial bundle.
		 */
		if (source[cursor + 4] == 3 && n < 8)
			return -4;
		cursor += n + 5;
	}
	result.legacy = !(cm4 && sink_dm_version >= 2);
	if (result.legacy) {
		/* Exact update_md_for_hdmi518..588 compatibility branch. */
		if (minimum > 614)
			minimum = 614;
		if (maximum < 2081)
			maximum = 2081;
		if (average < minimum + 700)
			average = minimum + 700;
		if (average >= maximum)
			average = maximum - 1;
	}
	memcpy(packet, native, native_len);
	put_be16(packet + 64, src.source_min_pq);
	put_be16(packet + 66, src.source_max_pq);
	cursor = 71;
	for (i = 0; i < packet[70]; i++) {
		n = dvm24_get_be32(packet + cursor);
		if (packet[cursor + 4] == 254) {
			if (native_seen_254 || n < 2)
				return -4;
			native_seen_254 = true;
			native_cm4 = packet[cursor + 6] >= 2;
		}
		if (packet[cursor + 4] == 1) {
			put_be16(packet + cursor + 5, minimum);
			put_be16(packet + cursor + 7, maximum);
			put_be16(packet + cursor + 9, average);
		}
		if (packet[cursor + 4] == 2) {
			dvm24_u32 original;
			if (native_l2_count == l2_count)
				return -4;
			original = source_l2[native_l2_count++];
			if (n != dvm24_get_be32(source + original) ||
			    memcmp(source + original + 5, native + cursor + 5, n))
				return -4;
			/* Pair the emitted trim offset with the same old control
			 * clamp used when source active area is present.
			 */
			if (cropped && dvm24_get_be16(packet + cursor + 9) > 2048) {
				put_be16(packet + cursor + 9, 2048);
				result.l2_offsets_clamped++;
			}
		}
		cursor += n + 5;
	}
	if (native_l2_count != l2_count)
		return -4;
	if (native_cm4 != cm4)
		return -4;
	result.output_l1[0] = minimum;
	result.output_l1[1] = maximum;
	result.output_l1[2] = average;
	result.changed = memcmp(packet, native, native_len) != 0;
	memcpy(output, packet, native_len);
	*out_len = native_len;
	*info = result;
	return 0;
}
