/*****************************************************************************/
/*                                                                           */
/* PROJECT : ANYSTORE II                                                     */
/* MODULE  : LLD                                                             */
/* NAME    : OneNAND Low-level Driver                                        */
/* FILE    : ONLD.cpp                                                        */
/* PURPOSE : OneNAND LLD Implementation                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2005, SAMSUNG ELECTRONICS CO., LTD.               */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing NAND/OneNAND based on Samsung              */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representations or warranties    */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/* - 26/MAY/2003 [Janghwan Kim] : First writing                              */
/* - 15/JUN/2003 [Janghwan Kim] : Code modification                          */
/* - 06/OCT/2003 [Janghwan Kim] : Code reorganization                        */
/* - 27/OCT/2003 [Janghwan Kim] : Add walkaround code for OndNand device bug */
/* - 07/NOV/2003 [Janghwan Kim] : Add ONLD_ERR_PRINT                         */
/* - 11/DEC/2003 [Janghwan Kim] : Add ONLD_IOCtrl() function                 */
/* - 15/DEC/2003 [Janghwan Kim] : CACHED_READ implementation                 */
/* - 20/DEC/2003 [Janghwan Kim] : DEFERRED_CHK operation implementation      */
/* - 07/JAN/2004 [Janghwan Kim] : Add _ChkPrevOpResult()                     */
/* - 15/JAN/2004 [Janghwan Kim] : Code Modification according to the Code    */
/*                                Inspection Result                          */
/* - 19/FEB/2004 [Janghwan Kim] : use static array for FIL context in place  */
/*                                of using the pointer and the OAM_Malloc    */
/* - 16/APR/2004 [Janghwan Kim] : Change Cached Read op flag policy          */
/*                                (Always ECC_ON -> follwing previous Read   */
/*                                 operation flag)                           */
/*   11-FEB-2004 [SongHo Yoon]  : added XSR_BIG_ENDIAN definition to support */
/*                                big-endian CPU                             */
/* - 19/FEB/2004 [Janghwan Kim] : use static array for FIL context in place  */
/*                                of using the pointer and the OAM_Malloc    */
/* - 16/APR/2004 [Janghwan Kim] : Change Cached Read op flag policy          */
/*                                (Always ECC_ON -> follwing previous Read   */
/*                                 operation flag)                           */
/* - 19/AUG/2004 [Younwon Park] : Reorganization for 1G                      */
/*                                (Lock Scheme change)                       */
/* - 28/OCT/2004 [Younwon Park] : Add onld_eraseverify, onld_merase functions*/
/* - 17/JAN/2005 [Younwon Park] : Code Modification for XSR1.3.1             */
/* - 25/MAR/2005 [Younwon Park] : LLD code merge                             */
/*                                512M M/A-die, 1G DDP, 1G M-die, 2G DDP     */
/*                                256M M/A-die, 128M M-die                   */
/* - 09/JUN/2005 [Younwon Park] : Solve the mismatch between                 */
/*                                OAM_Malloc() and OAM_Free()                */
/* - 22/JUN/2005 [Younwon Park] : Modify _ChkPrevOpResult()                  */
/* - 30/JUN/2005 [SeWook Na]    : Change function redirection of MEMCPY()    */
/*                                OAM_Memcpy() --> PAM_Memcpy()              */
/* - 22/JUL/2005 [YounWon Park] : Modify onld_mwrite()                       */
/*                                *pErrPsn = pstPrevOpInfo[nDev]->nPsn;      */
/* - 05/SEP/2005 [SeWook NA]    : Modify onld_eraseverify to use only after  */
/*                                MErase                                     */
/* - 22/SEP/2005 [Younwon Park] : Modify ONLD_Copyback to handle read error  */
/* - 27/SEP/2005 [Younwon Park] : Modify onld_mread/onld_mwrite to use SGL   */
/* - 11/JAN/2006 [WooYoung Yang] : Remove CACHED_READ                        */
/*                                 Modify onld_merase()                      */
/* - 18/JAN/2006 [WooYoung Yang] : Remove SYNC_BURST_READ Setting Code in    */
/*                                 onld_open                                 */
/* - 20/JAN/2006 [WooYoung Yang] : Remove All SYNC_BURST_READ define phase   */
/*                                 Add SYNC_MODE Setting Status Check Code   */
/*                                 in _InitDevInfo                           */
/* - 31/JAN/2006 [ByoungYoung Ahn] : added LLD_READ_DISTURBANCE, 			 */
/*									_ChkReadDisturb()						 */
/* - 31/JAN/2006 [ByoungYoung Ahn] : added AD_Idle()						 */
/* - 05/APR/2006 [WooYoung Yang] : Remove _SyncBurstBRead()                  */
/* - 22/AUG/2006 [WooYoung Yang] : Modified interrupt signal order           */
/*                                 for STL_ASyncMode                         */
/* - 19/OCT/2006 [WooYoung Yang] : In case of DDP chip, operand of chip      */
/*                                 selection is modified in onld_merase      */
/*                                 Added device information in ONLDSpec      */
/* - 30/NOV/2006 [Younwon Park] : fixed the defect in _mread()               */
/*                               (In case of Snapshot 4 in 1KB page depth)   */
/* - 11/JAN/2007 [WooYoung Yang] : removed error code of MErase due to       */
/*                                 scheme of MErase error verification       */
/* - 01/FEB/2007 [Younwon Park] : Modified erase refresh scheme              */
/*                                in onld_read() and onld_mread()            */
/* - 14/FEB/2007 [Yulwon Cho]: _cacheread() is implemented, and              */
/*                              _InitDevInfo() is modified.                   */
/* - 02/MAY/2007 [Yulwon Cho]: _cacheahead() is added for read-ahead feature */
/*                                                                           */
/*****************************************************************************/
#define	DENALI_IP
#define LLD_VERSION "OCLD_v1.0.0_XSR1.5.2p4_Linux_090427"

/*****************************************************************************/
/* Header file inclusion                                                     */
/*****************************************************************************/
#include <tbml_common.h>
#include <os_adapt.h>
#include <onenand_interface.h>
#include <onenand_lld.h>
#include <tbml_interface.h>
#if defined(CONFIG_RFS_TINYBML)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 19)
#include <linux/config.h>
#endif
#endif /* CONFIG_RFS_TINYBML */

/*****************************************************************************/
/* ONLD FN90 dedicated header file inclusion                                 */
/*****************************************************************************/
#include "onenand.h"
#include "onenand_reg.h"

/*****************************************************************************/
/* Local Configurations                                                      */
/*****************************************************************************/
#undef STRICT_CHK

#ifdef  ASYNC_MODE
#define DEFERRED_CHK    /* Should be set defined */
#else /* SYNC_MODE */
#define  DEFERRED_CHK   /* User Can Modify it */
#endif

#define ALIGNED_MEMCPY  /* User Can Modify it */
#undef  LOCK_TIGHT      /* for erase refresh, lock-tight should be disabled */


/*****************************************************************************/
/* Debug Configuration                                                       */
/*****************************************************************************/
#define _ONLD_DEBUG
#define _ONLD_ERR_TRACE

#define ONLD_RTL_PRINT(x)           LLD_RTL_PRINT(x)

#if defined(_ONLD_ERR_TRACE)
#define ONLD_ERR_PRINT(x)           LLD_RTL_PRINT(x)
#else
#define ONLD_ERR_PRINT(x)
#endif

#ifdef  _ONLD_DEBUG
#define ONLD_DBG_PRINT(x)           LLD_DBG_PRINT(x)
#else
#define ONLD_DBG_PRINT(x)
#endif

#define ONENANDCON_READ_FULL_SECTOR
//#define ONENANDCON_WRITE_FULL_SECTOR

#define OCLD_PREVENT_LOCKUP_ATTEMP_TIMES	0x10000000

/*****************************************************************************/
/* Function Redirection                                                      */
/*****************************************************************************/
#define MALLOC(x)                   AD_Malloc(x)
#define FREE(x)                     AD_Free(x)
#define MEMSET(x,y,z)               AD_Memset(x,y,z)
#define MEMCPY(x,y,z)               AD_Memcpy(x,y,z)

#define _SetRWRange                 _SetRWBlock
#define _LockTigRange               _LockTigBlock

#define _ReadMainData(x,y,z)        _ReadData(x,y,z,ONLD_MAIN_SIZE)
#define _WriteMainData(x,y,z)       _WriteData(x,y,z,ONLD_MAIN_SIZE)
#define _ReadSpareData(x,y,z)       _ReadData(x,y,z,ONLD_SPARE_SIZE)
#define _WriteSpareData(x,y,z)      _WriteData(x,y,z,ONLD_SPARE_SIZE)


/*****************************************************************************/
/* Local Constant Definiations                                               */
/*****************************************************************************/
#define MAX_SUPPORT_DEVS            8
#define NUM_OF_DEVICES              1

#define ONLD_READ_UERROR_A          0xAAAA
#define ONLD_READ_CERROR_A          0x5555

#define ONLD_MAIN_SIZE              512
#define ONLD_SPARE_SIZE             16

#define BUFF_MAIN_SIZE              128

#define FIRSTCHIP                   0
#define SECONDCHIP                  512

#define CHIP_128M                   0x0000
#define CHIP_256M                   0x0010
#define CHIP_512M                   0x0020
#define CHIP_1G                     0x0030
#define CHIP_2G                     0x0040
#define CHIP_4G                     0x0050
#define CHIP_8G                     0x0060

#define M_DIE                       0x0000
#define A_DIE                       0x0100
#define B_DIE                       0x0200

#define DDP_CHIP                    0x0008

/* OneNand Block Lock Scheme or Range Lock Scheme or none */
#define BLK_LK                      0
#define RNG_LK                      1
#define NON_LK                      2

#define MASK_8_BIT                  0xFF
#define MASK_16_BIT                 0xFFFF
#define MASK_32_BIT                 0xFFFFFFFF

// OneNAND Transfer Mode Selection
#define OND_DMA		(0x0)
#define OND_POL		(0x1)
#define OND_LDM		(0x2)

#define OND_TRANS_MODE	(OND_LDM)
#define ONENANDCON_CACHEREAD

/*
#if(ONENAND_TRANS_MODE == 0)
#define OND_TRANS_MODE	(OND_DMA)
#define ONENANDCON_CACHEREAD
#elif(ONENAND_TRANS_MODE == 1)
#define OND_TRANS_MODE	(OND_POL)
#elif(ONENAND_TRANS_MODE == 2)
#define OND_TRANS_MODE	(OND_LDM)
#define ONENANDCON_CACHEREAD
#endif
*/

#if (OND_TRANS_MODE == OND_DMA)
#include "s3c6410_dma_controller_macro.h"
#include "s3c6410_dma_controller.h"

#define USE_SDMA
#endif  // OND_TRANS_MODE

/* OneNand Block Lock Scheme or Range Lock Scheme or none */
#define SCLK_ONENAND_ON			(1<<4)

/*****************************************************************************/
/* ONLD Return Code                                                          */
/*****************************************************************************/
/* Major Return Value */
#define ONLD_READ_DISTURBANCE       LLD_READ_DISTURBANCE
#define ONLD_IOC_NOT_SUPPORT        LLD_IOC_NOT_SUPPORT
#define ONLD_DEV_NOT_OPENED         LLD_DEV_NOT_OPENED
#define ONLD_DEV_POWER_OFF          LLD_DEV_POWER_OFF
#define ONLD_DEV_NO_LLD_FUNC        LLD_DEV_NO_LLD_FUNC
#define ONLD_INIT_BADBLOCK          LLD_INIT_BADBLOCK
#define ONLD_INIT_GOODBLOCK         LLD_INIT_GOODBLOCK
#define ONLD_SUCCESS                LLD_SUCCESS
#define ONLD_ERASE_ERROR            LLD_ERASE_ERROR
#define ONLD_MERASE_ERROR           LLD_MERASE_ERROR
#define ONLD_PREV_ERASE_ERROR       LLD_PREV_ERASE_ERROR
#define ONLD_WRITE_ERROR            LLD_WRITE_ERROR
#define ONLD_PREV_WRITE_ERROR       LLD_PREV_WRITE_ERROR
#define ONLD_READ_ERROR             LLD_READ_ERROR
#define ONLD_CRITICAL_ERROR         LLD_CRITICAL_ERROR
#define ONLD_WR_PROTECT_ERROR       LLD_WR_PROTECT_ERROR
#define ONLD_ILLEGAL_ACCESS         LLD_ILLEGAL_ACCESS

#define ONLD_INIT_FAILURE           LLD_INIT_FAILURE
#define ONLD_OPEN_FAILURE           LLD_OPEN_FAILURE
#define ONLD_CLOSE_FAILURE          LLD_CLOSE_FAILURE

/* Minor Retun Value */
/* Previous Operation Flag */
/* LLD_PREV_OP_RESULT is combined with Major Return Value, and means
   Previous Operation Error except LLD_READ_ERROR */
#define ONLD_PREV_OP_RESULT         LLD_PREV_OP_RESULT


/*****************************************************************************/
/* ONLD Operation Flag Code                                                  */
/*****************************************************************************/
#define ONLD_FLAG_ASYNC_OP          LLD_FLAG_ASYNC_OP    /* Write/Erase/Copy */
#define ONLD_FLAG_SYNC_OP           LLD_FLAG_SYNC_OP     /* Write/Erase/Copy */
#define ONLD_FLAG_SYNC_MASK         (1 << 0)             /* Write/Erase/Copy  */
#define ONLD_FLAG_ECC_ON            LLD_FLAG_ECC_ON
#define ONLD_FLAG_ECC_OFF           LLD_FLAG_ECC_OFF

#define X08                         LLD_BW_X08
#define X16                         LLD_BW_X16

/* Multiple Erase support or not */
#define ME_OK                       LLD_ME_OK
#define ME_NO                       LLD_ME_NO

/*****************************************************************************/
/* Configuration for XSR LLD                                                 */
/*****************************************************************************/
#define LLD_OTP_UID	// OTP read enable
#define LLD_OTP_PBN	1000 //1800 // OTP block nPbn (should be in unlocked region)

/*****************************************************************************/
/* Local Data Structure                                                      */
/*****************************************************************************/
typedef struct
{
    UINT16      nMID;               /* Manufacturer ID                       */
    UINT16      nDID;               /* Device ID                             */
    UINT16      nGEN;               /* Version ID                            */

    UINT16      nNumOfBlks;         /* Number of Blocks                      */
    UINT16      nNumOfPlanes;       /* Number of Planes                      */
    UINT16      nBlksInRsv;         /* The Number of Blocks                  */
                                    /* in Reservior for Bad Blocks           */
    UINT8       nBadPos;            /* BadBlock Information Poisition        */
    UINT8       nLsnPos;            /* LSN Position                          */
    UINT8       nEccPos;            /* ECC Policy : HW_ECC, SW_ECC           */
    UINT8       nBWidth;            /* Device Band Width                     */

    UINT8       nMEFlag;            /* Multiple Erase Flag                   */
    UINT8       nLKFlag;            /* Lock scheme Flag                      */

    UINT32      nTrTime;            /* Typical Read Op Time                  */
    UINT32      nTwTime;            /* Typical Write Op Time                 */
    UINT32      nTeTime;            /* Typical Erase Op Time                 */
    UINT32      nTfTime;            /* Typical Transfer Op Time              */

} ONLDSpec;

typedef struct
{
	UINT32 nFBAShift;
	UINT32 nFPAShift;
	UINT32 nFSAShift;
	UINT32 nDFSDBSShift;
	UINT32 nFBAMask;
	UINT32 nFPAMask;
	UINT32 nFSAMask;
	UINT32 nDFSDBSMask;
}ONLDMemAddr;

typedef struct
{
    UINT32      BaseAddr;           /* Device Mapped Base Address            */
    UINT32      BaseAddr1;           /* Device Mapped Base Address            */
    BOOL32      bOpen;              /* Device Open Flag                      */
    BOOL32      bPower;             /* Device Power Flag                     */
    ONLDSpec*   pstDevSpec;         /* Device Information(Spec) pointer      */
} ONLDDev;

typedef struct
{
    UINT8   nBufSelSft;
    UINT8   nSctSelSft;
    UINT8   nBlkSelSft;
    UINT8   nPgSelSft;
    UINT8   nDDPSelSft;
    UINT8   nFPASelSft;

    UINT8   nFSAMask;
    UINT8   nSctsPerPG;

    UINT16  nDID;                   /* Real device ID */
    UINT16  nNumOfBlks;             /* Number of Blocks in device */
    UINT16  nBlksInRsv;             /* The Number of Blocks in Reservoir for bad Block */

    UINT32  (*SetRWArea)  (UINT32 nDev, UINT32 nSUbn, UINT32 nUBlks);
    UINT32  (*LockTight)  (UINT32  nDev, UINT8  *pBufI, UINT32 nLenI);
    INT32   (*MRead)      (UINT32 nDev, UINT32 nPsn, UINT32 nScts,
             SGL *pstSGL, UINT8 *pSBuf, UINT32 nFlag);
}ONLDInfo;

#if defined(DEFERRED_CHK)
typedef enum {
    NONE    = 0,
    READ    = 1,
    WRITE   = 2,
    ERASE   = 3,
    CPBACK  = 4,
    MERASE  = 5,
}OpType;
#endif /* #if defined(DEFERRED_CHK) */

typedef struct
{
#if defined(DEFERRED_CHK)
    OpType ePreOp;
    UINT32 nPsn;
    UINT32 nScts;
    UINT32 nFlag;
    UINT16 nCmd;
    LLDMEArg* pstPreMEArg;
#endif /* #if defined(DEFERRED_CHK)*/
    UINT16 nBufSel;
}PrevOpInfo;

/*****************************************************************************/
/* Local Variable                                                            */
/*****************************************************************************/
static UINT16        ONLD_CMD_LOCK_NAND;
static UINT16        ONLD_CMD_LOCK_TIGHT;
static ONLDInfo      astONLDInfo[MAX_SUPPORT_DEVS];
static PrevOpInfo   *pstPrevOpInfo[MAX_SUPPORT_DEVS];
static ONLDMemAddr   astONLDMemAddr[MAX_SUPPORT_DEVS];
static ONLDDev       astONLDDev[MAX_SUPPORT_DEVS];

#if (OND_TRANS_MODE == OND_DMA)
#define CACHED_TO_UNCACHED_OFFSET 0x20000000
#define ONLD_PHYSICAL_TO_VIRTUAL_OFFSET 0x76000000  // physical : 0x20000000,  cached : 0x96000000,  non-cached : 0xb6000000
#define ONLD_PAGEDATA_PA_BASEADDR 0x577FF7C0
#define ONLD_PAGEDATA_VA_BASEADDR 0x877FF7C0

static volatile S3C6410_DMAC_REG *g_pDMAC0Reg;
static volatile S3C6410_SYSCON_REG *g_pSYSCONReg;
static UINT32 g_nDMABufferPhyPage;
static UINT32 g_nDMABufferVirPage;
#endif  // OND_TRANS_MODE

