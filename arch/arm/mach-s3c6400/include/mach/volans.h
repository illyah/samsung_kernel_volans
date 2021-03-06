/*
 *  arch/arm/mach-s3c6400/include/mach/volans.h
 *
 *  Author:		Samsung Electronics
 *  Created:	17, Mar, 2009
 *  Copyright:	Samsung Electronics Co.Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef ASM_MACH_VOLANS_H
#define ASM_MACH_VOLANS_H

#define CONFIG_VOLANS_REV			0x60
/*
 * Memory Configuration (Reserved Memory Setting)
 */
#ifdef CONFIG_KERNEL_LOGGING
#define LOGGING_RAMBUF_SIZE     		(1024 * 1024)
#define	PHYS_SIZE				((208 * 1024 * 1024) - LOGGING_RAMBUF_SIZE)
#else
#define	PHYS_SIZE				(208 * 1024 * 1024)
#endif
#define CONFIG_RESERVED_MEM_CMM_JPEG_MFC_POST_CAMERA

/* Device Virtual Memory Map. */
#define VOLANS_PA_DPRAM			(0x5D000000)
#define VOLANS_VA_DPRAM			(0xF3000000)

/* Target booting mode. */
#define VOLANS_PA_BM			(0x50400000)
#define VOLANS_BM_MAGIC			(0x12341211)

/*
 * GPIO Configuration.
 */
/* GPA Group */
#define GPIO_AP_FLM_RXD			S3C64XX_GPA(0)
#define GPIO_AP_FLM_RXD_AF		(2)
#define GPIO_AP_FLM_TXD			S3C64XX_GPA(1)
#define GPIO_AP_FLM_TXD_AF		(2)

#define GPIO_AP_BT_RXD          S3C64XX_GPA(4)
#define GPIO_AP_BT_RXD_AF       (2)
#define GPIO_AP_BT_TXD          S3C64XX_GPA(5)
#define GPIO_AP_BT_TXD_AF       (2)
#define GPIO_AP_BT_CTS			S3C64XX_GPA(6)
#define GPIO_AP_BT_CTS_AF       (2)
#define GPIO_AP_BT_RTS          S3C64XX_GPA(7)
#define GPIO_AP_BT_RTS_AF       (2)

/* GPB Group */
#define GPIO_AP_RXD				S3C64XX_GPB(0)
#define GPIO_AP_RXD_AF			(2)
#define GPIO_AP_TXD				S3C64XX_GPB(1)
#define GPIO_AP_TXD_AF          (2)
#define GPIO_CAM_SCL			S3C64XX_GPB(2)
#define GPIO_CAM_SCL_AF			(6)
#define GPIO_CAM_SDA			S3C64XX_GPB(3)
#define GPIO_CAM_SDA_AF			(6)
#define GPIO_AP_SCL				S3C64XX_GPB(5)
#define GPIO_AP_SCL_AF			(2)
#define GPIO_AP_SDA				S3C64XX_GPB(6)
#define GPIO_AP_SDA_AF			(2)

/* GPC Group */
#define GPIO_LCD_READ			S3C64XX_GPC(0)
#define GPIO_LCD_READ_AF		(0)
#define GPIO_LCD_CLK			S3C64XX_GPC(1)
#define GPIO_LCD_CLK_AF			(1)
#define GPIO_LCD_SI				S3C64XX_GPC(2)
#define GPIO_LCD_SI_AF			(1)
#define GPIO_LCD_nCS			S3C64XX_GPC(3)
#define GPIO_LCD_nCS_AF			(1)

#define GPIO_TOUCH_SCL			S3C64XX_GPC(4)
#define GPIO_TOUCH_SCL_AF		(1)
#define GPIO_TOUCH_SDA			S3C64XX_GPC(5)
#define GPIO_TOUCH_SDA_AF		(1)
#define GPIO_TOUCH_RST			S3C64XX_GPC(6)
#define GPIO_TOUCH_RST_AF		(1)
#define GPIO_TOUCH_IRQ			S3C64XX_GPC(7)
#define GPIO_TOUCH_IRQ_AF		(7)

