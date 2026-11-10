/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STB24_METADATA_READER_H
#define DV_STB24_METADATA_READER_H

/* Bounded reader only. Never alters metadata or preserves values across frames.
 * Scope matches the current direct-IPT validator's PQ12/full-range ETSI header.
 * The caller still owns mode/epoch/frame qualification and output DM lifetime.
 */
#ifdef DV_STB24_METADATA_HOST_TEST
#include <stdbool.h>
#include <stdint.h>
typedef uint8_t dvm24_u8;
typedef uint16_t dvm24_u16;
typedef uint32_t dvm24_u32;
#else
#include <linux/types.h>
typedef u8 dvm24_u8;
typedef u16 dvm24_u16;
typedef u32 dvm24_u32;
#endif

#define DVM24_METADATA_MAX 512U
#define DVM24_METADATA_HEADER 71U

struct dvm24_scene_metadata {
	dvm24_u16 source_min_pq;
	dvm24_u16 source_max_pq;
	dvm24_u16 source_diagonal;
	dvm24_u16 scene_min_pq;
	dvm24_u16 scene_max_pq;
	dvm24_u16 scene_avg_pq;
};

enum dvm24_metadata_result {
	DVM24_METADATA_OK = 0,
	DVM24_METADATA_POINTER = -1,
	DVM24_METADATA_SIZE = -2,
	DVM24_METADATA_HEADER_UNSUPPORTED = -3,
	DVM24_METADATA_EXTENSION = -4,
	DVM24_METADATA_RANGE = -5,
	DVM24_METADATA_NO_L1 = -6,
	DVM24_METADATA_DUPLICATE_L1 = -7
};

static inline dvm24_u16 dvm24_get_be16(const dvm24_u8 *p)
{
	return ((dvm24_u16)p[0] << 8) | p[1];
}

static inline dvm24_u32 dvm24_get_be32(const dvm24_u8 *p)
{
	return ((dvm24_u32)p[0] << 24) | ((dvm24_u32)p[1] << 16) |
	       ((dvm24_u32)p[2] << 8) | p[3];
}

static inline int dvm24_read_scene_metadata(const dvm24_u8 *metadata,
					   dvm24_u32 len, bool output,
					   struct dvm24_scene_metadata *out)
{
	struct dvm24_scene_metadata parsed = {0};
	dvm24_u32 cursor = DVM24_METADATA_HEADER;
	dvm24_u32 i, payload_len;
	bool found_l1 = false;

	if (!metadata || !out)
		return DVM24_METADATA_POINTER;
	if (len < DVM24_METADATA_HEADER || len > DVM24_METADATA_MAX)
		return DVM24_METADATA_SIZE;
	/* Same ordinary-STDV PQ wire family admitted by dv_std_ipt_metadata(). */
	if (metadata[0] || metadata[1] > 1 || metadata[50] != 255 ||
	    metadata[51] != 255 || metadata[60] != 12 ||
	    metadata[61] != (output ? 2 : 0) || metadata[62] > 2 ||
	    (output && metadata[62] != 1) || metadata[63] != 1)
		return DVM24_METADATA_HEADER_UNSUPPORTED;
	for (i = 52; i < 60; i++)
		if (metadata[i])
			return DVM24_METADATA_HEADER_UNSUPPORTED;
	parsed.source_min_pq = dvm24_get_be16(metadata + 64);
	parsed.source_max_pq = dvm24_get_be16(metadata + 66);
	parsed.source_diagonal = dvm24_get_be16(metadata + 68);
	if (parsed.source_max_pq > 4095 ||
	    parsed.source_min_pq > parsed.source_max_pq)
		return DVM24_METADATA_RANGE;
	for (i = 0; i < metadata[70]; i++) {
		/* Test remaining length before any extension header read. Never form
		 * a speculative pointer using an untrusted 32-bit payload length.
		 */
		if (len - cursor < 5)
			return DVM24_METADATA_EXTENSION;
		payload_len = dvm24_get_be32(metadata + cursor);
		if (payload_len > len - cursor - 5)
			return DVM24_METADATA_EXTENSION;
		if (metadata[cursor + 4] == 1) {
			if (found_l1)
				return DVM24_METADATA_DUPLICATE_L1;
			if (payload_len < 6)
				return DVM24_METADATA_EXTENSION;
			parsed.scene_min_pq = dvm24_get_be16(metadata + cursor + 5);
			parsed.scene_max_pq = dvm24_get_be16(metadata + cursor + 7);
			parsed.scene_avg_pq = dvm24_get_be16(metadata + cursor + 9);
			if (parsed.scene_max_pq > 4095 ||
			    parsed.scene_min_pq > parsed.scene_max_pq ||
			    parsed.scene_avg_pq < parsed.scene_min_pq ||
			    parsed.scene_avg_pq > parsed.scene_max_pq)
				return DVM24_METADATA_RANGE;
			found_l1 = true;
		}
		cursor += 5 + payload_len;
	}
	if (cursor != len)
		return DVM24_METADATA_EXTENSION;
	if (!found_l1)
		return DVM24_METADATA_NO_L1;
	*out = parsed;
	return DVM24_METADATA_OK;
}

#endif