static ONLDSpec      astNandSpec[] = {
   /*****************************************************************************/
   /*nMID, nDID, nGEN, nNumOfBlks,                                              */
   /*                      nNumOfPlanes                                         */
   /*                          nBlocksInRsv                                     */
   /*                              nBadPos                                      */
   /*                                 nLsnPos                                   */
   /*                                    nEccPos                                */
   /*                                       nBWidth                             */
   /*                                             nMEFlag                       */
   /*                                                   nLKFlag                 */
   /*                                                       nTrTime;(ns)        */
   /*                                                           nTwTime;(ns)    */
   /*                                                               nTeTime;(ns)*/
   /*                                                                   nTfTime */
   /*****************************************************************************/
    /* 1G DEMUX */
    {0xEC, 0x34, 0x00, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x35, 0x00, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 512M DEMUX */
    {0xEC, 0x24, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x25, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x24, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x25, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G DDP DEMUX */
    {0xEC, 0x3c, 0x00, 1024, 2, 20, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x3d, 0x00, 1024, 2, 20, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP DEMUX */
    {0xEC, 0x4c, 0x00, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x4d, 0x00, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP DEMUX based on 1Gb A-die */
    {0xEC, 0x4c, 0x01, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x4d, 0x01, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 256M DEMUX */
    {0xEC, 0x14, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x15, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x14, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x15, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 128M DEMUX */
    {0xEC, 0x04, 0x00,  256, 1,  5, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x05, 0x00,  256, 1,  5, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},

    /* 1G MUX */
    {0xEC, 0x30, 0x00, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x31, 0x00, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 512M MUX */
    {0xEC, 0x20, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x21, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x20, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x21, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G DDP MUX */
    {0xEC, 0x38, 0x00, 1024, 2, 20, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x39, 0x00, 1024, 2, 20, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP MUX */
    {0xEC, 0x48, 0x00, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x49, 0x00, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP MUX based on 1Gb A-die */
    {0xEC, 0x48, 0x01, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x49, 0x01, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 256M MUX */
    {0xEC, 0x10, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x11, 0x00,  512, 1, 10, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x10, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x11, 0x01,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 128M MUX */
    {0xEC, 0x00, 0x00,  256, 1,  5, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x01, 0x00,  256, 1,  5, 0, 2, 8, X16, ME_NO, RNG_LK, 50000, 350000, 2000000, 50},

    /* 2G DEMUX M-die */
    {0xEC, 0x44, 0x00, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x45, 0x00, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DEMUX A-die */
    {0xEC, 0x44, 0x01, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x45, 0x01, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G DDP DEMUX  */
    {0xEC, 0x5c, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x5d, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G DDP DEMUX based on 2G A-die */
    {0xEC, 0x5c, 0x01, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x5d, 0x01, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G DEMUX A-die */
    {0xEC, 0x34, 0x01, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x35, 0x01, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

     /* 512M DEMUX B-die */
    {0xEC, 0x24, 0x02,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x25, 0x02,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G MUX M-die */
    {0xEC, 0x40, 0x00, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x41, 0x00, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G MUX A-die */
    {0xEC, 0x40, 0x01, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x41, 0x01, 2048, 1, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G DDP MUX */
    {0xEC, 0x58, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x59, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G DDP MUX based on 2G A-die*/
    {0xEC, 0x58, 0x01, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x59, 0x01, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G MUX A-die */
    {0xEC, 0x30, 0x01, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x31, 0x01, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

     /* 512M MUX B-die */
    {0xEC, 0x20, 0x02,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x21, 0x02,  512, 1, 10, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G DEMUX B-die */
    {0xEC, 0x34, 0x02, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x35, 0x02, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G MUX B-die */
    {0xEC, 0x30, 0x02, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x31, 0x02, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP DEMUX B-die */
    {0xEC, 0x4c, 0x02, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x4d, 0x02, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP MUX B-die */
    {0xEC, 0x48, 0x02, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x49, 0x02, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G DEMUX B-die */
    {0xEC, 0x34, 0x03, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x35, 0x03, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 1G MUX B-die */
    {0xEC, 0x30, 0x03, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x31, 0x03, 1024, 1, 20, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP DEMUX C-die */
    {0xEC, 0x4c, 0x03, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x4d, 0x03, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 2G DDP MUX C-die */
    {0xEC, 0x48, 0x03, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x49, 0x03, 2048, 2, 40, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G MUX M-die */
    {0xEC, 0x50, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x51, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 4G DEMUX M-die */
    {0xEC, 0x54, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x55, 0x00, 4096, 2, 80, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 8G DDP MUX based on 4G M-die */
    {0xEC, 0x68, 0x00, 8192, 2,160, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x69, 0x00, 8192, 2,160, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    /* 8G DDP DEMUX based on 4G M-die */
    {0xEC, 0x6C, 0x00, 8192, 2,160, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},
    {0xEC, 0x6D, 0x00, 8192, 2,160, 0, 2, 8, X16, ME_NO, BLK_LK, 50000, 350000, 2000000, 50},

    {0x00, 0x00, 0x00,   0,  0,  0, 0, 0, 0,   0,  0,   0, 0,   0,  0,     0}

};


/*****************************************************************************/
/* Local Function Declarations                                               */
/*****************************************************************************/
static VOID     _GetDevID   (UINT32  nDev, UINT16 *nMID,
                             UINT16 *nDID, UINT16 *nVID);
#if defined(STRICT_CHK)
static BOOL32   _StrictChk  (UINT32  nDev, UINT32  nPsn, UINT32 nScts);
#endif  /* #if defined(STRICT_CHK) */
static INT32 _M01_MRead(UINT32 nDev, UINT32 nPsn, UINT32 nScts, SGL *pstSGL, UINT8 *pSBuf, UINT32 nFlag);


/*****************************************************************************/
/* Local Macros                                                              */
/*****************************************************************************/
#define SET_PWR_FLAG(x)         {astONLDDev[x].bPower = TRUE32;}
#define CLEAR_PWR_FLAG(x)       {astONLDDev[x].bPower = FALSE32;}

/* Get Device Information */
#define GET_DEV_BADDR(n)        (astONLDDev[n].BaseAddr)
#define GET_DEV_BADDR1(n)        (astONLDDev[n].BaseAddr1)
#define CHK_DEV_OPEN_FLAG(n)    (astONLDDev[n].bOpen)
#define CHK_DEV_PWR_FLAG(n)     (astONLDDev[n].bPower)
#define GET_DEV_MID(n)          (astONLDDev[n].pstDevSpec->nMID)
#define GET_DEV_DID(n)          (astONLDInfo[n].nDID)
#define GET_DEV_VID(n)          (astONLDDev[n].pstDevSpec->nVID)
#define GET_NUM_OF_BLKS(n)      (astONLDDev[n].pstDevSpec->nNumOfBlks)
#define GET_PGS_PER_BLK(n)      (64)
#define GET_SCTS_PER_PG(n)       astONLDInfo[n].nSctsPerPG
#define GET_SCTS_PER_BLK(n)     (GET_SCTS_PER_PG(n) * GET_PGS_PER_BLK(n))
#define GET_NUM_OF_DIES(n)      (astONLDDev[n].pstDevSpec->nNumOfPlanes)
#define GET_ONENANDCON_BADDR(n)	((n==ONENANDC0_MEM_VIRTADDR)?ONENANDC0_VIRTADDR:  \
									((n==ONENANDC1_MEM_VIRTADDR)?ONENANDC1_VIRTADDR: \
										((n==ONENANDC0_MEM_BASEADDR)?ONENANDC0_BASEADDR: \
											((n==ONENANDC1_MEM_BASEADDR)?ONENANDC1_BASEADDR: NULL))))


#define GET_ONENANDCON_NUM(n)        ((n==ONENANDC0_MEM_VIRTADDR)?0:1)
#define GET_ONENANDCON_NUM_FOR_VIRMEM(n)        (((n>=ONENANDC0_MEM_VIRTADDR)&&(n<ONENANDC1_MEM_VIRTADDR))?0:1)
#define GET_ONENANDCON_NUM_FOR_PHYMEM(n)        (((n>=ONENANDC0_MEM_BASEADDR)&&(n<ONENANDC1_MEM_BASEADDR))?0:1)

/* Get Operation Information */
#define GET_PREV_OP_TYPE(n)     (pstPrevOpInfo[n]->ePreOp)
#define GET_CUR_BUF_SEL(n)      (pstPrevOpInfo[n]->nBufSel)
#define GET_NXT_BUF_SEL(n)      ((pstPrevOpInfo[n]->nBufSel + 1) & 1)

#define GET_ONLD_MBUF_ADDR(x,nSct,nBuf)\
            (ONLD_DT_MB00_ADDR(x) + ((((nSct) & astONLDInfo[nDev].nSctSelSft) | ((nBuf) << astONLDInfo[nDev].nBufSelSft )) << 9))
#define GET_ONLD_SBUF_ADDR(x,nSct,nBuf)\
            (ONLD_DT_SB00_ADDR(x) + ((((nSct) & astONLDInfo[nDev].nSctSelSft) | ((nBuf) << astONLDInfo[nDev].nBufSelSft )) << 4))
#define GET_PBN(x, psn)         ((psn) >> astONLDInfo[x].nBlkSelSft)
#define GET_PPN(x, psn)         ((psn) >> astONLDInfo[x].nPgSelSft)


/*****************************************************************************/
/* Extern Function Declarations                                              */
/*****************************************************************************/
#if (OND_TRANS_MODE == OND_LDM)
extern void OneNand4burstPageRead(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
extern void OneNand4burstPageWrite(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
extern void OneNand4burstPageEmptyWrite(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
extern void OneNand8burstPageRead(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
extern void OneNand8burstPageWrite(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
extern void OneNand8burstPageEmptyWrite(UINT32 nSrcAddr, UINT32 nDstAddr, UINT32 nDatSize);
#endif  // OND_TRANS_MODE

typedef enum OneNAND_CLOCK
{
	ONDNAND_RATIO_1_TO_HCLKx2 = 0,
	ONDNAND_RATIO_2_TO_HCLKx2 = 1,
	ONDNAND_RATIO_3_TO_HCLKx2 = 2,
	ONDNAND_RATIO_4_TO_HCLKx2 = 3,
}OneNAND_FlashClock;

typedef enum OneNAND_LATENCY
{
	eOND_LATENCY3 = 3,
	eOND_LATENCY4 = 4,
	eOND_LATENCY5 = 5,
	eOND_LATENCY6 = 6,
	eOND_LATENCY7 = 7,
	eOND_LATENCY8 = 0,
	eOND_LATENCY9 = 1,
	eOND_LATENCY10 = 2
}OneNAND_eLATENCY;

typedef enum OneNAND_MODE
{
	eOND_ASYNC, eOND_SYNC_CONT, eOND_SYNC_BURST4, eOND_SYNC_BURST8, eOND_SYNC_BURST16, eOND_SYNC_BURST32, eOND_RM_SYNC_WM_ASYNC_BURST16
}OneNAND_eMODE;


/*****************************************************************************/
/* OneNAND Register Address Definitions                                      */
/*****************************************************************************/

// MAP command
#define BUFFER			(0<<24)
#define ARRAY_RW		(1<<24)
#define COMMANDS		(2<<24)
#define DIRECT_ACCESS	(3<<24)

#define	FBASHIFT1		12
#define	FPASHIFT1		6
#define FSASHIFT1		4

#define	FBASHIFT2		12
#define	FPASHIFT2		6
#define FSASHIFT2		4

#define ONLD_BUFFER_BASE(n)    (n + BUFFER)
#define ONLD_ARRAY_BASE(n)     (n + ARRAY_RW)
#define ONLD_CMD_BASE(n)       (n + COMMANDS)
#define ONLD_DIRECT_BASE(n)    (n + DIRECT_ACCESS)

#define CMAKE_FBA(n,addr)    ((addr & astONLDMemAddr[n].nFBAMask) << astONLDMemAddr[n].nFBAShift)
#define CMAKE_FPA(n,addr)    ((addr & astONLDMemAddr[n].nFPAMask) << astONLDMemAddr[n].nFPAShift)
#define CMAKE_FSA(n,addr)    ((addr & astONLDMemAddr[n].nFSAMask) << astONLDMemAddr[n].nFSAShift)
#define CMAKE_DFS(n,addr)    ((addr & astONLDMemAddr[n].nDFSDBSMask) << astONLDMemAddr[n].nDFSDBSShift)

#define ONLD_BUFFER_DEF(n,a)               (ONLD_BUFFER_BASE(n) + ((0xffff & a)<<1))
#define ONLD_ARRAY_DEF(n,a,b,p)               (ONLD_ARRAY_BASE(a) + CMAKE_FBA(n,b) + CMAKE_FPA(n,p))
#define ONLD_CMD_DEF(n,a,b,p)               (ONLD_CMD_BASE(a) + CMAKE_FBA(n,b) + CMAKE_FPA(n,p))
#define ONLD_DIRECT_DEF(n,a)               (ONLD_DIRECT_BASE(n) + ((0xffff & a)<<2))

#define ONLD_READ_BUFFER(n,addr)              (*(volatile UINT32*)ONLD_BUFFER_DEF(n,addr))
#define ONLD_WRITE_BUFFER(n,addr,data)        ((*(volatile UINT32*)ONLD_BUFFER_DEF(n,addr)) = data)
#define ONLD_READ_ARRAY(n,a,b,p)          (*(volatile UINT32*)ONLD_ARRAY_DEF(n,a,b,p))
#define ONLD_WRITE_ARRAY(n,a,b,p,data)    ((*(volatile UINT32*)ONLD_ARRAY_DEF(n,a,b,p)) = data)
#define ONLD_READ_CMD(n,a,b,p)            (*(volatile UINT32*)ONLD_CMD_DEF(n,a,b,p))
#define ONLD_WRITE_CMD(n,a,b,p,data)     ((*(volatile UINT32*)ONLD_CMD_DEF(n,a,b,p)) = data)
#define ONLD_READ_DIRECT(n,addr)              (*(volatile UINT32*)ONLD_DIRECT_DEF(n,addr))
#define ONLD_WRITE_DIRECT(n,addr,data)        ((*(volatile UINT32*)ONLD_DIRECT_DEF(n,addr)) = data)




// OneNAND Controller Page Length
#define ONDC_MAIN		(2048)
#define ONDC_MAINSPARE	(2048+64)


//////////
// Function Name : ONENAND_SetMemSpace
// Function Description : Set the OneNand Device Infomation to Controller Register & Set variable about the MEM_ADDR field Information
// Input : 	Controller - OneNand Controller Port Number
// Version : v0.1
void ONENAND_SetMemSpace(UINT32 nDev, UINT16 *nDID)
{
	UINT32 uDeviceDensity, uDeviceDDP;
	UINT32 nBAddr;
	UINT32 nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONENAND_SetMemSpace() nDev = %d\r\n"), nDev));

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	uDeviceDensity = (*nDID & 0xF0)>>4;
	uDeviceDDP     = (*nDID & 0x8)>>3;

	astONLDMemAddr[nDev].nDFSDBSMask = 0;
	astONLDMemAddr[nDev].nDFSDBSShift = 0;

	switch(uDeviceDensity)
	{
		case 0 :
				OCLD_REG_FBA_WIDTH(nCAddr) = 8;
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 1;
				OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT1;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT1;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT1;
				astONLDMemAddr[nDev].nFBAMask  = 0xFF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x1;
				break;
		case 1 :
				OCLD_REG_FBA_WIDTH(nCAddr) = 9;
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 1;
				OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT1;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT1;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT1;
				astONLDMemAddr[nDev].nFBAMask  = 0x1FF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x1;
				break;
		case 2 :
				OCLD_REG_FBA_WIDTH(nCAddr) = 9;
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 2;
				OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT2;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT2;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT2;
				astONLDMemAddr[nDev].nFBAMask  = 0x1FF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x3;
				break;
		case 3 :
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 2;
				if(uDeviceDDP)
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 9;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 1;
					astONLDMemAddr[nDev].nDFSDBSMask = 1;
					astONLDMemAddr[nDev].nDFSDBSShift = 21;
				}
				else
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 10;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				}
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT2;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT2;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT2;
				astONLDMemAddr[nDev].nFBAMask  = 0x3FF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x3;
				break;
		case 4 :
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 2;
				if(uDeviceDDP)
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 10;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 1;
					astONLDMemAddr[nDev].nDFSDBSMask = 1;
					astONLDMemAddr[nDev].nDFSDBSShift = 22;
				}
				else
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 11;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				}
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT2;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT2;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT2;
				astONLDMemAddr[nDev].nFBAMask  = 0x7FF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x3;
				break;
		case 5 :
				OCLD_REG_FPA_WIDTH(nCAddr) = 6;
				OCLD_REG_FSA_WIDTH(nCAddr) = 2;
				if(uDeviceDDP)
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 11;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 1;
					astONLDMemAddr[nDev].nDFSDBSMask = 1;
					astONLDMemAddr[nDev].nDFSDBSShift = 23;
				}
				else
				{
					OCLD_REG_FBA_WIDTH(nCAddr) = 12;
					OCLD_REG_DBS_DFS_WIDTH(nCAddr) = 0;
				}
				astONLDMemAddr[nDev].nFBAShift = FBASHIFT2;
				astONLDMemAddr[nDev].nFPAShift = FPASHIFT2;
				astONLDMemAddr[nDev].nFSAShift = FSASHIFT2;
				astONLDMemAddr[nDev].nFBAMask  = 0xFFF;
				astONLDMemAddr[nDev].nFPAMask  = 0x3F;
				astONLDMemAddr[nDev].nFSAMask  = 0x3;
				break;
		default :
				break;
	}
}

//////////
// Function Name : ONENAND_EnableECCCorrection
// Function Description : Set the ECC Error Correction Operation
// Input : 	Controller - OneNand Controller Port Number
//			uEnable -ONENAND_WITH_CORRECT :  ECC Error Correction enable
//					ONENAND_WITHOUT_CORRECT : ECC Error Correction disable
// Version : v0.1
void ONENAND_EnableECCCorrection(UINT32 nDev, BOOL32 bEnable)
{
	UINT32 uEnableECC;
	UINT32 nBAddr;
	UINT32 nCAddr;
	UINT32 temp = 0xffffffff;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	uEnableECC = OCLD_REG_MEM_CFG(nCAddr);

	if(bEnable == TRUE32)
		uEnableECC &= ~BIT_ECC;
	else
		uEnableECC |= BIT_ECC;

	OCLD_REG_MEM_CFG(nCAddr) = uEnableECC;
    temp = ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0));
}

//////////
// Function Name : ONENAND_EnableIOBE
// Function Description : Set the I/O Buffer enable for INT and RDY signals
// Input : 	Controller - OneNand Controller Port Number
//			uEnable - IOBE Enable/Disable
// Version : v0.1
void ONENAND_EnableIOBE(UINT32 nDev, BOOL32 bEnable)
{
	UINT32 uEnableIOBE;
	UINT32 nBAddr;
	UINT32 nCAddr;
	UINT32 temp = 0xffffffff;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	uEnableIOBE = OCLD_REG_MEM_CFG(nCAddr);
	uEnableIOBE &= ~(0x7<<5);

	uEnableIOBE |= BIT_RDYPOL | BIT_INTPOL | (bEnable<<5);

	OCLD_REG_MEM_CFG(nCAddr) = uEnableIOBE;
//    temp = ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0));
}

//////////
// Function Name : ONENAND_SetBurstLatency
// Function Description : Sets the burst read latency in cycles
// Input : 	Controller - OneNand Controller Port Number
//			eLatency - (OneNAND_eLATENCY)latency cycle
// Version : v0.1
void ONENAND_SetBurstLatency(UINT32 nDev, OneNAND_eLATENCY eLatency)
{
	UINT32 uBurstReadLatency;
	UINT32 nBAddr;
	UINT32 nCAddr;
	UINT32 temp = 0xffffffff;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	uBurstReadLatency = OCLD_REG_MEM_CFG(nCAddr);
	uBurstReadLatency &= ~(0x7<<12);

	uBurstReadLatency |= (eLatency<<12);

	OCLD_REG_MEM_CFG(nCAddr) = uBurstReadLatency;
    temp = ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0));

}

//////////
// Function Name : ONENAND_SyncMode
// Function Description : Set the OneNand Transfer Mode include burst length
// Input : 	Controller - OneNand Controller Port Number
//			uMode - 	Read&Write Transfer Mode
// Version : v0.1
void ONENAND_SetSyncMode(UINT32 nDev, OneNAND_eMODE eMode)
{
	UINT32 uSyncMode;
	UINT32 nBAddr;
	UINT32 nCAddr;
	UINT32 temp = 0xffffffff;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	uSyncMode = OCLD_REG_MEM_CFG(nCAddr);
	uSyncMode &= ~((1<<15)|(0x7<<9)|(1<<1));

	switch (eMode)
	{
		case eOND_SYNC_CONT:
			uSyncMode |= (0x1<<15)|(0x0<<9)|(0x1<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_16WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//sync, burst:incr
			break;

		case eOND_SYNC_BURST4:
			uSyncMode |= (0x1<<15)|(0x1<<9)|(0x1<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_4WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//sync, burst:4
			break;

		case eOND_SYNC_BURST8:
			uSyncMode |= (0x1<<15)|(0x2<<9)|(0x1<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_8WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//sync, burst:8
			break;

		case eOND_SYNC_BURST16:
			uSyncMode |= (0x1<<15)|(0x3<<9)|(0x1<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_16WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//sync, burst:16
			break;

		case eOND_SYNC_BURST32:
			uSyncMode |= (0x1<<15)|(0x4<<9)|(0x1<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_32WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//sync, burst:16
			break;

		case eOND_RM_SYNC_WM_ASYNC_BURST16:
			uSyncMode |= (0x1<<15)|(0x3<<9)|(0x0<<1);
			OCLD_REG_BURST_LEN(nCAddr) = BL_16WORDS;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;	    //rm sync, wm async, burst:16
			break;

		case eOND_ASYNC:
		default:
			uSyncMode |= (0x0<<15)|(0x0<<9)|(0x0<<1);
			OCLD_REG_BURST_LEN(nCAddr) = 0;
			OCLD_REG_MEM_CFG(nCAddr) = uSyncMode;		//rm async, burst:4
			break;
	}
    temp = ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0));
}

//////////
// Function Name : ONENAND_EnableIntPin
// Function Description : Interrupt Pin Enable&Disable
// Input : 	Controller - OneNand Controller Port Number
//			Enable - 1:Enable, 0: Disable
// Version : v0.1
void ONENAND_EnableIntPin(UINT32 nDev, UINT32 nFlag)
{
	UINT32 nBAddr;
	UINT32 nCAddr;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	OCLD_REG_INT_PIN_ENABLE(nCAddr) = nFlag & 0x1;
}

void ONENAND_EnableSpareTransfer(UINT32 nDev, UINT32 nFlag)
{
	UINT32 nBAddr;
	UINT32 nCAddr;

	nBAddr = GET_DEV_BADDR(nDev);
	nCAddr = GET_DEV_BADDR1(nDev);

	OCLD_REG_TRANS_SPARE(nCAddr) = nFlag & 0x1;
}

#if (OND_TRANS_MODE == OND_DMA)
BOOL MapDMABuffers()
{
	BOOL bRet = TRUE;

    ONLD_DBG_PRINT((TEXT("[ONLD:MSG] ++MapDMABuffers()\r\n")));

	g_nDMABufferPhyPage = (UINT32)ONLD_PAGEDATA_PA_BASEADDR;
	g_nDMABufferVirPage = (UINT32)(ONLD_PAGEDATA_VA_BASEADDR | CACHED_TO_UNCACHED_OFFSET);  // uncached area
//	g_nDMABufferVirPage = (UINT32)ONLD_PAGEDATA_VA_BASEADDR;  // cached area

	return bRet;
}

BOOL DMAInitialize()
{
#ifndef USE_SDMA
	g_pSYSCONReg->HCLK_GATE |= HCLK_GATE_DMA0_EN; // DMAC0 HCLK Pass
#else
	g_pSYSCONReg->HCLK_GATE |= HCLK_GATE_SDMA0_EN;
#endif

	if (!(g_pDMAC0Reg->DMACC3Control0 & TCINT_ENABLE))	// Check channel in use with TC Int Enable Bit !!!
	{
		g_pDMAC0Reg->DMACC3Control0 |= TCINT_ENABLE;	// Set TC Int Enable Bit to reserve this channel!!!
	}

	if (!(g_pDMAC0Reg->DMACConfiguration & DMAC_ENABLE))		// DMAC0 is Disabled
	{
		// Enable DMAC0
		g_pDMAC0Reg->DMACConfiguration = M1_LITTLE_ENDIAN | M2_LITTLE_ENDIAN | DMAC_ENABLE;
		ONLD_DBG_PRINT((TEXT("[ONLD:MSG] DMAC0 Enabled\r\n")));
	}

	return TRUE;
}

BOOL DMASetOneNAND(UINT32 nOneNANDMode, UINT32 nTransUnit, UINT32 nBurstSize)
{
	g_pDMAC0Reg->DMACC3LLI = 0;		// Disable LLI
	g_pDMAC0Reg->DMACC3Configuration = ALLOW_REQUEST | UNLOCK | FLOWCTRL(MEM_TO_MEM) | DEST_PERI(0) | SRC_PERI(0);

	// setting source
	g_pDMAC0Reg->DMACC3Control0 = (g_pDMAC0Reg->DMACC3Control0 & ~((1<<26)|(1<<24)|SRC_UNIT_MASK|SRC_BURST_MASK))
						|(INCREASE<<26) |(AHB_M1<<24)|(nTransUnit<<18)|(nBurstSize<<12);

	// setting destination
	g_pDMAC0Reg->DMACC3Control0 = (g_pDMAC0Reg->DMACC3Control0 & ~((1<<27)|(1<<25)|DEST_UNIT_MASK|DEST_BURST_MASK))
						|(INCREASE<<27) |(AHB_M1<<25)|(nTransUnit<<21)|(nBurstSize<<15);

	// setting OneNAND readpage
	g_pDMAC0Reg->DMACC3Configuration &= ~(ONENANDMODEDST|ONENANDMODESRC);
	g_pDMAC0Reg->DMACC3Configuration |= nOneNANDMode;

	// Clear Interrupt Pending
	g_pDMAC0Reg->DMACIntTCClear |= (1<<3);
	g_pDMAC0Reg->DMACIntErrClear |= (1<<3);
	g_pDMAC0Reg->DMACC3Configuration &= ~(TCINT_UNMASK|ERRINT_UNMASK);		// mask interrupt, polling DMA

	return TRUE;
}

BOOL DMASetChannelTransSize_Read(UINT32 uSrcAddr, UINT32 uDstAddr, UINT32 uByteCount)
{
	UINT32 uBaseOffset;

	if (MMUFlag & 0x01)
	{

		if (GET_ONENANDCON_NUM_FOR_VIRMEM(uSrcAddr))
		{
			uBaseOffset = ONLD_PHYSICAL_TO_VIRTUAL_OFFSET_CON1;
		}
		else
		{
			uBaseOffset = ONLD_PHYSICAL_TO_VIRTUAL_OFFSET_CON0;
		}

		g_pDMAC0Reg->DMACC3SrcAddr = (UINT32)(uSrcAddr - uBaseOffset - CACHED_TO_UNCACHED_OFFSET);

	}
	else
	{
		g_pDMAC0Reg->DMACC3SrcAddr = (UINT32)(uSrcAddr);
	}

	g_pDMAC0Reg->DMACC3DestAddr = (UINT32)uDstAddr;

	// setting transter size
	g_pDMAC0Reg->DMACC3Control1 = uByteCount;

	return TRUE;
}

BOOL DMASetChannelTransSize_Write(UINT32 uSrcAddr, UINT32 uDstAddr, UINT32 uByteCount)
{
	UINT32 uBaseOffset;

	if (MMUFlag & 0x01)
	{
		if (GET_ONENANDCON_NUM_FOR_VIRMEM(uSrcAddr))
		{
			uBaseOffset = ONLD_PHYSICAL_TO_VIRTUAL_OFFSET_CON1;
		}
		else
		{
			uBaseOffset = ONLD_PHYSICAL_TO_VIRTUAL_OFFSET_CON0;
		}

		g_pDMAC0Reg->DMACC3DestAddr = (UINT32)(uDstAddr - uBaseOffset  - CACHED_TO_UNCACHED_OFFSET);

	}
	else
	{
		g_pDMAC0Reg->DMACC3DestAddr = (UINT32)(uDstAddr);
	}

	g_pDMAC0Reg->DMACC3SrcAddr = (UINT32)uSrcAddr;

	// setting transter size
	g_pDMAC0Reg->DMACC3Control1 = uByteCount;

	return TRUE;
}

BOOL DMASetChannelTransSize(UINT32 uSrcAddr, UINT32 uDstAddr, UINT32 uByteCount)
{
	if (g_pDMAC0Reg->DMACC3Configuration&ONENANDMODESRC)
	{
		g_pDMAC0Reg->DMACC3SrcAddr = (UINT32)(uSrcAddr - CACHED_TO_UNCACHED_OFFSET - ONLD_PHYSICAL_TO_VIRTUAL_OFFSET);
		g_pDMAC0Reg->DMACC3DestAddr = (UINT32)uDstAddr;
	}
	else if (g_pDMAC0Reg->DMACC3Configuration&ONENANDMODEDST)
	{
		g_pDMAC0Reg->DMACC3SrcAddr = (UINT32)uSrcAddr;
		g_pDMAC0Reg->DMACC3DestAddr = (UINT32)(uDstAddr - CACHED_TO_UNCACHED_OFFSET - ONLD_PHYSICAL_TO_VIRTUAL_OFFSET);
	}
	else
	{
		ONLD_ERR_PRINT((TEXT("[ONLD:ERR] DMACh is not set\r\n")));
	}

	// setting transter size
	g_pDMAC0Reg->DMACC3Control1 = uByteCount;

	return TRUE;
}

BOOL DMAChannelStart()
{
	// Clear Halt Bit
	g_pDMAC0Reg->DMACC3Configuration &= ~HALT;

	// Enable Channel
	g_pDMAC0Reg->DMACC3Configuration |= CHANNEL_ENABLE;

	return TRUE;
}
#endif  // OND_TRANS_MODE

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/
static VOID
_WriteEmptyData(UINT32 pAddr, UINT32 nScts, UINT32 nSize)
{
#if (OND_TRANS_MODE == OND_POL)
    UINT32 nBytes;

    while (nScts-- > 0)
    {
        for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
        {
            *(volatile UINT32 *)pAddr = 0xffffffff;
        }
    }
#elif (OND_TRANS_MODE == OND_LDM)
	if (((nSize * nScts) == 16) || ((nSize * nScts) == 48))
		OneNand4burstPageEmptyWrite(0, pAddr, (UINT32)((nSize * nScts)/16));
	else
		OneNand8burstPageEmptyWrite(0, pAddr, (UINT32)((nSize * nScts)/32));
#endif  // OND_TRANS_MODE
}

static VOID
_WriteData(UINT32 pAddr, UINT8 *pBuf, UINT32 nScts, UINT32 nSize)
{
    register UINT32 *pBuffer;
   	UINT8   cTmpBuf[ONDC_MAINSPARE];

	if ((UINT32)pBuf & 0x3)
	{
		MEMCPY(&(cTmpBuf[0]), pBuf, nSize * nScts);
		pBuffer = (UINT32 *)cTmpBuf;
	}
	else
	{
		pBuffer = (UINT32 *)pBuf;
	}

#if (OND_TRANS_MODE == OND_POL)
    UINT32 nBytes;

    while (nScts-- > 0)
    {
        for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
        {
            *(volatile UINT32 *)pAddr = *pBuffer++;
        }
    }
#elif (OND_TRANS_MODE == OND_LDM)
	if (((nSize * nScts) == 16) || ((nSize * nScts) == 48))
		OneNand4burstPageWrite((UINT32)pBuffer, pAddr, (UINT32)((nSize * nScts)/16));
	else
		OneNand8burstPageWrite((UINT32)pBuffer, pAddr, (UINT32)((nSize * nScts)/32));
#endif  // OND_TRANS_MODE
}

static VOID
_ReadEmptyData(UINT32 pAddr, UINT32 nScts, UINT32 nSize)
{
    UINT32 nBytes;
    volatile UINT32 nTmp;

    while (nScts-- > 0)
    {
        for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
        {
            nTmp = *(volatile UINT32 *)pAddr;
        }
    }
}

static VOID
_ReadData(volatile UINT32 pAddr, register UINT8 *pBuf, UINT32 nScts, UINT32 nSize)
{
    register UINT32 *pBuffer = (UINT32 *)pBuf;

#if (OND_TRANS_MODE == OND_POL)
    UINT32 nBytes;

    while (nScts-- > 0)
    {
        for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
        {
            *pBuffer++ = *(volatile UINT32 *)pAddr;
        }
    }
#elif (OND_TRANS_MODE == OND_LDM)
	OneNand8burstPageRead(pAddr, (UINT32)pBuffer, (UINT32)((nSize * nScts)/32));
#endif  // OND_TRANS_MODE
}

#ifdef ONENANDCON_WRITE_FULL_SECTOR
static VOID
_WriteData1(UINT32 pAddr, register UINT8 *pBuf)
{
    register UINT32 *pBuffer = (UINT32 *)pBuf;

#if (OND_TRANS_MODE == OND_POL)
    UINT32 nBytes;
    UINT32 nSize = ONDC_MAINSPARE;

    for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
    {
        *(volatile UINT32 *)pAddr = *pBuffer++;
    }
#elif (OND_TRANS_MODE == OND_LDM)
    OneNand8burstPageWrite((UINT32)pBuffer, (UINT32)pAddr, (UINT32)(ONDC_MAINSPARE/32));
#elif (OND_TRANS_MODE == OND_DMA)
	DMASetChannelTransSize_Write(g_nDMABufferPhyPage, pAddr, ONDC_MAINSPARE/4);
	DMAChannelStart();

	// Check transfer end
	while((g_pDMAC0Reg->DMACC3Configuration&CHANNEL_ENABLE));
#endif  // OND_TRANS_MODE
}
#endif  // ONENANDCON_WRITE_FULL_SECTOR

#ifdef ONENANDCON_READ_FULL_SECTOR
static VOID
_ReadData1(UINT32 pAddr, register UINT8 *pBuf, UINT32 nCAddr, UINT16 *pEccRes, UINT16 *pIntRes)
{
    register UINT32 *pBuffer = (UINT32 *)pBuf;

#if (OND_TRANS_MODE == OND_POL)
    UINT32 nBytes;
    UINT32 nSize = ONDC_MAINSPARE;

    for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT32))
    {
        *pBuffer++ = *(volatile UINT32 *)pAddr;
    }

#elif (OND_TRANS_MODE == OND_LDM)
//	DisableIRQ();
	OneNand8burstPageRead((UINT32)pAddr, (UINT32)pBuffer, 1);
	*pEccRes = (UINT16)OCLD_REG_ECC_ERR_STAT(nCAddr);
	*pIntRes = (UINT16)OCLD_REG_INT_ERR_STAT(nCAddr);
//	EnableIRQ();
	pBuffer += 8;
	OneNand8burstPageRead((UINT32)pAddr, (UINT32)pBuffer, (UINT32)((ONDC_MAINSPARE/32)-1));
#elif (OND_TRANS_MODE == OND_DMA)
	DMASetChannelTransSize_Read(pAddr, g_nDMABufferPhyPage, ONDC_MAINSPARE/4);
	DMAChannelStart();

	// Check transfer end
	while((g_pDMAC0Reg->DMACC3Configuration&CHANNEL_ENABLE));
#endif  // OND_TRANS_MODE
}
#endif  // ONENANDCON_READ_FULL_SECTOR

static VOID
_Read_M11_Data(UINT16 *pM11Addr, UINT16 *pBuf, UINT32 nScts, UINT32 nSize)
{
    UINT32 nBytes;

    while (nScts-- > 0)
    {
        for (nBytes = nSize; nBytes > 0; nBytes -= sizeof(UINT16))
        {
            *pBuf++ = (UINT16)*pM11Addr++;
        }
    }
}

UINT32 _SetECC(UINT32 nBAddr, UINT32 nCAddr, UINT32 nFlag)
{
	UINT32 nWaitingTimeOut = OCLD_PREVENT_LOCKUP_ATTEMP_TIMES;

    if ( (nFlag & ONLD_FLAG_ECC_ON) != 0 )
    {
        /* System Configuration Reg set (ECC On)*/
        if ((OCLD_REG_MEM_CFG(nCAddr) & BIT_ECC) == BIT_ECC)
        {
	        OCLD_REG_MEM_CFG(nCAddr) &= ~BIT_ECC;
		    while ( ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0)) & BIT_ECC )
		    {
		    	if (--nWaitingTimeOut == 0)
		    	{
					ONLD_ERR_PRINT((TEXT("[ONLD : ERR] _SetECC()  ECC_ON check, busy wait time-out\r\n")));
					return (ONLD_ILLEGAL_ACCESS);
		    	}
		        OCLD_REG_MEM_CFG(nCAddr) &= ~BIT_ECC;
		    }
	    }
    }
    else
    {
        /* System Configuration Reg set (ECC Off)*/
        if ((OCLD_REG_MEM_CFG(nCAddr) & BIT_ECC) != BIT_ECC)
        {
	        OCLD_REG_MEM_CFG(nCAddr) |= BIT_ECC;
		    while ( !(ONLD_READ_DIRECT(nBAddr, ONLD_REG_SYS_CONF1(0x0)) & BIT_ECC) )
		    {
		    	if (--nWaitingTimeOut == 0)
		    	{
					ONLD_ERR_PRINT((TEXT("[ONLD : ERR] _SetECC()  ECC_OFF check, busy wait time-out\r\n")));
					return (ONLD_ILLEGAL_ACCESS);
		    	}
		        OCLD_REG_MEM_CFG(nCAddr) |= BIT_ECC;
		    }
        }
    }

    return (ONLD_SUCCESS);
}

UINT32
_SetRWBlock(UINT32 nDev, UINT32 nSUbn, UINT32 nUBlks)
{
#if !defined(XSR_NBL2)
    UINT32 nBAddr;
    UINT32 nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD : IN] ++_SetRWBlock() nDev = 0x%x, nSUbn = 0x%x, nUBlks = 0x%x\r\n"), nDev, nSUbn, nUBlks));
    /* IOBE bit of pONENANDReg->MEM_CFG should be enabled for M10 execution. */

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

#if 0
    /* Clear INT */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /* Set start address to be unlocked */
    if (nUBlks > 1) ONLD_WRITE_CMD(nDev, nBAddr, nSUbn, 0, 0x08);

    /* Set unlock end address and initiate the unlock */
    ONLD_WRITE_CMD(nDev, nBAddr, nSUbn + nUBlks -1, 0, 0x09);

    /* wait until command is completed */
	while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
	{
        /* Wait until device ready */
        AD_Idle(INT_MODE);
    }

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == (UINT16)BIT_LOCKED_BLK)
    {
       /* Wait until device UnLock */
       AD_Idle(INT_MODE);
    }
#else
    while (nUBlks-- > 0)
    {
        /* Clear INT */
        OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

        /* Set unlock end address and initiate the unlock */
        ONLD_WRITE_CMD(nDev, nBAddr, nSUbn, 0, 0x09);

        /* wait until command is completed */
        while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
        {
            /* Wait until device ready */
            AD_Idle(INT_MODE);
        }

        ONLD_WRITE_DIRECT(nBAddr, ONLD_REG_START_ADDR1(0x0), nSUbn++);

        while((ONLD_READ_DIRECT(nBAddr, ONLD_REG_WR_PROTECT_STAT(0x0)) & (UINT16)UNLOCKED_STAT) != (UINT16)UNLOCKED_STAT)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Unlocked Status Error\r\n")));
         	AD_Idle(INT_MODE);
        }
    }
#endif

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));
    return ONLD_SUCCESS;
#endif /* #if !defined(XSR_NBL2) */
}

static UINT32
_LockBlock (UINT32  nDev, UINT8  *pBufI, UINT32 nLenI)
{
    UINT32    nBAddr;
    UINT32    nSbn;
    UINT32    nBlks;
    UINT32    nCAddr;

    nSbn  = *(UINT32*)pBufI;
    nBlks = *((UINT32*)(pBufI+4));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    for (; nBlks > 0; nBlks--, nSbn++)
    {
        /* IOBE bit of pONENANDReg->MEM_CFG should be enabled for M10 execution. */

        /* Clear INT */
        OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

        /* Set lock end address and initiate the lock */
        ONLD_WRITE_CMD(nDev, nBAddr, nSbn, 0, 0x0B);

        /* wait until command is completed */
        while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
        {
		 /* Wait until device ready */
		 	AD_Idle(INT_MODE);
        }

        ONLD_WRITE_DIRECT(nBAddr, ONLD_REG_START_ADDR1(0x0), nSbn);

        while((ONLD_READ_DIRECT(nBAddr, ONLD_REG_WR_PROTECT_STAT(0x0)) & (UINT16)LOCKED_STAT) != (UINT16)LOCKED_STAT)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Write Protection Error\r\n")));
         	AD_Idle(INT_MODE);
        }
    }
    return ONLD_SUCCESS;
}

static UINT32
_LockTigBlock (UINT32  nDev, UINT8  *pBufI, UINT32 nLenI)
{
    UINT32 nBAddr;
    UINT32 nSbn;
    UINT32 nBlks;
    UINT32 nCAddr;

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    nSbn  = *(UINT32*)pBufI;
    nBlks = *((UINT32*)(pBufI+4));

    /* Clear INT */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /* Set lock-tight start address */
	ONLD_WRITE_CMD(nDev, nBAddr, nSbn, 0, 0x0C);

    /* Set lock-tight end address and initiate the lock */
	ONLD_WRITE_CMD(nDev, nBAddr, nSbn + nBlks - 1, 0, 0x0D);

    // wait until command is completed
    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
    {
	 /* Wait until device ready */
	 	AD_Idle(INT_MODE);
    }

    ONLD_WRITE_DIRECT(nBAddr, ONLD_REG_START_ADDR1(0x0), nSbn);

    while((ONLD_READ_DIRECT(nBAddr, ONLD_REG_WR_PROTECT_STAT(0x0)) & (UINT16)LOCKED_STAT) != (UINT16)LOCKED_STAT)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Write Protection Error\r\n")));
     	AD_Idle(INT_MODE);
    }

    return ONLD_SUCCESS;
}

static VOID
_InitDevInfo(UINT32 nDev, UINT16 *pMID, UINT16 *pDID, UINT16 *pVID)
{
    if(pDID == NULL || pVID == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR] (_InitDevInfo) Illeagal Access\r\n")));
        return;
    }

    astONLDInfo[nDev].nBufSelSft       = 2;
    astONLDInfo[nDev].nSctSelSft       = 3;
    astONLDInfo[nDev].nBlkSelSft       = 8;
    astONLDInfo[nDev].nPgSelSft        = 2;
    astONLDInfo[nDev].nDDPSelSft       = 6;
    astONLDInfo[nDev].nFPASelSft       = 0;
    astONLDInfo[nDev].nFSAMask         = 3;
    astONLDInfo[nDev].nSctsPerPG       = 4;
    ONLD_CMD_LOCK_NAND                 = 0x002A;
    ONLD_CMD_LOCK_TIGHT                = 0x002C;
    astONLDInfo[nDev].MRead            = _M01_MRead;

    astONLDInfo[nDev].nDID                  = *pDID;

    if (((*pDID & 0x00F0) == CHIP_256M) || ((*pDID & 0x00F0) == CHIP_128M))
    {
        astONLDInfo[nDev].nBufSelSft    = 1;
        astONLDInfo[nDev].nSctSelSft    = 1;
        astONLDInfo[nDev].nBlkSelSft    = 7;
        astONLDInfo[nDev].nPgSelSft     = 1;
        astONLDInfo[nDev].nFPASelSft    = 1;

        astONLDInfo[nDev].nFSAMask   = 1;
        astONLDInfo[nDev].nSctsPerPG = 2;

        if ((*pVID & 0x0F00) == M_DIE)
        {
            ONLD_CMD_LOCK_NAND       = 0x002B;
            ONLD_CMD_LOCK_TIGHT      = 0x002D;
        }
        astONLDInfo[nDev].MRead      = _M01_MRead;
    }

    if ((*pDID & 0x0003) == 0x01)
    {
        astONLDInfo[nDev].MRead  = _M01_MRead;
    }

    if ((*pDID & 0x00F0) == CHIP_512M)
    {
        /* Cache-read is enabled only for 512Mb B-die 1.8V */
        if ((*pVID & 0x0F00) == M_DIE || (*pVID & 0x0F00) == A_DIE)
        {
            astONLDInfo[nDev].MRead  = _M01_MRead;
        }
    }
    if ((*pDID & 0x00F0) == CHIP_1G)
    {
        if ((*pVID & 0x0F00) == M_DIE)
        {
            astONLDInfo[nDev].MRead  = _M01_MRead;
        }
    }

    if ((*pDID & 0x00F0) == CHIP_2G)
    {
        astONLDInfo[nDev].nDDPSelSft    = 5;

        if ((*pDID & 0x0008) == DDP_CHIP)
        {
            if ((*pVID & 0x0F00) == M_DIE)
            {
                astONLDInfo[nDev].MRead  = _M01_MRead;
            }
        }
    }
    else if ((*pDID & 0x00F0) == CHIP_4G)
    {
        astONLDInfo[nDev].nDDPSelSft    = 4;
    }

	else if ((*pDID & 0x00F0) == CHIP_8G)
	{
		astONLDInfo[nDev].nDDPSelSft    = 3;
	}

    if (astONLDDev[nDev].pstDevSpec->nLKFlag == BLK_LK)
    {
        astONLDInfo[nDev].SetRWArea = _SetRWBlock;
#ifdef LOCK_TIGHT
        astONLDInfo[nDev].LockTight = _LockTigBlock;
#else
        /* for erase refresh, lock-tight is disabled */
        astONLDInfo[nDev].LockTight = _LockBlock;
#endif
    }
    else if (astONLDDev[nDev].pstDevSpec->nLKFlag == RNG_LK)
    {
        astONLDInfo[nDev].SetRWArea = _SetRWRange;
        astONLDInfo[nDev].LockTight = _LockTigRange;
    }
    else
    {
        astONLDInfo[nDev].SetRWArea = NULL;
        astONLDInfo[nDev].LockTight = NULL;
    }
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _StrictChk                                                           */
/* DESCRIPTION                                                               */
/*      This function performs strict checks                                 */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPsn                                                                 */
/*          Physical Sector Number                                           */
/*      nScts                                                                */
/*          Number of Sectors                                                */
/* RETURN VALUES                                                             */
/*      TRUE32                                                               */
/*          Strict Check OK                                                  */
/*      FALSE32                                                              */
/*          Strcit Check Failure                                             */
/* NOTES                                                                     */
/*      Device Power Flag and Open Flag Check                                */
/*      Each of Out of Bound Check                                           */
/*                                                                           */
/*****************************************************************************/
#if defined(STRICT_CHK)
static BOOL32
_StrictChk(UINT32 nDev, UINT32 nPsn, UINT32 nScts)
{
    UINT32 nPbn;

    /* Check Device Number */
    if (nDev > MAX_SUPPORT_DEVS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Invalid Device Number (nDev = %d)\r\n"),
                       nDev));
        return (FALSE32);
    }

    nPbn = GET_PBN(nDev, nPsn);

    /* Check Device Open Flag */
    if (CHK_DEV_OPEN_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Device is not opened\r\n")));
        return (FALSE32);
    }

    /* Check Device Power Flag */
    if (CHK_DEV_PWR_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Dev PwrFlg is not set\r\n")));
        return (FALSE32);
    }

    /* Check Block Out of Bound                      */
    /* if nBlk > GET_NUM_OF_BLKS then it is an error.*/
    if (CHK_BLK_OOB(nDev, nPbn))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Pbn is Out of Bound\r\n")));
        return (FALSE32);
    }

    /* Check Sector Out of Bound                                            */
    /* if ((nPsn & 0x03) + nSct) > Sectors per Page(=4) then it is an error */
    if (CHK_SCT_OOB(nDev, nPsn, nScts))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Psn + Scts is Out of Bound\r\n")));
        return (FALSE32);
    }

    /* Everything is OK, then TRUE32 can be returned */
    return (TRUE32);
}
#endif  /* #if defined(STRICT_CHK) */


#if defined(DEFERRED_CHK)
static INT32
_ChkPrevWriteResult(UINT32 nDev)
{
    UINT32 nBAddr;
    UINT32 nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++_ChkPrevWriteResult()\r\n")));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    // ---------------------------------------------------- //
    // Wait until clearing ONGO and PROG bit                //
    // in OneNAND native Controller Status Register         //
    // ---------------------------------------------------- //

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_WRITE) != (UINT16)CPEND_WRITE)
    {
        /* Wait until device ready */
        /* Write Protection Error Check */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Protection Error\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

            return (ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT);
        }
        AD_Idle(INT_MODE);
    }

    /* Write Operation Error Check */
    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_PGM_FAIL) == BIT_PGM_FAIL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

        return (ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT);
    }

    pstPrevOpInfo[nDev]->ePreOp = NONE;

    ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --_ChkPrevWriteResult(nRet = ONLD_SUCCESS)\r\n")));
    return ONLD_SUCCESS;
}


static INT32
_ChkPrevEraseResult(UINT32 nDev)
{
    UINT32 nBAddr;
    UINT32 nCAddr;
    UINT32 nPbn;

    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++_ChkPrevEraseResult()\r\n")));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    nPbn = GET_PBN(nDev, pstPrevOpInfo[nDev]->nPsn);

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_ERASE) != (UINT16)CPEND_ERASE)
    {
        /* Wait until device ready */
        /* Write Protection Error Check */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Previous Write Protection Error\r\n")));
            return (ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT);
        }
        AD_Idle(INT_MODE);
    }

    /* Erase Operation Error Check */
    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_ERS_FAIL) == BIT_ERS_FAIL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Erase Error(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkPrevEraseResult()\r\n")));

        return (ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --_ChkPrevEraseResult(nRet = ONLD_SUCCESS)\r\n")));

    return ONLD_SUCCESS;

}

static INT32
_ChkPrevMEraseResult(UINT32 nDev)
{
    UINT32 nBAddr;
    UINT32 nCAddr;
    UINT32 nPbn;

    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++_ChkPrevMEraseResult()\r\n")));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    nPbn = GET_PBN(nDev, pstPrevOpInfo[nDev]->nPsn);

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_ERASE) != (UINT16)CPEND_ERASE)
    {
        /* Wait until device ready */
        /* Write Protection Error Check */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  Previous Write Protection Error\r\n")));
            return (ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT);
        }
        AD_Idle(INT_MODE);
    }

    /* Erase Operation Error Check */
    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_ERS_FAIL) == BIT_ERS_FAIL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Erase Error(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkPrevMEraseResult()\r\n")));

        return (ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT);
    }

    if (ONLD_EraseVerify(nDev,
                         pstPrevOpInfo[nDev]->pstPreMEArg,
                         pstPrevOpInfo[nDev]->nFlag) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  Previous MErase Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --_ChkPrevMEraseResult()\r\n")));
        return (ONLD_MERASE_ERROR | ONLD_PREV_OP_RESULT);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --_ChkPrevMEraseResult(nRet = ONLD_SUCCESS)\r\n")));

    return ONLD_SUCCESS;

}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _ChkPrevOpResult                                                     */
/* DESCRIPTION                                                               */
/*      This function checks the result of previous opreation                */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/* RETURN VALUES                                                             */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure                                           */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/

static INT32
_ChkPrevOpResult(UINT32 nDev)
{

    UINT32 nBAddr, nPbn;
    INT32  nRet = ONLD_SUCCESS;
    OpType ePrevOp;
    UINT32 nCAddr;

    if (GET_PREV_OP_TYPE(nDev) == NONE)
    {
        return (nRet);
    }

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    /* Wait until the device is ready */
    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_INT_ACT) != (UINT16)BIT_INT_ACT);

    nPbn = GET_PBN(nDev, pstPrevOpInfo[nDev]->nPsn);
    ePrevOp = GET_PREV_OP_TYPE(nDev);

    /* Clear the previous operation type */
    pstPrevOpInfo[nDev]->ePreOp = NONE;

    switch(ePrevOp)
    {
        case WRITE:
        case CPBACK:
            nRet = _ChkPrevWriteResult(nDev);
            break;

        case ERASE:
            nRet = _ChkPrevEraseResult(nDev);
            break;

        case MERASE:
            nRet = _ChkPrevMEraseResult(nDev);
            break;
        case READ:
#if 0
		    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_READ) != (UINT16)CPEND_READ)
		    {
		        /* Wait until device ready */
		        AD_Idle(INT_MODE);
		    }
#endif
            break;

        default:
#if 0
		    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
		    {
		        /* Wait until device ready */
		        AD_Idle(INT_MODE);
		    }
#endif
            break;
    }

    /* Acknowledge interrupts */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    return (nRet);
}
#endif /* #if defined(DEFERRED_CHK) */


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      InitializeONDController                                              */
/* DESCRIPTION                                                               */
/*      This function gets Manufacturer ID, Device ID and Version ID of      */
/*      NAND flash device.                                                   */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pMID                                                                 */
/*          Manufacturer ID                                                  */
/*      pDID                                                                 */
/*          Device ID                                                        */
/*      pVID                                                                 */
/*          Version ID                                                       */
/* RETURN VALUES                                                             */
/*      None                                                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL bInitController = FALSE;

static VOID
InitializeONDController(UINT32 nDev, UINT16 *pMID, UINT16 *pDID, UINT16 *pVID)
{
    UINT32  nBAddr;
    UINT32  nCAddr;

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONDController_Init() nBAddr = 0x%x\r\n"), nBAddr));

    if ((pMID == NULL) || (pDID == NULL) || (pVID == NULL))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (InitializeONDController) NULL Pointer Error\r\n")));
		ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --InitializeONDController()\r\n")));
        return;
    }

	// drive strength for GPIO port toward OneNAND
	//pGPIOReg->MEM0DRVCON = 0x55000411;
	//*((volatile UINT32 *)0xb2b081d0) = 0xaaaaaaaa;  // physical:0x7f0081d0

	// setting clock for OneNAND controller
	//Disable Watchdog Timer reset logic in OneNandd controller
	OCLD_REG_FLASH_AUX_CNTRL(nCAddr)=0x1;

	// setting delay toward NAND Flash in case of Async mode
	OCLD_REG_ACC_CLOCK(nCAddr) = 7; //(35*(S3C6410_HCLK/1000))/1000000 + 1;

    *pMID = (UINT16)OCLD_REG_MANUFACT_ID(nCAddr);
    *pDID = (UINT16)OCLD_REG_DEVICE_ID(nCAddr);
    *pVID = (UINT16)OCLD_REG_FLASH_VER_ID(nCAddr);

	ONENAND_SetMemSpace(nDev, pDID);
	ONENAND_EnableECCCorrection(nDev, TRUE32);
	ONENAND_EnableIOBE(nDev, TRUE32);
	ONENAND_SetBurstLatency(nDev, eOND_LATENCY4);
	ONENAND_SetSyncMode(nDev, eOND_SYNC_BURST16);
	ONENAND_EnableIntPin(nDev, USE_STATUS_REG);
	ONENAND_EnableSpareTransfer(nDev, TSRF_DATA_SPARE);

	OCLD_REG_INT_ERR_MASK(nCAddr) = (UINT16)0x3fff;

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONDController_Init()\r\n")));
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _ChkReadDisturb                                                      */
/* DESCRIPTION                                                               */
/*      This function checks if ECC 1-bit error by read disturbance occurs   */
/* PARAMETERS                                                                */
/*      nScts                                                                */
/*          Number of sectors                                                */
/*      nEccStat                                                             */
/*          Value of Ecc status register                                     */
/*      aEccRes                                                              */
/*          Array of the value of Ecc result register                        */
/*      pMBuf                                                                */
/*          Buffer for main area                                             */
/*      pSBuf                                                                */
/*          Buffer for spare area                                            */
/* RETURN VALUES                                                             */
/*      TRUE32                                                               */
/*          If 1-bit error by read disturbance happens                       */
/*      FALSE32                                                              */
/*          If not                                                           */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
BOOL32
_ChkReadDisturb(UINT32 nScts, UINT16 nEccStat,
                UINT16* pEccRes, UINT8* pMBuf, UINT8* pSBuf)
{
    UINT16  nEccMMask;
    UINT16  nEccSMask;
    UINT16  nEccRes;
    UINT8   nEccPosWord;
    UINT8   nEccPosBit;
    UINT16  nEccPosByte;
    UINT8   nChkMask;
    UINT8   nIdx;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++_ChkReadDisturb() \r\n")));

    if (nEccStat == 0)
    {
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkReadDisturb()\r\n")));
        return FALSE32;
    }

    for (nIdx = 0;nIdx < nScts; nIdx++)
    {
        nEccMMask = (0x04 << (nIdx * 4));
        nEccSMask = (0x01 << (nIdx * 4));

        if (pMBuf != NULL)
        {
            if (nEccStat & nEccMMask)
            {
                /* 1-Bit error */
                nEccRes = pEccRes[nIdx * 2];

                nEccPosWord = (UINT8)((nEccRes & 0x0ff0)>>4);
                nEccPosBit = (UINT8)(nEccRes & 0xf);

                nEccPosByte = nIdx * 512 + nEccPosWord * 2;

                if (nEccPosBit >= 8)
                {
                    nEccPosByte++;
                    nEccPosBit -= 8;
                }

                nChkMask = 0x01 << nEccPosBit;

                if ((pMBuf[nEccPosByte] & nChkMask) != 0)
                {
                    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkReadDisturb()\r\n")));
                    return TRUE32;
                }
            }
        }

        if (pSBuf != NULL)
        {
            if (nEccStat & nEccSMask)
            {
                /* 1-Bit error */
                nEccRes = pEccRes[nIdx * 2 + 1];

                nEccPosWord = (UINT8)((nEccRes & 0x30)>>4);
                nEccPosBit = (UINT8)(nEccRes & 0xf);

                nEccPosByte = nIdx * 16 + (nEccPosWord + 1)* 2;

                if (nEccPosBit >= 8)
                {
                    nEccPosByte++;
                    nEccPosBit -= 8;
                }

                nChkMask = 0x01 << nEccPosBit;

                if ((pSBuf[nEccPosByte] & nChkMask) != 0)
                {
                    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkReadDisturb()\r\n")));
                    return TRUE32;
                }
            }
        }
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --_ChkReadDisturb()\r\n")));

    return FALSE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_FlushOp                                                         */
/* DESCRIPTION                                                               */
/*      This function waits until the completion of the last physical        */
/*      operation                                                            */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Flush operation success                                          */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure                                           */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_FlushOp(UINT32 nDev)
{
#if defined(DEFERRED_CHK)
    INT32  nRes;
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_FlushOp() nDev = %d\r\n"), nDev));

#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (FLUSH_OP)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (FLUSH_OP)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_FlushOp()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_FlushOp()\r\n")));

    return (ONLD_SUCCESS);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_GetDevInfo                                                      */
/* DESCRIPTION                                                               */
/*      This function reports device information to upper layer.             */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pstFILDev                                                            */
/*          Data structure of device information which is required from FIL  */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Access                                                   */
/*      ONLD_SUCCESS                                                         */
/*          Success                                                          */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_GetDevInfo(UINT32 nDev, LLDSpec *pstFILDev)
{
#ifdef LLD_OTP_UID_GETDEVINFO	// OTP Read
	BOOL bValidUID;
	INT32 nErr;
	UINT32 nIndex;
	UINT32 nPsn;
	UINT8 pMBuf[XSR_SECTOR_SIZE];
	UINT8 pSBuf[XSR_SPARE_SIZE];
#endif	// LLD_OTP_UID_GETDEVINFO

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_GetDevInfo() nDev = %d\r\n"), nDev));

#if defined(STRICT_CHK)
    if (astONLDDev[nDev].pstDevSpec == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_DEV_INFO)Illeagal Access\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetDevInfo()\r\n")));
        return(ONLD_ILLEGAL_ACCESS);
    }
    if (pstFILDev == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_DEV_INFO)Illeagal Access\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetDevInfo()\r\n")));
        return(ONLD_ILLEGAL_ACCESS);
    }
#endif  /* #if defined(STRICT_CHK) */

    pstFILDev->nMCode       = (UINT8)astONLDDev[nDev].pstDevSpec->nMID;
    pstFILDev->nDCode       = (UINT8)astONLDInfo[nDev].nDID;
    pstFILDev->nNumOfBlks   = (UINT16)astONLDInfo[nDev].nNumOfBlks;
    pstFILDev->nPgsPerBlk   = (UINT16)GET_PGS_PER_BLK(nDev);
    pstFILDev->nBlksInRsv   = (UINT16)astONLDInfo[nDev].nBlksInRsv;

    pstFILDev->nSctsPerPg   = (UINT8)GET_SCTS_PER_PG(nDev);
    pstFILDev->nNumOfPlane  = (UINT8)astONLDDev[nDev].pstDevSpec->nNumOfPlanes;

    pstFILDev->nBadPos      = (UINT8)astONLDDev[nDev].pstDevSpec->nBadPos;
    pstFILDev->nLsnPos      = (UINT8)astONLDDev[nDev].pstDevSpec->nLsnPos;
    pstFILDev->nEccPos      = (UINT8)astONLDDev[nDev].pstDevSpec->nEccPos;
    pstFILDev->nBWidth      = (UINT8)astONLDDev[nDev].pstDevSpec->nBWidth;
    pstFILDev->nMEFlag      = (UINT8)astONLDDev[nDev].pstDevSpec->nMEFlag;

    pstFILDev->aUID[0]      = 0xEC;
    pstFILDev->aUID[1]      = 0x00;
    pstFILDev->aUID[2]      = 0xC5;
    pstFILDev->aUID[3]      = 0x5F;
    pstFILDev->aUID[4]      = 0x3D;
    pstFILDev->aUID[5]      = 0x12;
    pstFILDev->aUID[6]      = 0x34;
    pstFILDev->aUID[7]      = 0x37;
    pstFILDev->aUID[8]      = 0x34;
    pstFILDev->aUID[9]      = 0x33;
    pstFILDev->aUID[10]     = 0x30;
    pstFILDev->aUID[11]     = 0x30;
    pstFILDev->aUID[12]     = 0x03;
    pstFILDev->aUID[13]     = 0x03;
    pstFILDev->aUID[14]     = 0x06;
    pstFILDev->aUID[15]     = 0x02;

    pstFILDev->nTrTime      = (UINT32)astONLDDev[nDev].pstDevSpec->nTrTime;
    pstFILDev->nTwTime      = (UINT32)astONLDDev[nDev].pstDevSpec->nTwTime;
    pstFILDev->nTeTime      = (UINT32)astONLDDev[nDev].pstDevSpec->nTeTime;
    pstFILDev->nTfTime      = (UINT32)astONLDDev[nDev].pstDevSpec->nTfTime;

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetDevInfo()\r\n")));

    return (ONLD_SUCCESS);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      onld_init                                                            */
/* DESCRIPTION                                                               */
/*      This function initializes ONLD                                       */
/*          - constructioin and initialization of internal data structure    */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pParm                                                                */
/*          Device Parmameter Pointer                                        */
/* RETURN VALUES                                                             */
/*      ONLD_INIT_FAILURE                                                    */
/*          ONLD initialization faiure                                       */
/*      ONLD_SUCCESS                                                         */
/*          ONLD initialization success                                      */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Init(VOID *pParm)
{
    UINT32        nCnt;
    UINT32        nIdx;
    UINT32        nVol;
    static BOOL32 nInitFlg = FALSE32;
    vol_param  *pstParm = (vol_param*)pParm;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_Init()\r\n")));

    ONLD_RTL_PRINT((TEXT("[ONLD] =============== OCLD local mode ===============\r\n")));
    ONLD_RTL_PRINT((TEXT("[ONLD] LLD VERSION : %s\r\n"),LLD_VERSION));
#ifdef ONENANDCON_CACHEREAD
    ONLD_RTL_PRINT((TEXT("[ONLD] OND_TRANS_MODE = 0x%x with    CacheRead\r\n"), OND_TRANS_MODE));
#else
    ONLD_RTL_PRINT((TEXT("[ONLD] OND_TRANS_MODE = 0x%x without CacheRead\r\n"), OND_TRANS_MODE));
#endif  // ONENANDCON_CACHEREAD

#ifdef ONENANDCON_READ_FULL_SECTOR
    ONLD_RTL_PRINT((TEXT("[ONLD] Enable  ONENANDCON_READ_FULL_SECTOR\r\n")));
#else
    ONLD_RTL_PRINT((TEXT("[ONLD] Disable ONENANDCON_READ_FULL_SECTOR\r\n")));
#endif  // ONENANDCON_READ_FULL_SECTOR

#ifdef ONENANDCON_WRITE_FULL_SECTOR
    ONLD_RTL_PRINT((TEXT("[ONLD] Enable  ONENANDCON_WRITE_FULL_SECTOR\r\n")));
#else
    ONLD_RTL_PRINT((TEXT("[ONLD] Disable ONENANDCON_WRITE_FULL_SECTOR\r\n")));
#endif  // ONENANDCON_WRITE_FULL_SECTOR

#ifdef DEFERRED_CHK
    ONLD_RTL_PRINT((TEXT("[ONLD] Enable  DEFERRED_CHK\r\n")));
#else
    ONLD_RTL_PRINT((TEXT("[ONLD] Disable DEFERRED_CHK\r\n")));
#endif  // DEFERRED_CHK
    ONLD_RTL_PRINT((TEXT("[ONLD] ===============================================\r\n")));

    if (nInitFlg == TRUE32)
    {
        ONLD_DBG_PRINT((TEXT("[ONLD:MSG]  already initialized\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Init()\r\n")));

        return(ONLD_SUCCESS);
    }

    for (nCnt = 0; nCnt < MAX_SUPPORT_DEVS; nCnt++)
    {
        pstPrevOpInfo[nCnt]         = (PrevOpInfo *) NULL;
        astONLDDev[nCnt].bOpen      = FALSE32;
        astONLDDev[nCnt].bPower     = FALSE32;
        astONLDDev[nCnt].pstDevSpec = NULL;
    }

    for (nVol = 0; nVol < XSR_MAX_VOL; nVol++)
    {
        for (nIdx = 0; nIdx < (XSR_MAX_DEV / XSR_MAX_VOL); nIdx++)
        {
            nCnt = nVol * (XSR_MAX_DEV / XSR_MAX_VOL) + nIdx;

            ONLD_DBG_PRINT((TEXT("[ONLD:MSG]  nVol = %d nDev : %d Base Address : 0x%x\r\n"), nVol, nCnt, pstParm[nVol].nBaseAddr[nIdx]));

            if (pstParm[nVol].nBaseAddr[nIdx] != NOT_MAPPED) {
                astONLDDev[nCnt].BaseAddr   = AD_Pa2Va_256M(pstParm[nVol].nBaseAddr[nIdx]);
                astONLDDev[nCnt].BaseAddr1   = AD_Pa2Va(pstParm[nVol].nBaseAddr1[nIdx]);
			}
            else
                astONLDDev[nCnt].BaseAddr   = NOT_MAPPED;
        }
    }

    nInitFlg = TRUE32;

    /* Wait until reset is completed */
//    while ((OCLD_REG_INT_ERR_STAT(ONENANDC0_VIRTADDR) & BIT_RST_CMP) != (UINT16)BIT_RST_CMP);
//    while ((OCLD_REG_INT_ERR_STAT(ONENANDC1_VIRTADDR) & BIT_RST_CMP) != (UINT16)BIT_RST_CMP);

#if (OND_TRANS_MODE == OND_DMA)
	{
#ifndef USE_SDMA
		g_pDMAC0Reg = (S3C6410_DMAC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_DMA0, FALSE);
#else
		g_pDMAC0Reg = (S3C6410_DMAC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SDMA0, FALSE);
#endif
		g_pSYSCONReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

		if (g_pDMAC0Reg == NULL || g_pSYSCONReg == NULL)
		{
			ONLD_ERR_PRINT((TEXT("[ONLD:ERR] Initialize Error : NULL pointer parameter\r\n")));
			return(ONLD_INIT_FAILURE);
		}
	}

	DMAInitialize();
	MapDMABuffers();
#endif  // OND_TRANS_MODE

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Init()\r\n")));

    return(ONLD_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      onld_open                                                            */
/* DESCRIPTION                                                               */
/*      This function opnes ONLD device driver                               */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/* RETURN VALUES                                                             */
/*      ONLD_OPEN_FAILURE                                                    */
/*          ONLD Open faiure                                                 */
/*      ONLD_SUCCESS                                                         */
/*          ONLD Open success                                                */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Open(UINT32 nDev)
{
    INT32    nCnt;
    static UINT16   nMID, nDID, nVID;
#if defined(DEFERRED_CHK)
    LLDMEArg *pstMEArg;
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_Open() nDev = %d\r\n"), nDev));

    SET_PWR_FLAG(nDev);

	if (bInitController == FALSE)
	{
		InitializeONDController(nDev, &nMID, &nDID, &nVID);
		bInitController = TRUE;
	}
/*
    ONLD_RTL_PRINT((TEXT("[ONLD : MSG]  nDev = %02d\r\n"), nDev));
    ONLD_RTL_PRINT((TEXT("[ONLD : MSG]  nMID = 0x%04x\r\n"), nMID));
    ONLD_RTL_PRINT((TEXT("[ONLD : MSG]  nDID = 0x%04x\r\n"), nDID));
    ONLD_RTL_PRINT((TEXT("[ONLD : MSG]  nVID = 0x%04x\r\n"), nVID));
*/
    for (nCnt = 0; astNandSpec[nCnt].nMID != 0; nCnt++)
    {
        if (nDID == astNandSpec[nCnt].nDID)
        {
            if ((nVID >> 8) == astNandSpec[nCnt].nGEN)
            {
                astONLDDev[nDev].bOpen      = TRUE32;
                astONLDDev[nDev].pstDevSpec = &astNandSpec[nCnt];
                astONLDInfo[nDev].nNumOfBlks = astONLDDev[nDev].pstDevSpec->nNumOfBlks;
                astONLDInfo[nDev].nBlksInRsv = astONLDDev[nDev].pstDevSpec->nBlksInRsv;
                break;
            }
            else
            {
                continue;
            }
        }
    }

    _InitDevInfo(nDev, &nMID, &nDID, &nVID);

    if (astONLDDev[nDev].bOpen != TRUE32)
    {
        CLEAR_PWR_FLAG(nDev);

        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (OPEN)Unknown Device\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Open()\r\n")));

        return (ONLD_OPEN_FAILURE);
    }

    if (pstPrevOpInfo[nDev] == (PrevOpInfo*)NULL)
    {
        pstPrevOpInfo[nDev] = (PrevOpInfo*)MALLOC(sizeof(PrevOpInfo));

        if (pstPrevOpInfo[nDev] == (PrevOpInfo*)NULL)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (OPEN)Memory Alloc Error\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_Open()\r\n")));

            return (ONLD_OPEN_FAILURE);
        }
    }

#if defined(DEFERRED_CHK)
    pstPrevOpInfo[nDev]->ePreOp      = NONE;
    pstPrevOpInfo[nDev]->nCmd        = 0;
    pstPrevOpInfo[nDev]->nPsn        = 0;
    pstPrevOpInfo[nDev]->nScts       = 0;
    pstPrevOpInfo[nDev]->nFlag       = 0;
    pstPrevOpInfo[nDev]->pstPreMEArg = (LLDMEArg*)MALLOC(sizeof(LLDMEArg));

    pstMEArg = pstPrevOpInfo[nDev]->pstPreMEArg;
    if (pstMEArg == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (OPEN)Memory Alloc Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_Open()\r\n")));
        return (ONLD_OPEN_FAILURE);
    }

    MEMSET(pstMEArg, 0x00, sizeof(LLDMEArg));
    pstMEArg->pstMEList  = (LLDMEList*)MALLOC(sizeof(LLDMEList) * XSR_MAX_MEBLK);
    if (pstMEArg->pstMEList == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (OPEN)Memory Alloc Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_Open()\r\n")));
        return (ONLD_OPEN_FAILURE);
    }

    pstMEArg->nNumOfMList = (UINT16)0x00;
    pstMEArg->nBitMapErr  = (UINT16)0x00;
    pstMEArg->bFlag       = FALSE32;
#endif /* #if defined(DEFERRED_CHK) */

    pstPrevOpInfo[nDev]->nBufSel = DATA_BUF0;
/*
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nNumOfBlks   = %d\r\n"), astONLDDev[nDev].pstDevSpec->nNumOfBlks));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nNumOfPlanes = %d\r\n"), astONLDDev[nDev].pstDevSpec->nNumOfPlanes));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nBlksInRsv   = %d\r\n"), astONLDDev[nDev].pstDevSpec->nBlksInRsv));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nBadPos      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nBadPos));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nLsnPos      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nLsnPos));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nECCPos      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nEccPos));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nBWidth      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nBWidth));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nMEFlag      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nMEFlag));
    LLD_RTL_PRINT((TEXT("[ONLD:MSG]  nLKFlag      = %d\r\n"), astONLDDev[nDev].pstDevSpec->nLKFlag));

#if (OND_TRANS_MODE == OND_POL)
	ONLD_RTL_PRINT((TEXT("[INFO] OneNand Translation Mode = OND_POL\r\n")));
#elif(OND_TRANS_MODE == OND_DMA)
	ONLD_RTL_PRINT((TEXT("[INFO] OneNand Translation Mode = OND_DMA\r\n")));
#elif(OND_TRANS_MODE == OND_LDM)
	ONLD_RTL_PRINT((TEXT("[INFO] OneNand Translation Mode = OND_LDM\r\n")));
#endif
*/
    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Open()\r\n")));

    return (ONLD_SUCCESS);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_Close                                                           */
/* DESCRIPTION                                                               */
/*      This function closes ONLD device driver                              */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Close Success                                                    */
/*      ONLD_CLOSE_FAILURE                                                   */
/*          Close Failure                                                    */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure                                           */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Close(UINT32 nDev)
{
    INT32   nRes = ONLD_SUCCESS;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_Close() nDev = %d\r\n"), nDev));

    if ((nRes =ONLD_FlushOp(nDev)) != ONLD_SUCCESS)
    {
        return nRes;
    }

    if (pstPrevOpInfo[nDev] != (PrevOpInfo*)NULL)
    {
#if defined(DEFERRED_CHK)
        FREE(pstPrevOpInfo[nDev]->pstPreMEArg->pstMEList);
        pstPrevOpInfo[nDev]->pstPreMEArg->pstMEList = (LLDMEList*)NULL;
        FREE(pstPrevOpInfo[nDev]->pstPreMEArg);
        pstPrevOpInfo[nDev]->pstPreMEArg = (LLDMEArg*)NULL;
#endif /* #if defined(DEFERRED_CHK) */
        FREE(pstPrevOpInfo[nDev]);
        pstPrevOpInfo[nDev] = (PrevOpInfo*)NULL;
    }

    astONLDDev[nDev].bOpen      = FALSE32;
    astONLDDev[nDev].bPower     = FALSE32;
    astONLDDev[nDev].pstDevSpec = NULL;

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Close()\r\n")));

    return (ONLD_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_Read                                                            */
/* DESCRIPTION                                                               */
/*      This function read data from NAND flash                              */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPsn                                                                 */
/*          Sector index to be read                                          */
/*      nScts                                                                */
/*          The number of sectors to be read                                 */
/*      pMBuf                                                                */
/*          Memory buffer for main array of NAND flash                       */
/*      pSBuf                                                                */
/*          Memory buffer for spare array of NAND flash                      */
/*      nFlag                                                                */
/*          Operation options such as ECC_ON, OFF                            */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Data Read success                                                */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Read Operation Try                                       */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_READ_ERROR | ECC result code                                    */
/*          Data intigrity fault. (1 or 2bit ECC error)                      */
/* NOTES                                                                     */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Read(UINT32 nDev, UINT32 nPsn, UINT32 nScts, UINT8 *pMBuf, UINT8 *pSBuf,
          UINT32 nFlag)
{
    SGL stSGL;

    stSGL.nElements = 1;
    stSGL.stSGLEntry[0].pBuf = pMBuf;
    stSGL.stSGLEntry[0].nSectors = nScts;
    stSGL.stSGLEntry[0].nFlag = SGL_ENTRY_USER_DATA;

    return ONLD_MRead(nDev, nPsn, nScts, &stSGL, pSBuf, nFlag);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_Write                                                           */
/* DESCRIPTION                                                               */
/*      This function write data into NAND flash                             */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPsn                                                                 */
/*          Sector index to be written                                       */
/*      nScts                                                                */
/*          The number of sectors to be written                              */
/*      pMBuf                                                                */
/*          Memory buffer for main array of NAND flash                       */
/*      pSBuf                                                                */
/*          Memory buffer for spare array of NAND flash                      */
/*      nFlag                                                                */
/*          Operation options such as ECC_ON, OFF                            */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Write Operation Try                                      */
/*      ONLD_WR_PROTECT_ERROR                                                */
/*          Attempt to write at Locked Area                                  */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WRITE_ERROR                                                     */
/*          Data write failure                                               */
/* NOTES                                                                     */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Write(UINT32 nDev, UINT32 nPsn, UINT32 nScts, UINT8 *pMBuf, UINT8 *pSBuf,
           UINT32 nFlag)
{
    SGL stSGL;
    UINT32 nErrPsn;

    stSGL.nElements = 1;
    stSGL.stSGLEntry[0].pBuf = pMBuf;
    stSGL.stSGLEntry[0].nSectors = nScts;
    stSGL.stSGLEntry[0].nFlag = SGL_ENTRY_USER_DATA;

    return ONLD_MWrite(nDev, nPsn, nScts, &stSGL, pSBuf, nFlag, &nErrPsn);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_Erase                                                           */
/* DESCRIPTION                                                               */
/*      This function erases a block                                         */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPbn                                                                 */
/*          Physical Block Number                                            */
/*      nFlag                                                                */
/*          Operation Option (Sync/Async)                                    */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Erase Operation Try                                      */
/*      ONLD_WR_PROTECT_ERROR                                                */
/*          Attempt to erase at Locked Area                                  */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WRITE_ERROROR                                                   */
/*          Data write failure                                               */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/* NOTES                                                                     */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_Erase(UINT32 nDev, UINT32 nPbn, UINT32 nFlag)
{
#if !defined(XSR_NBL2)
    UINT32  nBAddr;
#if defined(DEFERRED_CHK)
    UINT32   nRes;
#endif /* #if defined(DEFERRED_CHK) */
	UINT32  nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_Erase() nDev = %d, nPbn = %d, nFlag = 0x%x\r\n"),
                    nDev, nPbn, nFlag));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

#if defined(STRICT_CHK)
    /*  Check Device Number */
    if (nDev > MAX_SUPPORT_DEVS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Invalid Device Number (nDev = %d)\r\n"), nDev));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Open Flag */
    if (CHK_DEV_OPEN_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Device is not opened (nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Power Flag */
    if (CHK_DEV_PWR_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Dev PwrFlg is not set(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Block Out of Bound                      */
    /* if nPbn > GET_NUM_OF_BLKS then it is an error.*/
    if (CHK_BLK_OOB(nDev, nPbn))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Pbn is Out of Bound(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }
#endif  /* #if defined(STRICT_CHK) */

#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

    /* Clear INT_STAT */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /*-----------------------------------------------------------------------*/
    /* ONLD Erase CMD is issued                                              */
    /*-----------------------------------------------------------------------*/
	ONLD_WRITE_CMD(nDev, nBAddr, nPbn, 0, 0x03);

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
    {
        /* Wait until device ready */
        /* Write Protection Error Check */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Write Protection Error(nPbn = %d)\r\n"), nPbn));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

            return (ONLD_WR_PROTECT_ERROR);
        }
        AD_Idle(INT_MODE);
    }

    /* Erase Operation Error Check */
    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_ERS_FAIL) == BIT_ERS_FAIL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Erase Error(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ERASE_ERROR);
    }

    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_ERS_CMP) != BIT_ERS_CMP)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE)Erase Fail(nPbn = %d)\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

        return (ONLD_ERASE_ERROR);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Erase()\r\n")));

#endif /* #if !defined(XSR_NBL2) */

    return (ONLD_SUCCESS);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_CopyBack                                                        */
/* DESCRIPTION                                                               */
/*      This function read data from sector array of NAND flash              */
/* PARAMETERS                                                                */
/*      nSrcBlkNum                                                           */
/*          Source block index to be read                                    */
/*      nSrcPgNum                                                            */
/*          Source page index to be read                                     */
/*      nDestBlkNum                                                          */
/*          Destination block index to be written                            */
/*      nDestPgNum                                                           */
/*          Destination page index to be written                             */
/*      nCmdRdType                                                           */
/*          Read command type                                                */
/*      iCmdWrType                                                           */
/*          Write command type                                               */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Data Read success                                                */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Read Operation Try                                       */
/*      ONLD_READ_ERROR | ECC result code                                    */
/*          Data intigrity fault. (1 or 2bit ECC error)                      */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR                                                */
/*          Attempt to write at Locked Area                                  */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_CopyBack(UINT32 nDev, CpBkArg *pstCpArg, UINT32 nFlag)
{
#if !defined(XSR_NBL2)
    UINT8               aBABuf[4];
    UINT8              *pRIBuf;
    INT32               nSctNum, nOffset;
    UINT16              nEccRes;
    UINT16              nPbn;
    volatile UINT32    *pDevBuf;
    UINT32              nCnt;
    UINT32             *pONLDMBuf, *pONLDSBuf;
    UINT32              nBAddr;
    UINT32              nRISize;
    UINT32              nRsd;
    UINT32              nIdx;
    UINT32              nRet;

#if defined(DEFERRED_CHK)
    INT32       nRes;
#endif /* #if defined(DEFERRED_CHK) */
    RndInArg   *pstRIArg;

    UINT32      nCAddr;
	UINT32      temp = 0xffffffff;
	UINT8   cTmpBuf[ONDC_MAINSPARE];

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_CopyBack() nDev = %d, nFlag = 0x%x\r\n"),
                    nDev, nFlag));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

#if defined(STRICT_CHK)
    if (_StrictChk(nDev, pstCpArg->nSrcSn, GET_SCTS_PER_PG(nDev)) != TRUE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)Strict Check Error(nSrcSn)\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    if (_StrictChk(nDev, pstCpArg->nDstSn, GET_SCTS_PER_PG(nDev)) != TRUE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)Strict Check Error(nDstSn)\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }
    if (pstCpArg == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)Illeagal Access\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
        return(ONLD_ILLEGAL_ACCESS);
    }
#endif  /* #if defined(STRICT_CHK) */

#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  From SrcPsn = %d to DstPsn = %d\r\n"),
                    pstCpArg->nSrcSn, pstCpArg->nDstSn));

    /*-----------------------------------------------------------------------*/
    /* Step 1. READ                                                          */
    /*                                                                       */
    /*  a. Read command will be issued                                       */
    /*  b. Data will be loaded into OneNAND internal Buffer from NAND Cell   */
    /*  c. No memory transfer operation between Host memory and OneNAND      */
    /*     internal buffer                                                   */
    /*  d. ECC Read operation is possible                                    */
    /*-----------------------------------------------------------------------*/

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  | Step1. ReadOp Start    |\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));

    nPbn = GET_PBN(nDev, pstCpArg->nSrcSn);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  Source nPbn = %d\r\n"), nPbn));

	if ((nRet = _SetECC(nBAddr, nCAddr, nFlag)) != ONLD_SUCCESS)
	{
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  %S [%S:%d]\r\n"), __FILE__, __FUNCTION__, __LINE__));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
        return nRet;
	}

    /* INT Stat Reg Clear */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;
	OCLD_REG_ECC_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /*---------------------------------*/
    /* Load page into map00 XIP buffer */
    /*---------------------------------*/
    ONLD_WRITE_CMD(nDev, nBAddr, nPbn, (pstCpArg->nSrcSn)>>2, 0x10);

    pONLDMBuf = (UINT32 *)(ONLD_BUFFER_DEF(nBAddr, 0) + OCLD_MAP00_MB_ADDR);
    pONLDSBuf = (UINT32 *)(ONLD_BUFFER_DEF(nBAddr, 0) + OCLD_MAP00_SB_ADDR);

    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_LOAD) != (UINT16)CPEND_LOAD)
    {
        /* Wait until device ready */
        AD_Idle(INT_MODE);
    }

    /*-----------------------------------------------------------------------*/
    /* Note                                                                  */
    /* If ECC 1bit error occurs, there is no an error return                 */
    /* Only 2bit ECC error case, there is an error return                    */
    /*-----------------------------------------------------------------------*/
    if ( ((nFlag & ONLD_FLAG_ECC_ON) != 0) && ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LD_FAIL_ECC_ERR) == (UINT16)BIT_LD_FAIL_ECC_ERR) )
    {
		nEccRes = OCLD_REG_ECC_ERR_STAT(nCAddr);
        if ( (nEccRes & ONLD_READ_UERROR_A) != 0 )
        {
			#if 1
			if (pstCpArg->nSrcSn % 256)
			{
				ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)ECC Error (Pbn:%d, Psn:%d, ErrCode:0x04%x)\r\n"), nPbn, pstCpArg->nSrcSn, nEccRes));
			}
			#else
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)ECC Error Occurs during CPBack (Pbn : %d, Psn : %d)\r\n"), nPbn, pstCpArg->nSrcSn));
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)+----------M3S3M2S2M1S1M0S0-+\r\n")));
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)|ECC Error Code    : 0x04%x |\r\n"), nEccRes));
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CPBACK)+---------------------------+\r\n")));
            ONLD_ERR_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
			#endif

    		nPbn = GET_PBN(nDev, pstCpArg->nDstSn);

			MEMSET(cTmpBuf, 0xff, ONDC_MAINSPARE);

            MEMCPY((UINT8*)((UINT32)pONLDMBuf), (void *)cTmpBuf, ONDC_MAINSPARE);

		    /* INT Stat Reg Clear */
		    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

		    ONLD_WRITE_CMD(nDev, nBAddr, nPbn, (pstCpArg->nDstSn)>>2, 0x11);

		    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_WRITE) != (UINT16)CPEND_WRITE)
		    {
		        /* Wait until device ready */
		        /* Write Protection Error Check */
		        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
		        {
		            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Protection Error\r\n")));
		            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

		            return (ONLD_READ_ERROR | ONLD_WR_PROTECT_ERROR);
		        }
		        AD_Idle(INT_MODE);
		    }

		    /* Write Operation Error Check */
		    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_PGM_FAIL) == BIT_PGM_FAIL)
		    {
		        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Error\r\n")));
		        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

		        return (ONLD_READ_ERROR | nEccRes);
		    }

            return (ONLD_READ_ERROR | nEccRes);
        }
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  | Step1. ReadOp Finish   |\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));

    /*-----------------------------------------------------------------------*/
    /* Step 2. DATA RANDOM IN                                                */
    /*                                                                       */
    /*  a. Data is loaded into OneNAND internal buffer from Host memory      */
    /*-----------------------------------------------------------------------*/

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  | Step2. RandomIn Start  |\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));

    pstRIArg = pstCpArg->pstRndInArg;

    for (nCnt = 0 ; nCnt < pstCpArg->nRndInCnt ; nCnt++)
    {
        nSctNum = (pstRIArg[nCnt].nOffset / 1024);
        nOffset = (pstRIArg[nCnt].nOffset % 1024);
        pRIBuf  = pstRIArg[nCnt].pBuf;
        nRISize = pstRIArg[nCnt].nNumOfBytes;

        if (nOffset < ONLD_MAIN_SIZE)
        {
            /*--------------------------------------------------------------*/
            /* Byte Align Problem                                           */
            /*                                                              */
            /* OneNAND Controller is a 32-bit BusWidth Device.              */
            /*--------------------------------------------------------------*/
            nRsd = nOffset % sizeof(UINT32);

            if (nRsd != 0)
            {
                pDevBuf = (volatile UINT32 *)((UINT32)pONLDMBuf +
                                              (nSctNum * ONLD_MAIN_SIZE) +
                                              nOffset - nRsd);

                *(volatile UINT32 *)aBABuf = *pDevBuf;

                while (nRsd < sizeof(UINT32))
                {
                    if (nRISize > 0) nRISize--;
                    else break;

                    aBABuf[nRsd++] = *pRIBuf++;
                    nOffset++;
                }
                *pDevBuf = *(volatile UINT32 *)aBABuf;

                if (nRISize <= 0)
                {
                    continue;
                }
            }

            nRsd = nRISize % sizeof(UINT32);

            if (nRsd != 0)
            {
                pDevBuf = (volatile UINT32 *)((UINT32)pONLDMBuf +
                                              (nSctNum * ONLD_MAIN_SIZE) +
                                              nOffset + nRISize - nRsd);

                *(volatile UINT32 *)aBABuf = *pDevBuf;

                for (nIdx = 0; nIdx < nRsd; nIdx++, nRISize--)
                {
                    aBABuf[nIdx] = *(pRIBuf + nRISize - nRsd + nIdx);
                }

                *pDevBuf = *(volatile UINT32 *)aBABuf;

                if (nRISize <= 0)
                {
                    continue;
                }
            }

            MEMCPY((UINT8*)((UINT32)pONLDMBuf + (nSctNum * ONLD_MAIN_SIZE) + nOffset),
                   pRIBuf,
                   nRISize);
        }
        else
        {
            /*--------------------------------------------------------------*/
            /* Byte Align Problem                                           */
            /*                                                              */
            /* OneNAND Controller is a 32-bit BusWidth Device.              */
            /*--------------------------------------------------------------*/
            nRsd = nOffset % sizeof(UINT32);

            if (nRsd != 0)
            {
                pDevBuf = (volatile UINT32 *)((UINT32)pONLDSBuf +
                                              (nSctNum * ONLD_SPARE_SIZE) +
                                              (nOffset - ONLD_MAIN_SIZE) - nRsd);

                *(volatile UINT32 *)aBABuf = *pDevBuf;

                while (nRsd < sizeof(UINT32))
                {
                    if (nRISize > 0) nRISize--;
                    else break;

                    aBABuf[nRsd++] = *pRIBuf++;
                    nOffset++;
                }
                *pDevBuf = *(volatile UINT32 *)aBABuf;

                if (nRISize <= 0)
                {
                    continue;
                }
            }

            nRsd = nRISize % sizeof(UINT32);

            if (nRsd != 0)
            {
                pDevBuf = (volatile UINT32 *)((UINT32)pONLDSBuf +
                                              (nSctNum * ONLD_SPARE_SIZE) +
                                              (nOffset - ONLD_MAIN_SIZE) +
                                              nRISize - nRsd);

                *(volatile UINT32 *)aBABuf = *pDevBuf;

                for (nIdx = 0; nIdx < nRsd; nIdx++, nRISize--)
                {
                    aBABuf[nIdx] = *(pRIBuf + nRISize - nRsd + nIdx);
                }

                *pDevBuf = *(volatile UINT32 *)aBABuf;

                if (nRISize <= 0)
                {
                    continue;
                }
            }

            MEMCPY((UINT8*)((UINT32)pONLDSBuf + (nSctNum * ONLD_SPARE_SIZE) + (nOffset - ONLD_MAIN_SIZE)),
                   pRIBuf,
                   nRISize);
        }
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  | Step2. RandomIn Finish |\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));

    /*-----------------------------------------------------------------------*/
    /* Step 3. WRITE                                                         */
    /*                                                                       */
    /*  a. No memory transfer operation between Host memory and OneNAND      */
    /*     internal buffer                                                   */
    /*  b. Write command will be issued                                      */
    /*  c. ECC Read operation is possible                                    */
    /*-----------------------------------------------------------------------*/

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  | Step3. WriteOp Start   |\r\n"), nBAddr));
    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  +------------------------+\r\n"), nBAddr));

    nPbn = GET_PBN(nDev, pstCpArg->nDstSn);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  Destination nPbn = %d\r\n"), nPbn));

    /* INT Stat Reg Clear */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

	if ((nRet = _SetECC(nBAddr, nCAddr, nFlag)) != ONLD_SUCCESS)
	{
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  %S [%S:%d]\r\n"), __FILE__, __FUNCTION__, __LINE__));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
        return nRet;
	}

    ONLD_WRITE_CMD(nDev, nBAddr, nPbn, (pstCpArg->nDstSn)>>2, 0x11);


    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_WRITE) != (UINT16)CPEND_WRITE)
    {
        /* Wait until device ready */
        /* Write Protection Error Check */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Protection Error\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

            return (ONLD_WR_PROTECT_ERROR);
        }
        AD_Idle(INT_MODE);
    }

    /* Write Operation Error Check */
    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_PGM_FAIL) == BIT_PGM_FAIL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

        return (ONLD_WRITE_ERROR);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_CopyBack()\r\n")));
