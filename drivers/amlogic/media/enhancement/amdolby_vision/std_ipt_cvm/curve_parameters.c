#include "curve_parameters.h"
#include "fixed.h"
/* Generated from the fixed audited STB2.4 ELF; see generate-curve-parameters.py.
 * Register names are retained to audit width/rounding against the original ASM.
 * Pure C, no dynamic code/emulator or device accesses. */
static uint64_t sx32(uint32_t x){return x&0x80000000u?UINT64_C(0xffffffff00000000)|x:x;}
static uint64_t sx64(uint64_t x){return x;}
static int64_t dv_stb24_signed64(uint64_t x){int64_t v;memcpy(&v,&x,8);return v;}
static uint32_t a32(uint32_t x,uint64_t n){unsigned s=n&31;return s?((x>>s)|((0u-(x>>31))<<(32-s))):x;}
static uint64_t a64(uint64_t x,uint64_t n){unsigned s=n&63;return s?((x>>s)|((UINT64_C(0)-(x>>63))<<(64-s))):x;}
static uint64_t mul32(uint32_t a,uint32_t b){return (uint64_t)(dv_stb24_signed64(sx32(a))*dv_stb24_signed64(sx32(b)));}
static uint64_t div64(uint64_t a,uint64_t b){return dv_stb24_raw_div64(a,b);}
#define LDST(bits,type) static type ld##bits(uint64_t p){type v;memcpy(&v,(void*)(uintptr_t)p,sizeof(v));return v;} \
 static void st##bits(uint64_t p,uint64_t v){type q=(type)v;memcpy((void*)(uintptr_t)p,&q,sizeof(q));}
LDST(16,uint16_t) LDST(32,uint32_t) LDST(64,uint64_t)

