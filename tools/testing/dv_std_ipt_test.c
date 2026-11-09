/* SPDX-License-Identifier: GPL-2.0 */
/* Host-only arithmetic/contract test. Build from the kernel tree with:
 * cc -std=gnu11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 *    tools/testing/dv_std_ipt_test.c -o /tmp/dv_std_ipt_test
 * The independent oracle uses host __int128; production uses signed64 only.
 */
#define DV_STD_IPT_HOST_TEST
#include <assert.h>
#include <stdio.h>
#include "../../drivers/amlogic/media/enhancement/amdolby_vision/dv_std_ipt.h"

static unsigned int checks;
#define CHECK(c) do { assert(c); checks++; } while (0)

static const s32 s1[9] = {9574, 0, 13802, 9574, -1540, -5348, 9574, 17610, 0};
static const s32 s2[9] = {7222, 8771, 390, 2654, 12430, 1300, 0, 422, 15962};
static const s32 o1[9] = {8194, 799, 1681, 8194, -933, 1091, 8194, 267, -5545};
static const s32 o2[9] = {17080, -348, -348, -348, 17080, -348, -348, -348, 17080};
static const u32 native_video[5] = {
	0x430f34bd, 0x15550833, 0x0e745c36, 0x09a90316, 0x000f7341
};
static const u32 native_gui[5] = {
	0x430f34bd, 0x15540833, 0x0e745c35, 0x09a90316, 0x000f7340
};
static const u32 native_c[5] = {
	0x08000800, 0x0ce90000, 0x0dade56a, 0xbc12460d, 0x000cfde1
};
static const u32 mark14_a[5] = {
	0x43cc3696, 0x1509059e, 0x0c4d5eaa, 0x0679018a, 0x000f77fd
};
static const u32 mark14_c[5] = {
	0x06660666, 0x47490333, 0x0657b260, 0x05b70ce4, 0x000ced65
};

struct fixture {
	u8 source[512], output[512];
	u32 dm1[27], dm2[24], dm3[26];
	struct dv_std_ipt_input in;
};

static void put16(u8 *p, s32 value)
{
	p[0] = (u16)value >> 8;
	p[1] = value;
}

static void put32(u8 *p, u32 value)
{
	p[0] = value >> 24;
	p[1] = value >> 16;
	p[2] = value >> 8;
	p[3] = value;
}

static void metadata(u8 *md, const s32 *m1, const s32 *m2, bool output)
{
	int i;

	memset(md, 0, 512);
	for (i = 0; i < 9; i++) {
		put16(md + 2 + i * 2, m1[i]);
		put16(md + 32 + i * 2, m2[i]);
	}
	put32(md + 20, output ? 0 : 0x01000000);
	put32(md + 24, 0x08000000);
	put32(md + 28, 0x08000000);
	md[50] = md[51] = 255;
	md[60] = 12;
	md[61] = output ? 2 : 0;
	md[62] = output ? 1 : 0;
	md[63] = 1;
	put16(md + 66, 4095);
}

static void fixture(struct fixture *f)
{
	struct dv_std_ipt_matrix m;
	int i;

	memset(f, 0, sizeof(*f));
	metadata(f->source, s1, s2, false);
	metadata(f->output, o1, o2, true);
	f->in.source_meta = f->source;
	f->in.source_len = 71;
	f->in.output_meta = f->output;
	f->in.output_len = 71;
	f->in.dm1 = f->dm1;
	f->in.dm2 = f->dm2;
	f->in.dm3 = f->dm3;
	f->in.core3_mode = 5;
	f->dm1[0] = 4095;
	f->dm1[1] = 0x10010;
	f->dm1[13] = f->dm2[11] = 1;
	f->dm3[11] = 0x9c400000;
	for (i = 0; i < 9; i++)
		m.c[i] = s1[i];
	m.shift = 13;
	dv_std_ipt_pack(&m, f->dm1 + 5);
	for (i = 0; i < 3; i++)
		f->dm1[10 + i] = s1[3 * i] * 256 +
			s1[3 * i + 1] * 2048 + s1[3 * i + 2] * 2048;
	memcpy(f->dm1 + 14, native_video, sizeof(native_video));
	memcpy(f->dm2 + 12, native_gui, sizeof(native_gui));
	memcpy(f->dm1 + 19, native_c, sizeof(native_c));
	memcpy(f->dm2 + 17, native_c, sizeof(native_c));
}