#endif /* #if !defined(XSR_NBL2) */

    return (ONLD_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_ChkInitBadBlk                                                   */
/* DESCRIPTION                                                               */
/*      This function checks a block if it is an initial bad block           */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPbn                                                                 */
/*          Physical Block Number                                            */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Erase Operation Try                                      */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_INIT_BADBLOCK                                                   */
/*          In case of an Initial Bad Block                                  */
/*      ONLD_INIT_GOODBLOCK                                                  */
/*          In case of a Good Block                                          */
/* NOTES                                                                     */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_ChkInitBadBlk(UINT32 nDev, UINT32 nPbn)
{
    INT32   nRet;
    UINT16  aSpare[ONLD_SPARE_SIZE / 2]={0};
    UINT32  nPsn;

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_ChkInitBadBlk() nDev = %d, nPbn = %d\r\n"),
                    nDev, nPbn));

    nPsn = nPbn * GET_SCTS_PER_BLK(nDev);

    nRet = ONLD_Read(nDev,                      /* Device Number                 */
                     nPsn,                      /* Physical Sector Number        */
                     (UINT32)1,                 /* Number of Sectors to be read  */
                     (UINT8*)NULL,              /* Buffer pointer for Main area  */
                     (UINT8*)aSpare,            /* Buffer pointer for Spare area */
                     (UINT32)ONLD_FLAG_ECC_OFF);/* flag                          */

    if ((nRet != ONLD_SUCCESS) && ((nRet & (0xFFFF0000)) != (UINT32)ONLD_READ_ERROR))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CHK_INIT_BB)Previous Operation Error\r\n")));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (CHK_INIT_BB)Return Val = 0x%08x\r\n"), nRet));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_ChkInitBadBlk()\r\n")));

        return (nRet);
    }

    if (aSpare[0] != (UINT16)VALID_BLK_MARK)
    {
        ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nPbn = %d is an Initial BadBlk\r\n"), nPbn));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_ChkInitBadBlk()\r\n")));

        return (ONLD_INIT_BADBLOCK);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nPbn = %d is a GoodBlk\r\n"), nPbn));
    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_ChkInitBadBlk()\r\n")));

    return (ONLD_INIT_GOODBLOCK);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_SetRWArea                                                       */
