/* SPDX-License-Identifier: GPL-2.0 */
#include "control-builder.h"
#ifdef DV_STB24_METADATA_HOST_TEST
#include <string.h>
#else
#include <linux/string.h>
#endif

static void put16(dvm24_u8 *p, dvm24_u16 n)
{
	p[0] = n;
	p[1] = n >> 8;
}

static void put32(dvm24_u8 *p, dvm24_u32 n)
{
	p[0] = n;
	p[1] = n >> 8;
	p[2] = n >> 16;
	p[3] = n >> 24;
}

int dvm24_build_graphics_control(const struct dvm24_control_input *in,
				 dvm24_u8 output[DVM24_CONTROL_BYTES])
{
	dvm24_u8 data[DVM24_CONTROL_BYTES] = {0};
	const struct dvm24_scene_metadata *m;
	dvm24_u32 mode, minimum, maximum;

	if (!in || !output)
		return DVM24_CONTROL_POINTER;
	if (in->public_priority != DVM24_PUBLIC_VIDEO_PRIORITY)
		return DVM24_CONTROL_PRIORITY;
	m = &in->metadata;
	if (in->gui_min_q18 >= in->gui_max_q18 ||
	    in->gui_max_q18 > 2621440000U ||
	    in->gui_min_pq > in->gui_max_pq || in->gui_max_pq > 4095 ||
	    m->source_min_pq >= m->source_max_pq || m->source_max_pq > 4095 ||
	    !m->source_diagonal || m->scene_max_pq > 4095 ||
	    m->scene_min_pq > m->scene_avg_pq ||
	    m->scene_avg_pq > m->scene_max_pq)
		return DVM24_CONTROL_RANGE;
	/* Old CommitMds: GUI source limits from common PMin/PMax, diagonal42,
	 * and source crush/clip/mid from the full GUI range (81c0..8370).
	 */
	put16(data, in->gui_min_pq);
	put16(data + 2, in->gui_max_pq);
	put16(data + 4, 42);
	put16(data + 20, in->gui_min_pq);
	put16(data + 22, in->gui_max_pq);
	put16(data + 24, ((dvm24_u32)in->gui_min_pq + in->gui_max_pq) >> 1);
	/* Vpri primary target takes metadata source range; graphics borrows it
	 * from primary (87e4..8800,8a1c..8a3c). This is not display peak nits.
	 */
	put16(data + 28, m->source_min_pq);
	put16(data + 30, m->source_max_pq);
	put16(data + 32, m->source_diagonal);
	/* Scene anchors include GUI range then stay within source range
	 * (8944..8988,8d34..8d3c). This changes only the private curve control;
	 * the supplied/sent L1 is read-only, as requested for this experiment.
	 */
	minimum = m->scene_min_pq < in->gui_min_pq ? m->scene_min_pq : in->gui_min_pq;
	if (minimum < m->source_min_pq)
		minimum = m->source_min_pq;
	maximum = m->scene_max_pq > in->gui_max_pq ? m->scene_max_pq : in->gui_max_pq;
	if (maximum > m->source_max_pq)
		maximum = m->source_max_pq;
	put16(data + 48, minimum);
	put16(data + 50, maximum);
	put16(data + 52, m->scene_avg_pq);
	/* Exact one-code tolerance and priority-independent direction selector
	 * from 8a20..8ab8. InitTmCtrl.BwdInvertible is0 in this profile.
	 */
	if ((dvm24_u32)in->gui_max_pq + 1 < m->source_max_pq)
		mode = 1;
	else if ((dvm24_u32)m->source_max_pq + 1 < in->gui_max_pq)
		mode = 0;
	else if ((dvm24_u32)in->gui_min_pq + 1 < m->source_min_pq ||
		 (dvm24_u32)m->source_min_pq + 1 < in->gui_min_pq)
		mode = in->gui_max_pq < m->source_max_pq;
	else
		mode = 2;
	put32(data + 56, mode);
	/* Real old InitTmCtrl defaults, then direction-specific CommitMds
	 * weights (8af4..8b50). Neutral CommitTrim/default L2 is retained.
	 */
	put32(data + 64, 715827882U);
	put32(data + 68, 3);
	put32(data + 72, 12288);
	put32(data + 76, mode == 1 ? 81920 : 24576);
	put32(data + 80, 24576);
	put32(data + 84, 4096);
	put32(data + 136, 4096);
	memcpy(output, data, sizeof(data));
	return DVM24_CONTROL_OK;
}
