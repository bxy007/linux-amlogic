#
# Makefile for amdolby_vision
#

obj-$(CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_DOLBYVISION) += amdolby_vision.o
obj-$(CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_DOLBYVISION) += dv_std_ipt_cvm.o
dv_std_ipt_cvm-y := std_ipt_cvm/cvm.o std_ipt_cvm/control-builder.o \
	std_ipt_cvm/numeric.o std_ipt_cvm/curve_parameters.o \
	std_ipt_cvm/lut_generation.o std_ipt_cvm/fixed.o std_ipt_cvm/pq12.o \
	std_ipt_cvm/trim.o std_ipt_cvm/metadata-policy.o

obj-$(CONFIG_AMLOGIC_DV5_COMPAT_SHIM) += dv_compat_shim.o
CFLAGS_dv_compat_shim.o += -fno-stack-protector


