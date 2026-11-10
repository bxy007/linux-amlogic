#ifndef DV_STB24_FIXED_H
#define DV_STB24_FIXED_H
#include "types.h"
s32 dv_stb24_sadd_sss(u32 a, u32 b);
s32 dv_stb24_ssub_sss(u32 a, u32 b);
u64 dv_stb24_smpy_ss(u32 a, u32 b);
u32 dv_stb24_unorm_s(u32 value);
u32 dv_stb24_unorm_sl(u64 value);
s32 dv_stb24_sshl_ssu(u32 value, u32 shift);
s32 dv_stb24_sshr_sss(u32 value, u32 shift);
u64 dv_stb24_sdiv_sls(u64 numerator, u32 denominator);
s32 dv_stb24_fxLOG2Taylor(u32 value, u32 scale_out);
s32 dv_stb24_fxPOW2Taylor(u32 value, u32 scale_in, u32 scale_out);
u32 dv_stb24_ssqrt_s(u32 value);
#endif
