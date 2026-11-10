#ifndef DV_STB24_NUMERIC_H
#define DV_STB24_NUMERIC_H
#include "types.h"
#include "lut_generation.h"
#define DV_STB24_CONTROL_BYTES 160
#define DV_STB24_RAW_BYTES 4104
#define DV_STB24_PACKED_WORDS 1024

/* Allocate once in the caller. No allocator, lock or static mutable state is
 * used by the numerical functions. One workspace per concurrent invocation. */
struct dv_stb24_numeric_workspace {
	u8 raw[DV_STB24_RAW_BYTES];
	u16 tables[4][512];
	u8 metadata[56];
	u8 tuning[48];
	struct dv_stb24_val_norm parameters[3];
	s32 sor;
};

/* Always fully regenerate. Zero means success; original flat/cache flag bits
 * are not exposed as CP update flags. packed contains four tables only;
 * the caller must preserve same-generation native G2L separately. */
int dv_stb24_numeric_generate(struct dv_stb24_numeric_workspace *workspace,
			    const u8 control[DV_STB24_CONTROL_BYTES],
			    u32 packed[DV_STB24_PACKED_WORDS]);
#endif