/* GPD Group */
#define GPIO_I2S_SCLK           S3C64XX_GPD(0)
#define GPIO_I2S_SCLK_AF        (3)

#define GPIO_I2S_SYNC           S3C64XX_GPD(2)
#define GPIO_I2S_SYNC_AF        (3)
#define GPIO_I2S_SDI            S3C64XX_GPD(3)
#define GPIO_I2S_SDI_AF         (3)
#define GPIO_I2S_SDO            S3C64XX_GPD(4)
#define GPIO_I2S_SDO_AF         (3)

/* GPE Group */
#define GPIO_BT_nRST			S3C64XX_GPE(0)
#define GPIO_BT_nRST_AF			(1)
#define GPIO_BOOT_MODE			S3C64XX_GPE(1)
#define GPIO_BOOT_MODE_AF		(0)

#define GPIO_PWR_I2C_SCL		S3C64XX_GPE(3)
#define GPIO_PWR_I2C_SCL_AF		(1)
#define GPIO_PWR_I2C_SDA		S3C64XX_GPE(4)
#define GPIO_PWR_I2C_SDA_AF		(1)

/* GPF Group */
#define GPIO_CAM_MCLK           S3C64XX_GPF(0)
#define GPIO_CAM_MCLK_AF        (2)
#define GPIO_CAM_HSYNC          S3C64XX_GPF(1)
#define GPIO_CAM_HSYNC_AF       (2)
#define GPIO_CAM_PCLK           S3C64XX_GPF(2)
#define GPIO_CAM_PCLK_AF        (2)
#define GPIO_CAM_3M_nRST        S3C64XX_GPF(3)
#define GPIO_CAM_3M_nRST_AF     (1)
#define GPIO_CAM_VSYNC          S3C64XX_GPF(4)
#define GPIO_CAM_VSYNC_AF       (2)
#define GPIO_CAM_D0             S3C64XX_GPF(5)
#define GPIO_CAM_D0_AF          (2)
#define GPIO_CAM_D1             S3C64XX_GPF(6)
#define GPIO_CAM_D1_AF          (2)
#define GPIO_CAM_D2             S3C64XX_GPF(7)
#define GPIO_CAM_D2_AF          (2)
#define GPIO_CAM_D3             S3C64XX_GPF(8)
#define GPIO_CAM_D3_AF          (2)
#define GPIO_CAM_D4             S3C64XX_GPF(9)
#define GPIO_CAM_D4_AF          (2)
#define GPIO_CAM_D5             S3C64XX_GPF(10)
#define GPIO_CAM_D5_AF          (2)
#define GPIO_CAM_D6             S3C64XX_GPF(11)
#define GPIO_CAM_D6_AF          (2)
#define GPIO_CAM_D7             S3C64XX_GPF(12)
#define GPIO_CAM_D7_AF          (2)

#define GPIO_VIB_PWM            S3C64XX_GPF(15)
#define GPIO_VIB_PWM_AF         (2)

/* GPG Group */
#define GPIO_T_FLASH_CLK        S3C64XX_GPG(0)
#define GPIO_T_FLASH_CLK_AF     (2)
#define GPIO_T_FLASH_CMD        S3C64XX_GPG(1)
#define GPIO_T_FLASH_CMD_AF     (2)
#define GPIO_T_FLASH_D0         S3C64XX_GPG(2)
#define GPIO_T_FLASH_D0_AF      (2)
#define GPIO_T_FLASH_D1         S3C64XX_GPG(3)
#define GPIO_T_FLASH_D1_AF      (2)
#define GPIO_T_FLASH_D2         S3C64XX_GPG(4)
#define GPIO_T_FLASH_D2_AF      (2)
#define GPIO_T_FLASH_D3         S3C64XX_GPG(5)
#define GPIO_T_FLASH_D3_AF      (2)

