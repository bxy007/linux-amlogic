/* Minimal reachable fixed arithmetic for STB2.4 scalar CVM generation.
 * Raw bit operations retain original AArch64 wrapping and saturation. */
#include "fixed.h"

static u64 sign32(u32 value)
{
	return (u64)value | ((0ULL - (value >> 31)) << 32);
}

static s32 signed32(u32 value)
{
	return value <= 0x7fffffffU ? (s32)value : -1 - (s32)~value;
}

static u32 asr32(u32 value, u32 shift)
{
	shift &= 31;
	return !shift ? value :
		(value >> shift) | ((0U - (value >> 31)) << (32 - shift));
}

static u64 asr64(u64 value, u32 shift)
{
	shift &= 63;
	return !shift ? value :
		(value >> shift) | ((0ULL - (value >> 63)) << (64 - shift));
}

static u32 abs32(u32 value)
{
	if (value == 0x80000000U)
		return 0x7fffffffU;
	return value >> 31 ? 0U - value : value;
}

static u64 abs64(u64 value)
{
	if (value == 0x8000000000000000ULL)
		return 0x7fffffffffffffffULL;
	return value >> 63 ? 0ULL - value : value;
}

static u32 norm32(u32 value)
{
	u32 shifts = 0;
	if (!value)
		return 0;
	while (!(value & 0x80000000U)) {
		value <<= 1;
		++shifts;
	}
	return !shifts ? 0 : shifts - 1;
}

static u32 norm64(u64 value)
{
	u32 shifts = 0;
	if (!value)
		return 0;
	while (!(value & 0x8000000000000000ULL)) {
		value <<= 1;
		++shifts;
	}
	return !shifts ? 0 : shifts - 1;
}

s32 dv_stb24_sadd_sss(u32 a, u32 b)
{
	u32 value = a + b;
	if (~(a ^ b) & (a ^ value) & 0x80000000U)
		value = 0x7fffffffU + (a >> 31);
	return signed32(value);
}

s32 dv_stb24_ssub_sss(u32 a, u32 b)
{
	u32 value = a - b;
	if ((a ^ b) & (a ^ value) & 0x80000000U)
		value = 0x7fffffffU + (a >> 31);
	return signed32(value);
}

u64 dv_stb24_smpy_ss(u32 a, u32 b)
{
	return (sign32(a) * sign32(b)) >> 31;
}

u32 dv_stb24_unorm_s(u32 value)
{
	return norm32(abs32(value));
}

u32 dv_stb24_unorm_sl(u64 value)
{
	return norm64(abs64(value));
}

s32 dv_stb24_sshl_ssu(u32 value, u32 shift)
{
	u64 wide;
	/* Old 2.4 uses signed LT for the upper clamp, then a 64-bit variable
	 * LSL (count & 63). Negative counts do not take the later 2.6 early
	 * zero return: -1 shifts by 63, INT32_MIN shifts by zero. */
	if (signed32(shift) >= 31)
		shift = 31;
	wide = sign32(value) << (shift & 63);
	if (wide != sign32((u32)wide))
		return signed32(0x7fffffffU + (value >> 31));
	return signed32((u32)wide);
}

s32 dv_stb24_sshr_sss(u32 value, u32 shift)
{
	if (shift >> 31)
		return dv_stb24_sshl_ssu(value, 0U - shift);
	return signed32(asr32(value, shift < 31 ? shift : 31));
}

u64 dv_stb24_sdiv_sls(u64 numerator, u32 denominator)
{
	u64 a = numerator >> 63 ? 0ULL - numerator : numerator;
	u32 b = denominator >> 31 ? 0U - denominator : denominator;
	u64 quotient;
	if (!b)
		return 0;
	quotient = dv_stb24_udiv64(a, b);
	if ((numerator >> 63) ^ (denominator >> 31))
		quotient = 0ULL - quotient;
	if (quotient != sign32((u32)quotient))
		quotient = 0x7fffffffULL + (quotient >> 63);
	return quotient;
}

static u32 log_polynomial(u32 value)
{
	u32 p = (u32)dv_stb24_smpy_ss(value, 0xfe64613fU);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x07a2972dU);
	p = (u32)dv_stb24_smpy_ss(value, p + 0xee6c8ee6U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x1d275dc5U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0xd213873aU);
	return (u32)dv_stb24_smpy_ss(value, p + 0x5c518466U);
}

static u32 pow_polynomial(u32 value)
{
	u32 p = (u32)dv_stb24_smpy_ss(value, 0x00029080U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x00164080U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x009d8f80U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x038d3180U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x0f5fe000U);
	p = (u32)dv_stb24_smpy_ss(value, p + 0x2c5c8b80U);
	return p + 0x40000000U;
}

s32 dv_stb24_fxLOG2Taylor(u32 value, u32 scale_out)
{
	u32 norm, a, result, shift, integral;
	if (!value)
		return signed32(1U << (scale_out & 31));
	norm = dv_stb24_unorm_s(value);
	a = ((value << (norm & 31)) & 0x3fffffffU) << 1;
	result = log_polynomial(a);
	shift = 30U - scale_out;
	result = shift >> 31 ? shift :
		asr32(result + 0x8e8U, shift < 31 ? shift : 31);
	integral = scale_out >> 31 ? 0 : (30U - norm) << (scale_out & 31);
	return signed32(result + integral);
}

s32 dv_stb24_fxPOW2Taylor(u32 value, u32 scale_in, u32 scale_out)
{
	u32 integral = scale_in >> 31 ? scale_in :
		asr32(value, scale_in < 31 ? scale_in : 31);
	u32 result = pow_polynomial((value << ((0U - scale_in) & 31)) >> 1);
	return dv_stb24_sshr_sss(result, 30U - scale_out - integral);
}

u32 dv_stb24_ssqrt_s(u32 value)
{
	u32 norm = norm32(value), a = value << (norm & 31);
	u32 square = (u32)dv_stb24_smpy_ss(a, a);
	u32 cube = (u32)dv_stb24_smpy_ss(square, a);
	u32 fourth = (u32)dv_stb24_smpy_ss(cube, a);
	u64 accumulator = (u64)a * 0x52aa9432U +
		sign32(square) * sign32(0xc6f3beebU) +
		sign32(cube) * 0x1f11c934U +
		sign32(fourth) * sign32(0xf88b70b4U) + 0x0762247a00000000ULL;
	u64 result = asr64(accumulator, (norm >> 1) + 30U);
	if (result > 0x7fffffffULL)
		result = 0x7fffffffULL;
	return norm & 1 ? (u32)dv_stb24_smpy_ss((u32)result, 0x5a82799aU) :
		(u32)result;
}