/* DESCRIPTION                                                               */
/*      This function sets RW Area                                           */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nSUbn                                                                */
/*          Start block index of unlocked area                               */
/*      nUBlks                                                               */
/*          Total blocks of unlocked area                                    */
/*          If nNumOfUBs = 0, device is locked.                              */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          In case of illegal access                                        */
/*      ONLD_SUCCESS                                                         */
/*          Lock/Unlock Operation success                                    */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/* NOTES                                                                     */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_SetRWArea(UINT32 nDev, UINT32 nSUbn, UINT32 nUBlks)
{
#if !defined(XSR_NBL2)

    #if defined(DEFERRED_CHK)
        INT32   nRes;
    #endif /* #if defined(DEFERRED_CHK) */

        ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_SetRWArea() nDev = %d, nSUbn = %d, nUBlks = 0x%x\r\n"),
                        nDev, nSUbn, nUBlks));

    #if defined(STRICT_CHK)
        /*  Check Device Number */
        if (nDev > MAX_SUPPORT_DEVS)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)Invalid Device Number (nDev = %d)\r\n"), nDev));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

            return (ONLD_ILLEGAL_ACCESS);
        }

        /* Check Device Open Flag */
        if (CHK_DEV_OPEN_FLAG(nDev) == FALSE32)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)Device is not opened\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

            return (ONLD_ILLEGAL_ACCESS);
        }

        /* Check Device Power Flag */
        if (CHK_DEV_PWR_FLAG(nDev) == FALSE32)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)Dev PwrFlg is not set\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

            return (ONLD_ILLEGAL_ACCESS);
        }

        /* Check Block Out of Bound                       */
        /* if nSUbn > GET_NUM_OF_BLKS then it is an error.*/
        if (CHK_BLK_OOB(nDev, nSUbn))
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)nSUbn = %d is Out of Bound\r\n"), nSUbn));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

            return (ONLD_ILLEGAL_ACCESS);
        }

        /* Check Block Out of Bound                                  */
        /* if (nSUbn + nUBlks) > GET_NUM_OF_BLKS then it is an error.*/
        if (CHK_BLK_OOB(nDev, (nSUbn + nUBlks)))
        {
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)(nSUbn + nUBlks - 1) = %d is Out of Bound\r\n"),
                            nSUbn + nUBlks - 1));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

            return (ONLD_ILLEGAL_ACCESS);
        }
    #endif  /* #if defined(STRICT_CHK) */

    #if defined(DEFERRED_CHK)
        if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
        {
            pstPrevOpInfo[nDev]->ePreOp = NONE;
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)Previous Operation Error 0x%08x\r\n"),
                            nRes));
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (SET_RW)Dev = %d, Psn = %d, Scts = %d"),
                            nDev,
                            pstPrevOpInfo[nDev]->nPsn,
                            pstPrevOpInfo[nDev]->nScts));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));
            return nRes;
        }
    #endif /* #if defined(DEFERRED_CHK) */

    if (astONLDInfo[nDev].SetRWArea != NULL)
    {
        return astONLDInfo[nDev].SetRWArea(nDev, nSUbn, nUBlks);
    }
    else
    {
        return (ONLD_ILLEGAL_ACCESS);
    }