/* GPH Group */
#define GPIO_NAND_CLK			S3C64XX_GPH(0)
#define GPIO_NAND_CLK_AF		(2)
#define GPIO_NAND_CMD			S3C64XX_GPH(1)
#define GPIO_NAND_CMD_AF		(2)
#define GPIO_NAND_D0			S3C64XX_GPH(2)
#define GPIO_NAND_D0_AF			(2)
#define GPIO_NAND_D1			S3C64XX_GPH(3)
#define GPIO_NAND_D1_AF			(2)
#define GPIO_NAND_D2			S3C64XX_GPH(4)
#define GPIO_NAND_D2_AF			(2)
#define GPIO_NAND_D3			S3C64XX_GPH(5)
#define GPIO_NAND_D3_AF			(2)

#define GPIO_LCD_nRST			S3C64XX_GPH(7)
#define GPIO_LCD_nRST_AF		(1)
#define GPIO_LCD_ID				S3C64XX_GPH(9)
#define GPIO_LCD_ID_AF			(0)

/* GPI Group */
#define GPIO_LCD_D0				S3C64XX_GPI(0)
#define GPIO_LCD_D0_AF			(2)
#define GPIO_LCD_D1				S3C64XX_GPI(1)
#define GPIO_LCD_D1_AF			(2)
#define GPIO_LCD_D2				S3C64XX_GPI(2)
#define GPIO_LCD_D2_AF			(2)
#define GPIO_LCD_D3				S3C64XX_GPI(3)
#define GPIO_LCD_D3_AF			(2)
#define GPIO_LCD_D4				S3C64XX_GPI(4)
#define GPIO_LCD_D4_AF			(2)
#define GPIO_LCD_D5				S3C64XX_GPI(5)
#define GPIO_LCD_D5_AF			(2)
#define GPIO_LCD_D6				S3C64XX_GPI(6)
#define GPIO_LCD_D6_AF			(2)
#define GPIO_LCD_D7				S3C64XX_GPI(7)
#define GPIO_LCD_D7_AF			(2)
#define GPIO_LCD_D8				S3C64XX_GPI(8)
#define GPIO_LCD_D8_AF			(2)
#define GPIO_LCD_D9				S3C64XX_GPI(9)
#define GPIO_LCD_D9_AF			(2)
#define GPIO_LCD_D10			S3C64XX_GPI(10)
#define GPIO_LCD_D10_AF			(2)
#define GPIO_LCD_D11			S3C64XX_GPI(11)
#define GPIO_LCD_D11_AF			(2)
#define GPIO_LCD_D12			S3C64XX_GPI(12)
#define GPIO_LCD_D12_AF			(2)
#define GPIO_LCD_D13			S3C64XX_GPI(13)
#define GPIO_LCD_D13_AF			(2)
#define GPIO_LCD_D14			S3C64XX_GPI(14)
#define GPIO_LCD_D14_AF			(2)
#define GPIO_LCD_D15			S3C64XX_GPI(15)
#define GPIO_LCD_D15_AF			(2)

/* GPJ Group */
#define GPIO_LCD_D16			S3C64XX_GPJ(0)
#define GPIO_LCD_D16_AF			(2)
#define GPIO_LCD_D17			S3C64XX_GPJ(1)
#define GPIO_LCD_D17_AF			(2)
#define GPIO_LCD_D18			S3C64XX_GPJ(2)
#define GPIO_LCD_D18_AF			(2)
#define GPIO_LCD_D19			S3C64XX_GPJ(3)
#define GPIO_LCD_D19_AF			(2)
#define GPIO_LCD_D20			S3C64XX_GPJ(4)
#define GPIO_LCD_D20_AF			(2)
#define GPIO_LCD_D21			S3C64XX_GPJ(5)
#define GPIO_LCD_D21_AF			(2)
#define GPIO_LCD_D22			S3C64XX_GPJ(6)
#define GPIO_LCD_D22_AF			(2)
#define GPIO_LCD_D23			S3C64XX_GPJ(7)
#define GPIO_LCD_D23_AF			(2)
#define GPIO_LCD_HSYNC			S3C64XX_GPJ(8)
#define GPIO_LCD_HSYNC_AF		(2)
#define GPIO_LCD_VSYNC			S3C64XX_GPJ(9)
#define GPIO_LCD_VSYNC_AF		(2)
#define GPIO_LCD_DE				S3C64XX_GPJ(10)
#define GPIO_LCD_DE_AF			(2)
#define GPIO_LCD_MCLK			S3C64XX_GPJ(11)
#define GPIO_LCD_MCLK_AF		(2)