/* Independent signed128 rational constrained-rounding oracle. */
static s32 oracle_round(__int128 n, __int128 d)
{
	bool neg = (n < 0) != (d < 0);

	if (n < 0)
		n = -n;
	if (d < 0)
		d = -d;
	n = (n + d / 2) / d;
	return neg ? -(s32)n : (s32)n;
}

static void oracle_row(const s64 *n, s64 d, int shift, s32 target, s32 *q)
{
	__int128 ns[3], den = d, delta, cost, best_cost = 0;
	s32 sum = 0;
	int i, best, direction;

	CHECK(shift >= -31 && shift <= 46);
	if (shift < 0)
		den *= (__int128)1 << -shift;
	for (i = 0; i < 3; i++) {
		ns[i] = (__int128)n[i] * (shift < 0 ? 1 : ((__int128)1 << shift));
		if (den < 0)
			ns[i] = -ns[i];
	}
	if (den < 0)
		den = -den;
	for (i = 0; i < 3; i++) {
		q[i] = oracle_round(ns[i], den);
		sum += q[i];
	}
	while (sum != target) {
		direction = sum < target ? 1 : -1;
		best = -1;
		for (i = 0; i < 3; i++) {
			delta = q[i] * den - ns[i];
			cost = 2 * direction * delta * den + den * den;
			if (best < 0 || cost < best_cost) {
				best = i;
				best_cost = cost;
			}
		}
		q[best] += direction;
		sum += direction;
	}
}

static void atomic_failure(struct fixture *f, int reason)
{
	struct dv_std_ipt_setting output, before;

	memset(&output, 0xa5, sizeof(output));
	before = output;
	CHECK(dv_std_ipt_build(&f->in, &output) == reason);
	CHECK(!memcmp(&output, &before, sizeof(output)));
}

static void measured_fixture(void)
{
	struct fixture f;
	struct dv_std_ipt_setting out;
	struct dv_std_ipt_matrix a, b, g;
	s64 adj[9], det, middle[9], num[9], row;
	s32 expected[3], target;
	int i, r, c, k, result;

	fixture(&f);
	result = dv_std_ipt_build(&f.in, &out);
	if (result)
		fprintf(stderr, "fixture failed: %s\n", dv_std_ipt_result_name(result));
	CHECK(result == 0);
	CHECK(!memcmp(out.dm1 + 14, mark14_a, sizeof(mark14_a)));
	CHECK(!memcmp(out.dm1 + 19, mark14_c, sizeof(mark14_c)));
	CHECK(!memcmp(out.dm2 + 17, mark14_c, sizeof(mark14_c)));
	CHECK(out.core3_mode == 1 && !out.bypass_csc1 && out.bypass_cvm1 &&
	      out.bypass_cvm2 && out.gui_rebased);
	CHECK(!memcmp(out.dm1, f.dm1, 14 * sizeof(u32)));
	CHECK(!memcmp(out.dm1 + 24, f.dm1 + 24, 3 * sizeof(u32)));
	CHECK(!memcmp(out.dm2, f.dm2, 12 * sizeof(u32)));
	CHECK(!memcmp(out.dm2 + 22, f.dm2 + 22, 2 * sizeof(u32)));
	CHECK(!memcmp(out.dm3, f.dm3, 12 * sizeof(u32)));
	CHECK(!memcmp(out.dm3 + 16, f.dm3 + 16, 10 * sizeof(u32)));
	CHECK(out.dm3[12] == 4096 && !out.dm3[13] &&
	      out.dm3[14] == 2048 && out.dm3[15] == 2048);
	/* Independently compute the full rational GUI triple product. */
	CHECK(dv_std_ipt_decode(native_video, &a));
	CHECK(dv_std_ipt_decode(native_gui, &g));
	CHECK(dv_std_ipt_decode(mark14_a, &b));
	CHECK(dv_std_ipt_inverse_data(&a, adj, &det));
	for (r = 0; r < 3; r++)
		for (c = 0; c < 3; c++) {
			middle[3 * r + c] = 0;
			for (k = 0; k < 3; k++)
				middle[3 * r + c] += adj[3 * r + k] * g.c[3 * k + c];
		}
	for (r = 0; r < 3; r++)
		for (c = 0; c < 3; c++) {
			__int128 wide = 0;

			for (k = 0; k < 3; k++)
				wide += (__int128)b.c[3 * r + k] * middle[3 * k + c];
			CHECK(wide <= DV_STD_IPT_S64_MAX && wide >= -DV_STD_IPT_S64_MAX);
			num[3 * r + c] = wide;
		}
	CHECK(dv_std_ipt_decode(out.dm2 + 12, &g));
	CHECK(g.shift == 15);
	for (r = 0; r < 3; r++) {
		row = num[3 * r] + num[3 * r + 1] + num[3 * r + 2];
		target = oracle_round(row, det);
		oracle_row(num + 3 * r, det, 0, target, expected);
		for (i = 0; i < 3; i++)
			CHECK(g.c[3 * r + i] == expected[i]);
	}
}

