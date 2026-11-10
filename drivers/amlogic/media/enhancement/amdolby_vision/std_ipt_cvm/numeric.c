/* Fixed STB2.4 scalar curve preparation, bounded to the qualified 512 knots.
 * No L1 extraction, CSC matrix replacement, device access or CP flags here. */
#include "numeric.h"

static u16 read16(const u8 *p)
{
	return (u16)(p[0] | (u32)p[1] << 8);
}

static u32 read32(const u8 *p)
{
	return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}

static void write16(u8 *p, u16 value)
{
	p[0] = (u8)value;
	p[1] = (u8)(value >> 8);
}

static void write32(u8 *p, u32 value)
{
	p[0] = (u8)value;
	p[1] = (u8)(value >> 8);
	p[2] = (u8)(value >> 16);
	p[3] = (u8)(value >> 24);
}

int dv_stb24_numeric_generate(struct dv_stb24_numeric_workspace *w,
			    const u8 c[DV_STB24_CONTROL_BYTES],
			    u32 packed[DV_STB24_PACKED_WORDS])
{
	static const unsigned int offsets[4] = {4, 0x404, 0x808, 0xc08};
	static const unsigned int pq_fields[] = {0, 2, 20, 22, 24, 28, 30, 48, 50, 52};
	u32 direction;
	unsigned int base, t, k;
	u8 *metadata, *tuning;

	if (!w || !c || !packed)
		return -EINVAL;
	direction = read32(c + 56);
	if (direction > 2 || read16(c) > read16(c + 2) ||
	    read16(c + 28) > read16(c + 30))
		return -EINVAL;
	for (k = 0; k < sizeof(pq_fields) / sizeof(pq_fields[0]); ++k)
		if (read16(c + pq_fields[k]) > 4095)
			return -ERANGE;
	metadata = w->metadata;
	tuning = w->tuning;
	memset(metadata, 0, sizeof(w->metadata));
	memset(tuning, 0, sizeof(w->tuning));
	memset(w->parameters, 0, sizeof(w->parameters));
	w->sor = 0;
	write32(tuning, (u32)read16(c + 28) << 19);
	write32(tuning + 4, (u32)read16(c + 30) << 19);
	write32(tuning + 8, (u32)read16(c + 104) << 19);
	write32(tuning + 12, (u32)read16(c + 108) << 19);
	write32(tuning + 16, (u32)read16(c + 106) << 19);
	write32(tuning + 20, (u32)read16(c + 110) << 19);
	write32(tuning + 24, (u32)read16(c + 112) << 19);
	write32(tuning + 28, read16(c + 32));
	write32(tuning + 32, read32(c + 84));
	write32(tuning + 36, read32(c + 64));
	write32(tuning + 40, read32(c + 68));
	write16(tuning + 44, (u16)direction);
	base = direction == 1 ? 48 : 20;
	write32(metadata, (u32)read16(c + base) << 19);
	write32(metadata + 4, (u32)read16(c + base + 4) << 19);
	write32(metadata + 8, (u32)read16(c + base + 2) << 19);
	write32(metadata + 12, (u32)read16(c) << 19);
	write32(metadata + 16, (u32)read16(c + 2) << 19);
	write32(metadata + 20, read32(c + 72) << 4);
	write32(metadata + 24, read32(c + 120) << 16);
	write32(metadata + 28, read32(c + 124) << 16);
	write32(metadata + 32, read32(c + 128) << 16);
	write32(metadata + 40, read32(c + 136) << 16);
	write32(metadata + 44, read16(c + 4));
	write32(metadata + 48, read32(c + 76) << 4);
	write32(metadata + 52, read32(c + 80) << 4);
	dv_stb24_create_tone_curve_3pxx(metadata, tuning,
		&w->parameters[0], &w->parameters[1], &w->parameters[2], &w->sor,
		w->tables[0], w->tables[1], w->tables[2], w->tables[3],
		NULL, 512, direction != 2);
	for (t = 0; t < 4; ++t)
		for (k = 0; k < 512; ++k)
			if (w->tables[t][k] > 0x7fff)
				return -ERANGE;
	write16(w->raw, 511);
	write16(w->raw + 2, 511);
	write16(w->raw + 0x804, 511);
	write16(w->raw + 0x806, 511);
	for (t = 0; t < 4; ++t) {
		for (k = 0; k < 512; ++k)
			write16(w->raw + offsets[t] + 2 * k, w->tables[t][k]);
		for (k = 0; k < 256; ++k)
			packed[t * 256 + k] = w->tables[t][2 * k] |
					(u32)w->tables[t][2 * k + 1] << 15;
	}
	return 0;
}