/* GPK Group */
#define GPIO_TA_EN				S3C64XX_GPK(0)
#define GPIO_TA_EN_AF			(1)
#define GPIO_AUDIO_EN           S3C64XX_GPK(1)
#define GPIO_AUDIO_EN_AF        (1)
#define GPIO_FM_LDO_ON			S3C64XX_GPK(2)
#define GPIO_FM_LDO_ON_AF        (1)
#define GPIO_MICBIAS_EN			S3C64XX_GPK(3)
#define GPIO_MICBIAS_EN_AF		(1)
#define GPIO_UART_SEL			S3C64XX_GPK(4)
#define GPIO_UART_SEL_AF		(1)
#define GPIO_FM_nRST            S3C64XX_GPK(5)
#define GPIO_FM_nRST_AF         (1)
#define GPIO_CAM_EN             S3C64XX_GPK(6)
#define GPIO_CAM_EN_AF          (1)
#define GPIO_CP_RST				S3C64XX_GPK(7)
#define GPIO_CP_RST_AF			(1)
#define GPIO_KBR0               S3C64XX_GPK(8)
#define GPIO_KBR0_AF            (3)
#define GPIO_KBR1               S3C64XX_GPK(9)
#define GPIO_KBR1_AF            (3)
#define GPIO_KBR2               S3C64XX_GPK(10)
#define GPIO_KBR2_AF            (3)
#define GPIO_FLM_SEL			S3C64XX_GPK(11)
#define GPIO_FLM_SEL_AF			(1)
#define GPIO_CAM_CIF_nRST       S3C64XX_GPK(12)
#define GPIO_CAM_CIF_nRST_AF    (1)
#define GPIO_CAM_CIF_nSTBY      S3C64XX_GPK(13)
#define GPIO_CAM_CIF_nSTBY_AF   (1)

#define GPIO_LED_DRV_EN			S3C64XX_GPK(14)
#define GPIO_LED_DRV_EN_AF		(1)

#define GPIO_VREG_MSMP_26V		S3C64XX_GPK(15)
#define GPIO_VREG_MSMP_26V_AF	(0)

/* GPL Group */
#define GPIO_KBC0               S3C64XX_GPL(0)
#define GPIO_KBC0_AF            (3)
#define GPIO_KBC1               S3C64XX_GPL(1)
#define GPIO_KBC1_AF            (3)
#define GPIO_KBC2               S3C64XX_GPL(2)
#define GPIO_KBC2_AF            (3)
#define GPIO_CAM_3M_nSTBY		S3C64XX_GPL(3)
#define GPIO_CAM_3M_nSTBY_AF	(1)
#define GPIO_VIB_EN             S3C64XX_GPL(4)
#define GPIO_VIB_EN_AF          (1)
#define GPIO_CP_USB_ON			S3C64XX_GPL(5)
#define GPIO_CP_USB_ON_AF		(1)
#define GPIO_PHONE_ON			S3C64XX_GPL(6)
#define GPIO_PHONE_ON_AF		(1)
#define GPIO_USIM_BOOT          S3C64XX_GPL(7)
#define GPIO_USIM_BOOT_AF       (1)
#define GPIO_LCD_EN				S3C64XX_GPL(8)
#define GPIO_LCD_EN_AF			(1)

#define GPIO_MIC_SEL			S3C64XX_GPL(9)
#define GPIO_MIC_SEL_AF			(1)

#define GPIO_FM_INT             S3C64XX_GPL(10)
#define GPIO_FM_INT_AF          (3)
#define GPIO_TA_nCONNECTED      S3C64XX_GPL(11)
#define GPIO_TA_nCONNECTED_AF   (3)

#define GPIO_LED_DRV_SEL		S3C64XX_GPL(12)
#define GPIO_LED_DRV_SEL_AF		(1)

#define GPIO_TF_EN              S3C64XX_GPL(13)
#define GPIO_TF_EN_AF           (1)