#endif /* #if !defined(XSR_NBL2) */

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_SetRWArea()\r\n")));

    return (ONLD_SUCCESS);
}

//#define PREVOP_FULL_SECTOR_READ
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_GetPrevOpData                                                   */
/* DESCRIPTION                                                               */
/*      This function copys data of previous write operation into a given    */
/*      buffer                                                               */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pMBuf                                                                */
/*          Memory buffer for main array of NAND flash                       */
/*      pSBuf                                                                */
/*          Memory buffer for spare array of NAND flash                      */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Previous Write Data copy success                                 */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          In case of illegal access                                        */
/* NOTES                                                                     */
/*      This function can be called for the block management purpose         */
/*      under previous write error case.                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_GetPrevOpData(UINT32 nDev, UINT8 *pMBuf, UINT8 *pSBuf)
{
#if !defined(XSR_NBL2)
#if defined(DEFERRED_CHK)
    UINT32  nBAddr;
	UINT32  nCAddr;
    UINT32  nPbn;
#ifdef PREVOP_FULL_SECTOR_READ
	UINT8   cTmpBuf[ONDC_MAINSPARE];
#else
    UINT32 *pONLDMBuf, *pONLDSBuf;
#endif  // PREVOP_FULL_SECTOR_READ
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_GetPrevOpData() nDev = %d\r\n"),
                    nDev));

#if defined(DEFERRED_CHK)
    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);
#endif /* #if defined(DEFERRED_CHK) */