static void failures(void)
{
	struct fixture f;
	struct dv_std_ipt_setting out;
	int i;

	CHECK(dv_std_ipt_build(NULL, &out) == DV_STD_IPT_BAD_POINTER);
	fixture(&f);
	CHECK(dv_std_ipt_build(&f.in, NULL) == DV_STD_IPT_BAD_POINTER);
	f.in.low_latency = true;
	atomic_failure(&f, DV_STD_IPT_BAD_MODE);
	fixture(&f); f.in.core3_mode = 1;
	atomic_failure(&f, DV_STD_IPT_BAD_MODE);
	fixture(&f); f.dm1[1] = 0x10000;
	atomic_failure(&f, DV_STD_IPT_BAD_MODE);
	fixture(&f); f.source[61] = 2; /* P5/RGB is explicitly unsupported. */
	atomic_failure(&f, DV_STD_IPT_BAD_SOURCE);
	fixture(&f); f.output[61] = 0;
	atomic_failure(&f, DV_STD_IPT_BAD_OUTPUT);
	fixture(&f); f.in.source_len = 70;
	atomic_failure(&f, DV_STD_IPT_BAD_SOURCE);
	fixture(&f); f.in.output_len = 513;
	atomic_failure(&f, DV_STD_IPT_BAD_OUTPUT);
	fixture(&f); f.source[70] = 1; /* Missing extension header. */
	atomic_failure(&f, DV_STD_IPT_BAD_SOURCE);
	fixture(&f); f.output[70] = 1;
	f.in.output_len = 76; put32(f.output + 71, 1);
	atomic_failure(&f, DV_STD_IPT_BAD_OUTPUT);
	fixture(&f); f.source[23] = 1; /* Non-integral 12-bit source offset. */
	atomic_failure(&f, DV_STD_IPT_BAD_SOURCE);
	fixture(&f); f.output[24] = 7;
	atomic_failure(&f, DV_STD_IPT_BAD_OUTPUT);
	fixture(&f); f.dm1[5]++;
	atomic_failure(&f, DV_STD_IPT_SOURCE_MISMATCH);
	fixture(&f); f.dm1[10]++;
	atomic_failure(&f, DV_STD_IPT_SOURCE_MISMATCH);
	fixture(&f); put16(f.source + 32, s2[0] - 100);
	atomic_failure(&f, DV_STD_IPT_BAD_MATRIX); /* Excess row normalization. */
	fixture(&f); put16(f.source + 32, -32768);
	atomic_failure(&f, DV_STD_IPT_BAD_MATRIX);
	fixture(&f); put16(f.output + 2, o1[0] - 1);
	atomic_failure(&f, DV_STD_IPT_BAD_MATRIX); /* Non-neutral output white. */
	fixture(&f); memset(f.output + 32, 0, 18);
	atomic_failure(&f, DV_STD_IPT_BAD_MATRIX);
	fixture(&f); memset(f.dm1 + 14, 0, 5 * sizeof(u32));
	atomic_failure(&f, DV_STD_IPT_BAD_GUI);
	fixture(&f); memset(f.dm2 + 12, 0, 5 * sizeof(u32));
	atomic_failure(&f, DV_STD_IPT_BAD_GUI);
	fixture(&f); f.dm2[17]++;
	atomic_failure(&f, DV_STD_IPT_BAD_GUI);
	fixture(&f);
	for (i = 0; i < 9; i++)
		put16(f.output + 32 + 2 * i, i % 4 ? 8192 : 8193);
	atomic_failure(&f, DV_STD_IPT_BAD_MATRIX); /* Near-singular, nonzero det. */
}

