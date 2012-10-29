/****************************************************************************

**

** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED

**

****************************************************************************/

#ifndef __DPRAM_H__
#define __DPRAM_H__

/* 32KB Size */
#define DPRAM_SIZE									0x8000

#define DPRAM_VBASE							VOLANS_VA_DPRAM
/* Memory Address */
#define DPRAM_START_ADDRESS 						0x0000
#define DPRAM_MAGIC_CODE_ADDRESS					(DPRAM_START_ADDRESS)
#define DPRAM_ACCESS_ENABLE_ADDRESS					(DPRAM_START_ADDRESS + 0x0002)

#define DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS		(DPRAM_START_ADDRESS + 0x0004)
#define DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS		(DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS)
#define DPRAM_PDA2PHONE_FORMATTED_TAIL_ADDRESS		(DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 0x0002)
#define DPRAM_PDA2PHONE_FORMATTED_BUFFER_ADDRESS	(DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 0x0004)
#define DPRAM_PDA2PHONE_FORMATTED_BUFFER_SIZE		1020

#define DPRAM_PDA2PHONE_RAW_START_ADDRESS			(DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + (DPRAM_PDA2PHONE_FORMATTED_BUFFER_SIZE+4))
#define DPRAM_PDA2PHONE_RAW_HEAD_ADDRESS			(DPRAM_PDA2PHONE_RAW_START_ADDRESS)
#define DPRAM_PDA2PHONE_RAW_TAIL_ADDRESS			(DPRAM_PDA2PHONE_RAW_START_ADDRESS + 2)
#define DPRAM_PDA2PHONE_RAW_BUFFER_ADDRESS			(DPRAM_PDA2PHONE_RAW_START_ADDRESS + 4)
#define DPRAM_PDA2PHONE_RAW_BUFFER_SIZE				15344
//#define DPRAM_PDA2PHONE_RAW_BUFFER_SIZE			7900

#define DPRAM_PHONE2PDA_FORMATTED_START_ADDRESS		(DPRAM_PDA2PHONE_RAW_START_ADDRESS + (DPRAM_PDA2PHONE_RAW_BUFFER_SIZE+4))
#define DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS		(DPRAM_PHONE2PDA_FORMATTED_START_ADDRESS)
#define DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS		(DPRAM_PHONE2PDA_FORMATTED_START_ADDRESS + 0x0002)
#define DPRAM_PHONE2PDA_FORMATTED_BUFFER_ADDRESS	(DPRAM_PHONE2PDA_FORMATTED_START_ADDRESS + 0x0004)
#define DPRAM_PHONE2PDA_FORMATTED_BUFFER_SIZE		1020

#define DPRAM_PHONE2PDA_RAW_START_ADDRESS			(DPRAM_PHONE2PDA_FORMATTED_START_ADDRESS + (DPRAM_PHONE2PDA_FORMATTED_BUFFER_SIZE+4))
#define DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS			(DPRAM_PHONE2PDA_RAW_START_ADDRESS)
#define DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS			(DPRAM_PHONE2PDA_RAW_START_ADDRESS + 0x0002)
#define DPRAM_PHONE2PDA_RAW_BUFFER_ADDRESS			(DPRAM_PHONE2PDA_RAW_START_ADDRESS + 0x0004)
#define DPRAM_PHONE2PDA_RAW_BUFFER_SIZE				15344
#define DPRAM_PDA2PHONE_DGS_INFO_START_ADDRESS      0x0FFF000
//#define DPRAM_PHONE2PDA_RAW_BUFFER_SIZE			22788


#define DPRAM_INTERRUPT_PORT_SIZE			2
#define MAX_MODEM_IMG_SIZE					0x1000000	//16 * 1024 * 1024
#define MAX_DBL_IMG_SIZE					0x5000		//20 * 1024 

#define DPRAM_SFR							0xFFF800
#define	DPRAM_SMP							DPRAM_SFR				//semaphore
#define DPRAM_MBX_AB						DPRAM_SFR + 0x20		//mailbox a -> b
#define DPRAM_MBX_BA						DPRAM_SFR + 0x40		//mailbox b -> a
#define DPRAM_CHECK_BA						DPRAM_SFR + 0xC0		//check mailbox b -> a read

#define DPRAM_PDA2PHONE_INTERRUPT_ADDRESS	DPRAM_MBX_BA
#define DPRAM_PHONE2PDA_INTERRUPT_ADDRESS	DPRAM_MBX_AB

#define PARTITION_ID_MODEM_IMG				0x09
#define TRUE	1
#define FALSE	0

#define DPRAM_INDICATOR_START						(DPRAM_START_ADDRESS + 0x7FEC)
#define DPRAM_INDICATOR_SIZE						16
#define DPRAM_SILENT_RESET_ADDR					        (DPRAM_INDICATOR_START + 0x0004)	
/*
 * interrupt masks.
 */
#define INT_MASK_VALID					0x0080
#define INT_MASK_COMMAND				0x0040
#define INT_MASK_REQ_ACK_F				0x0020
#define INT_MASK_REQ_ACK_R				0x0010
#define INT_MASK_RES_ACK_F				0x0008
#define INT_MASK_RES_ACK_R				0x0004
#define INT_MASK_SEND_F					0x0002
#define INT_MASK_SEND_R					0x0001

