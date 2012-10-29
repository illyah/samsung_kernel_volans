/*****************************************************************************/
/*                                                                           */
/* PROJECT : ANYSTORE II                                                     */
/* MODULE  : LLD                                                             */
/* NAME    : OneNAND LLD header                                              */
/* FILE    : ONLD.h                                                          */
/* PURPOSE : This file implements the exported function declarations and     */
/*           the exported values return values, macros, types,...            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Copyright (C) 2003-2009 Samsung Electronics                               */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License version 2 as         */
/* published by the Free Software Foundation.                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/* - 03/JUN/2003 [Janghwan Kim] : first writing                              */
/* - 04/OCT/2003 [Janghwan Kim] : reorganization                             */
/* - 11/DEC/2003 [Janghwan Kim] : Add onld_ioctl() function                  */
/* - 11/DEC/2003 [Janghwan Kim] : Change parmameter of onld_init()           */
/*                                                                           */
/*****************************************************************************/

#ifndef _ONENAND_H_
#define _ONENAND_H_

/*****************************************************************************/
/* ONLD External Function declarations                                       */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

INT32 ONLD_Init         (VOID  *pParm);
INT32 ONLD_Open         (UINT32 nDev);
INT32 ONLD_Close        (UINT32 nDev);
INT32 ONLD_Read         (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts,
                         UINT8 *pMBuf,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_Write        (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts,
                         UINT8 *pMBuf,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_Erase        (UINT32 nDev,    UINT32   nPbn,     UINT32 nFlag);
INT32 ONLD_CopyBack     (UINT32 nDev,    CpBkArg *pstCpArg, UINT32 nFlag);
INT32 ONLD_ChkInitBadBlk(UINT32 nDev,    UINT32   nPbn);
INT32 ONLD_SetRWArea    (UINT32 nDev,    UINT32   nSUbn,    UINT32 nUBlks);
INT32 ONLD_FlushOp      (UINT32 nDev);
INT32 ONLD_GetDevInfo   (UINT32 nDev,    LLDSpec *pstLLDDev);
INT32 ONLD_GetPrevOpData(UINT32 nDev,    UINT8   *pMBuf,    UINT8 *pSBuf);
INT32 ONLD_IOCtl        (UINT32 nDev,    UINT32   nCmd,
                         UINT8 *pBufI,   UINT32   nLenI,
                         UINT8 *pBufO,   UINT32   nLenO,
                         UINT32 *pByteRet);
INT32 ONLD_MRead        (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts,
                         SGL  *pstSGL,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_MWrite       (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts,
                         SGL  *pstSGL,   UINT8   *pSBuf,    UINT32 nFlag,
						 UINT32 *pErrPsn);
INT32 ONLD_EraseVerify  (UINT32 nDev,    LLDMEArg *pstMEArg,
                         UINT32 nFlag);
INT32 ONLD_MErase       (UINT32 nDev,    LLDMEArg *pstMEArg,
                         UINT32 nFlag);
VOID  ONLD_GetPlatformInfo
                        (LLDPlatformInfo*    pstLLDPlatformInfo);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif /*  _ONENAND_H_ */
