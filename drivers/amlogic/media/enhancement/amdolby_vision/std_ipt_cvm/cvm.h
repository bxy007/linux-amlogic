/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STB24_CVM_H
#define DV_STB24_CVM_H

#include "numeric.h"
#include "pq12.h"
#include "control-builder.h"

#define DV_STB24_LUT_WORDS 1280

/* One context belongs to the driver's existing serialized control/apply path.
 * Preallocated: no allocations, FPU or sleeping operations in frame handling.
 * Native algorithm caches, Composer and input metadata are read only. The
 * coupled policy constructs a separate transmitted packet in its caller.
 */
struct dv_stb24_cvm {
	struct dv_stb24_numeric_workspace numeric;
	u32 lut[DV_STB24_LUT_WORDS];
	u8 control[DVM24_CONTROL_BYTES];
	bool valid;
	bool changed;
	const char *reason;
	int detail;
	u64 builds;
	u64 hits;
	u64 failures;
	u32 generation;
	u32 last_us;
	u32 max_us;
};

int dv_stb24_cvm_update(struct dv_stb24_cvm *state,
	const u8 *metadata, u32 bytes, u32 gui_min_q18, u32 gui_max_q18,
	u32 public_priority);

int dv_stb24_cvm_update_source(struct dv_stb24_cvm *state,
	const u8 *metadata, u32 bytes, u32 gui_min_q18, u32 gui_max_q18,
	u32 public_priority);

#endif