void dv_stb24_calc_curve_params(int32_t smin,int32_t smid,int32_t smax,int32_t tmin,int32_t tmid,int32_t tmax,int32_t slope,const uint8_t tuning[48],struct dv_stb24_val_norm *p1,struct dv_stb24_val_norm *p2,struct dv_stb24_val_norm *p3,int32_t *sor){
uint64_t x[31]={0},fa=0,fb=0,fu=0,fv=0;uint64_t stack[32]={0};uint64_t sp=(uintptr_t)((uint8_t *)stack+224);
x[0]=(uint32_t)smin;x[1]=(uint32_t)smid;x[2]=(uint32_t)smax;x[3]=(uint32_t)tmin;x[4]=(uint32_t)tmid;x[5]=(uint32_t)tmax;x[6]=(uint32_t)slope;x[7]=(uintptr_t)tuning;st64(sp,(uintptr_t)p1);st64(sp+8,(uintptr_t)p2);st64(sp+16,(uintptr_t)p3);st64(sp+24,(uintptr_t)sor);
sp=sp-UINT64_C(224); /* 0000f328 sub sp, sp, #0xe0 */
/* shadow call stack bookkeeping omitted */ /* 0000f32c str x30, [x18], #8 */
st64((sp+(uint64_t)INT64_C(128))+0,x[29]);st64((sp+(uint64_t)INT64_C(128))+8,x[30]); /* 0000f330 stp x29, x30, [sp, #0x80] */
st64((sp+(uint64_t)INT64_C(144))+0,x[28]);st64((sp+(uint64_t)INT64_C(144))+8,x[27]); /* 0000f334 stp x28, x27, [sp, #0x90] */
st64((sp+(uint64_t)INT64_C(160))+0,x[26]);st64((sp+(uint64_t)INT64_C(160))+8,x[25]); /* 0000f338 stp x26, x25, [sp, #0xa0] */
st64((sp+(uint64_t)INT64_C(176))+0,x[24]);st64((sp+(uint64_t)INT64_C(176))+8,x[23]); /* 0000f33c stp x24, x23, [sp, #0xb0] */
st64((sp+(uint64_t)INT64_C(192))+0,x[22]);st64((sp+(uint64_t)INT64_C(192))+8,x[21]); /* 0000f340 stp x22, x21, [sp, #0xc0] */
st64((sp+(uint64_t)INT64_C(208))+0,x[20]);st64((sp+(uint64_t)INT64_C(208))+8,x[19]); /* 0000f344 stp x20, x19, [sp, #0xd0] */
x[29]=(sp+UINT64_C(128)); /* 0000f348 add x29, sp, #0x80 */
x[20]=(x[7]); /* 0000f34c mov x20, x7 */
x[24]=(uint32_t)((uint32_t)x[6]); /* 0000f350 mov w24, w6 */
st32((x[29]+(uint64_t)INT64_C(-16)),(uint32_t)x[5]); /* 0000f354 stur w5, [x29, #-0x10] */
x[21]=(uint32_t)((uint32_t)x[4]); /* 0000f358 mov w21, w4 */
x[22]=(uint32_t)((uint32_t)x[3]); /* 0000f35c mov w22, w3 */
x[23]=(uint32_t)((uint32_t)x[2]); /* 0000f360 mov w23, w2 */
x[25]=(uint32_t)((uint32_t)x[1]); /* 0000f364 mov w25, w1 */
x[26]=(uint32_t)((uint32_t)x[0]); /* 0000f368 mov w26, w0 */
/* profiling omitted; no numerical effect */ /* 0000f36c bl #0xf36c */
x[8]=(uint32_t)(ld32((x[20]+(uint64_t)INT64_C(40)))); /* 0000f370 ldr w8, [x20, #0x28] */
x[9]=(uint32_t)(UINT64_C(2147483647)); /* 0000f374 mov w9, #0x7fffffff */
x[10]=(UINT64_C(18446744071562067968)); /* 0000f378 mov x10, #-0x80000000 */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f37c mov w1, #0x1a */
x[8]=(uint32_t)(((uint32_t)x[8] << (UINT64_C(29) & 31))); /* 0000f380 lsl w8, w8, #0x1d */
x[8]=(mul32((uint32_t)x[8],(uint32_t)x[24])); /* 0000f384 smull x8, w8, w24 */
x[8]=(a64(x[8],UINT64_C(31))); /* 0000f388 asr x8, x8, #0x1f */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000f38c cmp x8, x9 */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[9]); /* 0000f390 csel x8, x8, x9, lt */
fu=x[8];fv=x[10];fa=sx64(fu);fb=sx64(fv); /* 0000f394 cmp x8, x10 */
x[9]=(uint32_t)(UINT64_C(18446744071562067968)); /* 0000f398 mov w9, #-0x80000000 */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000f39c mov w0, w26 */
x[24]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[8]:(uint32_t)x[9]); /* 0000f3a0 csel w24, w8, w9, gt */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f3a4 bl #0xf3a4 */
x[27]=(uint32_t)(UINT64_C(18446744071629176832)); /* 0000f3a8 mov w27, #-0x7c000000 */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f3ac add w0, w0, w27 */
x[1]=(uint32_t)((uint32_t)x[24]); /* 0000f3b0 mov w1, w24 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f3b4 bl #0xf3b4 */
x[1]=(uint32_t)(UINT64_C(23)); /* 0000f3b8 mov w1, #0x17 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f3bc mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f3c0 bl #0xf3c0 */
st32((x[29]+(uint64_t)INT64_C(-4)),(uint32_t)x[0]); /* 0000f3c4 stur w0, [x29, #-4] */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f3c8 bl #0xf3c8 */
st32((x[29]+(uint64_t)INT64_C(-36)),(uint32_t)x[0]); /* 0000f3cc stur w0, [x29, #-0x24] */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f3d0 mov w1, #0x1a */
x[0]=(uint32_t)((uint32_t)x[25]); /* 0000f3d4 mov w0, w25 */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f3d8 bl #0xf3d8 */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f3dc add w0, w0, w27 */
x[1]=(uint32_t)((uint32_t)x[24]); /* 0000f3e0 mov w1, w24 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f3e4 bl #0xf3e4 */
x[1]=(uint32_t)(UINT64_C(23)); /* 0000f3e8 mov w1, #0x17 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f3ec mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f3f0 bl #0xf3f0 */
st32((x[29]+(uint64_t)INT64_C(-8)),(uint32_t)x[0]); /* 0000f3f4 stur w0, [x29, #-8] */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f3f8 bl #0xf3f8 */
st32((x[29]+(uint64_t)INT64_C(-40)),(uint32_t)x[0]); /* 0000f3fc stur w0, [x29, #-0x28] */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f400 mov w1, #0x1a */
x[0]=(uint32_t)((uint32_t)x[23]); /* 0000f404 mov w0, w23 */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f408 bl #0xf408 */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f40c add w0, w0, w27 */
x[1]=(uint32_t)((uint32_t)x[24]); /* 0000f410 mov w1, w24 */
st32((x[29]+(uint64_t)INT64_C(-56)),(uint32_t)x[24]); /* 0000f414 stur w24, [x29, #-0x38] */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f418 bl #0xf418 */
x[1]=(uint32_t)(UINT64_C(23)); /* 0000f41c mov w1, #0x17 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f420 mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f424 bl #0xf424 */
x[23]=(uint32_t)((uint32_t)x[0]); /* 0000f428 mov w23, w0 */
st32((sp+(uint64_t)INT64_C(52)),(uint32_t)x[0]); /* 0000f42c str w0, [sp, #0x34] */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f430 bl #0xf430 */
fu=(uint32_t)x[22];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f434 cmp w22, #0 */
st32((x[29]+(uint64_t)INT64_C(-44)),(uint32_t)x[0]); /* 0000f438 stur w0, [x29, #-0x2c] */
x[0]=(uint32_t)((dv_stb24_signed64(fa-fb)<0)?(0-(uint32_t)x[22]):(uint32_t)x[22]); /* 0000f43c cneg w0, w22, mi */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f440 mov w1, #0x1a */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f444 bl #0xf444 */
x[8]=(uint32_t)(ld32((x[20]+(uint64_t)INT64_C(40)))); /* 0000f448 ldr w8, [x20, #0x28] */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f44c add w0, w0, w27 */
x[1]=(uint32_t)(((uint32_t)x[8] << (UINT64_C(29) & 31))); /* 0000f450 lsl w1, w8, #0x1d */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f454 bl #0xf454 */
x[1]=(uint32_t)(UINT64_C(24)); /* 0000f458 mov w1, #0x18 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f45c mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f460 bl #0xf460 */
fu=(uint32_t)x[22];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f464 cmp w22, #0 */
x[28]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(0-(uint32_t)x[0]):(uint32_t)x[0]); /* 0000f468 cneg w28, w0, lt */
x[0]=(uint32_t)((uint32_t)x[28]); /* 0000f46c mov w0, w28 */
st32((x[29]+(uint64_t)INT64_C(-52)),(uint32_t)x[28]); /* 0000f470 stur w28, [x29, #-0x34] */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f474 bl #0xf474 */
fu=(uint32_t)x[21];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f478 cmp w21, #0 */
x[19]=(uint32_t)((uint32_t)x[0]); /* 0000f47c mov w19, w0 */
st32((x[29]+(uint64_t)INT64_C(-12)),(uint32_t)x[0]); /* 0000f480 stur w0, [x29, #-0xc] */
x[0]=(uint32_t)((dv_stb24_signed64(fa-fb)<0)?(0-(uint32_t)x[21]):(uint32_t)x[21]); /* 0000f484 cneg w0, w21, mi */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f488 mov w1, #0x1a */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f48c bl #0xf48c */
x[8]=(uint32_t)(ld32((x[20]+(uint64_t)INT64_C(40)))); /* 0000f490 ldr w8, [x20, #0x28] */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f494 add w0, w0, w27 */
x[1]=(uint32_t)(((uint32_t)x[8] << (UINT64_C(29) & 31))); /* 0000f498 lsl w1, w8, #0x1d */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f49c bl #0xf49c */
x[1]=(uint32_t)(UINT64_C(24)); /* 0000f4a0 mov w1, #0x18 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f4a4 mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f4a8 bl #0xf4a8 */
fu=(uint32_t)x[21];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f4ac cmp w21, #0 */
x[26]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(0-(uint32_t)x[0]):(uint32_t)x[0]); /* 0000f4b0 cneg w26, w0, lt */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000f4b4 mov w0, w26 */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f4b8 bl #0xf4b8 */
x[8]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-16)))); /* 0000f4bc ldur w8, [x29, #-0x10] */
x[21]=(uint32_t)((uint32_t)x[0]); /* 0000f4c0 mov w21, w0 */
st32((sp+(uint64_t)INT64_C(28)),(uint32_t)x[0]); /* 0000f4c4 str w0, [sp, #0x1c] */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000f4c8 mov w1, #0x1a */
fu=(uint32_t)x[8];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f4cc cmp w8, #0 */
x[0]=(uint32_t)((dv_stb24_signed64(fa-fb)<0)?(0-(uint32_t)x[8]):(uint32_t)x[8]); /* 0000f4d0 cneg w0, w8, mi */
x[22]=(uint32_t)((uint32_t)x[8]); /* 0000f4d4 mov w22, w8 */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000f4d8 bl #0xf4d8 */
x[8]=(uint32_t)(ld32((x[20]+(uint64_t)INT64_C(40)))); /* 0000f4dc ldr w8, [x20, #0x28] */
x[0]=(uint32_t)((uint32_t)x[0]+(uint32_t)x[27]); /* 0000f4e0 add w0, w0, w27 */
x[1]=(uint32_t)(((uint32_t)x[8] << (UINT64_C(29) & 31))); /* 0000f4e4 lsl w1, w8, #0x1d */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f4e8 bl #0xf4e8 */
x[1]=(uint32_t)(UINT64_C(24)); /* 0000f4ec mov w1, #0x18 */
x[2]=(uint32_t)(UINT64_C(31)); /* 0000f4f0 mov w2, #0x1f */
x[0]=((uint32_t)dv_stb24_fxPOW2Taylor(x[0],x[1],x[2])); /* 0000f4f4 bl #0xf4f4 */
x[8]=(ld64((x[29]+(uint64_t)INT64_C(112)))); /* 0000f4f8 ldr x8, [x29, #0x70] */
x[25]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-36)))); /* 0000f4fc ldur w25, [x29, #-0x24] */
x[24]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-40)))); /* 0000f500 ldur w24, [x29, #-0x28] */
fu=(uint32_t)x[22];fv=UINT64_C(0);fa=sx32(fu);fb=sx32(fv); /* 0000f504 cmp w22, #0 */
st64((sp+(uint64_t)INT64_C(64)),x[8]); /* 0000f508 str x8, [sp, #0x40] */
x[8]=(ld64((x[29]+(uint64_t)INT64_C(104)))); /* 0000f50c ldr x8, [x29, #0x68] */
x[22]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-44)))); /* 0000f510 ldur w22, [x29, #-0x2c] */
st64((sp+(uint64_t)INT64_C(56)),x[8]); /* 0000f514 str x8, [sp, #0x38] */
x[8]=(ld64((x[29]+(uint64_t)INT64_C(96)))); /* 0000f518 ldr x8, [x29, #0x60] */
x[23]=(uint32_t)(((uint32_t)x[23] << ((uint32_t)x[22] & 31))); /* 0000f51c lsl w23, w23, w22 */
st64((x[29]+(uint64_t)INT64_C(-32)),x[8]); /* 0000f520 stur x8, [x29, #-0x20] */
x[8]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-4)))); /* 0000f524 ldur w8, [x29, #-4] */
x[27]=(uint32_t)(((uint32_t)x[8] << ((uint32_t)x[25] & 31))); /* 0000f528 lsl w27, w8, w25 */
x[8]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-8)))); /* 0000f52c ldur w8, [x29, #-8] */
st32((sp+(uint64_t)INT64_C(48)),(uint32_t)x[27]); /* 0000f530 str w27, [sp, #0x30] */
x[20]=(uint32_t)(((uint32_t)x[8] << ((uint32_t)x[24] & 31))); /* 0000f534 lsl w20, w8, w24 */
x[8]=(uint32_t)(((uint32_t)x[28] << ((uint32_t)x[19] & 31))); /* 0000f538 lsl w8, w28, w19 */
x[19]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(0-(uint32_t)x[0]):(uint32_t)x[0]); /* 0000f53c cneg w19, w0, lt */
st32((x[29]+(uint64_t)INT64_C(-16)),(uint32_t)x[8]); /* 0000f540 stur w8, [x29, #-0x10] */
x[8]=(uint32_t)(((uint32_t)x[26] << ((uint32_t)x[21] & 31))); /* 0000f544 lsl w8, w26, w21 */
x[0]=(uint32_t)((uint32_t)x[19]); /* 0000f548 mov w0, w19 */
st32((sp+(uint64_t)INT64_C(44)),(uint32_t)x[20]); /* 0000f54c str w20, [sp, #0x2c] */
st32((x[29]+(uint64_t)INT64_C(-20)),(uint32_t)x[8]); /* 0000f550 stur w8, [x29, #-0x14] */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000f554 bl #0xf554 */
st32((sp+(uint64_t)INT64_C(36)),(uint32_t)x[0]); /* 0000f558 str w0, [sp, #0x24] */
x[8]=(uint32_t)(((uint32_t)x[19] << ((uint32_t)x[0] & 31))); /* 0000f55c lsl w8, w19, w0 */
x[0]=(uint32_t)((uint32_t)x[27]); /* 0000f560 mov w0, w27 */
x[1]=(uint32_t)((uint32_t)x[20]); /* 0000f564 mov w1, w20 */
st32((x[29]+(uint64_t)INT64_C(-48)),(uint32_t)x[8]); /* 0000f568 stur w8, [x29, #-0x30] */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f56c bl #0xf56c */
x[28]=(uint32_t)((uint32_t)x[0]); /* 0000f570 mov w28, w0 */
x[0]=(uint32_t)((uint32_t)x[27]); /* 0000f574 mov w0, w27 */
x[1]=(uint32_t)((uint32_t)x[23]); /* 0000f578 mov w1, w23 */
st32((sp+(uint64_t)INT64_C(40)),(uint32_t)x[23]); /* 0000f57c str w23, [sp, #0x28] */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f580 bl #0xf580 */
x[21]=(uint32_t)((uint32_t)x[0]); /* 0000f584 mov w21, w0 */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000f588 mov w0, w20 */
x[1]=(uint32_t)((uint32_t)x[23]); /* 0000f58c mov w1, w23 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f590 bl #0xf590 */
x[12]=(uint32_t)((uint32_t)x[25]); /* 0000f594 mov w12, w25 */
x[25]=(uint32_t)((uint32_t)x[22]+(uint32_t)x[25]); /* 0000f598 add w25, w22, w25 */
x[27]=(uint32_t)((uint32_t)x[22]+(uint32_t)x[24]); /* 0000f59c add w27, w22, w24 */
x[22]=(uint32_t)((uint32_t)x[24]+(uint32_t)x[12]); /* 0000f5a0 add w22, w24, w12 */
x[24]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-52)))); /* 0000f5a4 ldur w24, [x29, #-0x34] */
x[11]=(mul32((uint32_t)x[28],(uint32_t)x[26])); /* 0000f5a8 smull x11, w28, w26 */
x[8]=(mul32((uint32_t)x[21],(uint32_t)x[19])); /* 0000f5ac smull x8, w21, w19 */
x[11]=(a64(x[11],x[22])); /* 0000f5b0 asr x11, x11, x22 */
x[12]=(mul32((uint32_t)x[28],(uint32_t)x[24])); /* 0000f5b4 smull x12, w28, w24 */
x[12]=(a64(x[12],x[22])); /* 0000f5b8 asr x12, x12, x22 */
x[13]=(mul32((uint32_t)x[21],(uint32_t)x[24])); /* 0000f5bc smull x13, w21, w24 */
x[8]=(a64(x[8],x[25])); /* 0000f5c0 asr x8, x8, x25 */
x[11]=(x[12]-x[11]); /* 0000f5c4 sub x11, x12, x11 */
x[9]=(mul32((uint32_t)x[0],(uint32_t)x[19])); /* 0000f5c8 smull x9, w0, w19 */
x[13]=(a64(x[13],x[25])); /* 0000f5cc asr x13, x13, x25 */
x[8]=(x[11]+x[8]); /* 0000f5d0 add x8, x11, x8 */
x[10]=(mul32((uint32_t)x[0],(uint32_t)x[26])); /* 0000f5d4 smull x10, w0, w26 */
x[9]=(a64(x[9],x[27])); /* 0000f5d8 asr x9, x9, x27 */
x[8]=(x[8]-x[13]); /* 0000f5dc sub x8, x8, x13 */
x[10]=(a64(x[10],x[27])); /* 0000f5e0 asr x10, x10, x27 */
x[8]=(x[8]-x[9]); /* 0000f5e4 sub x8, x8, x9 */
x[23]=(x[8]+x[10]); /* 0000f5e8 add x23, x8, x10 */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000f5ec mov w20, w0 */
x[0]=(x[23]); /* 0000f5f0 mov x0, x23 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f5f4 bl #0xf5f4 */
x[8]=((x[23] << (x[0] & 63))); /* 0000f5f8 lsl x8, x23, x0 */
st64((sp+(uint64_t)INT64_C(8)),x[0]); /* 0000f5fc str x0, [sp, #8] */
x[1]=((x[8] >> (UINT64_C(32) & 63))); /* 0000f600 lsr x1, x8, #0x20 */
x[0]=(UINT64_C(2305843009213693952)); /* 0000f604 mov x0, #0x2000000000000000 */
x[0]=(dv_stb24_sdiv_sls(x[0],x[1])); /* 0000f608 bl #0xf608 */
x[1]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-16)))); /* 0000f60c ldur w1, [x29, #-0x10] */
st64((sp+(uint64_t)INT64_C(16)),x[0]); /* 0000f610 str x0, [sp, #0x10] */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000f614 mov w0, w20 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f618 bl #0xf618 */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000f61c mov w20, w0 */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000f620 mov w0, w26 */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000f624 mov w1, w19 */
st32((sp+(uint64_t)INT64_C(4)),(uint32_t)x[26]); /* 0000f628 str w26, [sp, #4] */
st32((sp+(uint64_t)INT64_C(32)),(uint32_t)x[19]); /* 0000f62c str w19, [sp, #0x20] */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000f630 bl #0xf630 */
x[9]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-12)))); /* 0000f634 ldur w9, [x29, #-0xc] */
x[1]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-20)))); /* 0000f638 ldur w1, [x29, #-0x14] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[20])); /* 0000f63c smull x8, w0, w20 */
x[0]=(uint32_t)((uint32_t)x[21]); /* 0000f640 mov w0, w21 */
x[9]=(uint32_t)((uint32_t)x[9]+(uint32_t)x[27]); /* 0000f644 add w9, w9, w27 */
x[27]=(a64(x[8],x[9])); /* 0000f648 asr x27, x8, x9 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f64c bl #0xf64c */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000f650 mov w20, w0 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000f654 mov w0, w24 */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000f658 mov w1, w19 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000f65c bl #0xf65c */
x[23]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(28)))); /* 0000f660 ldr w23, [sp, #0x1c] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[20])); /* 0000f664 smull x8, w0, w20 */
x[0]=(uint32_t)((uint32_t)x[28]); /* 0000f668 mov w0, w28 */
x[9]=(uint32_t)((uint32_t)x[23]+(uint32_t)x[25]); /* 0000f66c add w9, w23, w25 */
x[25]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-48)))); /* 0000f670 ldur w25, [x29, #-0x30] */
x[8]=(a64(x[8],x[9])); /* 0000f674 asr x8, x8, x9 */
x[21]=(x[27]-x[8]); /* 0000f678 sub x21, x27, x8 */
x[1]=(uint32_t)((uint32_t)x[25]); /* 0000f67c mov w1, w25 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f680 bl #0xf680 */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000f684 mov w20, w0 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000f688 mov w0, w24 */
x[1]=(uint32_t)((uint32_t)x[26]); /* 0000f68c mov w1, w26 */
x[19]=(uint32_t)((uint32_t)x[24]); /* 0000f690 mov w19, w24 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000f694 bl #0xf694 */
x[26]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(36)))); /* 0000f698 ldr w26, [sp, #0x24] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[20])); /* 0000f69c smull x8, w0, w20 */
x[9]=(uint32_t)((uint32_t)x[26]+(uint32_t)x[22]); /* 0000f6a0 add w9, w26, w22 */
x[8]=(a64(x[8],x[9])); /* 0000f6a4 asr x8, x8, x9 */
x[20]=(x[8]+x[21]); /* 0000f6a8 add x20, x8, x21 */
x[0]=(x[20]); /* 0000f6ac mov x0, x20 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f6b0 bl #0xf6b0 */
x[21]=(uint32_t)((uint32_t)x[0]); /* 0000f6b4 mov w21, w0 */
x[8]=((x[20] << (x[21] & 63))); /* 0000f6b8 lsl x8, x20, x21 */
x[8]=(a64(x[8],UINT64_C(32))); /* 0000f6bc asr x8, x8, #0x20 */
x[9]=(uint32_t)(UINT64_C(2147483647)); /* 0000f6c0 mov w9, #0x7fffffff */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000f6c4 cmp x8, x9 */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[9]); /* 0000f6c8 csel x8, x8, x9, lt */
x[9]=(UINT64_C(18446744071562067968)); /* 0000f6cc mov x9, #-0x80000000 */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000f6d0 cmp x8, x9 */
x[8]=((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?x[8]:x[9]); /* 0000f6d4 csel x8, x8, x9, gt */
x[9]=(ld64((sp+(uint64_t)INT64_C(16)))); /* 0000f6d8 ldr x9, [sp, #0x10] */
x[9]=(sx32((uint32_t)x[9])); /* 0000f6dc sxtw x9, w9 */
x[20]=(x[8]*x[9]); /* 0000f6e0 mul x20, x8, x9 */
x[0]=(x[20]); /* 0000f6e4 mov x0, x20 */
st64((sp+(uint64_t)INT64_C(16)),x[9]); /* 0000f6e8 str x9, [sp, #0x10] */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f6ec bl #0xf6ec */
x[10]=(ld64((sp+(uint64_t)INT64_C(8)))); /* 0000f6f0 ldr x10, [sp, #8] */
x[9]=(uint32_t)(UINT64_C(61)); /* 0000f6f4 mov w9, #0x3d */
x[8]=((x[20] << (x[0] & 63))); /* 0000f6f8 lsl x8, x20, x0 */
x[24]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(40)))); /* 0000f6fc ldr w24, [sp, #0x28] */
x[9]=(uint32_t)((uint32_t)x[9]-(uint32_t)x[10]); /* 0000f700 sub w9, w9, w10 */
x[10]=(ld64((x[29]+(uint64_t)INT64_C(-32)))); /* 0000f704 ldur x10, [x29, #-0x20] */
x[8]=((x[8] >> (UINT64_C(32) & 63))); /* 0000f708 lsr x8, x8, #0x20 */
x[1]=(uint32_t)((uint32_t)x[25]); /* 0000f70c mov w1, w25 */
st32((sp+(uint64_t)INT64_C(8)),(uint32_t)x[9]); /* 0000f710 str w9, [sp, #8] */
st32((x[10]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000f714 str w8, [x10] */
x[8]=(uint32_t)((uint32_t)x[21]+(uint32_t)x[9]); /* 0000f718 add w8, w21, w9 */
x[8]=(uint32_t)((uint32_t)x[8]+(uint32_t)x[0]); /* 0000f71c add w8, w8, w0 */
x[8]=(uint32_t)((uint32_t)x[8]-UINT64_C(32)); /* 0000f720 sub w8, w8, #0x20 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000f724 mov w0, w24 */
st16((x[10]+(uint64_t)INT64_C(4)),(uint32_t)x[8]); /* 0000f728 strh w8, [x10, #4] */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f72c bl #0xf72c */
x[9]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-44)))); /* 0000f730 ldur w9, [x29, #-0x2c] */
x[27]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(44)))); /* 0000f734 ldr w27, [sp, #0x2c] */
x[28]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-20)))); /* 0000f738 ldur w28, [x29, #-0x14] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[19])); /* 0000f73c smull x8, w0, w19 */
x[20]=(uint32_t)((uint32_t)x[26]+(uint32_t)x[9]); /* 0000f740 add w20, w26, w9 */
x[0]=(uint32_t)((uint32_t)x[27]); /* 0000f744 mov w0, w27 */
x[1]=(uint32_t)((uint32_t)x[28]); /* 0000f748 mov w1, w28 */
x[21]=(a64(x[8],x[20])); /* 0000f74c asr x21, x8, x20 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f750 bl #0xf750 */
x[9]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-40)))); /* 0000f754 ldur w9, [x29, #-0x28] */
x[26]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(48)))); /* 0000f758 ldr w26, [sp, #0x30] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[19])); /* 0000f75c smull x8, w0, w19 */
x[22]=(uint32_t)((uint32_t)x[23]+(uint32_t)x[9]); /* 0000f760 add w22, w23, w9 */
x[23]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-16)))); /* 0000f764 ldur w23, [x29, #-0x10] */
x[8]=(a64(x[8],x[22])); /* 0000f768 asr x8, x8, x22 */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000f76c mov w0, w26 */
x[21]=(x[21]-x[8]); /* 0000f770 sub x21, x21, x8 */
x[1]=(uint32_t)((uint32_t)x[23]); /* 0000f774 mov w1, w23 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f778 bl #0xf778 */
x[19]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(4)))); /* 0000f77c ldr w19, [sp, #4] */
x[9]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-12)))); /* 0000f780 ldur w9, [x29, #-0xc] */
x[10]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-36)))); /* 0000f784 ldur w10, [x29, #-0x24] */
x[1]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-48)))); /* 0000f788 ldur w1, [x29, #-0x30] */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[19])); /* 0000f78c smull x8, w0, w19 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000f790 mov w0, w24 */
x[25]=(uint32_t)((uint32_t)x[9]+(uint32_t)x[10]); /* 0000f794 add w25, w9, w10 */
x[8]=(a64(x[8],x[25])); /* 0000f798 asr x8, x8, x25 */
x[21]=(x[21]+x[8]); /* 0000f79c add x21, x21, x8 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f7a0 bl #0xf7a0 */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[19])); /* 0000f7a4 smull x8, w0, w19 */
x[8]=(a64(x[8],x[20])); /* 0000f7a8 asr x8, x8, x20 */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000f7ac mov w0, w26 */
x[1]=(uint32_t)((uint32_t)x[23]); /* 0000f7b0 mov w1, w23 */
x[20]=(x[21]-x[8]); /* 0000f7b4 sub x20, x21, x8 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f7b8 bl #0xf7b8 */
x[21]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(32)))); /* 0000f7bc ldr w21, [sp, #0x20] */
x[1]=(uint32_t)((uint32_t)x[28]); /* 0000f7c0 mov w1, w28 */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[21])); /* 0000f7c4 smull x8, w0, w21 */
x[8]=(a64(x[8],x[25])); /* 0000f7c8 asr x8, x8, x25 */
x[0]=(uint32_t)((uint32_t)x[27]); /* 0000f7cc mov w0, w27 */
x[20]=(x[20]-x[8]); /* 0000f7d0 sub x20, x20, x8 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000f7d4 bl #0xf7d4 */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[21])); /* 0000f7d8 smull x8, w0, w21 */
x[8]=(a64(x[8],x[22])); /* 0000f7dc asr x8, x8, x22 */
x[20]=(x[20]+x[8]); /* 0000f7e0 add x20, x20, x8 */
x[0]=(x[20]); /* 0000f7e4 mov x0, x20 */
x[23]=(uint32_t)((uint32_t)x[21]); /* 0000f7e8 mov w23, w21 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f7ec bl #0xf7ec */
x[21]=(uint32_t)((uint32_t)x[0]); /* 0000f7f0 mov w21, w0 */
x[8]=((x[20] << (x[21] & 63))); /* 0000f7f4 lsl x8, x20, x21 */
x[8]=(a64(x[8],UINT64_C(32))); /* 0000f7f8 asr x8, x8, #0x20 */
x[9]=(uint32_t)(UINT64_C(2147483647)); /* 0000f7fc mov w9, #0x7fffffff */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000f800 cmp x8, x9 */
x[22]=(ld64((sp+(uint64_t)INT64_C(16)))); /* 0000f804 ldr x22, [sp, #0x10] */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[9]); /* 0000f808 csel x8, x8, x9, lt */
x[9]=(UINT64_C(18446744071562067968)); /* 0000f80c mov x9, #-0x80000000 */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000f810 cmp x8, x9 */
x[8]=((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?x[8]:x[9]); /* 0000f814 csel x8, x8, x9, gt */
x[20]=(x[8]*x[22]); /* 0000f818 mul x20, x8, x22 */
x[0]=(x[20]); /* 0000f81c mov x0, x20 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f820 bl #0xf820 */
x[13]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-8))+0));x[12]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-8))+4)); /* 0000f824 ldp w13, w12, [x29, #-8] */
x[11]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(52)))); /* 0000f828 ldr w11, [sp, #0x34] */
x[24]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(8)))); /* 0000f82c ldr w24, [sp, #8] */
x[8]=((x[20] << (x[0] & 63))); /* 0000f830 lsl x8, x20, x0 */
x[8]=((x[8] >> (UINT64_C(32) & 63))); /* 0000f834 lsr x8, x8, #0x20 */
x[10]=(uint32_t)((uint32_t)x[11]-(uint32_t)x[13]); /* 0000f838 sub w10, w11, w13 */
x[11]=(uint32_t)((uint32_t)x[11]-(uint32_t)x[12]); /* 0000f83c sub w11, w11, w12 */
x[12]=(uint32_t)((uint32_t)x[13]-(uint32_t)x[12]); /* 0000f840 sub w12, w13, w12 */
x[13]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-52)))); /* 0000f844 ldur w13, [x29, #-0x34] */
x[9]=(uint32_t)((uint32_t)x[21]+(uint32_t)x[24]); /* 0000f848 add w9, w21, w24 */
x[21]=(ld64((sp+(uint64_t)INT64_C(56)))); /* 0000f84c ldr x21, [sp, #0x38] */
x[9]=(uint32_t)((uint32_t)x[9]+(uint32_t)x[0]); /* 0000f850 add w9, w9, w0 */
x[10]=(mul32((uint32_t)x[13],(uint32_t)x[10])); /* 0000f854 smull x10, w13, w10 */
st32((x[21]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000f858 str w8, [x21] */
x[8]=(uint32_t)((uint32_t)x[9]-UINT64_C(32)); /* 0000f85c sub w8, w9, #0x20 */
x[9]=(x[10]-mul32((uint32_t)x[19],(uint32_t)x[11])); /* 0000f860 smsubl x9, w19, w11, x10 */
x[19]=(x[9]+mul32((uint32_t)x[23],(uint32_t)x[12])); /* 0000f864 smaddl x19, w23, w12, x9 */
x[0]=(x[19]); /* 0000f868 mov x0, x19 */
st16((x[21]+(uint64_t)INT64_C(4)),(uint32_t)x[8]); /* 0000f86c strh w8, [x21, #4] */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f870 bl #0xf870 */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000f874 mov w20, w0 */
x[8]=((x[19] << (x[20] & 63))); /* 0000f878 lsl x8, x19, x20 */
x[8]=(a64(x[8],UINT64_C(32))); /* 0000f87c asr x8, x8, #0x20 */
x[19]=(x[8]*x[22]); /* 0000f880 mul x19, x8, x22 */
x[0]=(x[19]); /* 0000f884 mov x0, x19 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000f888 bl #0xf888 */
x[13]=(ld64((sp+(uint64_t)INT64_C(64)))); /* 0000f88c ldr x13, [sp, #0x40] */
x[11]=(ld64((x[29]+(uint64_t)INT64_C(-32)))); /* 0000f890 ldur x11, [x29, #-0x20] */
x[8]=((x[19] << (x[0] & 63))); /* 0000f894 lsl x8, x19, x0 */
x[9]=(uint32_t)((uint32_t)x[20]+(uint32_t)x[24]); /* 0000f898 add w9, w20, w24 */
x[8]=((x[8] >> (UINT64_C(32) & 63))); /* 0000f89c lsr x8, x8, #0x20 */
x[9]=(uint32_t)((uint32_t)x[9]+(uint32_t)x[0]); /* 0000f8a0 add w9, w9, w0 */
st32((x[13]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000f8a4 str w8, [x13] */
x[8]=(uint32_t)((uint32_t)x[9]-UINT64_C(32)); /* 0000f8a8 sub w8, w9, #0x20 */
st16((x[13]+(uint64_t)INT64_C(4)),(uint32_t)x[8]); /* 0000f8ac strh w8, [x13, #4] */
x[8]=(uint32_t)(ld16((x[11]+(uint64_t)INT64_C(4)))); /* 0000f8b0 ldrh w8, [x11, #4] */
fu=(uint32_t)x[8];fv=UINT64_C(32);fa=sx32(fu);fb=sx32(fv); /* 0000f8b4 cmp w8, #0x20 */
if(fu<fv) goto L0xf8e0; /* 0000f8b8 b.lo #0xf8e0 */
x[9]=(uint32_t)(ld32((x[11]+(uint64_t)INT64_C(0)))); /* 0000f8bc ldr w9, [x11] */
fu=(uint32_t)x[8];fv=UINT64_C(62);fa=sx32(fu);fb=sx32(fv); /* 0000f8c0 cmp w8, #0x3e */
x[10]=(uint32_t)(UINT64_C(62)); /* 0000f8c4 mov w10, #0x3e */
x[8]=(uint32_t)((fu<fv)?(uint32_t)x[8]:(uint32_t)x[10]); /* 0000f8c8 csel w8, w8, w10, lo */
x[8]=(uint32_t)((uint32_t)x[8]-UINT64_C(31)); /* 0000f8cc sub w8, w8, #0x1f */
x[8]=(uint32_t)(a32((uint32_t)x[9],(uint32_t)x[8])); /* 0000f8d0 asr w8, w9, w8 */
st32((x[11]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000f8d4 str w8, [x11] */
x[8]=(uint32_t)(UINT64_C(31)); /* 0000f8d8 mov w8, #0x1f */
st16((x[11]+(uint64_t)INT64_C(4)),(uint32_t)x[8]); /* 0000f8dc strh w8, [x11, #4] */
L0xf8e0: x[9]=(uint32_t)(ld16((x[21]+(uint64_t)INT64_C(4)))); /* 0000f8e0 ldrh w9, [x21, #4] */
x[8]=(ld64((x[29]+(uint64_t)INT64_C(120)))); /* 0000f8e4 ldr x8, [x29, #0x78] */
fu=(uint32_t)x[9];fv=UINT64_C(32);fa=sx32(fu);fb=sx32(fv); /* 0000f8e8 cmp w9, #0x20 */
if(fu<fv) goto L0xf914; /* 0000f8ec b.lo #0xf914 */
x[11]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(0)))); /* 0000f8f0 ldr w11, [x21] */
fu=(uint32_t)x[9];fv=UINT64_C(62);fa=sx32(fu);fb=sx32(fv); /* 0000f8f4 cmp w9, #0x3e */
x[10]=(uint32_t)(UINT64_C(62)); /* 0000f8f8 mov w10, #0x3e */
x[9]=(uint32_t)((fu<fv)?(uint32_t)x[9]:(uint32_t)x[10]); /* 0000f8fc csel w9, w9, w10, lo */
x[9]=(uint32_t)((uint32_t)x[9]-UINT64_C(31)); /* 0000f900 sub w9, w9, #0x1f */
x[12]=(uint32_t)(UINT64_C(31)); /* 0000f904 mov w12, #0x1f */
x[9]=(uint32_t)(a32((uint32_t)x[11],(uint32_t)x[9])); /* 0000f908 asr w9, w11, w9 */
st32((x[21]+(uint64_t)INT64_C(0)),(uint32_t)x[9]); /* 0000f90c str w9, [x21] */
st16((x[21]+(uint64_t)INT64_C(4)),(uint32_t)x[12]); /* 0000f910 strh w12, [x21, #4] */
L0xf914: x[9]=(uint32_t)(ld16((x[13]+(uint64_t)INT64_C(4)))); /* 0000f914 ldrh w9, [x13, #4] */
fu=(uint32_t)x[9];fv=UINT64_C(32);fa=sx32(fu);fb=sx32(fv); /* 0000f918 cmp w9, #0x20 */
if(fu<fv) goto L0xf944; /* 0000f91c b.lo #0xf944 */
x[11]=(uint32_t)(ld32((x[13]+(uint64_t)INT64_C(0)))); /* 0000f920 ldr w11, [x13] */
fu=(uint32_t)x[9];fv=UINT64_C(62);fa=sx32(fu);fb=sx32(fv); /* 0000f924 cmp w9, #0x3e */
x[10]=(uint32_t)(UINT64_C(62)); /* 0000f928 mov w10, #0x3e */
x[9]=(uint32_t)((fu<fv)?(uint32_t)x[9]:(uint32_t)x[10]); /* 0000f92c csel w9, w9, w10, lo */
x[9]=(uint32_t)((uint32_t)x[9]-UINT64_C(31)); /* 0000f930 sub w9, w9, #0x1f */
x[12]=(uint32_t)(UINT64_C(31)); /* 0000f934 mov w12, #0x1f */
x[9]=(uint32_t)(a32((uint32_t)x[11],(uint32_t)x[9])); /* 0000f938 asr w9, w11, w9 */
st32((x[13]+(uint64_t)INT64_C(0)),(uint32_t)x[9]); /* 0000f93c str w9, [x13] */
st16((x[13]+(uint64_t)INT64_C(4)),(uint32_t)x[12]); /* 0000f940 strh w12, [x13, #4] */
L0xf944: x[9]=(uint32_t)(ld32((x[29]+(uint64_t)INT64_C(-56)))); /* 0000f944 ldur w9, [x29, #-0x38] */
st32((x[8]+(uint64_t)INT64_C(0)),(uint32_t)x[9]); /* 0000f948 str w9, [x8] */
x[29]=(ld64((sp+(uint64_t)INT64_C(128))+0));x[30]=(ld64((sp+(uint64_t)INT64_C(128))+8)); /* 0000f94c ldp x29, x30, [sp, #0x80] */
x[20]=(ld64((sp+(uint64_t)INT64_C(208))+0));x[19]=(ld64((sp+(uint64_t)INT64_C(208))+8)); /* 0000f950 ldp x20, x19, [sp, #0xd0] */
x[22]=(ld64((sp+(uint64_t)INT64_C(192))+0));x[21]=(ld64((sp+(uint64_t)INT64_C(192))+8)); /* 0000f954 ldp x22, x21, [sp, #0xc0] */
x[24]=(ld64((sp+(uint64_t)INT64_C(176))+0));x[23]=(ld64((sp+(uint64_t)INT64_C(176))+8)); /* 0000f958 ldp x24, x23, [sp, #0xb0] */
x[26]=(ld64((sp+(uint64_t)INT64_C(160))+0));x[25]=(ld64((sp+(uint64_t)INT64_C(160))+8)); /* 0000f95c ldp x26, x25, [sp, #0xa0] */
x[28]=(ld64((sp+(uint64_t)INT64_C(144))+0));x[27]=(ld64((sp+(uint64_t)INT64_C(144))+8)); /* 0000f960 ldp x28, x27, [sp, #0x90] */
/* shadow call stack bookkeeping omitted */ /* 0000f964 ldr x30, [x18, #-8]! */
sp=sp+UINT64_C(224); /* 0000f968 add sp, sp, #0xe0 */
return; /* 0000f96c ret  */
}

void dv_stb24_calc_mapped_params_3pxx(int32_t smax,const uint8_t metadata[56],const uint8_t tuning[48],int32_t *tmin,int32_t *tmid,int32_t *tmax,int32_t *smid,int32_t *slope,int32_t *s2t_ratio,int32_t *screen_ratio){
uint64_t x[31]={0},fa=0,fb=0,fu=0,fv=0;uint64_t stack[32]={0};uint64_t sp=(uintptr_t)((uint8_t *)stack+224);
x[0]=(uint32_t)smax;x[1]=(uintptr_t)metadata;x[2]=(uintptr_t)tuning;x[3]=(uintptr_t)tmin;x[4]=(uintptr_t)tmid;x[5]=(uintptr_t)tmax;x[6]=(uintptr_t)smid;x[7]=(uintptr_t)slope;st64(sp,(uintptr_t)s2t_ratio);st64(sp+8,(uintptr_t)screen_ratio);
sp=sp-UINT64_C(176); /* 0000ea28 sub sp, sp, #0xb0 */
/* shadow call stack bookkeeping omitted */ /* 0000ea2c str x30, [x18], #8 */
st64((sp+(uint64_t)INT64_C(80))+0,x[29]);st64((sp+(uint64_t)INT64_C(80))+8,x[30]); /* 0000ea30 stp x29, x30, [sp, #0x50] */
st64((sp+(uint64_t)INT64_C(96))+0,x[28]);st64((sp+(uint64_t)INT64_C(96))+8,x[27]); /* 0000ea34 stp x28, x27, [sp, #0x60] */
st64((sp+(uint64_t)INT64_C(112))+0,x[26]);st64((sp+(uint64_t)INT64_C(112))+8,x[25]); /* 0000ea38 stp x26, x25, [sp, #0x70] */
st64((sp+(uint64_t)INT64_C(128))+0,x[24]);st64((sp+(uint64_t)INT64_C(128))+8,x[23]); /* 0000ea3c stp x24, x23, [sp, #0x80] */
st64((sp+(uint64_t)INT64_C(144))+0,x[22]);st64((sp+(uint64_t)INT64_C(144))+8,x[21]); /* 0000ea40 stp x22, x21, [sp, #0x90] */
st64((sp+(uint64_t)INT64_C(160))+0,x[20]);st64((sp+(uint64_t)INT64_C(160))+8,x[19]); /* 0000ea44 stp x20, x19, [sp, #0xa0] */
x[29]=(sp+UINT64_C(80)); /* 0000ea48 add x29, sp, #0x50 */
x[8]=(ld64((x[29]+(uint64_t)INT64_C(96)))); /* 0000ea4c ldr x8, [x29, #0x60] */
st64((sp+(uint64_t)INT64_C(16)),x[7]); /* 0000ea50 str x7, [sp, #0x10] */
st64((x[29]+(uint64_t)INT64_C(-24)),x[6]); /* 0000ea54 stur x6, [x29, #-0x18] */
st64((sp+(uint64_t)INT64_C(40)),x[5]); /* 0000ea58 str x5, [sp, #0x28] */
x[26]=(x[4]); /* 0000ea5c mov x26, x4 */
x[20]=(x[3]); /* 0000ea60 mov x20, x3 */
x[21]=(x[2]); /* 0000ea64 mov x21, x2 */
x[19]=(x[1]); /* 0000ea68 mov x19, x1 */
st64((x[29]+(uint64_t)INT64_C(-8)),x[8]); /* 0000ea6c stur x8, [x29, #-8] */
x[23]=(uint32_t)((uint32_t)x[0]); /* 0000ea70 mov w23, w0 */
/* profiling omitted; no numerical effect */ /* 0000ea74 bl #0xea74 */
x[24]=(uint32_t)(ld32((x[19]+(uint64_t)INT64_C(0))+0));x[10]=(uint32_t)(ld32((x[19]+(uint64_t)INT64_C(0))+4)); /* 0000ea78 ldp w24, w10, [x19] */
x[9]=(uint32_t)(ld16((x[21]+(uint64_t)INT64_C(44)))); /* 0000ea7c ldrh w9, [x21, #0x2c] */
x[8]=(sx32(ld32((x[19]+(uint64_t)INT64_C(20))))); /* 0000ea80 ldrsw x8, [x19, #0x14] */
st64((x[29]+(uint64_t)INT64_C(-32)),x[20]); /* 0000ea84 stur x20, [x29, #-0x20] */
st32((sp+(uint64_t)INT64_C(36)),(uint32_t)x[10]); /* 0000ea88 str w10, [sp, #0x24] */
st64((sp+(uint64_t)INT64_C(8)),x[8]); /* 0000ea8c str x8, [sp, #8] */
if((uint32_t)x[9]==0) goto L0xeac4; /* 0000ea90 cbz w9, #0xeac4 */
x[8]=(uint32_t)(ld32((x[19]+(uint64_t)INT64_C(12)))); /* 0000ea94 ldr w8, [x19, #0xc] */
fu=(uint32_t)x[9];fv=UINT64_C(1);fa=sx32(fu);fb=sx32(fv); /* 0000ea98 cmp w9, #1 */
x[9]=(x[19]+UINT64_C(16)); /* 0000ea9c add x9, x19, #0x10 */
if(fa!=fb) goto L0xead8; /* 0000eaa0 b.ne #0xead8 */
x[10]=(x[21]); /* 0000eaa4 mov x10, x21 */
x[13]=(uint32_t)(ld32((x[10]+(uint64_t)INT64_C(0))));x[10]=(x[10]+UINT64_C(28)); /* 0000eaa8 ldr w13, [x10], #0x1c */
x[14]=(uint32_t)((uint32_t)x[8]); /* 0000eaac mov w14, w8 */
x[12]=(x[9]); /* 0000eab0 mov x12, x9 */
x[9]=(x[21]+UINT64_C(4)); /* 0000eab4 add x9, x21, #4 */
x[11]=(x[19]+UINT64_C(44)); /* 0000eab8 add x11, x19, #0x2c */
x[8]=(uint32_t)((uint32_t)x[13]); /* 0000eabc mov w8, w13 */
goto L0xeae8; /* 0000eac0 b #0xeae8 */
L0xeac4: x[11]=(x[21]); /* 0000eac4 mov x11, x21 */
x[8]=(uint32_t)(ld32((x[19]+(uint64_t)INT64_C(12)))); /* 0000eac8 ldr w8, [x19, #0xc] */
x[14]=(uint32_t)(ld32((x[11]+(uint64_t)INT64_C(0))));x[11]=(x[11]+UINT64_C(28)); /* 0000eacc ldr w14, [x11], #0x1c */
x[9]=(x[19]+UINT64_C(16)); /* 0000ead0 add x9, x19, #0x10 */
goto L0xeae0; /* 0000ead4 b #0xeae0 */
L0xead8: x[11]=(x[21]); /* 0000ead8 mov x11, x21 */
x[14]=(uint32_t)(ld32((x[11]+(uint64_t)INT64_C(0))));x[11]=(x[11]+UINT64_C(28)); /* 0000eadc ldr w14, [x11], #0x1c */
L0xeae0: x[10]=(x[19]+UINT64_C(44)); /* 0000eae0 add x10, x19, #0x2c */
x[12]=(x[21]+UINT64_C(4)); /* 0000eae4 add x12, x21, #4 */
L0xeae8: x[19]=(uint32_t)(ld32((x[10]+(uint64_t)INT64_C(0)))); /* 0000eae8 ldr w19, [x10] */
x[10]=(uint32_t)(ld32((x[9]+(uint64_t)INT64_C(0)))); /* 0000eaec ldr w10, [x9] */
x[27]=(uint32_t)(ld32((x[12]+(uint64_t)INT64_C(0)))); /* 0000eaf0 ldr w27, [x12] */
x[25]=(ld64((x[29]+(uint64_t)INT64_C(104)))); /* 0000eaf4 ldr x25, [x29, #0x68] */
x[20]=(uint32_t)(ld32((x[11]+(uint64_t)INT64_C(0)))); /* 0000eaf8 ldr w20, [x11] */
x[9]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(32)))); /* 0000eafc ldr w9, [x21, #0x20] */
x[10]=(uint32_t)(a32((uint32_t)x[10],UINT64_C(1))); /* 0000eb00 asr w10, w10, #1 */
x[11]=(uint32_t)(a32((uint32_t)x[27],UINT64_C(1))); /* 0000eb04 asr w11, w27, #1 */
x[22]=(uint32_t)((uint32_t)x[10]+a32((uint32_t)x[8],UINT64_C(1))); /* 0000eb08 add w22, w10, w8, asr #1 */
x[8]=(uint32_t)((uint32_t)x[11]+a32((uint32_t)x[14],UINT64_C(1))); /* 0000eb0c add w8, w11, w14, asr #1 */
x[28]=(uint32_t)((uint32_t)x[22]-(uint32_t)x[8]); /* 0000eb10 sub w28, w22, w8 */
st32((sp+(uint64_t)INT64_C(32)),(uint32_t)x[23]); /* 0000eb14 str w23, [sp, #0x20] */
st64((x[29]+(uint64_t)INT64_C(-16)),x[26]); /* 0000eb18 stur x26, [x29, #-0x10] */
st32((sp+(uint64_t)INT64_C(28)),(uint32_t)x[14]); /* 0000eb1c str w14, [sp, #0x1c] */
if((uint32_t)x[9]==0) goto L0xec0c; /* 0000eb20 cbz w9, #0xec0c */
x[10]=(uint32_t)(a32((uint32_t)x[24],UINT64_C(1))); /* 0000eb24 asr w10, w24, #1 */
x[10]=(uint32_t)((uint32_t)x[10]+a32((uint32_t)x[23],UINT64_C(1))); /* 0000eb28 add w10, w10, w23, asr #1 */
fu=(uint32_t)x[10];fv=(uint32_t)x[8];fa=sx32(fu);fb=sx32(fv);x[26]=(uint32_t)((uint32_t)x[10]-(uint32_t)x[8]); /* 0000eb2c subs w26, w10, w8 */
if(dv_stb24_signed64(fa)>=dv_stb24_signed64(fb)) goto L0xeb3c; /* 0000eb30 b.ge #0xeb3c */
x[8]=(uint32_t)(0); /* 0000eb34 mov w8, wzr */
goto L0xebf8; /* 0000eb38 b #0xebf8 */
L0xeb3c: fu=(uint32_t)x[10];fv=(uint32_t)x[22];fa=sx32(fu);fb=sx32(fv); /* 0000eb3c cmp w10, w22 */
if(dv_stb24_signed64(fa)>dv_stb24_signed64(fb)) goto L0xebf4; /* 0000eb40 b.gt #0xebf4 */
fu=(uint32_t)x[28];fv=UINT64_C(1);fa=sx32(fu);fb=sx32(fv); /* 0000eb44 cmp w28, #1 */
if(dv_stb24_signed64(fa)<dv_stb24_signed64(fb)) goto L0xebf4; /* 0000eb48 b.lt #0xebf4 */
x[0]=(uint32_t)((uint32_t)x[28]); /* 0000eb4c mov w0, w28 */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000eb50 bl #0xeb50 */
x[23]=(uint32_t)((uint32_t)x[0]); /* 0000eb54 mov w23, w0 */
x[0]=(uint32_t)((uint32_t)x[28]); /* 0000eb58 mov w0, w28 */
x[1]=(uint32_t)((uint32_t)x[23]); /* 0000eb5c mov w1, w23 */
x[0]=((uint32_t)dv_stb24_sshl_ssu(x[0],x[1])); /* 0000eb60 bl #0xeb60 */
x[8]=(sx32(x[26]) << (UINT64_C(30)&63)); /* 0000eb64 sbfiz x8, x26, #0x1e, #0x20 */
x[9]=(sx32((uint32_t)x[0])); /* 0000eb68 sxtw x9, w0 */
x[8]=(div64(x[8],x[9])); /* 0000eb6c sdiv x8, x8, x9 */
x[9]=(uint32_t)(UINT64_C(29)); /* 0000eb70 mov w9, #0x1d */
x[10]=(uint32_t)(UINT64_C(30)); /* 0000eb74 mov w10, #0x1e */
x[9]=(uint32_t)((uint32_t)x[9]-(uint32_t)x[23]); /* 0000eb78 sub w9, w9, w23 */
x[11]=(uint32_t)(UINT64_C(1)); /* 0000eb7c mov w11, #1 */
x[10]=(uint32_t)((uint32_t)x[10]-(uint32_t)x[23]); /* 0000eb80 sub w10, w10, w23 */
x[9]=(uint32_t)(((uint32_t)x[11] << ((uint32_t)x[9] & 31))); /* 0000eb84 lsl w9, w11, w9 */
fu=(uint32_t)x[9];fv=(uint32_t)x[8];fa=sx32(fu);fb=sx32(fv); /* 0000eb88 cmp w9, w8 */
x[11]=(uint32_t)(((uint32_t)x[11] << ((uint32_t)x[10] & 31))); /* 0000eb8c lsl w11, w11, w10 */
x[11]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?0:(uint32_t)x[11]); /* 0000eb90 csel w11, wzr, w11, gt */
x[8]=(uint32_t)((uint32_t)x[8]-(uint32_t)x[11]); /* 0000eb94 sub w8, w8, w11 */
x[11]=(sx32((uint32_t)x[8])); /* 0000eb98 sxtw x11, w8 */
x[8]=(mul32((uint32_t)x[8],(uint32_t)x[8])); /* 0000eb9c smull x8, w8, w8 */
x[8]=((x[8] >> (UINT64_C(31) & 63))); /* 0000eba0 lsr x8, x8, #0x1f */
x[8]=(x[8]*x[8]); /* 0000eba4 mul x8, x8, x8 */
x[8]=(a64(x[8],UINT64_C(31))); /* 0000eba8 asr x8, x8, #0x1f */
x[10]=(uint32_t)((uint32_t)x[10]+((uint32_t)x[10] << (UINT64_C(2) & 31))); /* 0000ebac add w10, w10, w10, lsl #2 */
x[8]=(x[8]*x[11]); /* 0000ebb0 mul x8, x8, x11 */
x[10]=(uint32_t)((uint32_t)x[10]-UINT64_C(117)); /* 0000ebb4 sub w10, w10, #0x75 */
x[8]=(a64(x[8],x[10])); /* 0000ebb8 asr x8, x8, x10 */
x[10]=(uint32_t)((dv_stb24_signed64(fa)<=dv_stb24_signed64(fb))?1:0); /* 0000ebbc cset w10, le */
x[9]=(uint32_t)((uint32_t)x[28]); /* 0000ebc0 mov w9, w28 */
x[8]=(x[8]+(x[10] << (UINT64_C(20) & 63))); /* 0000ebc4 add x8, x8, x10, lsl #20 */
x[8]=(x[8]*x[9]); /* 0000ebc8 mul x8, x8, x9 */
x[11]=(uint32_t)(UINT64_C(2147483647)); /* 0000ebcc mov w11, #0x7fffffff */
x[8]=(a64(x[8],UINT64_C(20))); /* 0000ebd0 asr x8, x8, #0x14 */
x[9]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(32)))); /* 0000ebd4 ldr w9, [x21, #0x20] */
fu=x[8];fv=x[11];fa=sx64(fu);fb=sx64(fv); /* 0000ebd8 cmp x8, x11 */
x[10]=(UINT64_C(18446744071562067968)); /* 0000ebdc mov x10, #-0x80000000 */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[11]); /* 0000ebe0 csel x8, x8, x11, lt */
fu=x[8];fv=x[10];fa=sx64(fu);fb=sx64(fv); /* 0000ebe4 cmp x8, x10 */
x[10]=(uint32_t)(UINT64_C(18446744071562067968)); /* 0000ebe8 mov w10, #-0x80000000 */
x[8]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[8]:(uint32_t)x[10]); /* 0000ebec csel w8, w8, w10, gt */
goto L0xebf8; /* 0000ebf0 b #0xebf8 */
L0xebf4: x[8]=(uint32_t)((uint32_t)x[28]); /* 0000ebf4 mov w8, w28 */
L0xebf8: x[10]=(uint32_t)(UINT64_C(4096)); /* 0000ebf8 mov w10, #0x1000 */
x[10]=(uint32_t)((uint32_t)x[10]-(uint32_t)x[9]); /* 0000ebfc sub w10, w10, w9 */
x[10]=(mul32((uint32_t)x[10],(uint32_t)x[28])); /* 0000ec00 smull x10, w10, w28 */
x[8]=(x[10]+mul32((uint32_t)x[8],(uint32_t)x[9])); /* 0000ec04 smaddl x8, w8, w9, x10 */
x[28]=((x[8] >> (UINT64_C(12) & 63))); /* 0000ec08 lsr x28, x8, #0xc */
L0xec0c: x[8]=(ld64((x[29]+(uint64_t)INT64_C(-8)))); /* 0000ec0c ldur x8, [x29, #-8] */
fu=(uint32_t)x[20];fv=(uint32_t)x[19];fa=sx32(fu);fb=sx32(fv); /* 0000ec10 cmp w20, w19 */
st32((x[8]+(uint64_t)INT64_C(0)),(uint32_t)x[28]); /* 0000ec14 str w28, [x8] */
if(fa!=fb) goto L0xec24; /* 0000ec18 b.ne #0xec24 */
x[0]=(uint32_t)(UINT64_C(134217728)); /* 0000ec1c mov w0, #0x8000000 */
goto L0xec9c; /* 0000ec20 b #0xec9c */
L0xec24: x[1]=(uint32_t)(UINT64_C(26)); /* 0000ec24 mov w1, #0x1a */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000ec28 mov w0, w20 */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000ec2c bl #0xec2c */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000ec30 mov w20, w0 */
x[1]=(uint32_t)(UINT64_C(26)); /* 0000ec34 mov w1, #0x1a */
x[0]=(uint32_t)((uint32_t)x[19]); /* 0000ec38 mov w0, w19 */
x[0]=((uint32_t)dv_stb24_fxLOG2Taylor(x[0],x[1])); /* 0000ec3c bl #0xec3c */
x[19]=(uint32_t)((uint32_t)x[0]); /* 0000ec40 mov w19, w0 */
x[0]=((uint32_t)dv_stb24_unorm_s(x[0])); /* 0000ec44 bl #0xec44 */
x[23]=(uint32_t)((uint32_t)x[0]); /* 0000ec48 mov w23, w0 */
x[1]=(uint32_t)(((uint32_t)x[19] << ((uint32_t)x[0] & 31))); /* 0000ec4c lsl w1, w19, w0 */
x[0]=(UINT64_C(2305843009213693952)); /* 0000ec50 mov x0, #0x2000000000000000 */
x[0]=(dv_stb24_sdiv_sls(x[0],x[1])); /* 0000ec54 bl #0xec54 */
x[9]=(uint32_t)(UINT64_C(34)); /* 0000ec58 mov w9, #0x22 */
x[9]=(uint32_t)((uint32_t)x[9]-(uint32_t)x[23]); /* 0000ec5c sub w9, w9, w23 */
x[10]=(uint32_t)(UINT64_C(63)); /* 0000ec60 mov w10, #0x3f */
fu=(uint32_t)x[9];fv=UINT64_C(63);fa=sx32(fu);fb=sx32(fv); /* 0000ec64 cmp w9, #0x3f */
x[8]=(mul32((uint32_t)x[0],(uint32_t)x[20])); /* 0000ec68 smull x8, w0, w20 */
x[9]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(uint32_t)x[9]:(uint32_t)x[10]); /* 0000ec6c csel w9, w9, w10, lt */
x[10]=(uint32_t)(UINT64_C(2147483647)); /* 0000ec70 mov w10, #0x7fffffff */
x[8]=(a64(x[8],x[9])); /* 0000ec74 asr x8, x8, x9 */
fu=x[8];fv=x[10];fa=sx64(fu);fb=sx64(fv); /* 0000ec78 cmp x8, x10 */
x[9]=(UINT64_C(18446744071562067968)); /* 0000ec7c mov x9, #-0x80000000 */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[10]); /* 0000ec80 csel x8, x8, x10, lt */
fu=x[8];fv=x[9];fa=sx64(fu);fb=sx64(fv); /* 0000ec84 cmp x8, x9 */
x[9]=(uint32_t)(UINT64_C(18446744071562067968)); /* 0000ec88 mov w9, #-0x80000000 */
x[0]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[8]:(uint32_t)x[9]); /* 0000ec8c csel w0, w8, w9, gt */
x[0]=((uint32_t)dv_stb24_ssqrt_s(x[0])); /* 0000ec90 bl #0xec90 */
x[1]=(uint32_t)(UINT64_C(2)); /* 0000ec94 mov w1, #2 */
x[0]=((uint32_t)dv_stb24_sshr_sss(x[0],x[1])); /* 0000ec98 bl #0xec98 */
L0xec9c: x[26]=(ld64((x[29]+(uint64_t)INT64_C(-8)))); /* 0000ec9c ldur x26, [x29, #-8] */
st32((x[25]+(uint64_t)INT64_C(0)),(uint32_t)x[0]); /* 0000eca0 str w0, [x25] */
x[28]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(36)))); /* 0000eca4 ldr w28, [sp, #0x24] */
x[1]=(uint32_t)(ld32((x[26]+(uint64_t)INT64_C(0)))); /* 0000eca8 ldr w1, [x26] */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000ecac bl #0xecac */
x[8]=(uint32_t)(ld32((x[25]+(uint64_t)INT64_C(0)))); /* 0000ecb0 ldr w8, [x25] */
x[1]=(uint32_t)((uint32_t)x[0]); /* 0000ecb4 mov w1, w0 */
x[0]=(uint32_t)((uint32_t)x[8]); /* 0000ecb8 mov w0, w8 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ecbc bl #0xecbc */
x[1]=(uint32_t)(UINT64_C(3)); /* 0000ecc0 mov w1, #3 */
x[0]=((uint32_t)dv_stb24_sshl_ssu(x[0],x[1])); /* 0000ecc4 bl #0xecc4 */
x[1]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(20)))); /* 0000ecc8 ldr w1, [x21, #0x14] */
x[19]=(uint32_t)((uint32_t)x[0]); /* 0000eccc mov w19, w0 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000ecd0 bl #0xecd0 */
x[1]=(uint32_t)((uint32_t)x[0]); /* 0000ecd4 mov w1, w0 */
x[0]=(uint32_t)((uint32_t)x[19]); /* 0000ecd8 mov w0, w19 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ecdc bl #0xecdc */
x[9]=(ld64((sp+(uint64_t)INT64_C(8))+0));x[8]=(ld64((sp+(uint64_t)INT64_C(8))+8)); /* 0000ece0 ldp x9, x8, [sp, #8] */
x[20]=(uint32_t)(UINT64_C(2147483647)); /* 0000ece4 mov w20, #0x7fffffff */
x[23]=(uint32_t)(UINT64_C(18446744071562067968)); /* 0000ece8 mov w23, #-0x80000000 */
x[1]=(uint32_t)(UINT64_C(1073741824)); /* 0000ecec mov w1, #0x40000000 */
st32((x[8]+(uint64_t)INT64_C(0)),(uint32_t)x[0]); /* 0000ecf0 str w0, [x8] */
x[8]=(uint32_t)((uint32_t)x[28]-(uint32_t)x[22]); /* 0000ecf4 sub w8, w28, w22 */
x[8]=(sx32((uint32_t)x[8])); /* 0000ecf8 sxtw x8, w8 */
x[8]=(x[8]*x[9]); /* 0000ecfc mul x8, x8, x9 */
x[8]=(a64(x[8],UINT64_C(17))); /* 0000ed00 asr x8, x8, #0x11 */
fu=x[8];fv=x[20];fa=sx64(fu);fb=sx64(fv); /* 0000ed04 cmp x8, x20 */
x[22]=(UINT64_C(18446744071562067968)); /* 0000ed08 mov x22, #-0x80000000 */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[20]); /* 0000ed0c csel x8, x8, x20, lt */
fu=x[8];fv=x[22];fa=sx64(fu);fb=sx64(fv); /* 0000ed10 cmp x8, x22 */
x[0]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[8]:(uint32_t)x[23]); /* 0000ed14 csel w0, w8, w23, gt */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ed18 bl #0xed18 */
x[8]=(sx32(ld32((x[26]+(uint64_t)INT64_C(0))))); /* 0000ed1c ldrsw x8, [x26] */
x[9]=(sx32((uint32_t)x[0])); /* 0000ed20 sxtw x9, w0 */
x[19]=(x[8]*x[9]); /* 0000ed24 mul x19, x8, x9 */
x[0]=(x[19]); /* 0000ed28 mov x0, x19 */
x[0]=((uint32_t)dv_stb24_unorm_sl(x[0])); /* 0000ed2c bl #0xed2c */
x[9]=(sx32(ld32((x[25]+(uint64_t)INT64_C(0))))); /* 0000ed30 ldrsw x9, [x25] */
x[8]=((x[19] << (x[0] & 63))); /* 0000ed34 lsl x8, x19, x0 */
x[10]=(uint32_t)((uint32_t)x[0]+UINT64_C(26)); /* 0000ed38 add w10, w0, #0x1a */
x[11]=(uint32_t)(UINT64_C(63)); /* 0000ed3c mov w11, #0x3f */
fu=(uint32_t)x[10];fv=UINT64_C(63);fa=sx32(fu);fb=sx32(fv); /* 0000ed40 cmp w10, #0x3f */
x[8]=(a64(x[8],UINT64_C(32))); /* 0000ed44 asr x8, x8, #0x20 */
x[10]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(uint32_t)x[10]:(uint32_t)x[11]); /* 0000ed48 csel w10, w10, w11, lt */
x[8]=(x[8]*x[9]); /* 0000ed4c mul x8, x8, x9 */
x[8]=(a64(x[8],x[10])); /* 0000ed50 asr x8, x8, x10 */
fu=x[8];fv=x[20];fa=sx64(fu);fb=sx64(fv); /* 0000ed54 cmp x8, x20 */
x[1]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(24)))); /* 0000ed58 ldr w1, [x21, #0x18] */
x[8]=((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?x[8]:x[20]); /* 0000ed5c csel x8, x8, x20, lt */
fu=x[8];fv=x[22];fa=sx64(fu);fb=sx64(fv); /* 0000ed60 cmp x8, x22 */
x[0]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[8]:(uint32_t)x[23]); /* 0000ed64 csel w0, w8, w23, gt */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ed68 bl #0xed68 */
x[19]=(uint32_t)((uint32_t)x[0]); /* 0000ed6c mov w19, w0 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000ed70 mov w0, w24 */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000ed74 mov w1, w19 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000ed78 bl #0xed78 */
x[26]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(28)))); /* 0000ed7c ldr w26, [sp, #0x1c] */
fu=(uint32_t)x[0];fv=(uint32_t)x[26];fa=sx32(fu);fb=sx32(fv); /* 0000ed80 cmp w0, w26 */
x[0]=(uint32_t)((uint32_t)x[26]); /* 0000ed84 mov w0, w26 */
if(dv_stb24_signed64(fa)<=dv_stb24_signed64(fb)) goto L0xed98; /* 0000ed88 b.le #0xed98 */
x[0]=(uint32_t)((uint32_t)x[24]); /* 0000ed8c mov w0, w24 */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000ed90 mov w1, w19 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000ed94 bl #0xed94 */
L0xed98: x[1]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(8)))); /* 0000ed98 ldr w1, [x21, #8] */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ed9c bl #0xed9c */
x[22]=(ld64((x[29]+(uint64_t)INT64_C(-32)))); /* 0000eda0 ldur x22, [x29, #-0x20] */
x[20]=(uint32_t)(ld32((sp+(uint64_t)INT64_C(32)))); /* 0000eda4 ldr w20, [sp, #0x20] */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000eda8 mov w1, w19 */
st32((x[22]+(uint64_t)INT64_C(0)),(uint32_t)x[0]); /* 0000edac str w0, [x22] */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000edb0 mov w0, w20 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000edb4 bl #0xedb4 */
x[26]=(ld64((x[29]+(uint64_t)INT64_C(-16)))); /* 0000edb8 ldur x26, [x29, #-0x10] */
fu=(uint32_t)x[0];fv=(uint32_t)x[27];fa=sx32(fu);fb=sx32(fv); /* 0000edbc cmp w0, w27 */
if(dv_stb24_signed64(fa)>=dv_stb24_signed64(fb)) goto L0xedd4; /* 0000edc0 b.ge #0xedd4 */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000edc4 mov w0, w20 */
x[1]=(uint32_t)((uint32_t)x[19]); /* 0000edc8 mov w1, w19 */
x[0]=((uint32_t)dv_stb24_ssub_sss(x[0],x[1])); /* 0000edcc bl #0xedcc */
x[27]=(uint32_t)((uint32_t)x[0]); /* 0000edd0 mov w27, w0 */
L0xedd4: x[1]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(16)))); /* 0000edd4 ldr w1, [x21, #0x10] */
x[0]=(uint32_t)((uint32_t)x[27]); /* 0000edd8 mov w0, w27 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000eddc bl #0xeddc */
x[25]=(ld64((sp+(uint64_t)INT64_C(40)))); /* 0000ede0 ldr x25, [sp, #0x28] */
x[20]=(uint32_t)((uint32_t)x[20]-(uint32_t)x[24]); /* 0000ede4 sub w20, w20, w24 */
x[1]=(uint32_t)(UINT64_C(52428)); /* 0000ede8 mov w1, #0xcccc */
x[1]=(uint32_t)(((uint32_t)x[1]&~UINT64_C(4294901760))|(UINT64_C(27852)<<16)); /* 0000edec movk w1, #0x6ccc, lsl #16 */
st32((x[25]+(uint64_t)INT64_C(0)),(uint32_t)x[0]); /* 0000edf0 str w0, [x25] */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000edf4 mov w0, w20 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000edf8 bl #0xedf8 */
x[1]=(uint32_t)((uint32_t)x[24]); /* 0000edfc mov w1, w24 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ee00 bl #0xee00 */
x[1]=(uint32_t)(UINT64_C(13107)); /* 0000ee04 mov w1, #0x3333 */
x[23]=(uint32_t)((uint32_t)x[0]); /* 0000ee08 mov w23, w0 */
x[1]=(uint32_t)(((uint32_t)x[1]&~UINT64_C(4294901760))|(UINT64_C(4915)<<16)); /* 0000ee0c movk w1, #0x1333, lsl #16 */
x[0]=(uint32_t)((uint32_t)x[20]); /* 0000ee10 mov w0, w20 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000ee14 bl #0xee14 */
x[1]=(uint32_t)((uint32_t)x[24]); /* 0000ee18 mov w1, w24 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ee1c bl #0xee1c */
fu=(uint32_t)x[23];fv=(uint32_t)x[28];fa=sx32(fu);fb=sx32(fv); /* 0000ee20 cmp w23, w28 */
x[8]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(uint32_t)x[23]:(uint32_t)x[28]); /* 0000ee24 csel w8, w23, w28, lt */
x[23]=(ld64((x[29]+(uint64_t)INT64_C(-24)))); /* 0000ee28 ldur x23, [x29, #-0x18] */
fu=(uint32_t)x[0];fv=(uint32_t)x[8];fa=sx32(fu);fb=sx32(fv); /* 0000ee2c cmp w0, w8 */
x[8]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[0]:(uint32_t)x[8]); /* 0000ee30 csel w8, w0, w8, gt */
x[1]=(uint32_t)(UINT64_C(52428)); /* 0000ee34 mov w1, #0xcccc */
st32((x[23]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000ee38 str w8, [x23] */
x[8]=(uint32_t)(ld32((x[25]+(uint64_t)INT64_C(0)))); /* 0000ee3c ldr w8, [x25] */
x[9]=(uint32_t)(ld32((x[22]+(uint64_t)INT64_C(0)))); /* 0000ee40 ldr w9, [x22] */
x[1]=(uint32_t)(((uint32_t)x[1]&~UINT64_C(4294901760))|(UINT64_C(27852)<<16)); /* 0000ee44 movk w1, #0x6ccc, lsl #16 */
x[0]=(uint32_t)((uint32_t)x[8]-(uint32_t)x[9]); /* 0000ee48 sub w0, w8, w9 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000ee4c bl #0xee4c */
x[1]=(uint32_t)(ld32((x[22]+(uint64_t)INT64_C(0)))); /* 0000ee50 ldr w1, [x22] */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ee54 bl #0xee54 */
x[8]=(uint32_t)(ld32((x[25]+(uint64_t)INT64_C(0)))); /* 0000ee58 ldr w8, [x25] */
x[9]=(uint32_t)(ld32((x[22]+(uint64_t)INT64_C(0)))); /* 0000ee5c ldr w9, [x22] */
x[1]=(uint32_t)(UINT64_C(13107)); /* 0000ee60 mov w1, #0x3333 */
x[20]=(uint32_t)((uint32_t)x[0]); /* 0000ee64 mov w20, w0 */
x[1]=(uint32_t)(((uint32_t)x[1]&~UINT64_C(4294901760))|(UINT64_C(4915)<<16)); /* 0000ee68 movk w1, #0x1333, lsl #16 */
x[0]=(uint32_t)((uint32_t)x[8]-(uint32_t)x[9]); /* 0000ee6c sub w0, w8, w9 */
x[0]=(dv_stb24_smpy_ss(x[0],x[1])); /* 0000ee70 bl #0xee70 */
x[1]=(uint32_t)(ld32((x[22]+(uint64_t)INT64_C(0)))); /* 0000ee74 ldr w1, [x22] */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ee78 bl #0xee78 */
x[8]=(uint32_t)(ld32((x[23]+(uint64_t)INT64_C(0)))); /* 0000ee7c ldr w8, [x23] */
x[1]=(uint32_t)(ld32((x[21]+(uint64_t)INT64_C(12)))); /* 0000ee80 ldr w1, [x21, #0xc] */
x[22]=(uint32_t)((uint32_t)x[0]); /* 0000ee84 mov w22, w0 */
x[0]=(uint32_t)((uint32_t)x[8]-(uint32_t)x[19]); /* 0000ee88 sub w0, w8, w19 */
x[0]=((uint32_t)dv_stb24_sadd_sss(x[0],x[1])); /* 0000ee8c bl #0xee8c */
fu=(uint32_t)x[20];fv=(uint32_t)x[0];fa=sx32(fu);fb=sx32(fv); /* 0000ee90 cmp w20, w0 */
x[8]=(uint32_t)((dv_stb24_signed64(fa)<dv_stb24_signed64(fb))?(uint32_t)x[20]:(uint32_t)x[0]); /* 0000ee94 csel w8, w20, w0, lt */
fu=(uint32_t)x[22];fv=(uint32_t)x[8];fa=sx32(fu);fb=sx32(fv); /* 0000ee98 cmp w22, w8 */
x[8]=(uint32_t)((dv_stb24_signed64(fa)>dv_stb24_signed64(fb))?(uint32_t)x[22]:(uint32_t)x[8]); /* 0000ee9c csel w8, w22, w8, gt */
st32((x[26]+(uint64_t)INT64_C(0)),(uint32_t)x[8]); /* 0000eea0 str w8, [x26] */
x[29]=(ld64((sp+(uint64_t)INT64_C(80))+0));x[30]=(ld64((sp+(uint64_t)INT64_C(80))+8)); /* 0000eea4 ldp x29, x30, [sp, #0x50] */
x[20]=(ld64((sp+(uint64_t)INT64_C(160))+0));x[19]=(ld64((sp+(uint64_t)INT64_C(160))+8)); /* 0000eea8 ldp x20, x19, [sp, #0xa0] */
x[22]=(ld64((sp+(uint64_t)INT64_C(144))+0));x[21]=(ld64((sp+(uint64_t)INT64_C(144))+8)); /* 0000eeac ldp x22, x21, [sp, #0x90] */
x[24]=(ld64((sp+(uint64_t)INT64_C(128))+0));x[23]=(ld64((sp+(uint64_t)INT64_C(128))+8)); /* 0000eeb0 ldp x24, x23, [sp, #0x80] */
x[26]=(ld64((sp+(uint64_t)INT64_C(112))+0));x[25]=(ld64((sp+(uint64_t)INT64_C(112))+8)); /* 0000eeb4 ldp x26, x25, [sp, #0x70] */
x[28]=(ld64((sp+(uint64_t)INT64_C(96))+0));x[27]=(ld64((sp+(uint64_t)INT64_C(96))+8)); /* 0000eeb8 ldp x28, x27, [sp, #0x60] */
/* shadow call stack bookkeeping omitted */ /* 0000eebc ldr x30, [x18, #-8]! */
sp=sp+UINT64_C(176); /* 0000eec0 add sp, sp, #0xb0 */
return; /* 0000eec4 ret  */
}
