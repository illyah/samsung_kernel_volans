/*
 *  linux/kernel/panic2disk.h
 */

#ifndef __KPL_H__
#define __KPL_H__

#include <asm/mach/arch.h>

#include <linux/module.h>
#include <linux/init.h>

#define PANIC_PARTITON_ID		0x9

#ifndef CONFIG_KPL
#define  KPL_PRINTK(fmt, ... ) do{}while(0);

#define alloc_panic_log_buffer() do{}while(0);

#define free_panic_log_buffer() do{}while(0);
#define panic_log_write() do{}while(0);
#else

//#define NEXT_PANIC_DUMP_POINT(panic_dump) (panic_dump + strlen(panic_dump))
//#define KPL_PRINTK(fmt, args...) sprintf(panic_dump+strlen(panic_dump), fmt, ## args)
//#include "../../../drivers/txsr/Inc/XsrTypes.h"
//#include "../../../drivers/txsr/Inc/LLD.h"
//#include "../../../drivers/txsr/Inc/BML.h"
#include "../../../modules/dpram/XsrTypes.h"
#include "../../../modules/dpram/LLD.h"
#include "../../../modules/dpram/BML.h"

typedef struct KPL_log_fops {
	int   (*KPL_open)(unsigned int dev_id);
	int   (*KPL_close)(unsigned int dev_id);
	int   (*KPL_read)(unsigned int  nVol,  unsigned int  nVsn,  unsigned int nNumOfScts,\
				u8  *pMBuf, u8  *pSBuf, unsigned int nFlag);
	int   (*KPL_write)(unsigned int  nVol,  unsigned int  nVsn,  unsigned int nNumOfScts,\
				u8  *pMBufs, u8  *pSBuf, unsigned int nFlag);
	int   (*KPL_eraseblk)(unsigned int  nVol,  unsigned int  nVbn,  unsigned int nFlag);
	int   (*KPL_ioctl)(unsigned int  nVol, unsigned int nCode, u8 *pBufIn, \
			unsigned int nLenIn , u8 *pBufOut, unsigned int nLenOut, \
			unsigned int *pByteReturned);

	BMLVolSpec	*(*KPL_xsr_get_vol_spec)(UINT32 volume);
	XSRPartI	*(*KPL_xsr_get_part_spec)(UINT32 volume);
} KPL_log_fops_t ; 

void  KPL_PRINTK(const char* fmt, ...);

int   KPL_Open(void);
int   KPL_Close(void);
int   KPL_Read(unsigned int nVol, unsigned int nVsn, unsigned int nNumOfScts, u8 *pMBuf, u8 *pSBuf, 
		unsigned int nFlag);
int   KPL_Write(unsigned int  nVol,  unsigned int  nVsn,  unsigned int nNumOfScts, u8  *pMBufs, u8  *pSBuf, 
		unsigned int nFlag);
int   KPL_EraseBlk(unsigned int  nVol,  unsigned int  nVbn,  unsigned int nFlag);
int   KPL_IOCtl(unsigned int  nVol, unsigned int nCode, u8 *pBufIn, unsigned int nLenIn , u8 *pBufOut, 
		unsigned int nLenOut, unsigned int *pByteReturned);


#define     SPB                     256
#define     SPP                     4
#define 	SIZE_PER_SECTOR			512
#define 	SIZE_PER_PAGE			2048
#define     BML_IOCTL_UNLOCK_WHOLEAREA   XSR_IOCTL_CODE(XSR_MODULE_BML,         \
                                                        0,                      \
                                                        XSR_METHOD_IN_DIRECT,   \
                                                        XSR_WRITE_ACCESS)
void alloc_panic_log_buffer(void);
void free_panic_log_buffer(void);
int panic_log_write(void);

extern char *panic_dump;
#endif
					 
#endif 

