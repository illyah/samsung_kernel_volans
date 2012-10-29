#
# Makefile for the s3c framebuffer driver
#

ifeq ($(CONFIG_FB_S3C),y)
obj-y 				+= s3cfb.o s3cfb_spi.o
obj-$(CONFIG_PLAT_S3C64XX)	+= s3cfb_fimd4x.o
obj-$(CONFIG_PLAT_S5P64XX)	+= s3cfb_fimd5x.o
obj-$(CONFIG_PLAT_S5PC1XX)	+= s3cfb_fimd5x.o
obj-$(CONFIG_FB_S3C_LTE480WV)	+= s3cfb_lte480wv.o
obj-$(CONFIG_FB_S3C_LTV350QV)	+= s3cfb_ltv350qv.o
obj-$(CONFIG_FB_S3C_LTS222QV)	+= s3cfb_lts222qv.o
obj-$(CONFIG_FB_S3C_L5F30961)     += s3cfb_l5f30961.o
obj-$(CONFIG_FB_S3C_S6D04D1X21)     +=s3cfb_s6d04d1x21.o
obj-$(CONFIG_FB_S3C_BD6086)		+= bd6086.o
obj-$(CONFIG_FB_S3C_BL_AAT1231_1)	+= bl_aat1231_1.o
endif

# version 2 rules
ifeq ($(CONFIG_FB_S3C_V2),y)
obj-y					+= s3cfb2.o
obj-$(CONFIG_PLAT_S3C64XX)		+= s3cfb2_fimd4x.o
obj-$(CONFIG_PLAT_S5P64XX)		+= s3cfb2_fimd5x.o
obj-$(CONFIG_CPU_S5PC100)		+= s3cfb2_fimd5x.o
obj-$(CONFIG_CPU_S5PC110)		+= s3cfb2_fimd6x.o
obj-$(CONFIG_FB_S3C_V2_LTE480WV)	+= s3cfb2_lte480wv.o
obj-$(CONFIG_FB_S3C_V2_L5F30961)     += s3cfb_l5f30961.o
obj-$(CONFIG_FB_S3C_V2_S6D04D1X21)     +=s3cfb_s6d04d1x21.o
obj-$(CONFIG_FB_S3C_V2_BD6086)		+= bd6086.o
obj-$(CONFIG_FB_S3C_V2_BL_AAT1231_1)	+= bl_aat1231_1.o
endif

ifeq ($(CONFIG_FB_S3C_V2_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif

