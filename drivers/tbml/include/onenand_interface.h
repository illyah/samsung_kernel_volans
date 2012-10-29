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
 *  @file	onenand_interface.h
 *  @brief	This file is onenand interface header file
 *          
 *
 */

#ifndef _ONENAND_INTERFACE_H_
#define _ONENAND_INTERFACE_H_

/*****************************************************************************/
/* Definitions of PAM                                                        */
/*****************************************************************************/
#define     NOT_MAPPED              0xFFFFFFFF  /* Device not mapped 
                                                   in platform memory map    */

/* ECC Policy Selection */
#define     NO_ECC                  0       /* No ECC or ECC execution by 
                                               another type of ECC mechanism */
#define     SW_ECC                  1       /* ECC execution by XSR Software 
                                               (based on Hamming code)       */
#define     HW_ECC                  2       /* ECC execution by HW     
                                               (based on Hamming code)       */
                                               
/* Lock Scheme Policy Selection */
#define     NO_LOCK_SCHEME          0       /* No Lock Scheme execution      */
#define     SW_LOCK_SCHEME          1       /* Lock Scheme execution by SW   */
#define     HW_LOCK_SCHEME          2       /* Lock Scheme execution by HW   */

/*****************************************************************************/
/* return typedef of inter_setparam()                                         */
/*****************************************************************************/
typedef struct {
    unsigned int  nBaseAddr[XSR_MAX_DEV/XSR_MAX_VOL];
    unsigned int  nBaseAddr1[XSR_MAX_DEV/XSR_MAX_VOL];
                                    /* the base address for accessing NAND 
                                       device                                */
    unsigned short  nEccPol;                /* Ecc Execution Section                  
                                       NO_ECC : No Ecc or another type of ECC
                                       SW_ECC : Ecc Execution by XSR SW
                                       HW_ECC : Ecc Execution by HW 
                                                (based on Hamming code)      */
    unsigned short  nLSchemePol;            /* Lock Scheme Policy Section
                                       NO_LOCK_SCHEME : No Lock Scheme
                                       SW_LOCK_SCHEME : Lock Scheme Execution
                                                        by SW
                                       HW_LOCK_SCHEME : Lock Scheme Execution
                                                        by HW
                                        (if HW has Lock/Unlock functionality) */
    unsigned int  bByteAlign;             /* Memory Byte Alignment Problem
                                       TRUE32 : Byte Align Problem Free in
                                                XSR and LLD
                                               (Memory usage increased)
                                       FALSE32 : No Action for Byte Align
                                                 Problem                     */

    unsigned int  nDevsInVol;        /* number of devices in the volume */

    void   *pExInfo;                /* For Device Extension.
                                       For Extra Information of Device,
                                       data structure can be mapped.         */
} vol_param;

/*****************************************************************************/
/* Exported Function Prototype of PAM                                        */
/*****************************************************************************/
void   *inter_getparam		(void);
void    inter_low_ftable	(void  *pContext);

#endif  /* _ONENAND_INTERFACE_H_ */