#define INT_MASK_CMD_INIT_START			0x0001
#define INT_MASK_CMD_INIT_END			0x0002
#define INT_MASK_CMD_REQ_ACTIVE			0x0003
#define INT_MASK_CMD_RES_ACTIVE			0x0004
#define INT_MASK_CMD_REQ_TIME_SYNC		0x0005
#define INT_MASK_CMD_PHONE_START 		0x0008
#define INT_MASK_CMD_ERR_DISPLAY		0x0009
#define INT_MASK_CMD_PHONE_DEEP_SLEEP	0x000A
#define INT_MASK_CMD_NV_REBUILDING		0x000B
#define INT_MASK_CMD_EMER_DOWN			0x000C
#define INT_MASK_CMD_SMP_REQ			0x000D
#define INT_MASK_CMD_SMP_REP			0x000E

#define INT_COMMAND(x)					(INT_MASK_VALID | INT_MASK_COMMAND | x)
#define INT_NON_COMMAND(x)				(INT_MASK_VALID | x)

#define FORMATTED_INDEX					0
#define RAW_INDEX						1
#define MAX_INDEX						2

typedef struct
{
    char                    HeaderCode[4];          //magic code 7F AA AD 7E
    char                    ModelName[20];          // Model name in English
    char                    Country[40];            // Country/
    char                    CountryCode[8];         // Country code
    char                    Date[14];               // yyyymmdd
    char                    Manager[24];            // name of developer or manager. 
    char                    SWVer[20];              // Software version
    char                    CheckSum[10];           // check sum english(HEX)
    char                    CRCSum[10];             // CRC sum
    char                    ProductNumber[20];      // product number
    char                    MemoryName[20];         //
    char                    SEC_Code[20];           //
    char                    Reserved[40];           //
}HeaderInformation;
/* ioctl command definitions. */
#define IOC_MZ_MAGIC					('o')
#define DPRAM_PHONE_ON					_IO(IOC_MZ_MAGIC, 0xd0)
#define DPRAM_PHONEIMG_LOAD				_IO(IOC_MZ_MAGIC, 0xd1)
#define DPRAM_NVDATA_LOAD				_IO(IOC_MZ_MAGIC, 0xd2)
#define DPRAM_PHONE_BOOTSTART			_IO(IOC_MZ_MAGIC, 0xd3)
#define DPRAM_PHONE_BOOTTYPE			_IOW(IOC_MZ_MAGIC, 0xd5, unsigned int)
#define DPRAM_PHONE_OFF                 _IO(IOC_MZ_MAGIC, 0xd6)
#define DPRAM_WAKEUP		            _IOW(IOC_MZ_MAGIC, 0xd7, unsigned int)
#define DPRAM_PHONE_RESET		        _IO(IOC_MZ_MAGIC, 0xd8)

#define _ENABLE_RAM_DUMP

#ifdef _ENABLE_RAM_DUMP
#define DPRAM_RAM_DUMP                  _IO(IOC_MZ_MAGIC, 0xd9)  
#define DPRAM_RAM_READ_DONE             _IO(IOC_MZ_MAGIC, 0xdb)

#define DPRAM_DUMP_MMAP_START_ADDR       VOLANS_VA_DPRAM
#define DPRAM_DUMP_MMAP_SIZE             0x1000000      //16M 
#define DPRAM_DUMP_MMAP_END_ADDR         DPRAM_DUMP_MMAP_START_ADDR+DPRAM_DUMP_MMAP_SIZE
#endif 
#define DPRAM_GET_DSG_INFO              _IOR(IOC_MZ_MAGIC, 0xdc, HeaderInformation)             
#define DPRAM_SILENT_RESET		    _IO(IOC_MZ_MAGIC, 0xdd)

/*
 * structure definitions.
 */
typedef struct dpram_serial {
	/* pointer to the tty for this device */
	struct tty_struct *tty;

	/* number of times this port has been opened */
	int open_count;

	/* locks this structure */
	struct semaphore sem;
} dpram_serial_t;

typedef struct dpram_device {
	/* DPRAM memory addresses */
	unsigned long in_head_addr;
	unsigned long in_tail_addr;
	unsigned long in_buff_addr;
	unsigned long in_buff_size;

	unsigned long out_head_addr;
	unsigned long out_tail_addr;
	unsigned long out_buff_addr;
	unsigned long out_buff_size;

	u_int16_t mask_req_ack;
	u_int16_t mask_res_ack;
	u_int16_t mask_send;

	dpram_serial_t serial;
} dpram_device_t;

typedef struct dpram_tasklet_data {
	dpram_device_t *device;
	u_int16_t non_cmd;
} dpram_tasklet_data_t;

struct _mem_param {
	unsigned short addr;
	unsigned long data;
	int dir;
};

struct _param_nv {
	unsigned char *addr;
	unsigned int size;
};

/* TODO: add more definitions */

#endif	/* __DPRAM_H__ */

