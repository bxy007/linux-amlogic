/* SPDX-License-Identifier: GPL-2.0 */
/* Experimental direct-IPT GUI mapping, including frame-matched source trims. */
#include "cvm.h"
#include "trim.h"
#include "metadata-policy.h"
#ifdef DV_STB24_HOST_TEST
#include <string.h>
#include <errno.h>
#else
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/ktime.h>
#endif

static int dv_stb24_cvm_update_input(struct dv_stb24_cvm *s,
	const u8 *metadata, u32 bytes, u32 gui_min_q18, u32 gui_max_q18,
	u32 public_priority, bool output_packet, bool source_trims)
{
	struct dvm24_control_input in;
	u8 control[DVM24_CONTROL_BYTES];
	int ret;
#ifndef DV_STB24_HOST_TEST
	u64 started;
#endif
	if (!s)
		return -EINVAL;
	s->changed = false;
	s->reason = "gui_range";
	/* Matches direct IPT's existing <=1000-nit GUI headroom contract. */
	if (!gui_max_q18 || gui_min_q18 >= gui_max_q18 ||
	    gui_max_q18 > 262144000U) {
		ret = -ERANGE;
		goto fail;
	}
	memset(&in, 0, sizeof(in));
	s->reason = "metadata";
	ret = dvm24_read_scene_metadata(metadata, bytes, output_packet, &in.metadata);
	if (ret)
		goto fail;
	in.gui_min_q18 = gui_min_q18;
	in.gui_max_q18 = gui_max_q18;
	in.gui_min_pq = dv_stb24_linear_to_pq12(gui_min_q18);
	in.gui_max_pq = dv_stb24_linear_to_pq12(gui_max_q18);
	in.public_priority = public_priority;
	s->reason = "control";
	ret = dvm24_build_graphics_control(&in, control);
	if (ret)
		goto fail;
	if (source_trims) {
		s->reason = "source_trim";
		ret = dvm24_apply_graphics_trim(metadata, bytes, control);
		if (ret)
			goto fail;
	}
	if (s->valid && !memcmp(s->control, control, sizeof(control))) {
		s->hits++;
		s->reason = "active";
		s->detail = 0;
		return 0;
	}
#ifndef DV_STB24_HOST_TEST
	started = ktime_get_ns();
#endif
	/* Only these four tables are private. G2L is appended by the caller from
	 * the exact native Core2 payload after its normal const-table handling.
	 */
	s->reason = "numeric";
	ret = dv_stb24_numeric_generate(&s->numeric, control, s->lut);
#ifndef DV_STB24_HOST_TEST
	s->last_us = (u32)((ktime_get_ns() - started) / 1000);
	if (s->last_us > s->max_us)
		s->max_us = s->last_us;
#endif
	if (ret)
		goto fail;
	memcpy(s->control, control, sizeof(control));
	s->valid = true;
	s->changed = true;
	s->reason = "active";
	s->detail = 0;
	s->generation++;
	s->builds++;
	return 0;
fail:
	/* An invalid current packet never consumes a previous frame's mapping. */
	s->valid = false;
	s->detail = ret;
	s->failures++;
	return -EINVAL;
}

/* Retained for neutral-profile offline regression. Production uses the
 * original source packet and consumes its applicable old graphics trims.
 */
int dv_stb24_cvm_update(struct dv_stb24_cvm *s,
	const u8 *metadata, u32 bytes, u32 gui_min_q18, u32 gui_max_q18,
	u32 public_priority)
{
	return dv_stb24_cvm_update_input(s, metadata, bytes, gui_min_q18,
		gui_max_q18, public_priority, true, false);
}

int dv_stb24_cvm_update_source(struct dv_stb24_cvm *s,
	const u8 *metadata, u32 bytes, u32 gui_min_q18, u32 gui_max_q18,
	u32 public_priority)
{
	return dv_stb24_cvm_update_input(s, metadata, bytes, gui_min_q18,
		gui_max_q18, public_priority, false, true);
}

#ifndef DV_STB24_HOST_TEST
EXPORT_SYMBOL_GPL(dv_stb24_cvm_update);
EXPORT_SYMBOL_GPL(dv_stb24_cvm_update_source);
EXPORT_SYMBOL_GPL(dvm24_output_policy);
EXPORT_SYMBOL_GPL(dvm24_sink_version);
MODULE_DESCRIPTION("Experimental standard-DV direct-IPT GUI CVM and coupled metadata policy");
MODULE_LICENSE("GPL");
#endif
