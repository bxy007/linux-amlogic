/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DVM24_GRAPHICS_TRIM_H
#define DVM24_GRAPHICS_TRIM_H
#include "metadata-reader.h"

enum dvm24_trim_result {
	DVM24_TRIM_OK = 0,
	DVM24_TRIM_POINTER = -1,
	DVM24_TRIM_PACKET = -2,
	DVM24_TRIM_UNSUPPORTED = -3,
	DVM24_TRIM_RANGE = -4,
	DVM24_TRIM_COUNT = -5
};

/* Caller must already have selected old Video Priority. Only effective Bwd1
 * takes metadata L2; Fwd0/Pass2 preserve the builder's old defaults.
 * Source is the ordinary public DM input packet (not raw RPU bits).
 * Failure never partly modifies control. L1 and output metadata are read-only.
 */
int dvm24_apply_graphics_trim(const dvm24_u8 *source, dvm24_u32 len,
			    dvm24_u8 control[160]);
#endif
