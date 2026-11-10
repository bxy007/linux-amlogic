#include "lut_generation.h"
#include "curve_parameters.h"
#include "fixed.h"
#include "tables.h"
/* Offline instruction-derived STB2.4 LUT generation, not production code.
 * Original AArch64 ELF SHA256:
 * a66b5260dd7cf58531ab65426e19538f2f0732eca7d262bfac435277dbe2ffb9
 * Entry points fa04 GenerateToneCurveLut, e7b4 SaturationMapI, dfa8
 * CreateToneCurve3pxx.  Arithmetic deliberately retains native W/X widths.
 */


static int32_t i32(uint32_t v) { int32_t s; memcpy(&s, &v, 4); return s; }
static int64_t i64(uint64_t v) { int64_t s; memcpy(&s, &v, 8); return s; }
static uint64_t sx32(uint32_t v) { return (uint64_t)(int64_t)i32(v); }
static uint32_t add32(uint32_t a, uint32_t b) { return a+b; }
static uint32_t sub32(uint32_t a, uint32_t b) { return a-b; }
static uint64_t asr64(uint64_t v, uint32_t shift)
{
    shift &= 63;
    if (!shift) return v;
    return (v >> shift) | ((0ull-(v >> 63)) << (64-shift));
}
static uint32_t asr32(uint32_t v, uint32_t shift)
{
    shift &= 31;
    if (!shift) return v;
    return (v >> shift) | ((0u-(v >> 31)) << (32-shift));
}
static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (uint32_t)p[1]<<8); }
static void wr32(uint8_t *p, uint32_t v)
{
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
static uint32_t min_s32(uint32_t a, uint32_t b) { return i32(a)<i32(b) ? a:b; }
static uint32_t max_s32(uint32_t a, uint32_t b) { return i32(a)>i32(b) ? a:b; }
static uint32_t sat_s32(uint64_t v)
{
    if (i64(v)>INT32_MAX) return INT32_MAX;
    if (i64(v)<INT32_MIN) return 0x80000000u;
    return (uint32_t)v;
}
static uint64_t sdiv64(uint64_t a,uint64_t b)
{
    return dv_stb24_raw_div64(a,b);
}
static uint32_t sdiv32(uint32_t a, uint32_t b)
{
    if (!b) return 0;
    if (a==0x80000000u && b==UINT32_MAX) return a;
    return (uint32_t)(i32(a)/i32(b));
}
static uint32_t mulq31(uint32_t a, uint32_t b)
{
    return (uint32_t)dv_stb24_smpy_ss(a,b);
}
static uint32_t pow2_(uint32_t a, uint32_t in, uint32_t out)
{
    return (uint32_t)dv_stb24_fxPOW2Taylor(a,in,out);
}
static uint32_t log2_(uint32_t a, uint32_t out)
{
    return (uint32_t)dv_stb24_fxLOG2Taylor(a,out);
}
static uint32_t clamp_pq(uint32_t value, const uint8_t tuning[48], uint16_t clamp)
{
    if (!clamp) return value;
    value &= 65535;
    value=max_s32(asr32(rd32(tuning),19),value);
    return min_s32(asr32(rd32(tuning+4),19),value);
}

void dv_stb24_generate_tone_curve_lut(
    int32_t xmin, int32_t xmax,
    const struct dv_stb24_val_norm *p1, const struct dv_stb24_val_norm *p2,
    const struct dv_stb24_val_norm *p3, int32_t sor, const uint8_t tuning[48],
    int32_t trim_slope, int32_t trim_power, int32_t trim_offset,
    uint16_t *out515, uint16_t *out4k, int16_t size, uint16_t clamp_out)
{
    uint16_t mode=rd16(tuning+44), last=0;
    uint16_t *out=out515+(out4k!=NULL);
    int32_t step=4096/size;
    uint32_t slope=add32((uint32_t)trim_slope,0x10000000u);
    uint32_t power=add32((uint32_t)trim_power,0x10000000u);
    int32_t index=0;
    (void)xmin; (void)xmax; /* present in ABI, unused by this compiled function */
    if (mode==1) {
        uint32_t n=p2->norm<p3->norm ? p2->norm:p3->norm;
        uint32_t inverse_sor=(uint32_t)dv_stb24_sdiv_sls(1ull<<59,(uint32_t)sor);
        uint32_t inverse_power=(uint32_t)dv_stb24_sdiv_sls(1ull<<56,power);
        uint32_t inverse_slope=(uint32_t)dv_stb24_sdiv_sls(1ull<<56,slope);
        for (index=0; index<4096; index+=step) {
            uint32_t a=pow2_(mulq31(inverse_power,dv_stb24_tone_xlog[index]),23,28);
            uint64_t v;
            uint32_t code, output, norm, denom_arg, ratio, log_value;
            uint64_t cubic, denominator, numerator;
            a=(uint32_t)dv_stb24_ssub_sss(a,(uint32_t)trim_offset);
            v=asr64(sx32(a)*sx32(inverse_slope),43);
            a=(uint32_t)v;
            code=asr32(add32(a,1),i32(a)>8188 ? 2:1);
            code=min_s32(code,4095);
            if (i32(code)<0) code=0;
            if (i32(code)<3) cubic=0;
            else {
                uint32_t l=dv_stb24_tone_xlog[code];
                uint32_t y;
                if (i32(a)>8188) l=add32(l,0x04000000u);
                y=l*3u;
                if (i32(y)>=0x04000001) {
                    uint32_t excess=sub32(y,0x04000001u);
                    uint32_t exponent=excess>>26;
                    uint32_t mantissa=sub32(sub32(y,excess & 0xfc000000u),0x04000000u);
                    cubic=sx32(pow2_(mantissa,26,29))*sx32(2u<<(exponent & 31));
                } else cubic=sx32(pow2_(y,26,29));
            }
            denominator=asr64(sx32((uint32_t)p2->val)<<29,p2->norm-n)
                       -asr64(cubic*sx32((uint32_t)p3->val),p3->norm-n);
            numerator=(cubic<<29)-(sx32((uint32_t)p1->val)<<((58u-p1->norm)&63));
            norm=dv_stb24_unorm_sl(denominator);
            denom_arg=add32((uint32_t)((denominator<<(norm&63))>>32),1);
            ratio=(uint32_t)dv_stb24_sdiv_sls(numerator,denom_arg);
            ratio=(uint32_t)dv_stb24_sshr_sss(ratio,sub32(30u-n,norm));
            output=0;
            if (i32(ratio)>=1) {
                log_value=add32(log2_(ratio,26),0x84000000u);
                a=(uint32_t)asr64(sx32(log_value)*sx32(inverse_sor),33);
                output=pow2_(a,24,31);
                if (i32(output)<0) output=0;
                output=asr32((uint32_t)dv_stb24_sadd_sss(output,0x40000),19);
            }
            output=clamp_pq(output,tuning,clamp_out);
            if ((uint16_t)output>last) last=(uint16_t)output;
            *out++=last;
        }
    } else if (mode==0) {
        uint32_t n=p1->norm<p2->norm ? p1->norm:p2->norm;
        uint32_t n3=p3->norm<31 ? p3->norm:31;
        uint32_t gain=slope << 2; /* fd0c uses w25 after fa74 adds unity. */
        uint64_t base=1ull<<((n3+31)&63);
        for (index=0; index<4096; index+=step) {
            uint32_t x=pow2_(mulq31((uint32_t)sor,dv_stb24_tone_xlog[index]),23,31);
            uint32_t p1shift=sub32(p1->norm,n), norm, value, exponent, output;
            uint64_t numerator, denominator, quotient, normalized;
            if (i32(p1shift)>31) p1shift=31;
            numerator=asr64(sx32((uint32_t)p2->val)*sx32(x),sub32(p2->norm,n));
            denominator=asr64(sx32((uint32_t)p3->val)*sx32(x),sub32(p3->norm,n3));
            denominator=asr64(denominator+base,n3)+1;
            numerator+=sx32(0x7fffffffu>>(p1shift&31))*sx32((uint32_t)p1->val);
            quotient=sdiv64(numerator,denominator);
            norm=dv_stb24_unorm_sl(quotient);
            normalized=asr64(quotient<<(norm&63),32);
            value=i64(normalized)>0 ? (uint32_t)normalized:0;
            if (value) {
                exponent=add32(32u+n,norm);
                value=sub32(log2_(value,26),exponent<<26);
                value=(uint32_t)asr64(sx32(rd32(tuning+36))*sx32(value),33);
                value=pow2_(value,24,29);
            }
            value=mulq31(value,gain);
            value=(uint32_t)dv_stb24_sadd_sss(value,(uint32_t)trim_offset);
            output=0;
            if (i32(value)>=1) {
                value=add32(log2_(value,26),0x90000000u);
                value=sat_s32(asr64(sx32(value)*sx32(power),30));
                output=pow2_(value,24,31);
                if (i32(output)<0) output=0;
            }
            output=asr32((uint32_t)dv_stb24_sadd_sss(output,0x40000),19);
            output=clamp_pq(output,tuning,clamp_out);
            if ((uint16_t)output>last) last=(uint16_t)output;
            *out++=last;
        }
    } else {
        for (index=0; index<4096; index+=step)
            *out++=(uint16_t)clamp_pq((uint32_t)index,tuning,clamp_out);
        last=0; /* original ff50 explicitly sets w22=0 in this branch */
    }
    if (out4k) {
        static const int32_t coef[7][4]={
            {-2272,60523,8651,-1366}, {-3638,53827,17949,-2602},
            {-4225,45833,27505,-3577}, {-4161,36929,36929,-4161},
            {-3577,27505,45833,-4225}, {-2602,17949,53827,-3638},
            {-1366,8651,60523,-2272}};
        unsigned i,j,k;
        out515[0]=out515[1];
        out515[513]=out515[512]; out515[514]=out515[512];
        for (i=0;i<512;i++) {
            int32_t anchor=(int16_t)out515[i+1];
            out4k[8*i]=(uint16_t)anchor;
            for (j=0;j<7;j++) {
                uint32_t sum=0;
                for (k=0;k<4;k++)
                    sum+=(uint32_t)(int32_t)(int16_t)out515[i+k]*(uint32_t)coef[j][k];
                sum=asr32(sum+0x8000u,16);
                sum=max_s32(sum,(uint32_t)anchor);
                sum=min_s32(sum,last);
                out4k[8*i+j+1]=(uint16_t)sum;
            }
        }
    }
}

void dv_stb24_saturation_map_i(int32_t tmin, int32_t tmax, int32_t smin,
                          int32_t sat_gain, const uint16_t *tmap,
                          uint16_t mode, uint16_t *smap, int16_t size)
{
    uint32_t index, shift=size==256 ? 4:3;
    if (mode==1) {
        for (index=0;index<(uint16_t)size;index++) {
            uint32_t x=(uint32_t)(int32_t)(int16_t)(index<<shift);
            uint32_t value=sub32(x,(uint32_t)(int32_t)(int16_t)tmap[index])+4096u;
            value=sdiv32(0x1000000u,value);
            value=(uint32_t)((sx32(value)*sx32((uint32_t)sat_gain))>>28);
            if (i32(value)<0) value=0;
            value=min_s32(value,8191);
            smap[index]=(uint16_t)value;
        }
    } else if (mode==0) {
        uint32_t minpq=asr32((uint32_t)tmin,19), maxpq=asr32((uint32_t)tmax,19);
        uint32_t span=sub32(maxpq,minpq), range_gain=0x20000000u;
        uint32_t gap=sub32(minpq,asr32((uint32_t)smin,19));
        uint64_t factor;
        if (i32(span)>0) range_gain/=span;
        factor=sx32(gap*3u)*sx32(0x02c6734cu);
        for (index=0;index<(uint16_t)size;index++) {
            uint32_t mapped=(uint32_t)(int32_t)(int16_t)tmap[index];
            uint32_t x=(uint32_t)(int32_t)(int16_t)(index<<shift);
            uint32_t value=sub32(mapped,x)+4096u;
            value=(uint32_t)((sx32(value)*sx32((uint32_t)sat_gain))>>28);
            if (i32(gap)>=1) {
                uint32_t delta=sub32(maxpq,mapped), lookup, fraction, linear;
                uint64_t address, correction, multiplier;
                if (i32(delta)<0) delta=0;
                address=sx32(delta)*(uint64_t)range_gain+8192;
                lookup=(uint32_t)((address>>20)&1023);
                linear=dv_stb24_depq[lookup];
                if (lookup!=1023) {
                    fraction=(uint32_t)((address>>14)&63);
                    linear+=(uint32_t)(((uint64_t)fraction*(uint32_t)(dv_stb24_depq[lookup+1]-linear))>>6);
                }
                correction=factor*(uint64_t)linear+(1ull<<49);
                if (i64(correction)>INT64_C(0x3fffffffffffffff)) value=0;
                else {
                    multiplier=4096u-asr64(correction,50);
                    value=(uint32_t)((multiplier*sx32(value)+2048)>>12);
                }
            }
            if (i32(value)>8190) value=8191;
            smap[index]=(uint16_t)value;
        }
    } else {
        for (index=0;index<(uint16_t)size;index++) smap[index]=4095;
    }
}

static void vector_table(uint16_t *table, int16_t size, uint16_t mode,
                         uint32_t weight, int32_t s2t_ratio)
{
    int j;
    uint32_t norm, increment, counter=0, shift;
    if (mode>=2) {
        for (j=0;j<size;j++) table[j]=4095;
        return;
    }
    norm=dv_stb24_unorm_s(weight);
    increment=(uint32_t)dv_stb24_sshr_sss(weight,9u-norm);
    shift=norm+33;
    if (i32(shift)>63) shift=63;
    for (j=0;j<size;j++) {
        uint32_t delta=sat_s32(asr64(sx32(counter)*sx32((uint32_t)s2t_ratio),shift));
        uint32_t value;
        delta=asr32(delta+1,1);
        if (mode) delta=0u-delta;
        value=min_s32(4095u-delta,8191);
        if (i32(value)<0) value=0;
        table[j]=(uint16_t)value;
        counter+=increment;
    }
}

void dv_stb24_create_tone_curve_3pxx(
    uint8_t metadata[56], const uint8_t tuning[48],
    struct dv_stb24_val_norm *p1, struct dv_stb24_val_norm *p2,
    struct dv_stb24_val_norm *p3, int32_t *sor,
    uint16_t *tmap_i, uint16_t *tmap_s, uint16_t *smap_i, uint16_t *smap_s,
    int32_t *ratio, int16_t size, uint16_t clamp_out)
{
    uint32_t slope=rd32(metadata+24), power=rd32(metadata+32), offset=rd32(metadata+28);
    uint32_t crush=rd32(metadata), mid=rd32(metadata+4), clip=rd32(metadata+8);
    uint16_t mode=rd16(tuning+44);
    int all_equal;
    int32_t tmin=0,tmid=0,tmax=0,smid=0,curve_slope=0,s2t_ratio=0,screen_ratio=0;
    uint32_t source_max;
    slope=min_s32(max_s32(slope,0xf8000000u),0x08000000u);
    power=min_s32(max_s32(power,0xf8000000u),0x08000000u);
    offset=min_s32(max_s32(offset,0xf8000000u),0x08000000u);
    clip=max_s32(clip,0x4108b6b6u);
    crush=min_s32(crush,0x13316bf8u);
    mid=max_s32(mid,crush+0x15e00000u);
    mid=min_s32(mid,clip-0x80000u);
    wr32(metadata,crush);wr32(metadata+4,mid);wr32(metadata+8,clip);
    all_equal=(clip==mid && mid==crush);
    if (mode==1) {
        if (all_equal) {
            crush=rd32(metadata+12);clip=rd32(metadata+16);
            wr32(metadata,crush);wr32(metadata+4,crush+0x80000u);wr32(metadata+8,clip);
        }
        source_max=clip;
    } else {
        uint64_t magnitude;
        uint32_t radius;
        if (all_equal) {
            crush=rd32(tuning);clip=rd32(tuning+4);
            wr32(metadata,crush);wr32(metadata+4,crush+0x80000u);wr32(metadata+8,clip);
        }
        magnitude=asr64(sx32(rd32(tuning+12))*sx32(0x10aabu),12);
        if (magnitude>>63) magnitude=0ull-magnitude;
        radius=i64(magnitude)<INT32_MAX ? (uint32_t)magnitude:INT32_MAX;
        source_max=mulq31(0x7fffffffu-radius,clip)+radius;
    }
    dv_stb24_calc_mapped_params_3pxx(i32(source_max),metadata,tuning,
        &tmin,&tmid,&tmax,&smid,&curve_slope,&s2t_ratio,&screen_ratio);
    dv_stb24_calc_curve_params(i32(rd32(metadata)),smid,i32(source_max),
        tmin,tmid,tmax,curve_slope,tuning,p1,p2,p3,sor);
    dv_stb24_generate_tone_curve_lut(i32(rd32(metadata)),i32(source_max),p1,p2,p3,*sor,tuning,
        i32(slope),i32(power),i32(offset),tmap_i,NULL,size,clamp_out);
    if (ratio) { *ratio=s2t_ratio; return; }
    vector_table(tmap_s,size,mode,rd32(metadata+48),s2t_ratio);
    vector_table(smap_s,size,mode,rd32(metadata+52),s2t_ratio);
    dv_stb24_saturation_map_i(i32(rd32(tuning)),i32(rd32(tuning+4)),i32(rd32(metadata+12)),
        i32(rd32(metadata+40)),tmap_i,mode,smap_i,size);
    (void)screen_ratio;
}
