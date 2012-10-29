/*
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Copyright (C) 2003-2009 Samsung Electronics                               *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License version 2 as         *
 * published by the Free Software Foundation.                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
*/
/**
 *  @version 	TinyBML_1.1.1_b008-XSR_1.5.2p4_b122
 *  @file	drivers/tbml/Adapt/onenand_interface.c
 *  @brief	Platform adaptation module for board
 *          
 *
 */

#include <linux/kernel.h>

#include <tbml_common.h>
#include <os_adapt.h>
#include <onenand_interface.h>
#include <onenand_lld.h>
#include "../TinyBML/onenand_reg.h"
#include "../TinyBML/onenand.h"

void inter_low_ftable(void *pstFunc)
{
	lld_ftable *pstLFT;

	pstLFT = (lld_ftable*)pstFunc;

    pstLFT[0].init          = ONLD_Init;
    pstLFT[0].open          = ONLD_Open;
    pstLFT[0].close         = ONLD_Close;
    pstLFT[0].read          = ONLD_Read;
    pstLFT[0].mread         = ONLD_MRead;
    pstLFT[0].dev_info      = ONLD_GetDevInfo;
    pstLFT[0].check_bad     = ONLD_ChkInitBadBlk;
    pstLFT[0].flush         = ONLD_FlushOp;
    pstLFT[0].set_rw        = ONLD_SetRWArea;
    pstLFT[0].ioctl         = ONLD_IOCtl;
    pstLFT[0].platform_info = ONLD_GetPlatformInfo;

#ifdef CONFIG_XSR_DUAL_DEVICE
    pstLFT[1].init          = ONLD_Init;
    pstLFT[1].open          = ONLD_Open;
    pstLFT[1].close         = ONLD_Close;
    pstLFT[1].read          = ONLD_Read;
    pstLFT[1].mread         = ONLD_MRead;
    pstLFT[1].dev_info      = ONLD_GetDevInfo;
    pstLFT[1].check_bad     = ONLD_ChkInitBadBlk;
    pstLFT[1].flush         = ONLD_FlushOp;
    pstLFT[1].set_rw        = ONLD_SetRWArea;
    pstLFT[1].ioctl         = ONLD_IOCtl;
    pstLFT[1].platform_info = ONLD_GetPlatformInfo;
#endif

}    

static vol_param gstParm[XSR_MAX_VOL];

void* inter_getparam(void)
{
	gstParm[0].nBaseAddr[0]	= CONFIG_TINY_FLASH_PHYS_ADDR0;
	gstParm[0].nBaseAddr1[0] = CONFIG_TINY_FLASH_PHYS_ADDR1;
	gstParm[0].nBaseAddr[1]	= NOT_MAPPED;
	gstParm[0].nBaseAddr[2]	= NOT_MAPPED;
	gstParm[0].nBaseAddr[3]	= NOT_MAPPED;

	gstParm[0].nEccPol	= HW_ECC;
	gstParm[0].nLSchemePol	= HW_LOCK_SCHEME;
	gstParm[0].bByteAlign	= TRUE32;
	gstParm[0].nDevsInVol	= 1;
	gstParm[0].pExInfo	= NULL;

#ifdef CONFIG_XSR_DUAL_DEVICE
	gstParm[1].nBaseAddr[0]	= CONFIG_XSR_FLASH_PHYS_ADDR2;
	gstParm[1].nBaseAddr[1]	= NOT_MAPPED;
	gstParm[1].nBaseAddr[2]	= NOT_MAPPED;
	gstParm[1].nBaseAddr[3]	= NOT_MAPPED;

	gstParm[1].nEccPol	= HW_ECC;
	gstParm[1].nLSchemePol	= HW_LOCK_SCHEME;
	gstParm[1].bByteAlign	= TRUE32;
	gstParm[1].nDevsInVol	= 1;
	gstParm[1].pExInfo	= NULL;
#else
	gstParm[1].nBaseAddr[0]	= NOT_MAPPED;
	gstParm[1].nBaseAddr[1]	= NOT_MAPPED;
	gstParm[1].nBaseAddr[2]	= NOT_MAPPED;
	gstParm[1].nBaseAddr[3]	= NOT_MAPPED;

	gstParm[1].nEccPol	= HW_ECC;
	gstParm[1].nLSchemePol	= HW_LOCK_SCHEME;
	gstParm[1].bByteAlign	= TRUE32;
	gstParm[1].nDevsInVol	= 0;
	gstParm[1].pExInfo	= NULL;
#endif

	return (void *) gstParm;
}