#if defined(STRICT_CHK)
    /*  Check Device Number */
    if (nDev > MAX_SUPPORT_DEVS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_PREV_DAT)Invalid Device Number (nDev = %d)\r\n"),
                        nDev));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetPrevOpData()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Open Flag */
    if (CHK_DEV_OPEN_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_PREV_DAT)Device is not opened\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetPrevOpData()\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Power Flag */
    if (CHK_DEV_PWR_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_PREV_DAT)Dev PwrFlg is not set\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetPrevOpData()\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }
#endif /* #if defined(STRICT_CHK) */

    if ((pMBuf == NULL) && (pSBuf == NULL))
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (GET_PREV_DAT)Buffer Pointer Error during GetPrevOpData\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetPrevOpData()\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }


#if defined(DEFERRED_CHK)

    /* ONLD_GetPrevOpData does not return previous operation error
       because physical operation must be flushed before entering
       this function.
       Before calling ONLD_GetPrevOpData, actually, the write operation
       was called always */
    nPbn = GET_PBN(nDev, pstPrevOpInfo[nDev]->nPsn);
#ifdef PREVOP_FULL_SECTOR_READ
	OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

	_ReadData1(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, (pstPrevOpInfo[nDev]->nPsn)>>2), cTmpBuf);

	while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
	{
		AD_Idle(INT_MODE);
	}
	OCLD_REG_INT_ERR_STAT(nCAddr) = CPEND_INT;
#else
	OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    pONLDMBuf = (UINT32 *)(ONLD_DIRECT_DEF(nBAddr, GET_ONLD_MBUF_ADDR(nBAddr, pstPrevOpInfo[nDev]->nPsn, GET_CUR_BUF_SEL(nDev))));
    pONLDSBuf = (UINT32 *)(ONLD_DIRECT_DEF(nBAddr, GET_ONLD_SBUF_ADDR(nBAddr, pstPrevOpInfo[nDev]->nPsn, GET_CUR_BUF_SEL(nDev))));
#endif  // PREVOP_FULL_SECTOR_READ

    /*-------------------------------------------------------------------*/
    /* Data is loaded into Memory                                        */
    /*-------------------------------------------------------------------*/
    if (pMBuf != NULL)
    {
        /* Memcopy for main data */
#ifdef PREVOP_FULL_SECTOR_READ
        MEMCPY(pMBuf, &(cTmpBuf[ONLD_MAIN_SIZE*(pstPrevOpInfo[nDev]->nPsn%4)]), (ONLD_MAIN_SIZE * pstPrevOpInfo[nDev]->nScts));
#else
        /* pMBuf should be 2 byte aligned */
        _Read_M11_Data((UINT16 *)pONLDMBuf, (UINT16 *)pMBuf, pstPrevOpInfo[nDev]->nScts, ONLD_MAIN_SIZE);
#endif  // PREVOP_FULL_SECTOR_READ
    }
    if (pSBuf != NULL)
    {
        /* Memcopy for spare data */
#ifdef PREVOP_FULL_SECTOR_READ
        MEMCPY(pSBuf, &(cTmpBuf[ONDC_MAIN+ONLD_SPARE_SIZE*(pstPrevOpInfo[nDev]->nPsn%4)]), (ONLD_SPARE_SIZE * pstPrevOpInfo[nDev]->nScts));
#else
        _Read_M11_Data((UINT16 *)pONLDSBuf, (UINT16 *)pSBuf, pstPrevOpInfo[nDev]->nScts, ONLD_SPARE_SIZE);
#endif  // PREVOP_FULL_SECTOR_READ
    }
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_GetPrevOpData()\r\n")));
#endif /* #if !defined(XSR_NBL2) */
    return (ONLD_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_IOCtl                                                           */
/* DESCRIPTION                                                               */
/*      This function copys data of previous write operation into a given    */
/*      buffer                                                               */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nCode                                                                */
/*          IO Control Command                                               */
/*      pBufI                                                                */
/*          Input Buffer pointer                                             */
/*      nLenI                                                                */
/*          Length of Input Buffer                                           */
/*      pBufO                                                                */
/*          Output Buffer pointer                                            */
/*      nLenO                                                                */
/*          Length of Output Buffer                                          */
/*      pByteRet                                                             */
/*          The number of bytes (length) of Output Buffer as the result      */
/*          of function call                                                 */
/* RETURN VALUES                                                             */
/*      ONLD_IOC_NOT_SUPPORT                                                 */
/*          In case of Command which is not supported                        */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          In case of illegal access                                        */
/*      ONLD_SUCCESS                                                         */
/*          Data of previous write op copy success                           */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/* NOTES                                                                     */
/*      This function can be called for LLD function extension               */
/*                                                                           */
/*      Deferred Check Operation : The method of operation perfromance       */
/*          optimization. The DCOP method is to terminate the function (write*/
/*          or erase) right after command issue. Then it checks the result of*/
/*          operation at next function call.                                 */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_IOCtl(UINT32  nDev,  UINT32 nCode,
           UINT8  *pBufI, UINT32 nLenI,
           UINT8  *pBufO, UINT32 nLenO,
           UINT32 *pByteRet)
{
    INT32   nRet;
    UINT32  nBAddr;
    UINT16  nRegVal;
    UINT32  nCAddr;
	UINT32  nPbn;

#if defined(DEFERRED_CHK)
    INT32   nRes;
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_IOCtrl() nDev = %d, nCode = 0x%08x\r\n"),
                    nDev, nCode));

    if (pByteRet == NULL) /* To avoid refer to 0 Address */
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Illegal Access pByteRet = NULL\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

