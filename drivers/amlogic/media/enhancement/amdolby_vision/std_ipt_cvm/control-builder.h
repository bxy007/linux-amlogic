/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DV_STB24_CONTROL_BUILDER_H
#define DV_STB24_CONTROL_BUILDER_H

#include "metadata-reader.h"

#define DVM24_CONTROL_BYTES 160U
#define DVM24_PUBLIC_VIDEO_PRIORITY 0U

struct dvm24_control_input {
	struct dvm24_scene_metadata metadata;
	dvm24_u32 gui_min_q18;
	dvm24_u32 gui_max_q18;
	/* Caller supplies corresponding stock LToPQ12 results. No floating point
	 * or approximation is used inside the control constructor.
	 */
	dvm24_u16 gui_min_pq;
	dvm24_u16 gui_max_pq;
	dvm24_u32 public_priority;
};

enum dvm24_control_result {
	DVM24_CONTROL_OK = 0,
	DVM24_CONTROL_POINTER = -1,
	DVM24_CONTROL_PRIORITY = -2,
	DVM24_CONTROL_RANGE = -3
};

/* Restricted old-stock profile: PQ RGB graphics, SecIn=1, Video Priority,
 * graphics diagonal42, neutral trims/default InitTmCtrl, common GUI PMin/PMax
 * and grcSigEnv.Min/Max. Does not rewrite source/output metadata or L1.
 * This explicit profile does not preserve arbitrary CM2/CM4 trims.
 * On failure the caller's160B output remains unchanged.
 */
int dvm24_build_graphics_control(const struct dvm24_control_input *input,
				 dvm24_u8 output[DVM24_CONTROL_BYTES]);

#endif