/* GPM Group */
#define GPIO_FM_SCL             S3C64XX_GPM(0)
#define GPIO_FM_SCL_AF          (1)
#define GPIO_FM_SDA             S3C64XX_GPM(1)
#define GPIO_FM_SDA_AF          (1)
#define GPIO_TA_nCHG            S3C64XX_GPM(2)
#define GPIO_TA_nCHG_AF         (0)
#define GPIO_PDA_ACTIVE			S3C64XX_GPM(3)
#define GPIO_PDA_ACTIVE_AF      (1)
#define GPIO_BT_WAKE			S3C64XX_GPM(4)
#define GPIO_BT_WAKE_AF			(1)

/* GPN Group */
#define GPIO_nONED_INT_AP       S3C64XX_GPN(0)
#define GPIO_nONED_INT_AP_AF    (2)
#define GPIO_HOST_WAKE			S3C64XX_GPN(1)
#define GPIO_HOST_WAKE_AF		(0)

#define GPIO_ACC_INT			S3C64XX_GPN(3)
#define GPIO_ACC_INT_AF			(2)
#define GPIO_BT_EN				S3C64XX_GPN(4)
#define GPIO_BT_EN_AF			(1)
#define GPIO_nPOWER             S3C64XX_GPN(5)
#define GPIO_nPOWER_AF          (2)
#define GPIO_T_FLASH_DETECT     S3C64XX_GPN(6)
#define GPIO_T_FLASH_DETECT_AF  (2)
#define GPIO_PHONE_ACTIVE_AP	S3C64XX_GPN(7)
#define GPIO_PHONE_ACTIVE_AP_AF (2)
#define GPIO_PM_INT_N           S3C64XX_GPN(8)
#define GPIO_PM_INT_N_AF        (2)
#define GPIO_INTB               S3C64XX_GPN(9)
#define GPIO_INTB_AF            (2)
#define GPIO_DET_35				S3C64XX_GPN(10)
#define GPIO_DET_35_AF			(2)
#define GPIO_EAR_SEND_END		S3C64XX_GPN(11)
#define GPIO_EAR_SEND_END_AF	(2)
#define GPIO_RESOUT_N_AP		S3C64XX_GPN(12)
#define GPIO_RESOUT_N_AP_AF		(0)
#define GPIO_BOOT_EINT13        S3C64XX_GPN(13)
#define GPIO_BOOT_EINT13_AF     (0)
#define GPIO_BOOT_EINT14        S3C64XX_GPN(14)
#define GPIO_BOOT_EINT14_AF     (0)
#define GPIO_BOOT_EINT15        S3C64XX_GPN(15)
#define GPIO_BOOT_EINT15_AF     (0)

/* GPO Group */
#define GPIO_REV_2				S3C64XX_GPO(6)
#define GPIO_REV_2_AF			(0)
#define GPIO_REV_3				S3C64XX_GPO(7)
#define GPIO_REV_3_AF			(0)

#define GPIO_TOUCH_LDO_EN		S3C64XX_GPO(12)
#define GPIO_TOUCH_LDO_EN_AF	(1)

/* GPP Group */
#define GPIO_PS_HOLD_PDA		S3C64XX_GPP(13)
#define GPIO_PS_HOLD_PDA_AF     (2)

/* GPQ Group */
#define GPIO_SET_DVS3			S3C64XX_GPQ(0)
#define GPIO_SET_DVS3_AF		(1)
#define GPIO_SET_DVS2			S3C64XX_GPQ(7)
#define GPIO_SET_DVS2_AF		(1)
#define GPIO_SET_DVS1			S3C64XX_GPQ(8)
#define GPIO_SET_DVS1_AF		(1)

/* gpio configuration structure */
struct __gpio_config {
	unsigned int gpio;
	unsigned int af;
	unsigned int level;
	unsigned int pull;
	unsigned int slp_con;
	unsigned int slp_pull;
};

/* Macro for PM sysfs nodes. */
#define VOLANS_PM_ATTR(_name)	\
	struct kobj_attribute _name = \
	__ATTR(_name, 0644, show_##_name, store_##_name)

/* runtime revision check. */
extern unsigned int CURRENT_REV(void);

#endif	/* ASM_MACH_VOLANS_H */