#if defined(STRICT_CHK)
    /*  Check Device Number */
    if (nDev > MAX_SUPPORT_DEVS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Invalid Device Number (nDev = %d)\r\n"), nDev));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_IOCtl()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Open Flag */
    if (CHK_DEV_OPEN_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Device is not opened\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_IOCtl()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

    /* Check Device Power Flag */
    if (CHK_DEV_PWR_FLAG(nDev) == FALSE32)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Dev PwrFlg is not set\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_IOCtl()\r\n")));

        return (ONLD_ILLEGAL_ACCESS);
    }

#endif  /* #if defined(STRICT_CHK) */


#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_IOCtl()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

    switch (nCode)
    {
        case LLD_IOC_SET_SECURE_LT:
            if (astONLDInfo[nDev].LockTight != NULL)
                astONLDInfo[nDev].LockTight(nDev, pBufI, nLenI);
            else
                return (ONLD_ILLEGAL_ACCESS);

            *pByteRet = (UINT32)0;
            nRet = ONLD_SUCCESS;
            break;

        case LLD_IOC_SET_BLOCK_LOCK:

            if (_LockBlock(nDev, pBufI, nLenI) != ONLD_SUCCESS)
            {
                return (ONLD_ILLEGAL_ACCESS);
            }

            *pByteRet = (UINT32)0;
            nRet = ONLD_SUCCESS;
            break;

        case LLD_IOC_GET_SECURE_STAT:
            if ((pBufO == NULL) || (nLenO < sizeof(UINT16)))
            {
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (IO_CTRL)Illegal Access pBufO = 0x%08x, nLenO = %d\r\n"), pBufO, nLenO));
                return (ONLD_ILLEGAL_ACCESS);
            }

			nPbn = *(UINT32*)pBufI;
            ONLD_WRITE_DIRECT(nBAddr, ONLD_REG_START_ADDR1(0x0), nPbn);
			nRegVal = ONLD_READ_DIRECT(nBAddr, ONLD_REG_WR_PROTECT_STAT(0x0));
            MEMCPY(pBufO, &nRegVal, sizeof(UINT16));
            *pByteRet = sizeof(UINT16);
            nRet = ONLD_SUCCESS;
            break;

        case LLD_IOC_RESET_NAND_DEV:
            OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

            /*-------------------------------------------------------------------*/
            /* ONLD Unlock CMD is issued                                         */
            /*-------------------------------------------------------------------*/
            OCLD_REG_MEM_RESET(nCAddr) = WARMRESET;

            while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_RESET) != (UINT16)CPEND_RESET)
            {
                /* Wait until device ready */
                AD_Idle(INT_MODE);
            }

            *pByteRet = (UINT32)0;
            nRet = ONLD_SUCCESS;
            break;

        default:
            nRet = ONLD_IOC_NOT_SUPPORT;
            break;
    }
    return nRet;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_MErase                                                          */
/* DESCRIPTION                                                               */
/*      This function erases a List of physical blocks                       */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pstMEArg                                                             */
/*          List of Physical Blocks                                          */
/*      nFlag                                                                */
/*          Operation Option (Sync/Async)                                    */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Erase Operation Try                                      */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_MErase(UINT32 nDev, LLDMEArg *pstMEArg, UINT32 nFlag)
{
    UINT32       nBAddr, nFirstchipPbn = 0xffff, nSecondchipPbn = 0xffff;
    UINT16       nCnt;
#if defined(DEFERRED_CHK)
    INT32        nRes;
#endif /* #if defined(DEFERRED_CHK) */
    INT32        nRet = ONLD_SUCCESS;
    LLDMEList    *pstMEPList;
    UINT32       nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++ONLD_MultiBLKErase()\r\n")));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD:MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (MERASE)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (MERASE)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_MultiBLKErase()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

#if defined(STRICT_CHK)
    if (pstMEArg == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (MERASE)Illeagal Access\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_MultiBLKErase()\r\n")));
        return(ONLD_ILLEGAL_ACCESS);
    }
#endif  /* #if defined(STRICT_CHK) */

    /* Clear INT_STAT */
    OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /* Issue ONC multi-block erase commands */
    pstMEPList = pstMEArg->pstMEList;
    for(nCnt = 0; nCnt < pstMEArg->nNumOfMList; nCnt++)
    {
	    /* Issue a final command */
		ONLD_WRITE_CMD(nDev, nBAddr, pstMEPList[nCnt].nMEListPbn, 0, 0x03);

#if !defined(DEFERRED_CHK) || defined(SYNC_MODE)
	    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_ERASE) != (UINT16)CPEND_ERASE)
	    {
	        /* Wait until device ready */
        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == (UINT16)BIT_LOCKED_BLK)
	    {
	        ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (ERASE)Write Protection Error(nPbn = %d)\r\n"), pstMEPList[nCnt].nMEListPbn));
	        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_MultiBLKErase)\r\n")));

	        return (ONLD_WR_PROTECT_ERROR);
	    }
        AD_Idle(INT_MODE);
    }
#endif /* #if defined(DEFERRED_CHK) || defined(SYNC_MODE) */
    }

#if defined(DEFERRED_CHK) && defined(ASYNC_MODE)
    pstPrevOpInfo[nDev]->ePreOp     = MERASE;
    pstPrevOpInfo[nDev]->nFlag      = nFlag;
    pstPrevOpInfo[nDev]->nPsn       = pstMEPList[nCnt].nMEListPbn * GET_SCTS_PER_BLK(nDev);
    MEMCPY(pstPrevOpInfo[nDev]->pstPreMEArg, pstMEArg, sizeof(LLDMEArg));
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_MultiBLKErase()\r\n")));

    return nRet;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_MRead                                                           */
/* DESCRIPTION                                                               */
/*      This function read multiple sectors from NAND flash                  */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPsn                                                                 */
/*          Sector index to be read                                          */
/*      nScts                                                                */
/*          The number of sectors to be read                                 */
/*      pstSGL                                                               */
/*          SGL structure for main array of NAND flash                       */
/*      pSBuf                                                                */
/*          Memory buffer for spare array of NAND flash                      */
/*      nFlg                                                                 */
/*          Operation options such as ECC_ON, OFF                            */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Read Operation Try                                       */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          In case of Deferred Check Operation. Previous Write failure.     */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          In case of Deferred Check Operation. Previous Erase failure.     */
/*      ONLD_READ_ERROR                                                      */
/*          In case of ECC Error Occur.                                      */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_MRead(UINT32 nDev, UINT32 nPsn, UINT32 nScts, SGL *pstSGL, UINT8 *pSBuf,
          UINT32 nFlag)
{

    UINT32  nBAddr;
    UINT32  nRet = ONLD_SUCCESS;
    UINT32  nCAddr;
    UINT32  temp = 0xffffffff;

#if defined(DEFERRED_CHK)
    INT32   nRes;
#endif /* #if defined(DEFERRED_CHK) */

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_MRead() nDev = %d, nPsn = %d, nScts = %d, nFlag = 0x%x\r\n"),
                    nDev, nPsn, nScts, nFlag));

    if (pstSGL == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (READ)Buffer Pointer Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MRead(()\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

#if defined(DEFERRED_CHK)
    if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
    {
        pstPrevOpInfo[nDev]->ePreOp = NONE;
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (READ)Previous Operation Error 0x%08x\r\n"),
                        nRes));
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (READ)Dev = %d, Psn = %d, Scts = %d"),
                        nDev,
                        pstPrevOpInfo[nDev]->nPsn,
                        pstPrevOpInfo[nDev]->nScts));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));
        return nRes;
    }
#endif /* #if defined(DEFERRED_CHK) */

	if ((nRet = _SetECC(nBAddr, nCAddr, nFlag)) != ONLD_SUCCESS)
	{
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  %S [%S:%d]\r\n"), __FILE__, __FUNCTION__, __LINE__));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MRead()\r\n")));
        return nRet;
	}

    if (astONLDInfo[nDev].MRead != NULL)
    {
        nRet = astONLDInfo[nDev].MRead(nDev, nPsn, nScts, pstSGL, pSBuf, nFlag);
    }
    else
    {
        return (ONLD_ILLEGAL_ACCESS);
    }

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MRead()\r\n")));

    return (nRet);
}

static INT32
_M01_MRead(UINT32 nDev, UINT32 nPsn, UINT32 nScts, SGL *pstSGL, UINT8 *pSBuf, UINT32 nFlag)
{
    UINT32  nBAddr;
    UINT32  nSGLIdx = 0;
    UINT32  nSGLSctCnt = 0;

    BOOL32  bLastPage = FALSE32;
    UINT32  nNumOfScts;
    UINT32  nMetaScts = 0;
    UINT32  nLeadPadScts;
    UINT32  nTrailPadScts;
    UINT32  nRemainScts;
    UINT32  nTotPages;

    UINT8  *pMBuf;
    INT32   nRet = ONLD_SUCCESS;

    UINT16  nIntRes      = (UINT16)0x0;
    UINT16  nEccRes      = (UINT16)0x0;
    UINT16  n1bitEccMask = (UINT16)0x0;
    UINT16  n2bitEccMask = (UINT16)0x0;
    UINT16  aEccRes[8];
    UINT8   nIdx;
    UINT8   nCnt;
    UINT8  *pCurMBuf;
    UINT8  *pCurSBuf;

    UINT32  nPbn;
    UINT32  nCAddr;
    BOOL32  bIsOneBitError = FALSE32;

#ifdef ONENANDCON_READ_FULL_SECTOR
#if (OND_TRANS_MODE == OND_DMA)
	UINT8  *cTmpBuf = NULL;

	cTmpBuf = (UINT8 *)(g_nDMABufferVirPage);
	DMASetOneNAND(ONENANDMODESRC, WORD_UNIT, BURST_4);
#else
	UINT8   cTmpBuf[ONDC_MAINSPARE];
#endif  // OND_TRANS_MODE
#endif  // ONENANDCON_READ_FULL_SECTOR

    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++_M01_MRead() nDev = %d, nPsn = %d, nScts = %d, nFlag = 0x%x\r\n"),
                    nDev, nPsn, nScts, nFlag));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    /*----------------*/
    /* Initialization */
    /*----------------*/
    nPbn = GET_PBN(nDev, nPsn);
    nLeadPadScts = nPsn & astONLDInfo[nDev].nFSAMask;
    nTrailPadScts = GET_SCTS_PER_PG(nDev) - ((nScts + nLeadPadScts) & astONLDInfo[nDev].nFSAMask);
    if (nTrailPadScts == GET_SCTS_PER_PG(nDev)) nTrailPadScts = 0;

    nTotPages = GET_PPN(nDev, nScts + nLeadPadScts + nTrailPadScts);
#ifdef ONENANDCON_CACHEREAD
    if (nTotPages > 1)
    {
        /* Setup the pipeline read-ahead */
        ONLD_WRITE_CMD(nDev, nBAddr, nPbn, nPsn>>2, 0x4000 | (UINT8)nTotPages);
    }
#endif  // ONENANDCON_CACHEREAD

    pMBuf = pstSGL->stSGLEntry[nSGLIdx].pBuf;
    nRemainScts = nScts;

    nNumOfScts = GET_SCTS_PER_PG(nDev) - nLeadPadScts;
    if (nNumOfScts > nRemainScts)
    {
        nNumOfScts = nRemainScts;
        bLastPage = TRUE32;
    }

    /* Acknowledge all interrupts */
	OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;
	OCLD_REG_ECC_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

    /*-------------------*/
    /*   Read pages      */
    /*-------------------*/
    while (1)
    {
	    nPbn = GET_PBN(nDev, nPsn);

        /*-------------------*/
        /* Skip meta area    */
        /*-------------------*/
        switch (pstSGL->stSGLEntry[nSGLIdx].nFlag)
        {
        case SGL_ENTRY_META_DATA:
            nMetaScts = pstSGL->stSGLEntry[nSGLIdx].nSectors;
            nPsn += nMetaScts;
            nSGLSctCnt += nMetaScts;
            nNumOfScts -= nMetaScts;
            nRemainScts -= nMetaScts;
            break;

        case SGL_ENTRY_BISCT_INVALID_DATA:
        case SGL_ENTRY_BISCT_VALID_DATA:
        case SGL_ENTRY_USER_DATA:
            break;

        default:
            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  SGL nFlag Error\r\n")));
            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MRead(()\r\n")));
            return (ONLD_ILLEGAL_ACCESS);
        }

        /*-----------------------------*/
        /* Read sectors within a page  */
        /*-----------------------------*/
#ifdef ONENANDCON_READ_FULL_SECTOR
        _ReadData1(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), cTmpBuf, nCAddr, &nEccRes, &nIntRes);
#ifdef ONENANDCON_CACHEREAD
		if ((nRemainScts - nNumOfScts) <= 0)
		{
		    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT ) != (UINT16)CPEND_INT)
		    {
		        /* Wait until device ready */
		        AD_Idle(INT_MODE);
		    }
	    }
#else
	    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_READ) !=  (UINT16)CPEND_READ)
	    {
	        /* Wait until device ready */
	        AD_Idle(INT_MODE);
	    }
#endif  // ONENANDCON_CACHEREAD

		if (pMBuf != NULL)
		{
			MEMCPY(pMBuf, &(cTmpBuf[ONLD_MAIN_SIZE*(nPsn%4)]), ONLD_MAIN_SIZE * nNumOfScts);
			pMBuf += ONLD_MAIN_SIZE * nNumOfScts;
		}

		if (pSBuf != NULL)
		{
			MEMCPY(pSBuf, &(cTmpBuf[ONDC_MAIN+ONLD_SPARE_SIZE*(nPsn%4)]), ONLD_SPARE_SIZE * nNumOfScts);
			pSBuf += ONLD_SPARE_SIZE * nNumOfScts;
		}
#else  // ONENANDCON_READ_FULL_SECTOR
        // Main area
        if (nLeadPadScts > 0)
        {
        	_ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nLeadPadScts, ONLD_MAIN_SIZE);
        }
        if (nMetaScts > 0)
        {
        	_ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nMetaScts, ONLD_MAIN_SIZE);
        }
        if (pMBuf != NULL)
        {
            _ReadMainData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), pMBuf, nNumOfScts);
            pMBuf += ONLD_MAIN_SIZE * nNumOfScts;
        }
        else {
            _ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nNumOfScts, ONLD_MAIN_SIZE);
        }

        if (bLastPage == TRUE32 && nTrailPadScts > 0)
        	_ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nTrailPadScts, ONLD_MAIN_SIZE);

        // Spare area
        if (nLeadPadScts > 0)
        {
            _ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nLeadPadScts, ONLD_SPARE_SIZE);
            nLeadPadScts = 0;
        }
        if (nMetaScts > 0)
        {
            _ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nMetaScts, ONLD_SPARE_SIZE);
            nMetaScts = 0;
        }
        if (pSBuf != NULL)
        {
            _ReadSpareData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), pSBuf, nNumOfScts);
            pSBuf += ONLD_SPARE_SIZE * nNumOfScts;
        }
        else {
            _ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nNumOfScts, ONLD_SPARE_SIZE);
        }

        if (bLastPage == TRUE32 && nTrailPadScts > 0)
        	_ReadEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nPsn>>2), nTrailPadScts, ONLD_SPARE_SIZE);

#ifdef ONENANDCON_CACHEREAD
		if ((nRemainScts - nNumOfScts) <= 0)
		{
		    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT ) !=  (UINT16)CPEND_INT)
		    {
		        /* Wait until device ready */
		        AD_Idle(INT_MODE);
		    }
	    }
#else
	    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_READ) !=  (UINT16)CPEND_READ)
	    {
	        /* Wait until device ready */
	        AD_Idle(INT_MODE);
	    }
#endif  // ONENANDCON_CACHEREAD
#endif  // ONENANDCON_READ_FULL_SECTOR

        /*--------------*/
        /*  Check ECC   */
        /*--------------*/
        if ( ((nFlag & ONLD_FLAG_ECC_ON) != 0) && ((nIntRes & BIT_LD_FAIL_ECC_ERR) == (UINT16)BIT_LD_FAIL_ECC_ERR) )
        {
            // Make ECC mask
                nIdx = (nPsn & astONLDInfo[nDev].nFSAMask) * 4;

                for (n2bitEccMask = 0, nCnt = nNumOfScts; nCnt > 0; nCnt--, nIdx += 4)
                {
                    if (pMBuf != NULL)
                    {
                        n2bitEccMask |= (0x08 << nIdx);
                        n1bitEccMask |= (0x04 << nIdx);
                    }
                    if (pSBuf != NULL)
                    {
                        n2bitEccMask |= (0x02 << nIdx);
                        n1bitEccMask |= (0x01 << nIdx);
                    }
                }

            if (nEccRes & n2bitEccMask)
            {
                /* 2bit ECC error occurs */
				#if 1
				if (nPsn % 256)
				{
					ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ONLD_Read)ECC Error (Pbn:%d, Psn:%d, ErrCode:0x04%x)\r\n"), GET_PBN(nDev, nPsn), nPsn, nEccRes));
				}
				#else
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  ECC Error Occurs during Read (Pbn : %d, Psn : %d)\r\n"), GET_PBN(nDev, nPsn), nPsn));
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  +----------M3S3M2S2M1S1M0S0-+\r\n")));
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  |ECC Error Code    : 0x%04x |\r\n"), nEccRes));
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  +---------------------------+\r\n")));
                ONLD_ERR_PRINT((TEXT("[ONLD : OUT] --ONLD_Read()\r\n")));
				#endif

                nRet = ONLD_READ_ERROR | (nEccRes & n2bitEccMask);
            }
            else if (nEccRes & n1bitEccMask)
            {
                    /* 1bit ECC error occurs */
                    for (nCnt = 0, nIdx = (nPsn & astONLDInfo[nDev].nFSAMask) * 2; nCnt < nNumOfScts; nCnt++)
                        aEccRes[nCnt] = (&ONLD_READ_DIRECT(nBAddr, ONLD_REG_ECC_RSLT_MB0(0x0)))[nIdx + nCnt];

                    if (pMBuf != NULL)
                    {
                        pCurMBuf = pMBuf - nNumOfScts * ONLD_MAIN_SIZE;
                    }
                    else {
                        pCurMBuf = NULL;
                    }
                    if (pSBuf != NULL)
                    {
                        pCurSBuf = pSBuf - nNumOfScts * ONLD_SPARE_SIZE;
                    }
                    else {
                        pCurSBuf = NULL;
                    }

                    if (_ChkReadDisturb(nNumOfScts, nEccRes, aEccRes, pCurMBuf, pCurSBuf)
                        == TRUE32)
                    {
                    		bIsOneBitError = TRUE32;
                    }
                }

            // Ack ECC error
            OCLD_REG_INT_ERR_STAT(nCAddr) &= ~(UINT32)BIT_LD_FAIL_ECC_ERR;
        }

        nSGLSctCnt += nNumOfScts;
        nRemainScts -= nNumOfScts;

        /* Escape condition  */
        if (nRemainScts <= 0) break;

        /* Update variables */
        while (nSGLSctCnt >= pstSGL->stSGLEntry[nSGLIdx].nSectors)
        {
            nSGLSctCnt -= pstSGL->stSGLEntry[nSGLIdx].nSectors;
            nSGLIdx++;
        }

        nPsn += nNumOfScts;

        nNumOfScts = GET_SCTS_PER_PG(nDev);
        if (nNumOfScts > nRemainScts)
        {
            nNumOfScts = nRemainScts;
            bLastPage = TRUE32;
        }

    }

    if (bIsOneBitError == TRUE32 && nRet == ONLD_SUCCESS)
    {
    		nRet = ONLD_READ_DISTURBANCE;
    }
    ONLD_DBG_PRINT((TEXT("\r\n[ONLD : OUT] --_M01_MRead()\r\n")));

    return (nRet);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_MWrite                                                          */
