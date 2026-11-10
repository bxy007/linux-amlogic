/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STB24_METADATA_POLICY_H
#define DV_STB24_METADATA_POLICY_H
#include "metadata-reader.h"

struct dvm24_output_info {
	dvm24_u16 input_l1[3];
	dvm24_u16 output_l1[3];
	dvm24_u32 l2_offsets_clamped;
	bool changed;
	bool legacy; /* Old L1 compatibility clamp, not VSVDB version. */
};

/* vsvdb is the complete effective CTA extended Dolby block, including the
 * eN/01/46d000 prefix. Returns target_param.sink_dm_version, not VSVDB version.
 */
int dvm24_sink_version(const dvm24_u8 *vsvdb, dvm24_u32 len,
			 dvm24_u32 *sink_dm_version);

/* Apply old scalar output policy while retaining all CE blocks/order/L5.
 * source and native are immutable; output must be a distinct512B object.
 * control is the same frame's complete old graphics control160, including
 * consumed source trim state. This helper does not generate those controls.
 * Unknown/unsupported/ambiguous inputs leave output/out_len/info unchanged.
 */
int dvm24_output_policy(const dvm24_u8 *source, dvm24_u32 source_len,
		       const dvm24_u8 *native, dvm24_u32 native_len,
		       const dvm24_u8 control[160], dvm24_u32 sink_dm_version,
		       dvm24_u8 output[512], dvm24_u32 *out_len,
		       struct dvm24_output_info *info);
#endif
