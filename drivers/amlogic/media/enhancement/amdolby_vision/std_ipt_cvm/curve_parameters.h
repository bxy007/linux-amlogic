#include "types.h"
#include "lut_generation.h"
#ifndef DV_STB24_CURVE_PARAMETERS_H
#define DV_STB24_CURVE_PARAMETERS_H
void dv_stb24_calc_curve_params(int32_t smin, int32_t smid, int32_t smax,
    int32_t tmin, int32_t tmid, int32_t tmax, int32_t slope,
    const uint8_t tuning[48], struct dv_stb24_val_norm *p1,
    struct dv_stb24_val_norm *p2, struct dv_stb24_val_norm *p3, int32_t *sor);
void dv_stb24_calc_mapped_params_3pxx(int32_t smax, const uint8_t metadata[56],
    const uint8_t tuning[48], int32_t *tmin, int32_t *tmid, int32_t *tmax,
    int32_t *smid, int32_t *slope, int32_t *s2t_ratio, int32_t *screen_ratio);
#endif