/* DESCRIPTION                                                               */
/*      This function write multiple sectors into NAND flash                 */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      nPsn                                                                 */
/*          Sector index to be written                                       */
/*      nScts                                                                */
/*          The number of sectors to be written                              */
/*      pMBuf                                                                */
/*          Memory buffer for main array of NAND flash                       */
/*      pSBuf                                                                */
/*          Memory buffer for spare array of NAND flash                      */
/*      nFlg                                                                 */
/*          Operation options such as ECC_ON, OFF                            */
/*      pErrPsn                                                              */
/*          Physical sector number which error occur                         */
/* RETURN VALUES                                                             */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Write Operation Try                                      */
/*      ONLD_WR_PROTECT_ERROR                                                */
/*          Attempt to write at Locked Area                                  */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          In case of Deferred Check Operation. Previous Write failure.     */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          In case of Deferred Check Operation. Previous Erase failure.     */
/*      ONLD_WRITE_ERROR                                                     */
/*          Data write failure                                               */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_MWrite(UINT32 nDev, UINT32 nPsn, UINT32 nScts, SGL *pstSGL, UINT8 *pSBuf,
           UINT32 nFlag, UINT32 *pErrPsn)
{
    UINT32  nBAddr;
    UINT32  nPbn;
	UINT32  nWritePsn;
	UINT32  nWriteScts = 0;
	UINT32  nTmpWriteScts;
	UINT8   nBiWriteScts;
	UINT8   nSnapWriteScts;
	UINT32  nRemainScts;
#if defined(DEFERRED_CHK)
    INT32   nRes;
#endif
    UINT8   nSGLIdx;
    UINT8   nSGLCount;
    UINT32  nSctCount = 0;

    UINT8  *pMBuf = NULL;
    UINT8  *pSGLBuf = NULL;
    UINT8   nIdx;
    UINT8   nPreSGLIdx = 0;
    UINT8   nTmpRemainScts = 0;
    INT32   nRet = ONLD_SUCCESS;

    UINT32  nCAddr;

		UINT32  nLeadPadScts;
		UINT32  nTrailPadScts;
		BOOL32  bLastPage = FALSE32;


    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++ONLD_MWrite() nDev = %d, nPsn = %d, nScts = %d, nFlag = 0x%x\r\n"),
                    nDev, nPsn, nScts, nFlag));

    if (pstSGL == NULL || pErrPsn == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Buffer Pointer Error\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MWrite(()\r\n")));
        return (ONLD_ILLEGAL_ACCESS);
    }

#if defined(DEFERRED_CHK)
	if ((nRes = _ChkPrevOpResult(nDev)) != ONLD_SUCCESS)
	{
		ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (WRITE)Previous Operation Error 0x%08x\r\n"),
						nRes));
		ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  (WRITE)Dev = %d, Psn = %d, Scts = %d"),
						nDev,
						pstPrevOpInfo[nDev]->nPsn,
						pstPrevOpInfo[nDev]->nScts));
		ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --ONLD_MWrite()\r\n")));
		return nRes;
	}
#endif /* #if defined(DEFERRED_CHK) */

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

    /* Configure ECC */
	if ((nRet = _SetECC(nBAddr, nCAddr, nFlag)) != ONLD_SUCCESS)
	{
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  %S [%S:%d]\r\n"), __FILE__, __FUNCTION__, __LINE__));
        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MWrite()\r\n")));
        return nRet;
	}

    /* Set user Data Buffer point */
    for (nSGLIdx = 0; nSGLIdx < pstSGL->nElements; nSGLIdx++)
    {
        if (pstSGL->stSGLEntry[nSGLIdx].nFlag == SGL_ENTRY_USER_DATA)
        {
            pMBuf = pstSGL->stSGLEntry[nSGLIdx].pBuf;
            break;
        }
    }

    nSGLIdx     = 0;
	nRemainScts = nScts;
	nWritePsn   = nPsn;

	nLeadPadScts = nPsn & astONLDInfo[nDev].nFSAMask;
	nTrailPadScts = GET_SCTS_PER_PG(nDev) - ((nScts + nLeadPadScts) & astONLDInfo[nDev].nFSAMask);
	if (nTrailPadScts == GET_SCTS_PER_PG(nDev)) nTrailPadScts = 0;

	switch (nWritePsn & astONLDInfo[nDev].nSctSelSft)
    {
        case 0:     nWriteScts  = (astONLDInfo[nDev].nSctsPerPG > nRemainScts) ? nRemainScts : astONLDInfo[nDev].nSctsPerPG;  break;
        case 1:     nWriteScts  = (astONLDInfo[nDev].nSctSelSft > nRemainScts) ? nRemainScts : astONLDInfo[nDev].nSctSelSft;  break;
        case 2:     nWriteScts  = (2 > nRemainScts) ? nRemainScts : 2;  break;
        case 3:     nWriteScts  = (1 > nRemainScts) ? nRemainScts : 1;  break;
    }

    if (nWriteScts == nRemainScts)
    {
        bLastPage = TRUE32;
    }

	while (nRemainScts > 0)
	{
		nPbn = GET_PBN(nDev, nWritePsn);

		ONLD_DBG_PRINT((TEXT("[ONLD : MSG]  nPbn = %d\r\n"), nPbn));

	    /* Acknowledge all interrupts */
		OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

		if (nLeadPadScts > 0)
		{
			_WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nLeadPadScts, ONLD_MAIN_SIZE);
		}

        nTmpWriteScts = nWriteScts;
        nSGLCount = 0;

        if (nSctCount != 0 && pstSGL->stSGLEntry[nSGLIdx].nSectors - nSctCount < nTmpWriteScts)
        {
            nTmpRemainScts = pstSGL->stSGLEntry[nSGLIdx].nSectors - nSctCount;
        }

        do
        {
            nSctCount += 1;
            if (nSctCount >= pstSGL->stSGLEntry[nSGLIdx].nSectors)
            {
                nSctCount = 0;
                nSGLIdx++;
                nSGLCount++;
            }
            else if (nWriteScts == 1)
            {
                nSGLCount++;
            }
        } while(--nWriteScts);

        nBiWriteScts = 0;
        nSnapWriteScts = 0;

        for (nIdx = 0; nIdx < nSGLCount; nIdx++)
        {
            switch(pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nFlag)
            {
            case SGL_ENTRY_BISCT_VALID_DATA:
                pSGLBuf        = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].pBuf;
                nBiWriteScts  += pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                nWriteScts     = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                break;
            case SGL_ENTRY_BISCT_INVALID_DATA:
                pSGLBuf        = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].pBuf;
                nBiWriteScts  += pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                nWriteScts     = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                break;
            case SGL_ENTRY_META_DATA:
                pSGLBuf         = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].pBuf;
                nSnapWriteScts += pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                nWriteScts      = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                break;
            case SGL_ENTRY_USER_DATA:
                if (nTmpRemainScts != 0)
                {
                    nWriteScts      = nTmpRemainScts;
                    nTmpRemainScts = 0;
                }
                else
                {
                    nWriteScts      = nTmpWriteScts - nBiWriteScts - nSnapWriteScts;
                    if (nWriteScts > pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors)
                    {
                        nWriteScts = pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nSectors;
                    }
                }
                break;
            default:
                ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  SGL nFlag Error\r\n")));
                ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MWrite(()\r\n")));
                return (ONLD_ILLEGAL_ACCESS);
            }
            if (pstSGL->stSGLEntry[nPreSGLIdx + nIdx].nFlag != SGL_ENTRY_USER_DATA && pSGLBuf != NULL)
            {
                _WriteMainData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), pSGLBuf, nWriteScts);
            }
            else if (pMBuf != NULL)
            {
    			/* Memcopy for main data */
                _WriteMainData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), pMBuf, nWriteScts);
    			pMBuf     += (ONLD_MAIN_SIZE * nWriteScts);
            }
            else if (pMBuf == NULL)
            {
                _WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nWriteScts, ONLD_MAIN_SIZE);
            }
        }

        if (bLastPage == TRUE32 && nTrailPadScts > 0)
        {
            _WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nTrailPadScts, ONLD_MAIN_SIZE);
        }

        nPreSGLIdx = nSGLIdx;
        nWriteScts = nTmpWriteScts;

		if (nLeadPadScts > 0)
		{
			_WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nLeadPadScts, ONLD_SPARE_SIZE);
			nLeadPadScts = 0;
		}

		if (pSBuf != NULL)
		{
            _WriteSpareData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), pSBuf, nWriteScts);
			pSBuf += (ONLD_SPARE_SIZE * nWriteScts);
		}
		else
		{
			_WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nWriteScts, ONLD_SPARE_SIZE);
		}

        if (bLastPage == TRUE32 && nTrailPadScts > 0)
        {
            _WriteEmptyData(ONLD_ARRAY_DEF(nDev, nBAddr, nPbn, nWritePsn>>2), nTrailPadScts, ONLD_SPARE_SIZE);
        }

        /*--------------*/
        /* Check error  */
        /*--------------*/
	    while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_WRITE) != (UINT16)CPEND_WRITE)
	    {
	        /* Wait until device ready */
	        /* Write Protection Error Check */
	        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_LOCKED_BLK) == BIT_LOCKED_BLK)
	        {
	            ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Protection Error\r\n")));
	            ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

	            return (ONLD_WR_PROTECT_ERROR);
	        }
	        AD_Idle(INT_MODE);
	    }

	    /* Write Operation Error Check */
	    if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_PGM_FAIL) == BIT_PGM_FAIL)
	    {
            *pErrPsn = nWritePsn;
	        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (WRITE)Write Error\r\n")));
	        ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_Write()\r\n")));

	        return (ONLD_WRITE_ERROR);
	    }

		nRemainScts -= nWriteScts;
		nWritePsn   += nWriteScts;
		nWriteScts   = (astONLDInfo[nDev].nSctsPerPG > nRemainScts) ? nRemainScts : astONLDInfo[nDev].nSctsPerPG;

        if (nWriteScts == nRemainScts)
        {
            bLastPage = TRUE32;
        }
        else
        {
            bLastPage = FALSE32;
        }

		pstPrevOpInfo[nDev]->nBufSel    = GET_NXT_BUF_SEL(nDev);
	}

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_MWrite()\r\n")));

    return (ONLD_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_EraseVerify                                                     */
/* DESCRIPTION                                                               */
/*      This function verify a erase operation whether success or not        */
/* PARAMETERS                                                                */
/*      nDev                                                                 */
/*          Physical Device Number (0 ~ 7)                                   */
/*      pstMEArg                                                             */
/*          List of Physical Blocks                                          */
/*      nFlag                                                                */
/*          Operation Option (Sync/Async)                                    */
/* RETURN VALUES                                                             */
/*      ONLD_ERASE_ERROR                                                     */
/*          Erase Failure.                                                   */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_SUCCESS                                                         */
/*          Data write success                                               */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
ONLD_EraseVerify(UINT32 nDev, LLDMEArg *pstMEArg, UINT32 nFlag)
{
    UINT32       nBAddr;
    UINT32       nCnt;
    INT32        nRet = ONLD_SUCCESS;
    LLDMEList    *pstMEPList;
    UINT32       nCAddr;

    ONLD_DBG_PRINT((TEXT("[ONLD: IN] ++Erase Verify nDev = %d\r\n"),nDev));

    nBAddr = GET_DEV_BADDR(nDev);
    nCAddr = GET_DEV_BADDR1(nDev);

    ONLD_DBG_PRINT((TEXT("[ONLD:MSG]  nBAddr = 0x%08x\r\n"), nBAddr));

#if defined(STRICT_CHK)
    if (pstMEArg == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  (ERASE_VERIFY)Illeagal Access\r\n")));
        ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --Erase Verify()\r\n")));
        return(ONLD_ILLEGAL_ACCESS);
    }
#endif  /* #if defined(STRICT_CHK) */

    /* for verify */
    pstMEPList = pstMEArg->pstMEList;

    if((nFlag & ONLD_FLAG_SYNC_MASK) == ONLD_FLAG_SYNC_OP)
    {
        while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_ERASE) != (UINT16)CPEND_ERASE)
        {
            /* Wait until device ready */
            AD_Idle(INT_MODE);
        }
#if defined(DEFERRED_CHK)
        pstPrevOpInfo[nDev]->ePreOp     = NONE;
#endif /* #if defined(DEFERRED_CHK) */
    }


    for(nCnt = 0; nCnt < pstMEArg->nNumOfMList; nCnt++)
    {
        /* Acknowledge all interrupts */
    	OCLD_REG_INT_ERR_STAT(nCAddr) = (UINT16)CINT_CLEAR;

		ONLD_WRITE_CMD(nDev, nBAddr, pstMEPList[nCnt].nMEListPbn, 0, 0x15);

        while ((OCLD_REG_INT_ERR_STAT(nCAddr) & CPEND_INT) != (UINT16)CPEND_INT)
        {
            /* Wait until device ready */
            AD_Idle(INT_MODE);
        }

        if ((OCLD_REG_INT_ERR_STAT(nCAddr) & BIT_ERS_FAIL) == BIT_ERS_FAIL)
        {
            ONLD_ERR_PRINT((TEXT("[ONLD:ERR]  Erase Verifying Error(nPbn = %d)\r\n"), pstMEPList[nCnt].nMEListPbn));
            pstMEArg->nBitMapErr |= (1 << nCnt);
            nRet = LLD_MERASE_ERROR;
        }
    }

    ONLD_DBG_PRINT((TEXT("[ONLD:OUT] --Erase Verify()\r\n")));

    return (nRet);
}

INT32
ONLD_PowerUp(UINT32 nVol, UINT32 n1stVbn, UINT32 nNumOfBlks)
{
    ONLD_DBG_PRINT((TEXT("[ONLD :  IN] ++ONLD_PowerUp()\r\n")));

//	_SetRWBlock(nVol, n1stVbn, nNumOfBlks);

    ONLD_DBG_PRINT((TEXT("[ONLD : OUT] --ONLD_PowerUp()\r\n")));

    return (ONLD_SUCCESS);
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      ONLD_GetPlatformInfo                                                 */
/* DESCRIPTION                                                               */
/*      This function returns platform information                           */
/* PARAMETERS                                                                */
/*      pstLLDPlantformInfo                                                  */
/*          Platform information                                             */
/* RETURN VALUES                                                             */
/*****************************************************************************/
VOID
ONLD_GetPlatformInfo(LLDPlatformInfo* pstLLDPlatformInfo)
{
    //LLD_RTL_PRINT((TEXT("[ONLD: IN] ++ONLD_GetPlatformInfo()\r\n")));

    if (pstLLDPlatformInfo == NULL)
    {
        ONLD_ERR_PRINT((TEXT("[ONLD : ERR]  ONLD_GetPlatformInfo() Error\r\n")));
        return;
    }

#if defined(DENALI_IP)
    /* Warning : Do not change !!!! */
    pstLLDPlatformInfo->nType = 100;
    /* Address of command register in MAP11 */
    pstLLDPlatformInfo->nAddrOfCmdReg = astONLDDev[0].BaseAddr + 0x303c880;
    /* Address of address register */
    pstLLDPlatformInfo->nAddrOfAdrReg = astONLDDev[0].BaseAddr + 0x303c400;
    /* Address of register for reading ID*/
    pstLLDPlatformInfo->nAddrOfReadIDReg = astONLDDev[0].BaseAddr + 0x303c000;
    /* Address of status register */
    pstLLDPlatformInfo->nAddrOfStatusReg = astONLDDev[0].BaseAddr + 0x303c900;
    /* Command of reading Device ID  */
    pstLLDPlatformInfo->nCmdOfReadID = (UINT32)NULL;
    /* Command of read page */
    pstLLDPlatformInfo->nCmdOfReadPage = (UINT32)NULL;
    /* Command of read status */
    pstLLDPlatformInfo->nCmdOfReadStatus = (UINT32)NULL;
    /* Mask value for Ready or Busy status */
    pstLLDPlatformInfo->nMaskOfRnB = (UINT32)NULL;
#else
    /* Warning : Do not change !!!! */
    /* 0 : OneNAND, 1 : read ID from NAND directly, 2 : read ID from register of NAND controller */
    pstLLDPlatformInfo->nType = 0;
    /* Address of command register */
    pstLLDPlatformInfo->nAddrOfCmdReg = astONLDDev[0].BaseAddr + 0x1E440;
    /* Address of address register */
    pstLLDPlatformInfo->nAddrOfAdrReg = astONLDDev[0].BaseAddr + 0x1E200;
    /* Address of register for reading ID*/
    pstLLDPlatformInfo->nAddrOfReadIDReg = astONLDDev[0].BaseAddr + 0x1E000;
    /* Address of status register */
    pstLLDPlatformInfo->nAddrOfStatusReg = astONLDDev[0].BaseAddr + 0x1E480;
    /* Command of reading Device ID  */
    pstLLDPlatformInfo->nCmdOfReadID = NULL;
    /* Command of read page */
    pstLLDPlatformInfo->nCmdOfReadPage = NULL;
    /* Command of read status */
    pstLLDPlatformInfo->nCmdOfReadStatus = NULL;
    /* Mask value for Ready or Busy status */
    pstLLDPlatformInfo->nMaskOfRnB = NULL;
#endif

    //LLD_RTL_PRINT((TEXT("[ONLD: IN] --ONLD_GetPlatformInfo()\r\n")));
    return;
}


/* parameter: */

/*   buf: [IN][OUT] unsigned char buf[2048] */

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      read_dgs_info                                                        */
/* DESCRIPTION                                                               */
/*      This function read dgs info data from NAND flash                     */
/* PARAMETERS                                                                */
/*      buf                                                                  */
/*              [IN][OUT] unsigned char buf[2048]                            */
/* RETURN VALUES                                                             */
/*      ONLD_SUCCESS                                                         */
/*          Data Read success                                                */
/*      ONLD_ILLEGAL_ACCESS                                                  */
/*          Illegal Read Operation Try                                       */
/*      ONLD_WRITE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Write Failure                                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_ERASE_ERROR | ONLD_PREV_OP_RESULT                               */
/*          Previous Erase Failure.                                          */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_WR_PROTECT_ERROR | ONLD_PREV_OP_RESULT                          */
/*          Previous Operation Write Protect Error                           */
/*          DCOP(Deferred Check Operation)                                   */
/*      ONLD_READ_ERROR | ECC result code                                    */
/*          Data intigrity fault. (1 or 2bit ECC error)                      */
/*                                                                           */
/*****************************************************************************/
INT32 read_DGS_info(void *buf)
{
	INT32 ret = ONLD_READ_ERROR;

	if (buf == NULL)
	{
	    BIF_ERR_PRINT((TEXT("[BIF:ERR]  buf is NULL\r\n")));
	    return ONLD_READ_ERROR;
	}

	do {
		
		if (AD_AcquireSM(0) == FALSE32)
		{
		    BIF_ERR_PRINT((TEXT("[BIF:ERR]  Acquiring semaphore is failed\r\n")));
		}
		else
		{
		//      2047th block. 1block=64*2048 bytes, 1sector=512Bytes. a ((2047*64*2048)/512)+(4*5) = 524052
		        ret = ONLD_Read(0, 524052, 4, (UINT8 *)buf, NULL, BML_FLAG_ECC_ON|BML_FLAG_BBM_OFF);
			
			if (ret != ONLD_SUCCESS)
			{
			    BIF_ERR_PRINT((TEXT("[BIF:ERR]   returns error(0x%x)\r\n"), ret ));
			}
		
			do{	
				if (AD_ReleaseSM(0) == FALSE32)
				{
				    BIF_ERR_PRINT((TEXT("[BIF:ERR]  Releasing semaphore is failed\r\n")));
				}
				else
					break;
			}while(1);
			break;
		}	
	} while(1);

	return ret;
}
EXPORT_SYMBOL(read_DGS_info);