static u32 seed = 0x41b023;
static u32 random32(void)
{
	seed = seed * 1664525U + 1013904223U;
	return seed;
}

static void random_exact(void)
{
	struct fixture f;
	struct dv_std_ipt_matrix m, decoded;
	struct dv_std_ipt_setting out;
	s64 numerator[3], d, error, denom;
	s32 target, expected[3], value;
	int i, r, iteration, accepted = 0;

	for (iteration = 0; iteration < 3000; iteration++) {
		fixture(&f);
		memset(&m, 0, sizeof(m));
		m.shift = 14;
		m.c[0] = m.c[4] = m.c[8] = 16384;
		dv_std_ipt_pack(&m, f.dm1 + 14);
		dv_std_ipt_pack(&m, f.dm2 + 12);
		for (i = 0; i < 9; i++)
			put16(f.output + 32 + 2 * i, m.c[i]);
		for (r = 0; r < 3; r++) {
			m.c[3 * r] = 100 + random32() % 7000;
			m.c[3 * r + 1] = 100 + random32() % 7000;
			m.c[3 * r + 2] = 16384 - m.c[3 * r] - m.c[3 * r + 1];
			for (i = 0; i < 3; i++)
				put16(f.source + 32 + 2 * (3 * r + i), m.c[3 * r + i]);
		}
		if (dv_std_ipt_build(&f.in, &out))
			continue; /* Independently generated matrices may exceed cond<=64. */
		accepted++;
		CHECK(dv_std_ipt_decode(out.dm1 + 14, &decoded));
		for (i = 0; i < 9; i++)
			CHECK(decoded.c[i] == 2 * m.c[i]);
		CHECK(!memcmp(out.dm1 + 14, out.dm2 + 12, 5 * sizeof(u32)));
	}
	CHECK(accepted > 2000);
	/* Rational scaling and constrained rounding, both signs and shifts. */
	for (iteration = 0; iteration < 10000; iteration++) {
		i = (int)(random32() % 31) - 15;
		d = 1 + random32() % 100000;
		for (r = 0; r < 3; r++)
			numerator[r] = (s64)(s32)(random32() % 2000001) - 1000000;
		target = oracle_round((__int128)(numerator[0] + numerator[1] + numerator[2]) *
			(i < 0 ? 1 : ((__int128)1 << i)),
			(__int128)d * (i < 0 ? ((__int128)1 << -i) : 1));
		if (!dv_std_ipt_quant_row(numerator, d, i, target, false, expected))
			continue;
		oracle_row(numerator, d, i, target, decoded.c);
		for (r = 0; r < 3; r++) {
			CHECK(expected[r] == decoded.c[r]);
			CHECK(dv_std_ipt_scaled(numerator[r], d, i, 32768, &value, &error, &denom));
			CHECK(value == oracle_round((__int128)numerator[r] *
				(i < 0 ? 1 : ((__int128)1 << i)),
				(__int128)d * (i < 0 ? ((__int128)1 << -i) : 1)));
		}
	}
}

int main(void)
{
	measured_fixture();
	failures();
	random_exact();
	printf("dv_std_ipt: %u checks passed\n", checks);
	return 0;
}
