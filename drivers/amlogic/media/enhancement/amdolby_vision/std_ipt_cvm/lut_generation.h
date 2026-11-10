#include "types.h"
/* Offline STB2.4 instruction-derived curve-generation ABI. */
#ifndef DV_STB24_LUT_GENERATION_H
#define DV_STB24_LUT_GENERATION_H

struct dv_stb24_val_norm {
    int32_t val;
    uint16_t norm;
    uint16_t reserved;
};

/* tuning is the original 48-byte displayPqTuning_t_ little-endian image.
 * size is the signed-16-bit native argument, qualified for 256/512 entries.
 * If out4k is non-NULL, out515 must have 515 entries and size must be 512.
 * Otherwise out515 points at size entries. */
void dv_stb24_generate_tone_curve_lut(
    int32_t xmin, int32_t xmax,
    const struct dv_stb24_val_norm *p1, const struct dv_stb24_val_norm *p2,
    const struct dv_stb24_val_norm *p3, int32_t sor, const uint8_t tuning[48],
    int32_t trim_slope, int32_t trim_power, int32_t trim_offset,
    uint16_t *out515, uint16_t *out4k, int16_t size, uint16_t clamp_out);

void dv_stb24_saturation_map_i(int32_t tmin, int32_t tmax, int32_t smin,
                          int32_t sat_gain, const uint16_t *tmap,
                          uint16_t mode, uint16_t *smap, int16_t size);

/* metadata is the mutable 56-byte srcMetadata_t_ image; the native function
 * clamps and adjusts its first words.  ratio may be NULL. */
void dv_stb24_create_tone_curve_3pxx(
    uint8_t metadata[56], const uint8_t tuning[48],
    struct dv_stb24_val_norm *p1, struct dv_stb24_val_norm *p2,
    struct dv_stb24_val_norm *p3, int32_t *sor,
    uint16_t *tmap_i, uint16_t *tmap_s, uint16_t *smap_i, uint16_t *smap_s,
    int32_t *ratio, int16_t size, uint16_t clamp_out);
#endif
