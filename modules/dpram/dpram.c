/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
**                Onedram Device Driver
**
****************************************************************************/

#define _DEBUG
#define _ENABLE_ERROR_DEVICE
#define _COMMAND_TASKLET 

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/atomic.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <linux/sched.h>

#ifdef _ENABLE_RAM_DUMP
#include <linux/mm.h>
#include <asm/page.h>
#endif 
#ifdef _ENABLE_ERROR_DEVICE
#include <linux/poll.h>
#include <linux/cdev.h>
#endif	/* _ENABLE_ERROR_DEVICE */


#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif	/* CONFIG_PROC_FS */

#include "dpram.h"
#include "BML.h"

#define DRIVER_ID			"$Id: dpram.c, v0.01 2009/05/03 08:00:00 $"
#define DRIVER_NAME 		"DPRAM"
#define DRIVER_PROC_ENTRY	"driver/dpram"
#define DRIVER_MAJOR_NUM	252

#define COMMAND_SUCCESS                 0x00
#define COMMAND_GET_AUTHORITY_FAIL      0x01
#define COMMAND_FAIL                    0x02


#ifdef _DEBUG
#define dprintk(s, args...) printk("[DPRAM] %s:%d - " s, __func__, __LINE__,  ##args)
#else
#define dprintk(s, args...)
#endif	/* _DEBUG */

#define WRITE_TO_DPRAM(dest, src, size)		_memcpy((volatile void *)(DPRAM_VBASE + dest), src, size)
#define READ_FROM_DPRAM(dest, src, size)	_memcpy(dest, (volatile void *)(DPRAM_VBASE + src), size)

#ifdef _ENABLE_ERROR_DEVICE
#define DPRAM_ERR_MSG_LEN			65
#define DPRAM_ERR_DEVICE			"dpramerr"
#endif	/* _ENABLE_ERROR_DEVICE */

#ifdef _ENABLE_RAM_DUMP
#define DPRAM_RAM_DUMP_DEVICE   "dpramdump" 
#define DPRAM_RAM_DUMP_DEVICE_MINOR 3
#define DPRAM_RAM_DUMP_MAJOR        223

static  int modem_ram_dump_status = 0;  
static  char dump_count = 0;
static int dpram_ram_dump_open(struct inode *inode, struct file *filp);
static unsigned int dpram_ram_dump_poll (struct file *fiip, struct poll_table_struct *wait);
static int dpram_ram_dump_mmap(struct file *filp, struct vm_area_struct *vma);
static int dpram_ram_dump_read(struct file *filp, char *buf, size_t count, loff_t *ppos);
static void unregister_modem_ram_dump_device(void);
static int dpram_dump_read_done;
static DECLARE_WAIT_QUEUE_HEAD(dpram_dump_wait_q);
static void dpram_get_dsg_info(HeaderInformation *DGS_INFO);
struct class *dpram_dump_class;
static int temp =0;
#endif 
static unsigned short dpram_silent_reset = 0;
extern INT32 read_DGS_info(void *buf);


#define MESG_PHONE_FATAL            0
#define MESG_PHONE_OFF		        1
#define MESG_PHONE_ABNORMAL_RESET   2
#define MESG_PHONE_RAMDUMP_RESET    3
#define MESG_PHONE_POR_RESET		4


#define IRQ_nONED_INT_AP	IRQ_EINT(0)
#define IRQ_PHONE_ACTIVE	IRQ_EINT(7)

static struct file *DPRAM_FILE_OPEN (const char *pathname,int open_flag, int mode);
static ssize_t DPRAM_FILE_READ (struct file *filp, char *buf, size_t size);
static void DPRAM_FILE_CLOSE (struct file *flip);


/* -------------------------------- */
/* -----   OneDRAM Interface  ----- */
/* -------------------------------- */
static int dpram_get_authority(void);
static int dpram_return_authority(void);
static int new_dpram_get_authority(void);
static int sleep_dpram_get_authority(void);
static void kill_tasklets(void);

#define MEMORY_LOCK()	{			\
	atomic_inc(&shared_count);		\
}

#define MEMORY_UNLOCK()	{			\
	atomic_dec(&shared_count);		\
}

#define GET_SEMAPHORE()	dpram_get_authority()
#define RELEASE_SEM()	dpram_return_authority()

#define MODEM_STATUS_UNKNOWN        0x00
#define MODEM_STATUS_OFF            0x01
#define MODEM_STATUS_RESET          0x02
#define MODEM_STATUS_FATAL_ERROR    0x03
#define MODEM_STATUS_RAM_DUMP       0x04
#define RET_MODEM_OFF               -101


atomic_t shared_count = ATOMIC_INIT(0);

static unsigned int request_smp_modem = 0;
static unsigned int modem_status = MODEM_STATUS_UNKNOWN; 
static u16 smp_wait_status = 0;
/* -------------------------------- */

static struct tty_driver *dpram_tty_driver;
static dpram_tasklet_data_t dpram_tasklet_data[MAX_INDEX];
static dpram_device_t dpram_table[MAX_INDEX] = {
	{
		.in_head_addr = DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS,
		.in_tail_addr = DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS,
		.in_buff_addr = DPRAM_PHONE2PDA_FORMATTED_BUFFER_ADDRESS,
		.in_buff_size = DPRAM_PHONE2PDA_FORMATTED_BUFFER_SIZE,

		.out_head_addr = DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS,
		.out_tail_addr = DPRAM_PDA2PHONE_FORMATTED_TAIL_ADDRESS,
		.out_buff_addr = DPRAM_PDA2PHONE_FORMATTED_BUFFER_ADDRESS,
		.out_buff_size = DPRAM_PDA2PHONE_FORMATTED_BUFFER_SIZE,

		.mask_req_ack = INT_MASK_REQ_ACK_F,
		.mask_res_ack = INT_MASK_RES_ACK_F,
		.mask_send = INT_MASK_SEND_F,
	},
	{
		.in_head_addr = DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS,
		.in_tail_addr = DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS,
		.in_buff_addr = DPRAM_PHONE2PDA_RAW_BUFFER_ADDRESS,
		.in_buff_size = DPRAM_PHONE2PDA_RAW_BUFFER_SIZE,

		.out_head_addr = DPRAM_PDA2PHONE_RAW_HEAD_ADDRESS,
		.out_tail_addr = DPRAM_PDA2PHONE_RAW_TAIL_ADDRESS,
		.out_buff_addr = DPRAM_PDA2PHONE_RAW_BUFFER_ADDRESS,
		.out_buff_size = DPRAM_PDA2PHONE_RAW_BUFFER_SIZE,

		.mask_req_ack = INT_MASK_REQ_ACK_R,
		.mask_res_ack = INT_MASK_RES_ACK_R,
		.mask_send = INT_MASK_SEND_R,
	},
};

static struct tty_struct *dpram_tty[MAX_INDEX];
static struct ktermios *dpram_termios[MAX_INDEX];
static struct ktermios *dpram_termios_locked[MAX_INDEX];
static unsigned int phone_boottype = 0;
extern int forced_dpram_wakeup; 

static void res_ack_tasklet_handler(unsigned long data);
static void send_tasklet_handler(unsigned long data);

static DECLARE_TASKLET(fmt_send_tasklet, send_tasklet_handler, 0);
static DECLARE_TASKLET(raw_send_tasklet, send_tasklet_handler, 0);
static DECLARE_TASKLET(fmt_res_ack_tasklet, res_ack_tasklet_handler,
		(unsigned long)&dpram_table[FORMATTED_INDEX]);
static DECLARE_TASKLET(raw_res_ack_tasklet, res_ack_tasklet_handler,
		(unsigned long)&dpram_table[RAW_INDEX]);

#ifdef _COMMAND_TASKLET
static void commnad_tasklet_handler(unsigned long data);
static DECLARE_TASKLET(cmd_send_tasklet, commnad_tasklet_handler,0);
#endif 
static void semaphore_control_handler(unsigned long data);
static DECLARE_TASKLET(semaphore_control_tasklet, semaphore_control_handler, 0);

#ifdef _ENABLE_ERROR_DEVICE
static unsigned int dpram_err_len;
static char dpram_err_buf[DPRAM_ERR_MSG_LEN];

struct class *dpram_class;

static DECLARE_WAIT_QUEUE_HEAD(phone_power_wait);
static DECLARE_WAIT_QUEUE_HEAD(dpram_err_wait_q);
static struct fasync_struct *dpram_err_async_q;
#endif	/* _ENABLE_ERROR_DEVICE */

// 2008.10.20.
static DECLARE_MUTEX(write_mutex);

static struct file *DPRAM_FILE_OPEN (const char *pathname,int open_flag, int mode)
{
    return filp_open(pathname, open_flag, mode);    
}

static ssize_t DPRAM_FILE_READ (struct file *filp, char *buf, size_t size)
{
    ssize_t read_size = 0; 
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(get_ds());
    if (filp->f_op->read != NULL)
        read_size = filp->f_op->read(filp, buf, size,&filp->f_pos);
    set_fs(oldfs);
    return read_size; 
}

static void DPRAM_FILE_CLOSE (struct file *flip)
{
    filp_close(flip, NULL);
}


/* tty related functions. */
static inline void byte_align(unsigned long dest, unsigned long src)
{
	u16 *p_src;
	volatile u16 *p_dest;

	if (!(dest % 2) && !(src % 2)) {
		p_dest = (u16 *)dest;
		p_src = (u16 *)src;

		*p_dest = (*p_dest & 0xFF00) | (*p_src & 0x00FF);
	}

	else if ((dest % 2) && (src % 2)) {
		p_dest = (u16 *)(dest - 1);
		p_src = (u16 *)(src - 1);

		*p_dest = (*p_dest & 0x00FF) | (*p_src & 0xFF00);
	}

	else if (!(dest % 2) && (src % 2)) {
		p_dest = (u16 *)dest;
		p_src = (u16 *)(src - 1);

		*p_dest = (*p_dest & 0xFF00) | ((*p_src >> 8) & 0x00FF);
	}

	else if ((dest % 2) && !(src % 2)) {
		p_dest = (u16 *)(dest - 1);
		p_src = (u16 *)src;

		*p_dest = (*p_dest & 0x00FF) | ((*p_src << 8) & 0xFF00);
	}

	else {
		dprintk("oops.~\n");
	}
}

static inline void _memcpy(void *p_dest, const void *p_src, int size)
{
	unsigned long dest = (unsigned long)p_dest;
	unsigned long src = (unsigned long)p_src;

	if (size <= 0) {
		return;
	}

	if (dest & 1) {
		byte_align(dest, src);
		dest++, src++;
		size--;
	}

	if (size & 1) {
		byte_align(dest + size - 1, src + size - 1);
		size--;
	}

	if (src & 1) {
		unsigned char *s = (unsigned char *)src;
		volatile u16 *d = (unsigned short *)dest;

		size >>= 1;

		while (size--) {
			*d++ = s[0] | (s[1] << 8);
			s += 2;
		}
	}

	else {
		u16 *s = (u16 *)src;
		volatile u16 *d = (unsigned short *)dest;

		size >>= 1;

		while (size--) { *d++ = *s++; }
	}
}

static inline int _memcmp(u8 *dest, u8 *src, int size)
{
	int i = 0;

	while (i++ < size) {
		if (*(dest + i) != *(src + i)) {
			return 1;
		}
	}

	return 0;
}

static inline int WRITE_TO_DPRAM_VERIFY(u32 dest, void *src, int size)
{
	int cnt = 3;

	while (cnt--) {
		_memcpy((volatile void *)(DPRAM_VBASE + dest), (void *)src, size);

		if (!_memcmp((volatile u8 *)(DPRAM_VBASE + dest), (u8 *)src, size))
			return 0;
	}

	return -1;
}

static inline int READ_FROM_DPRAM_VERIFY(void *dest, u32 src, int size)
{
	int cnt = 3;

	while (cnt--) {
		_memcpy((void *)dest, (volatile void *)(DPRAM_VBASE + src), size);

		if (!_memcmp((u8 *)dest, (volatile u8 *)(DPRAM_VBASE + src), size))
			return 0;
	}

	return -1;
}


static void send_interrupt_to_phone(u16 irq_mask)
{
	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &irq_mask,
			DPRAM_INTERRUPT_PORT_SIZE);
}

static int dpram_write(dpram_device_t *device,
		const unsigned char *buf, int len)
{
	int retval = 0;
	int size = 0;
	u16 head, tail;
	u16 irq_mask = 0;
    u16 timeout = 0; 
	
	down_interruptible(&write_mutex);

    retval = FALSE; 

    while(retval != TRUE) {
        retval = sleep_dpram_get_authority();
        if (retval == RET_MODEM_OFF) {
            printk("[ONEDRAM] dpram_write().. MODEM OFF Status.. \n"); 
            up(&write_mutex);
            return -1; 
        }
        else if (retval == -2)
            timeout++; 
        else if (retval == FALSE) 
            msleep(1);
        if (timeout > 50) {
            printk("[ONEDRAM] dpram_write() timeout .. \n");
            up(&write_mutex);
            return -EAGAIN;
        }
    }

	READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
	READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

	// +++++++++ head ---------- tail ++++++++++ //
	if (head < tail) {
		size = tail - head - 1;
		size = (len > size) ? size : len;

		WRITE_TO_DPRAM(device->out_buff_addr + head, buf, size);
		retval = size;
	}

	// tail +++++++++++++++ head --------------- //
	else if (tail == 0) {
		size = device->out_buff_size - head - 1;
		size = (len > size) ? size : len;

		WRITE_TO_DPRAM(device->out_buff_addr + head, buf, size);
		retval = size;
	}

	// ------ tail +++++++++++ head ------------ //
	else {
		size = device->out_buff_size - head;
		size = (len > size) ? size : len;
		
		WRITE_TO_DPRAM(device->out_buff_addr + head, buf, size);
		retval = size;

		if (len > retval) {
			size = (len - retval > tail - 1) ? tail - 1 : len - retval;
			
			WRITE_TO_DPRAM(device->out_buff_addr, buf + retval, size);
			retval += size;
		}
	}

	/* @LDK@ calculate new head */
	head = (u16)((head + retval) % device->out_buff_size);
	WRITE_TO_DPRAM_VERIFY(device->out_head_addr, &head, sizeof(head));

	/* @LDK@ send interrupt to the phone, if.. */
	irq_mask = INT_MASK_VALID;

	if (retval > 0)
		irq_mask |= device->mask_send;

	if (len > retval)
		irq_mask |= device->mask_req_ack;

	send_interrupt_to_phone(irq_mask);

	MEMORY_UNLOCK();
	up(&write_mutex);

	dpram_return_authority();

	return retval;
}

//
// ++ geunyoung on 20090331 for cluster data.
// 데이터가 1500보다 클 경우 안정적으로 조금씩 할당해서 넣는다.
// ex) cal data.
//
static inline
int dpram_tty_insert_data(dpram_device_t *device, const u8 *psrc, u16 size)
{
#define CLUSTER_SEGMENT	1600

	u16 copied_size = 0;
	int retval = 0;

	// 대용량 데이터이고 multipdp로 올라가는 raw data인 경우..
	if (size > CLUSTER_SEGMENT && (device->serial.tty->index == 1)) {
		while (size) {
			copied_size = (size > CLUSTER_SEGMENT) ? CLUSTER_SEGMENT : size;
			tty_insert_flip_string(device->serial.tty, psrc + retval, copied_size);

			size = size - copied_size;
			retval += copied_size;
		}

		return retval;
	}

	return tty_insert_flip_string(device->serial.tty, psrc, size);
}

/*
 * dpram_read function is modified by geunyoung on 20090331 for stable raw data transmission.
 */
static int dpram_read(dpram_device_t *device, const u16 non_cmd)
{
	int retval = 0;
	int size = 0;

	u16 head, tail;
    int ret; 


    ret = new_dpram_get_authority();
    if (ret == 100) {
        return -100; 
    }
    else if (ret == FALSE){ 
        return -1; 
    }

	READ_FROM_DPRAM_VERIFY(&head, device->in_head_addr, sizeof(head));
	READ_FROM_DPRAM_VERIFY(&tail, device->in_tail_addr, sizeof(tail));

	if (head != tail) {
		u16 up_tail = 0;

		// ------- tail ++++++++++++ head -------- //
		if (head > tail) {
			retval = dpram_tty_insert_data(device, (u8 *)(DPRAM_VBASE + (device->in_buff_addr + tail)), head - tail);

			if (retval != (head - tail)) {
				printk("%s:%d - \"dpram_tty_insert_data!\" Real Size: %u, Returned Size: %u\n",
						__func__, __LINE__, (head - tail), retval);
			}
		}

		// +++++++ head ------------ tail ++++++++ //
		else {
			int tmp_size = 0;

			// Total Size.
			size = device->in_buff_size - tail + head;

			// 1. tail -> buffer end.
			tmp_size = device->in_buff_size - tail;
			retval = dpram_tty_insert_data(device, (u8 *)(DPRAM_VBASE + (device->in_buff_addr + tail)), tmp_size);

			// 2. buffer start -> head.
			if (size > tmp_size) {
				dpram_tty_insert_data(device, (u8 *)(DPRAM_VBASE + device->in_buff_addr), size - tmp_size);
				retval += (size - tmp_size);
			}
		}

		/* new tail */
		up_tail = (u16)((tail + retval) % device->in_buff_size);
		WRITE_TO_DPRAM_VERIFY(device->in_tail_addr, &up_tail, sizeof(up_tail));
	}

	if (non_cmd & device->mask_req_ack)
		send_interrupt_to_phone(INT_NON_COMMAND(device->mask_res_ack));

	MEMORY_UNLOCK();
	//dpram_return_authority();

	return retval;
}


atomic_t smp_get_authoriry = ATOMIC_INIT(0);

#define LOCK_GET_AUTHORITY() {  \
    atomic_inc(&smp_get_authoriry);   \
}

#define UNLOCK_GET_AUTHORITY() {    \
    atomic_dec(&smp_get_authoriry);  \
}



static int dpram_get_authority(void)
{
    u16 cmd = 0; 
    u16 i = 0; 
    unsigned int sem; 

    if (modem_ram_dump_status)
        return FALSE; 
    
    if (request_smp_modem)
        return FALSE; 

    if (atomic_read(&smp_get_authoriry))
        return FALSE; 

    if (atomic_read(&shared_count))
        return FALSE; 

    READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem));     
    if (sem == 0x00){
        cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
    	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
    }
    LOCK_GET_AUTHORITY();
    i = 300;
    do {
        sem = 0x00; 
        READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem)); 
		if (sem == 0x1){
            MEMORY_LOCK(); 
            UNLOCK_GET_AUTHORITY();
            return TRUE;
		}
        i--;
        if (atomic_read(&smp_get_authoriry) > 1) {
            printk("count : %d, value : %d\n",atomic_read(&smp_get_authoriry),i);        
        }
    }while(i); 
    UNLOCK_GET_AUTHORITY();
    //printk("!! Time out get authority ..\n");
    return FALSE; 
    
}


static int sleep_dpram_get_authority(void)
{
    u16 cmd = 0; 
    u16 i = 0; 
    unsigned int sem; 

    if (modem_ram_dump_status)
        return FALSE; 
    
    if (modem_status == MODEM_STATUS_OFF)
        return RET_MODEM_OFF; 

    if (request_smp_modem)
        return FALSE; 

    if (atomic_read(&shared_count))
        return FALSE; 

    if (atomic_read(&smp_get_authoriry))
        return FALSE; 

    READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem));     
    if (sem == 0x00){
        cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
    	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
    }
    
    LOCK_GET_AUTHORITY();
    i = 2999;
    do {
        sem = 0x00; 
        READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem)); 
		if (sem == 0x1){
            MEMORY_LOCK(); 
            UNLOCK_GET_AUTHORITY();
            return TRUE;
		}
        if ((i%500) == 0) {
            READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem));     
            if (sem == 0x00){
                cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
            	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
            }    
            mdelay(1); 
        }
        
        if (modem_status == MODEM_STATUS_OFF)
            return RET_MODEM_OFF; 
        
        if (modem_ram_dump_status)
            return RET_MODEM_OFF; 
        i--;    
    }while(i); 
    UNLOCK_GET_AUTHORITY();
    return -2; 
    
}



static int new_dpram_get_authority(void)
{
    u16 cmd = 0; 
    u16 i = 0; 
    unsigned int sem; 

    if (modem_ram_dump_status)
        return FALSE; 

    if (atomic_read(&smp_get_authoriry))
        return FALSE; 

    if (atomic_read(&shared_count))
        return FALSE; 
    
    READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem));     
    if (sem == 0){
        cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
    	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
        return 100; 
    }
    
    LOCK_GET_AUTHORITY();
    i = 1;
    do {
        READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem)); 
		if (sem == 0x1){
            MEMORY_LOCK(); 
            UNLOCK_GET_AUTHORITY();
            return TRUE;
		}
        i--;
    }while(i); 
    
    READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem));     
    if (sem == 0){
        cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
    	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
    }
    UNLOCK_GET_AUTHORITY();
    return FALSE; 
    
}




static int dpram_return_authority(void)
{
    unsigned int sem; 
    u16 cmd; 

    if (atomic_read(&smp_get_authoriry))
        return FALSE; 

    LOCK_GET_AUTHORITY();    
    if (!atomic_read(&shared_count)){
        sem = 0x0;
        WRITE_TO_DPRAM(DPRAM_SMP, &sem, sizeof(sem)); 
        if(request_smp_modem) {
            cmd = INT_COMMAND(INT_MASK_CMD_SMP_REP);
            WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, &cmd, DPRAM_INTERRUPT_PORT_SIZE);
            request_smp_modem = 0; 
        }
        UNLOCK_GET_AUTHORITY();
        return TRUE;
    }
    UNLOCK_GET_AUTHORITY();
    return FALSE; 
}

static int ReadPhoneFile(
	unsigned char *DblBuffer, 
	unsigned char *ImageBuffer, 
	unsigned long Dbl_Length, 
	unsigned long Total_length)
{
	int   nRet;
	XSRPartEntry stPartEntry;
	unsigned int nVol = 0;
	BMLVolSpec stVolSpec;
	int PAGE_PER_BLOCK;
	int BYTE_PER_PAGE;
	int SECTOR_PER_PAGE;
	unsigned long totalPageNumber = 0;
	unsigned long DblPageNumber = 0;
	unsigned long TotalBLKNumber = 0;
	unsigned int VStartSector;
	unsigned int VStartBlock;
	unsigned int BLKNumberPartition;

	BML_Open(0);

	//block entry 
	if (BML_LoadPIEntry(nVol, PARTITION_ID_MODEM_IMG, &stPartEntry) == BML_SUCCESS) {
//		printk("  - FlashRead : BML_LoadPIEntry is succesed. partition = %x\n", PARTITION_ID_MODEM_IMG);
		printk("  - BML_LoadPIEntry OK. partitionID: %x\n", PARTITION_ID_MODEM_IMG);
	}
	else {
		printk(" # FlashRead : BML_LoadPIEntry is failed\n");
		return FALSE;
	}
	//block get information
	if (BML_GetVolInfo(nVol, &stVolSpec) != BML_SUCCESS) {
		printk(" # EOND# FlashRead : BML_GetVolInfo is failed\n");
		return FALSE;
	}

	PAGE_PER_BLOCK = stVolSpec.nPgsPerBlk;
	SECTOR_PER_PAGE = stVolSpec.nSctsPerPg;
	BYTE_PER_PAGE = SECTOR_PER_PAGE * XSR_SECTOR_SIZE;

	//이미지 길이에 맞춰 총 page수 알아낸다.
	if(Dbl_Length % BYTE_PER_PAGE)
		printk(" # EOND spare length error \n");
	else
		DblPageNumber = Dbl_Length/BYTE_PER_PAGE;
	if(Total_length % BYTE_PER_PAGE)
		totalPageNumber = (Total_length/BYTE_PER_PAGE)+1;
	else
		totalPageNumber = Total_length/BYTE_PER_PAGE;

	// page 개수에 맞춰 총 Block 수를 알아낸다.
	if(totalPageNumber % PAGE_PER_BLOCK)
		TotalBLKNumber    = (totalPageNumber/PAGE_PER_BLOCK) + 1;
	else
		TotalBLKNumber    = totalPageNumber/PAGE_PER_BLOCK;

	VStartBlock = stPartEntry.n1stVbn;
	BLKNumberPartition = stPartEntry.nNumOfBlks;

	if(TotalBLKNumber > BLKNumberPartition)
		// block read
	{
		printk(" EOND: error --too long size to read\n");
		return FALSE;
	}

	/*
//	printk("  - DBL image size : %x\n", Dbl_Length);
	printk("  - Partition Info\n");
	printk("   - Volumn number: %d\n", nVol);
	printk("   - BLKNumber: %d\n",BLKNumberPartition);
	printk("   - PAGEperBLK: %d\n",PAGE_PER_BLOCK);
	printk("   - SECTORperPAGE: %d\n",SECTOR_PER_PAGE);
	printk("   - BYTEperPAGE: %d\n",BYTE_PER_PAGE);
//	printk("  - Virtual Address of Start Address: %x\n", VStartBlock);

	printk("  - Phone Image Info\n");
	printk("   - SectorIndex: %lu\n", VStartSector + (DblPageNumber * SECTOR_PER_PAGE));
	printk("   - SectorsNumber: %lu\n", (totalPageNumber - DblPageNumber) * SECTOR_PER_PAGE);
	printk("   - BLKNumber: %lu\n", TotalBLKNumber);
	printk("   - Maxsize: %lu\n", Total_length);
	printk("   - Shared memory addr: %p\n", ImageBuffer);
	*/

	VStartSector = VStartBlock * PAGE_PER_BLOCK * SECTOR_PER_PAGE; //virtual start sector

#if 0
	nRet = BML_MRead(nVol,
			VStartSector,              /* Virtual Sector Number    */
			DblPageNumber * SECTOR_PER_PAGE,                /* Number of Sectors        */
			DblBuffer,                 /* Buffer pointer for Main  */
			NULL,                   /* Buffer pointer for Dbl */
			BML_FLAG_ECC_ON | BML_FLAG_BBM_ON);     /* Operation Flag           */
	if (nRet != BML_SUCCESS)
	{
		printk(" # FlashRead : BML_Read is failed\n");
		return FALSE;
	}
	// write log
#endif
	nRet = BML_MRead(nVol,
			VStartSector + (DblPageNumber * SECTOR_PER_PAGE),      /* Virtual Sector Number    */
			(totalPageNumber - DblPageNumber) * SECTOR_PER_PAGE,     /* Number of Sectors        */
			ImageBuffer,                 /* Buffer pointer for Main  */
			NULL,                   /* Buffer pointer for Dbl */
			BML_FLAG_ECC_ON | BML_FLAG_BBM_ON);     /* Operation Flag           */
	if (nRet != BML_SUCCESS)
	{
		printk("# FlashRead : BML_Read is failed\n");
		return FALSE;
	}

	printk("0x%8x, 0x%8x, 0x%8x, 0x%8x\n",
			*ImageBuffer,
			*(ImageBuffer + 4),
			*(ImageBuffer + 8),
			*(ImageBuffer + 12));

	return TRUE;
}

static void dpram_clear(void)
{
	long i = 0;
	unsigned long flags;
	
	u16 value = 0;

	/* @LDK@ clear DPRAM except interrupt area */
	local_irq_save(flags);

	for (i = DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS;
			i < DPRAM_SIZE - (DPRAM_INTERRUPT_PORT_SIZE * 2);
			i += 2)
	{
		*((u16 *)(DPRAM_VBASE + i)) = 0;
	}

	local_irq_restore(flags);

	READ_FROM_DPRAM(&value, DPRAM_PHONE2PDA_INTERRUPT_ADDRESS,
			sizeof(value));
}

static int dpram_init_and_report(void)
{
	const u16 magic_code = 0x00aa;
	const u16 init_start = INT_COMMAND(INT_MASK_CMD_INIT_START);
	u16 init_end = INT_COMMAND(INT_MASK_CMD_INIT_END);

	u16 ac_code = 0;


	/* @LDK@ send init start code to phone */
	if (dpram_get_authority() != TRUE)
        return COMMAND_GET_AUTHORITY_FAIL;
	//MEMORY_LOCK();

	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS,
			&init_start, DPRAM_INTERRUPT_PORT_SIZE);

	/* @LDK@ write DPRAM disable code */
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &ac_code, sizeof(ac_code));

	/* @LDK@ dpram clear */
	dpram_clear();

	/* @LDK@ write magic code */
	WRITE_TO_DPRAM(DPRAM_MAGIC_CODE_ADDRESS,
			&magic_code, sizeof(magic_code));

	/* @LDK@ write DPRAM enable code */
	ac_code = 0x0001;
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &ac_code, sizeof(ac_code));

	MEMORY_UNLOCK();
	dpram_return_authority();

	/* 2009.05.07 added by hobac for LPM boot mode. */
	if (phone_boottype)
		init_end |= 0x1000;

	if (dpram_silent_reset){
		init_end |= 0x2000;
		dpram_silent_reset = 0;
	}

	/* @LDK@ send init end code to phone */
	WRITE_TO_DPRAM(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS,
			&init_end, DPRAM_INTERRUPT_PORT_SIZE);

	dprintk("Initialized!\n");
    return COMMAND_SUCCESS;
}

static inline int dpram_get_read_available(dpram_device_t *device)
{
	u16 head = 0, tail = 0;

#if 0 
	if (dpram_get_authority() == TRUE) {
		MEMORY_LOCK();
		READ_FROM_DPRAM_VERIFY(&head, device->in_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->in_tail_addr, sizeof(tail));
		MEMORY_UNLOCK();
	}
#endif 

	return head - tail;
}

static void dpram_drop_data(dpram_device_t *device)
{
	u16 head;

	if (dpram_get_authority() == TRUE) {
		READ_FROM_DPRAM_VERIFY(&head, device->in_head_addr, sizeof(head));
		WRITE_TO_DPRAM_VERIFY(device->in_tail_addr, &head, sizeof(head));
		MEMORY_UNLOCK();
	}

}

static void dpram_phone_image_load(void)
{
	dprintk("PHONE IMAGE LOAD START!\n");

    while(dpram_get_authority() != TRUE) 
        msleep(1);
    //MEMORY_LOCK();
	ReadPhoneFile(NULL, (u8 *)DPRAM_VBASE, MAX_DBL_IMG_SIZE, MAX_MODEM_IMG_SIZE);
	MEMORY_UNLOCK();
}

static void dpram_nvdata_load(struct _param_nv *param)
{
    HeaderInformation DGS_INFO;
	dprintk("NVDATA LOAD START!\n");

    while(dpram_get_authority() != TRUE) 
        msleep(1); 
    //MEMORY_LOCK();
	WRITE_TO_DPRAM( 0xF80000 - 0x5000, param->addr, param->size);

    //copy dgs info to modem ..
    dpram_get_dsg_info(&DGS_INFO);
#if 0
    if (dpram_silent_reset) {
        printk("[ONEDRAM] PDA Request Silent Boot \n");
        WRITE_TO_DPRAM(DPRAM_SILENT_RESET_ADDR, &dpram_silent_reset, sizeof(dpram_silent_reset));
    }
#endif
    //printk("===== PRINT DGS INFO [ONEDRAM]===== \n");
    //printk("[ONEDRAM] Model Name : %s\n",DGS_INFO.ModelName);
    //printk("[ONEDRAM] DATA       : %s\n",DGS_INFO.Date);
    //memcpy((volatile HeaderInformation *)DPRAM_PDA2PHONE_DGS_INFO_START_ADDRESS,&DGS_INFO,sizeof(DGS_INFO));
    WRITE_TO_DPRAM(DPRAM_PDA2PHONE_DGS_INFO_START_ADDRESS,&DGS_INFO,sizeof(DGS_INFO));

    
	MEMORY_UNLOCK();
}
	
static void dpram_phone_on(void)
{
	dprintk("PHONE POWER ON!\n");
    s3c_gpio_setpin(GPIO_FLM_SEL, 1);
	s3c_gpio_setpin(GPIO_USIM_BOOT, 1);
    s3c_gpio_setpin(GPIO_PHONE_ON,0);
	mdelay(100);

	s3c_gpio_setpin(GPIO_CP_RST, 1);
	mdelay(200);

	s3c_gpio_setpin(GPIO_PHONE_ON, 1);
	s3c_gpio_setpin(GPIO_CP_RST, 0);
	mdelay(100);

    s3c_gpio_setpin(GPIO_PHONE_ON,0);
	s3c_gpio_setpin(GPIO_CP_RST, 1);
    mdelay(600);
}

static void dpram_phone_off(void)
{
    dprintk("PHONE POWER OFF! \n"); 
    s3c_gpio_setpin(GPIO_FLM_SEL,0);
    s3c_gpio_setpin(GPIO_USIM_BOOT,0);

    s3c_gpio_setpin(GPIO_CP_RST,0);
    s3c_gpio_setpin(GPIO_PHONE_ON,0);
    
}

static void dpram_phone_reset(void)
{
    printk("[ONEDRAM] dpram_phone_reset() called \n");
    s3c_gpio_setpin(GPIO_CP_RST,0);
    interruptible_sleep_on_timeout(&phone_power_wait, 30);
    s3c_gpio_setpin(GPIO_CP_RST,1);
}
#define PAGE_SIZE 2*1024 

static void dpram_get_dsg_info(HeaderInformation *DGS_INFO)
{
    u32 DGS_HEADER;
    u8 i;
    memset(DGS_INFO,0,sizeof(HeaderInformation));
    char *dgs_buffer = NULL; 

    dgs_buffer = (char *)kmalloc(PAGE_SIZE,GFP_ATOMIC); 
    if (dgs_buffer == NULL) {
        printk("Error.. dgs_buffer Allocate Failed..\n");
        return; 
    }
    if (read_DGS_info((void *)dgs_buffer) != 0) {
        printk("Error.. read_DGS_info() failed ..\n");
        kfree(dgs_buffer);
        return;
    }
    memcpy(DGS_INFO,(HeaderInformation *)dgs_buffer,sizeof(HeaderInformation));
    printk("Print DGS Header : ");
    for (i=0;i<4;i++)
        printk("%x",DGS_INFO->HeaderCode[i]);    
    printk("\n");

    kfree(dgs_buffer);
    return;
}
#ifdef _ENABLE_RAM_DUMP
struct file_operations dpram_ram_dump_ops = {
    .owner  = THIS_MODULE,
    .open   = dpram_ram_dump_open,
    .poll   = dpram_ram_dump_poll,
    .mmap   = dpram_ram_dump_mmap,
    .read   = dpram_ram_dump_read, 
};

static int dpram_ram_dump_open(struct inode *inode, struct file *filp)
{
    printk("[ONEDRAM] dpram_ram_dump_open called .. \n");
    return 0;
}

static unsigned int dpram_ram_dump_poll (struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    printk("[ONDRAM]dpram_ram_dump_poll() called waiting.. \n"); 
    interruptible_sleep_on(&dpram_dump_wait_q);
    printk("[ONEDRAM]dpram_ram_dump_poll() event occur.. \n");
    mask = POLLIN|POLLRDNORM;
    return mask;
}


static int dpram_ram_dump_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int loopCnt;
    unsigned long pageFrameNo = 0;
    
    printk("[ONDRAM]dpram_ram_dump_mmap() called \n"); 
    
    vma->vm_flags |= VM_RESERVED; 
    vma->vm_flags |= VM_IO; 

    pageFrameNo = __phys_to_pfn(VOLANS_PA_DPRAM);
    if (remap_pfn_range(vma, vma->vm_start, pageFrameNo, DPRAM_DUMP_MMAP_SIZE, vma->vm_page_prot)) {
        printk("[ONEDRAM] dpram_ram_dump_mmap() failed .. \n");
        return -1; 
    }

    return 0; 
}

static int dpram_ram_dump_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
    #define BUFFER_SIZE 2
    int ret; 
    unsigned long flags;
    
    DECLARE_WAITQUEUE(wait, current); 
    char temp_buf[BUFFER_SIZE] = {0,0};
    
    printk("[ONDRAM]dpram_ram_dump_read() called \n"); 

    if (filp->f_flags & O_NONBLOCK) {
		return -EAGAIN;
	}

    add_wait_queue(&dpram_dump_wait_q, &wait);
    set_current_state(TASK_INTERRUPTIBLE);
    local_irq_save(flags);
    printk("[ONEDRAM] dpram_ram_dump_read() return value : %d\n",dump_count);

    temp_buf[1] = dump_count; 
    if (copy_to_user(buf,&dump_count,sizeof(dump_count))) {
        printk("[ONEDRAM] dpram_ram_dump_read() copy_to_user failed ..\n");
        return -EFAULT; 
    }
    else 
        ret = sizeof(dump_count);

    local_irq_restore(flags);
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&dpram_dump_wait_q, &wait);

    return ret; 
}



static int register_modem_ram_dump_device(void)
{
    struct device *ram_dump_dev_t; 
    int ret; 
    printk("[ONEDRAM] Register MODEM_RAM_DUMP_DEVICE .. ..");
    ret = register_chrdev(DPRAM_RAM_DUMP_MAJOR, DPRAM_RAM_DUMP_DEVICE, &dpram_ram_dump_ops);
    if (ret < 0) {
        printk("Failed .. Error No : %d\n",ret); 
        return ret; 
    }

    dpram_dump_class = class_create(THIS_MODULE, "dump");
    if (IS_ERR(dpram_dump_class))
	{
		unregister_modem_ram_dump_device();
		return -EFAULT;
	}
    ram_dump_dev_t = device_create(dpram_dump_class, NULL,
			MKDEV(DPRAM_RAM_DUMP_MAJOR, 0), NULL, DPRAM_RAM_DUMP_DEVICE);
    printk("Success .. \n");
    if (IS_ERR(ram_dump_dev_t))
	{
		unregister_modem_ram_dump_device();
		return -EFAULT;
	}
    return 0;

}

static void unregister_modem_ram_dump_device(void)
{
    modem_ram_dump_status = 0; 
    unregister_chrdev(DRIVER_MAJOR_NUM,DPRAM_ERR_DEVICE);
    class_destroy(dpram_dump_class);
    return;
}

static void dpram_modem_ram_dump_read_done(void)
{
     unsigned int send_mail,sem; 
     printk("[ONEDRAM] dpram_modem_ram_dump_read_done() called \n");
     send_mail = 0x34563456;
     sem =0; 
     WRITE_TO_DPRAM(DPRAM_SMP, &sem, sizeof(sem)); 
     WRITE_TO_DPRAM(DPRAM_MBX_BA, &send_mail, sizeof(send_mail));
     dump_count++;
     return;
}




static irqreturn_t dpram_dump_irq_handler(int irq, void *dev_id)
{
    u32 recv_mail_box;
    u32 send_mail_box;
    u32 sem; 
    
    READ_FROM_DPRAM(&recv_mail_box, DPRAM_PHONE2PDA_INTERRUPT_ADDRESS, sizeof(recv_mail_box));
    printk("[ONEDRAM] dpram_dump_irq_handler() received mail box : %x\n",recv_mail_box);
    
    if (recv_mail_box == 0x12341234) {
        printk("[ONEDRAM] Received Mail box value: 0x12341234 \n");
        printk("[ONEDRAM] Send request to start RAM DUMP to Modem .. \n");
        send_mail_box = 0x34563456;
        sem = 0; 
        WRITE_TO_DPRAM(DPRAM_SMP, &sem, sizeof(sem)); 
        WRITE_TO_DPRAM(DPRAM_MBX_BA, &send_mail_box, sizeof(send_mail_box));
    }
    else if (recv_mail_box == 0x23452345){
        printk("[ONEDRAM] Received 0x23452345 .. Start Read data .. \n");
        printk("[ONEDRAM] Read Count : %d\n",dump_count);
        wake_up_interruptible(&dpram_dump_wait_q);   
    }

    temp = __raw_readl(S3C64XX_EINT0PEND);
    temp |= 0x01; 
    __raw_writel(temp,S3C64XX_EINT0PEND);
	return IRQ_HANDLED;
}


static int dpram_modem_ram_dump(void)
{
    int ret; 
    //add dpramdump driver.. 
    printk("[ONEDRAM] MODEM_RAM_DUMP   .. \n"); 

    free_irq(IRQ_nONED_INT_AP, NULL);
    modem_ram_dump_status = 1; 
    kill_tasklets();


    ret = request_irq(IRQ_nONED_INT_AP, dpram_dump_irq_handler, IRQF_DISABLED, "dpram_irq", NULL);
    if (ret) {
        printk("[ONEDRAM] interrupt handler failed .. \n");
    }
    return 0;
}


#endif 
static irqreturn_t dpram_irq_handler(int irq, void *dev_id);
static void dpram_phone_boot_start(void)
{
	unsigned int send_mail, recv_mail, sem = 0x0;
	unsigned int count = 0;

	send_mail = 0x45674567;

    s3c_gpio_setpin(GPIO_USIM_BOOT, 0);
	s3c_gpio_setpin(GPIO_FLM_SEL, 0);


	WRITE_TO_DPRAM(DPRAM_MBX_BA, &send_mail, sizeof(send_mail));
	WRITE_TO_DPRAM(DPRAM_SMP, &sem, sizeof(sem)); 
	
	dprintk("DBL DOWNLOAD COMPLETE!\n");

	/* work around */
	free_irq(IRQ_nONED_INT_AP, NULL);

	mdelay(1);

	while(count++ < 1000) {
		recv_mail = 0x0;
		READ_FROM_DPRAM(&recv_mail, DPRAM_PHONE2PDA_INTERRUPT_ADDRESS, sizeof(recv_mail));
//		printk("%x\n", recv_mail);
		if(recv_mail == 0xabcdabcd) {
			int retval = 0;

			printk(" +-------------------------------------+\n");
			printk(" |        PHONE BOOTING COMPLETE       |\n");
			printk(" +-------------------------------------+\n");
			printk("  - MailboxAB: 0x%8x\n", recv_mail);

			retval = request_irq(IRQ_nONED_INT_AP, dpram_irq_handler, IRQF_DISABLED, "dpram_irq", NULL);

			if (retval) {
				printk("DPRAM interrupt handler failed.\n");
			}

			mdelay(10);
			break;
		}

		mdelay(1);
	}
}

static int dpram_phone_getstatus(void)
{
	return gpio_get_value(GPIO_PHONE_ACTIVE_AP);
}

#ifdef CONFIG_PROC_FS
static int dpram_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	char *p = page;
	int len;

	u16 magic, enable;
	u16 fmt_in_head, fmt_in_tail, fmt_out_head, fmt_out_tail;
	u16 raw_in_head, raw_in_tail, raw_out_head, raw_out_tail;
	u16 in_interrupt = 0, out_interrupt = 0;
	unsigned int sem = 0x0;
    unsigned short silent_reset_bit;
	READ_FROM_DPRAM(&sem, DPRAM_SMP, sizeof(sem)); 
	printk(" [ LAST semaphore value : 0x%x ]\n", sem);

#ifdef _ENABLE_ERROR_DEVICE
	char buf[DPRAM_ERR_MSG_LEN];
	unsigned long flags;
#endif	/* _ENABLE_ERROR_DEVICE */
	
	
	if (dpram_get_authority() != TRUE) {
		return 0;
	}
	//MEMORY_LOCK();
	
	READ_FROM_DPRAM((void *)&magic, DPRAM_MAGIC_CODE_ADDRESS, sizeof(magic));
	READ_FROM_DPRAM((void *)&enable, DPRAM_ACCESS_ENABLE_ADDRESS, sizeof(enable));

	READ_FROM_DPRAM((void *)&fmt_in_head, DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS, 
			sizeof(fmt_in_head));
	READ_FROM_DPRAM((void *)&fmt_in_tail, DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS, 
		    sizeof(fmt_in_tail));
	READ_FROM_DPRAM((void *)&fmt_out_head, DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS, 
		    sizeof(fmt_out_head));
	READ_FROM_DPRAM((void *)&fmt_out_tail, DPRAM_PDA2PHONE_FORMATTED_TAIL_ADDRESS, 
		    sizeof(fmt_out_tail));

	READ_FROM_DPRAM((void *)&raw_in_head, DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS, 
		    sizeof(raw_in_head));
	READ_FROM_DPRAM((void *)&raw_in_tail, DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS, 
		    sizeof(raw_in_tail));
	READ_FROM_DPRAM((void *)&raw_out_head, DPRAM_PDA2PHONE_RAW_HEAD_ADDRESS, 
		    sizeof(raw_out_head));
	READ_FROM_DPRAM((void *)&raw_out_tail, DPRAM_PDA2PHONE_RAW_TAIL_ADDRESS, 
		    sizeof(raw_out_tail));
	READ_FROM_DPRAM((void *)&in_interrupt, DPRAM_PHONE2PDA_INTERRUPT_ADDRESS, 
		    DPRAM_INTERRUPT_PORT_SIZE);
	READ_FROM_DPRAM((void *)&out_interrupt, DPRAM_PDA2PHONE_INTERRUPT_ADDRESS, 
		    DPRAM_INTERRUPT_PORT_SIZE);
    READ_FROM_DPRAM((void *)&silent_reset_bit, DPRAM_SILENT_RESET_ADDR, sizeof(silent_reset_bit));

	MEMORY_UNLOCK();

#ifdef _ENABLE_ERROR_DEVICE
	memset((void *)buf, '\0', DPRAM_ERR_MSG_LEN);
	local_irq_save(flags);
	memcpy(buf, dpram_err_buf, DPRAM_ERR_MSG_LEN - 1);
	local_irq_restore(flags);
#endif	/* _ENABLE_ERROR_DEVICE */

	p += sprintf(p,
			"-------------------------------------\n"
			"| NAME\t\t\t| VALUE\n"
			"-------------------------------------\n"
			"| MAGIC CODE\t\t| 0x%04x\n"
			"| ENABLE CODE\t\t| 0x%04x\n"
			"| PHONE->PDA FMT HEAD\t| %u\n"
			"| PHONE->PDA FMT TAIL\t| %u\n"
			"| PDA->PHONE FMT HEAD\t| %u\n"
			"| PDA->PHONE FMT TAIL\t| %u\n"
			"| PHONE->PDA RAW HEAD\t| %u\n"
			"| PHONE->PDA RAW TAIL\t| %u\n"
			"| PDA->PHONE RAW HEAD\t| %u\n"
			"| PDA->PHONE RAW TAIL\t| %u\n"
			"| PHONE->PDA INT.\t| 0x%04x\n"
			"| PDA->PHONE INT.\t| 0x%04x\n"
#ifdef _ENABLE_ERROR_DEVICE
			"| LAST PHONE ERR MSG\t| %s\n"
#endif	/* _ENABLE_ERROR_DEVICE */
			"| PHONE ACTIVE\t\t| %s\n"
			"| DPRAM INT Level\t| %d\n"
			"| FORCED DPRAM WAKEUP\t| %d\n"
			"| PHONE SILENT RESET\t| %d\n"
			"-------------------------------------\n",
			magic, enable,
			fmt_in_head, fmt_in_tail, fmt_out_head, fmt_out_tail,
			raw_in_head, raw_in_tail, raw_out_head, raw_out_tail,
			in_interrupt, out_interrupt,

#ifdef _ENABLE_ERROR_DEVICE
			(buf[0] != '\0' ? buf : "NONE"),
#endif	/* _ENABLE_ERROR_DEVICE */

			(dpram_phone_getstatus() ? "ACTIVE" : "INACTIVE"),
				gpio_get_value(GPIO_nONED_INT_AP),
				forced_dpram_wakeup,
				silent_reset_bit
		);

	len = (p - page) - off;
	if (len < 0) {
		len = 0;
	}

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}
#endif /* CONFIG_PROC_FS */

/* dpram tty file operations. */
static int dpram_tty_open(struct tty_struct *tty, struct file *file)
{
	dpram_device_t *device = &dpram_table[tty->index];

	device->serial.tty = tty;
	device->serial.open_count++;

	if (device->serial.open_count > 1) {
		device->serial.open_count--;
		return -EBUSY;
	}

	#if 1	// hobac.

	if (tty->index == 1)	// dpram1
	{
		struct termios termios;
		mm_segment_t oldfs;

		oldfs = get_fs(); set_fs(get_ds());

		if (file->f_op->ioctl)
		{
			file->f_op->ioctl(file->f_dentry->d_inode, file,
					TCGETA, (unsigned long)&termios);
		}

		else if (file->f_op->unlocked_ioctl)
		{
			file->f_op->unlocked_ioctl(file, TCGETA, (unsigned long)&termios);
		}

		set_fs(oldfs);

		termios.c_cflag = CS8 | CREAD | HUPCL | CLOCAL | B115200;
		termios.c_iflag = IGNBRK | IGNPAR;
		termios.c_lflag = 0;
		termios.c_oflag = 0;
		termios.c_cc[VMIN] = 1;
		termios.c_cc[VTIME] = 1;

		oldfs = get_fs(); set_fs(get_ds());

		if (file->f_op->ioctl)
		{
			file->f_op->ioctl(file->f_dentry->d_inode, file,
					TCSETA, (unsigned long)&termios);
		}

		else if (file->f_op->unlocked_ioctl)
		{
			file->f_op->unlocked_ioctl(file, TCSETA, (unsigned long)&termios);
		}

		set_fs(oldfs);
	}

	#endif

	tty->driver_data = (void *)device;
	tty->low_latency = 1;
	return 0;
}

static void dpram_tty_close(struct tty_struct *tty, struct file *file)
{
	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (device && (device == &dpram_table[tty->index])) {
		down(&device->serial.sem);
		device->serial.open_count--;
		device->serial.tty = NULL;
		up(&device->serial.sem);
	}
}

static int dpram_tty_write(struct tty_struct *tty,
		const unsigned char *buffer, int count)
{
	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (!device) {
		return 0;
	}

	return dpram_write(device, buffer, count);
}

static int dpram_tty_write_room(struct tty_struct *tty)
{
	int avail = 0;
	u16 head, tail;
    int retval,count;
    u16 timeout = 0;
    
	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

    retval = FALSE; 

    while(retval != TRUE) {
        retval = sleep_dpram_get_authority();
        if (retval == RET_MODEM_OFF) {
            printk("[ONEDRAM] dpram_tty_write_room().. MODEM OFF Status.. \n"); 
            return -1; 
        }
        else if (retval == -2)
            timeout++; 
        else if (retval == FALSE) 
            msleep(1);
        if (timeout > 50) {
            printk("[ONEDRAM] dpram_tty_write_room() timeout .. \n");
            return -EAGAIN;
        }  
    }
	if (device != NULL) {
		READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

		avail = (head < tail) ? tail - head - 1 :
			device->out_buff_size + tail - head - 1;
	}

	MEMORY_UNLOCK();
	return avail;
}

static int dpram_tty_ioctl(struct tty_struct *tty, struct file *file,
		unsigned int cmd, unsigned long arg)
{
    unsigned int val;
    int ret;
    HeaderInformation DGS_INFO;
    
	switch (cmd) {
		case DPRAM_PHONE_ON:
			dpram_phone_on();
			return 0;

		case DPRAM_PHONEIMG_LOAD:
			dpram_phone_image_load();
			return 0;
            
        case DPRAM_PHONE_OFF:     
            dpram_phone_off(); 
            return 0;
            
		case DPRAM_NVDATA_LOAD:
		{
			struct _param_nv param;
			unsigned long retval = 0;
			retval = copy_from_user((void *)&param, (void *)arg, sizeof (param));
			retval = retval;
			dpram_nvdata_load(&param);
			return 0;
		}

		case DPRAM_PHONE_BOOTSTART:
			dpram_phone_boot_start();
			return 0;

		case DPRAM_PHONE_BOOTTYPE:
			phone_boottype = (unsigned int)arg;
			if (phone_boottype)
				printk("[DPRAM] phone LPM mode boot start!\n");
			return 0;

        case DPRAM_WAKEUP: 
            val = (unsigned int)arg;
            if (val > 0) {
               printk("[DPRAM] user can't set the dpram_wakeup bit.\n");
				return -1;
            }
            forced_dpram_wakeup = val; 
            return 0; 

        case DPRAM_PHONE_RESET:
            dpram_phone_reset();
            return 0; 
            
#ifdef _ENABLE_RAM_DUMP
        case DPRAM_RAM_DUMP:    
            ret = dpram_modem_ram_dump();
            dpram_phone_reset();
            return ret;
        case DPRAM_RAM_READ_DONE:
            dpram_modem_ram_dump_read_done();
            return 0;
#endif      
        case DPRAM_GET_DSG_INFO:
            dpram_get_dsg_info(&DGS_INFO);
            copy_to_user((void *)arg,(const void*)&DGS_INFO,sizeof(HeaderInformation));
            return 0;
        case DPRAM_SILENT_RESET: 
            dpram_silent_reset = 1;
			//dpram_phone_reset();
			//WRITE_TO_DPRAM(DPRAM_SILENT_RESET, &dpram_silent_reset, sizeof(dpram_silent_reset));
			printk("[DPRAM] modem is silently rebooted.\n");
            return 0;
		default:
			break;
	}

	return -ENOIOCTLCMD;
}

static int dpram_tty_chars_in_buffer(struct tty_struct *tty)
{
	int data = 0;
	u16 head, tail;

	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (dpram_get_authority() != TRUE) {
		return 0;
	}
	//MEMORY_LOCK();

	if (device != NULL) {
		READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

		data = (head > tail) ? head - tail - 1 :
			device->out_buff_size - tail + head;
	}

	MEMORY_UNLOCK();
	return data;
}

#ifdef _ENABLE_ERROR_DEVICE
static int dpram_err_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
	DECLARE_WAITQUEUE(wait, current);

	unsigned long flags;
	ssize_t ret;
	size_t ncopy;

	add_wait_queue(&dpram_err_wait_q, &wait);
	set_current_state(TASK_INTERRUPTIBLE);

	while (1) {
		local_irq_save(flags);

		if (dpram_err_len) {
			ncopy = min(count, dpram_err_len);

			if (copy_to_user(buf, dpram_err_buf, ncopy)) {
				ret = -EFAULT;
			}

			else {
				ret = ncopy;
			}

			dpram_err_len = 0;
			
			local_irq_restore(flags);
			break;
		}

		local_irq_restore(flags);

		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			break;
		}

		schedule();
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&dpram_err_wait_q, &wait);

	return ret;
}

static int dpram_err_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &dpram_err_async_q);
}

static unsigned int dpram_err_poll(struct file *filp,
		struct poll_table_struct *wait)
{
	poll_wait(filp, &dpram_err_wait_q, wait);
	return ((dpram_err_len) ? (POLLIN | POLLRDNORM) : 0);
}
#endif	/* _ENABLE_ERROR_DEVICE */

/* handlers. */
static void res_ack_tasklet_handler(unsigned long data)
{
	dpram_device_t *device = (dpram_device_t *)data;

	if (device && device->serial.tty) {
		struct tty_struct *tty = device->serial.tty;

		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
				tty->ldisc.ops->write_wakeup) {
			(tty->ldisc.ops->write_wakeup)(tty);
		}

		wake_up_interruptible(&tty->write_wait);
	}
}

static void send_tasklet_handler(unsigned long data)
{
	dpram_tasklet_data_t *tasklet_data = (dpram_tasklet_data_t *)data;

	dpram_device_t *device = tasklet_data->device;
	u16 non_cmd = tasklet_data->non_cmd;

	int ret = 0;

    if (modem_status == MODEM_STATUS_OFF){
        printk("[ONEDRAM] send_tasklet_handler() Modem OFF \n");
        return;
    }

    
	if (device != NULL && device->serial.tty) {
		struct tty_struct *tty = device->serial.tty;

        do {
            ret = dpram_read(device,non_cmd); 
            if (ret == -1){
                if (non_cmd & INT_MASK_SEND_F) {
                    tasklet_schedule(&fmt_send_tasklet);
                }
                if (non_cmd & INT_MASK_SEND_R) {
                    //tasklet_hi_schedule(&raw_send_tasklet);
                    tasklet_schedule(&raw_send_tasklet);
                }
                return;
            }
            else if (ret == -100) {
                if (non_cmd & INT_MASK_SEND_F) {
                    smp_wait_status |= INT_MASK_SEND_F;
                }
                if (non_cmd & INT_MASK_SEND_R) {
                    smp_wait_status |= INT_MASK_SEND_R;
                }
                return;
            }
        }while(ret); 
        
        tty_flip_buffer_push(tty);
        
        if (smp_wait_status & INT_MASK_SEND_F) {
            smp_wait_status &= ~(INT_MASK_SEND_F);
        }
        if (smp_wait_status & INT_MASK_SEND_R) {
            smp_wait_status &= ~(INT_MASK_SEND_R);
        }
	}

	else {
		dpram_drop_data(device);
	}

	/* @LDK@ modified by hobac on 2008.09.04. */
	/* @LDK@ dpram_read()에서 인터럽트 처리 */
//	if (non_cmd & device->mask_req_ack) {
//		send_interrupt_to_phone(INT_NON_COMMAND(device->mask_res_ack));
//	}
}

static int cmd_req_active_handler(void)
{
	send_interrupt_to_phone(INT_COMMAND(INT_MASK_CMD_RES_ACTIVE));
    return COMMAND_SUCCESS; 
}

static int cmd_error_display_handler(void)
{
	char buf[65];

#ifdef _ENABLE_ERROR_DEVICE
	unsigned long flags;
#endif	/* _ENABLE_ERROR_DEVICE */

	memset((void *)buf, 0, sizeof (buf));
	buf[0] = '0';
	buf[1] = ' ';

	if (dpram_get_authority() != TRUE) {
		return COMMAND_GET_AUTHORITY_FAIL;
	}
	//MEMORY_LOCK();

	READ_FROM_DPRAM((buf + 2), DPRAM_PHONE2PDA_FORMATTED_BUFFER_ADDRESS,
			sizeof (buf) - 3);

	MEMORY_UNLOCK();
	dpram_return_authority();

	printk("[PHONE ERROR] ->> %s\n", buf);

#ifdef _ENABLE_ERROR_DEVICE
	local_irq_save(flags);
	memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
	dpram_err_len = 64;
	local_irq_restore(flags);

	wake_up_interruptible(&dpram_err_wait_q);
	kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
#endif	/* _ENABLE_ERROR_DEVICE */
    return COMMAND_SUCCESS;
}

static int cmd_phone_start_handler(void)
{
	dprintk("0xc8 command execute!\n");
	return dpram_init_and_report();
}

static int cmd_req_time_sync_handler(void)
{
	/* TODO: add your codes here.. */
    return COMMAND_SUCCESS;
}

static int cmd_phone_deep_sleep_handler(void)
{
	/* TODO: add your codes here.. */
    return COMMAND_SUCCESS;
}

static int cmd_nv_rebuilding_handler(void)
{
	/* TODO: add your codes here.. */
    return COMMAND_SUCCESS;
}

static int cmd_emer_down_handler(void)
{
	/* TODO: add your codes here.. */
    return COMMAND_SUCCESS;
}

static int cmd_smp_rep_handler(void)
{
	if (smp_wait_status & INT_MASK_SEND_F) {
        tasklet_schedule(&fmt_send_tasklet);
    }
    if (smp_wait_status & INT_MASK_SEND_R) {
        //tasklet_hi_schedule(&raw_send_tasklet);
        tasklet_schedule(&raw_send_tasklet);
    } 
}

static void semaphore_control_handler(unsigned long data)
{
    int ret; 

    request_smp_modem = 1; 
    ret = dpram_return_authority(); 
    if (ret == FALSE) {
        tasklet_schedule(&semaphore_control_tasklet);
    }
    return;
}

#ifdef _COMMAND_TASKLET

static int cmd_smp_req_handler(void)
{
    int ret; 

    request_smp_modem = 1; 
    ret = dpram_return_authority(); 
    if (ret == FALSE) {
        //tasklet_schedule(&cmd_send_tasklet);
        return COMMAND_FAIL;
    }
    return COMMAND_SUCCESS;
}

#define BIT_INT_MASK_CMD_REQ_ACTIVE         0x001 
#define BIT_INT_MASK_CMD_ERR_DISPLAY        0x002
#define BIT_INT_MASK_CMD_PHONE_START        0x004
#define BIT_INT_MASK_CMD_REQ_TIME_SYNC      0x008
#define BIT_INT_MASK_CMD_PHONE_DEEP_SLEEP   0x010
#define BIT_INT_MASK_CMD_NV_REBUILDING      0x020
#define BIT_INT_MASK_CMD_EMER_DOWN          0x040
#define BIT_INT_MASK_CMD_SMP_REQ            0x080
#define BIT_INT_MASK_CMD_SMP_REP            0x100
#define BIT_MAX                             0x200

u16 tasklet_cmd_list = 0;

static void commnad_tasklet_handler(unsigned long data)
{
    if (modem_status == MODEM_STATUS_OFF){
        printk("[ONEDRAM] commnad_tasklet_handler() Modem OFF \n");
        return;
    }
        
    if (data & BIT_INT_MASK_CMD_REQ_ACTIVE){
        if (cmd_req_active_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_REQ_ACTIVE);
        }
    }
    if (data & BIT_INT_MASK_CMD_ERR_DISPLAY){
        if (cmd_error_display_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_ERR_DISPLAY);
        }
    }
    if (data & BIT_INT_MASK_CMD_PHONE_START){
        if (cmd_phone_start_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_PHONE_START);
        }
    }
    if (data & BIT_INT_MASK_CMD_REQ_TIME_SYNC){
        if (cmd_req_time_sync_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_REQ_TIME_SYNC);
        } 
    }
    if (data & BIT_INT_MASK_CMD_PHONE_DEEP_SLEEP){
        if (cmd_phone_deep_sleep_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_PHONE_DEEP_SLEEP);
        }  
    }
    if (data & BIT_INT_MASK_CMD_NV_REBUILDING){
        if (cmd_nv_rebuilding_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_NV_REBUILDING);
        } 
    }
    if (data & BIT_INT_MASK_CMD_EMER_DOWN){
        if (cmd_emer_down_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_EMER_DOWN);
        } 
    }
    if (data & BIT_INT_MASK_CMD_SMP_REQ){
        if (cmd_smp_req_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_SMP_REQ);
        }
    }
    if (data & BIT_INT_MASK_CMD_SMP_REP){
        if (cmd_smp_rep_handler() == COMMAND_SUCCESS){
            cmd_send_tasklet.data &= ~(BIT_INT_MASK_CMD_SMP_REP);
        }
    }
    if (cmd_send_tasklet.data){
        tasklet_schedule(&cmd_send_tasklet);   
    }
    return ;
}
static void command_handler(u16 data)
{
    u16 temp = 0; 
    u16 cmd = (u16)data;
    switch (cmd) {
		case INT_MASK_CMD_REQ_ACTIVE:
            temp |= BIT_INT_MASK_CMD_REQ_ACTIVE;
			break;

		case INT_MASK_CMD_ERR_DISPLAY:
            temp |= BIT_INT_MASK_CMD_ERR_DISPLAY;
			break;

		case INT_MASK_CMD_PHONE_START:
            temp |= BIT_INT_MASK_CMD_PHONE_START;
			break;

		case INT_MASK_CMD_REQ_TIME_SYNC:
            temp |= BIT_INT_MASK_CMD_REQ_TIME_SYNC;
			break;

		case INT_MASK_CMD_PHONE_DEEP_SLEEP:
            temp |= BIT_INT_MASK_CMD_PHONE_DEEP_SLEEP;
			break;

		case INT_MASK_CMD_NV_REBUILDING:
            temp |= BIT_INT_MASK_CMD_NV_REBUILDING;
			break;

		case INT_MASK_CMD_EMER_DOWN:
            temp |= BIT_INT_MASK_CMD_EMER_DOWN;
			break;

		case INT_MASK_CMD_SMP_REQ:
            temp |= BIT_INT_MASK_CMD_SMP_REQ;
			break;

		case INT_MASK_CMD_SMP_REP:
            temp |= BIT_INT_MASK_CMD_SMP_REP;
			break;
  
		default:
			dprintk("Unknown command.. %x\n", cmd);
            return ;      
	}
    cmd_send_tasklet.data |= temp; 
    tasklet_schedule(&cmd_send_tasklet); 
}
#else
static void command_handler(u16 cmd)
{
	switch (cmd) {
		case INT_MASK_CMD_REQ_ACTIVE:
			cmd_req_active_handler();
			break;

		case INT_MASK_CMD_ERR_DISPLAY:
			cmd_error_display_handler();
			break;

		case INT_MASK_CMD_PHONE_START:
			cmd_phone_start_handler();
			break;

		case INT_MASK_CMD_REQ_TIME_SYNC:
			cmd_req_time_sync_handler();
			break;

		case INT_MASK_CMD_PHONE_DEEP_SLEEP:
			cmd_phone_deep_sleep_handler();
			break;

		case INT_MASK_CMD_NV_REBUILDING:
			cmd_nv_rebuilding_handler();
			break;

		case INT_MASK_CMD_EMER_DOWN:
			cmd_emer_down_handler();
			break;

		case INT_MASK_CMD_SMP_REQ:
            //tasklet_enable(&semaphore_control_tasklet);
			tasklet_schedule(&semaphore_control_tasklet);
			break;

		case INT_MASK_CMD_SMP_REP:
			cmd_smp_rep_handler();
			break;

		default:
			dprintk("Unknown command.. %x\n", cmd);
	}
}

#endif
static void non_command_handler(u16 non_cmd)
{
	u16 head, tail;

	if (non_cmd & INT_MASK_SEND_F) {
		dpram_tasklet_data[FORMATTED_INDEX].device = &dpram_table[FORMATTED_INDEX];
		dpram_tasklet_data[FORMATTED_INDEX].non_cmd = non_cmd;
        forced_dpram_wakeup = 1;
		fmt_send_tasklet.data = (unsigned long)&dpram_tasklet_data[FORMATTED_INDEX];
		tasklet_schedule(&fmt_send_tasklet);
	}

	if (non_cmd & INT_MASK_SEND_R) {
		dpram_tasklet_data[RAW_INDEX].device = &dpram_table[RAW_INDEX];
		dpram_tasklet_data[RAW_INDEX].non_cmd = non_cmd;

		raw_send_tasklet.data = (unsigned long)&dpram_tasklet_data[RAW_INDEX];
		tasklet_schedule(&raw_send_tasklet);
	}

	if (non_cmd & INT_MASK_RES_ACK_F) {
		tasklet_schedule(&fmt_res_ack_tasklet);
	}

	if (non_cmd & INT_MASK_RES_ACK_R) {
		//tasklet_hi_schedule(&raw_res_ack_tasklet);
		tasklet_schedule(&raw_res_ack_tasklet);
	}
}


/* @LDK@ interrupt handlers. */
static irqreturn_t dpram_irq_handler(int irq, void *dev_id)
{
	u16 irq_mask = 0;
    u32 temp; 
	READ_FROM_DPRAM(&irq_mask, DPRAM_PHONE2PDA_INTERRUPT_ADDRESS, sizeof(irq_mask));

	/* valid bit verification. @LDK@ */
	if (!(irq_mask & INT_MASK_VALID)) {
		if(irq_mask == 0x1234) {
			printk("  - Phone Init OK.\n");
			return IRQ_HANDLED;
		}
		else {
			printk("Invalid interrupt mask: 0x%04x\n", irq_mask);
			return IRQ_NONE;
		}
	}

	if (irq_mask == 0xabcd) {
		return IRQ_HANDLED;
	}

	/* command or non-command? @LDK@ */
	if (irq_mask & INT_MASK_COMMAND) {
		irq_mask &= ~(INT_MASK_VALID | INT_MASK_COMMAND);
		command_handler(irq_mask);
	}

	else {
		irq_mask &= ~INT_MASK_VALID;
		non_command_handler(irq_mask);
	}

    temp = __raw_readl(S3C64XX_EINT0PEND);
    temp |= 0x01; 
    __raw_writel(temp,S3C64XX_EINT0PEND);
	return IRQ_HANDLED;
}

static void phone_error_off_message_handler(int status)
{
	char buf[65];

#ifdef _ENABLE_ERROR_DEVICE
	unsigned long flags;
#endif	/* _ENABLE_ERROR_DEVICE */

	memset((void *)buf, 0, sizeof (buf));

	if(status == MESG_PHONE_OFF) {
		sprintf(buf, "%d ModemOff", status);
	}

	else if(status == MESG_PHONE_POR_RESET) {
		sprintf(buf, "%d Modem POR RESET", status);
	} 
#ifdef _ENABLE_RAM_DUMP
    else if (status == MESG_PHONE_RAMDUMP_RESET) {
        sprintf(buf, "%d Modem RAM DUMP RESET", status);
    }
#endif
    else if (status == MESG_PHONE_ABNORMAL_RESET) {
        sprintf(buf, "%d Modem ABNORMAL RESET", status);
    }

	printk("[PHONE ERROR] ->> %s\n", buf);

#ifdef _ENABLE_ERROR_DEVICE
	local_irq_save(flags);
	memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
	dpram_err_len = 64;
	local_irq_restore(flags);

	wake_up_interruptible(&dpram_err_wait_q);
	kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
#endif	/* _ENABLE_ERROR_DEVICE */
}


static irqreturn_t phone_active_irq_handler(int irq, void *dev_id)
{
	printk("[IRQ] IRQ PHONE_ACTIVE  level(%d)\n", gpio_get_value(GPIO_PHONE_ACTIVE_AP));
    if(gpio_get_value(GPIO_PHONE_ACTIVE_AP)) {
        printk("PHONE ACTIVE : RESET \n");
        if (modem_status == MODEM_STATUS_OFF) {
            printk("PHONE_POWER_ON_RESET .. \n");
            phone_error_off_message_handler(MESG_PHONE_POR_RESET);
            modem_status = MODEM_STATUS_RESET;  
        }
        else {
#ifdef _ENABLE_RAM_DUMP             
            if (modem_ram_dump_status == 1) {
                 printk("PHONE_POWER_ON_RESET .. \n");
                 phone_error_off_message_handler(MESG_PHONE_RAMDUMP_RESET);
                 modem_status = MODEM_STATUS_RESET;  
            }
            else {
                 printk("PHONE_ABNORMAL_RESET .. \n");
                 phone_error_off_message_handler(MESG_PHONE_ABNORMAL_RESET);
                 modem_status = MODEM_STATUS_RESET;  
            }
#else 
            printk("PHONE_ABNORMAL_RESET .. \n");
            phone_error_off_message_handler(MESG_PHONE_ABNORMAL_RESET);
            modem_status = MODEM_STATUS_RESET;  
#endif 
        }
    }
    else {
        printk("PHONE ACTIVE : POWER OFF \n");
        phone_error_off_message_handler(MESG_PHONE_OFF);
        modem_status = MODEM_STATUS_OFF; 
    }
	return IRQ_HANDLED;
}

/* basic functions. */
#ifdef _ENABLE_ERROR_DEVICE
static struct file_operations dpram_err_ops = {
	.owner = THIS_MODULE,
	.read = dpram_err_read,
	.fasync = dpram_err_fasync,
	.poll = dpram_err_poll,
	.llseek = no_llseek,

	/* TODO: add more operations */
};
#endif	/* _ENABLE_ERROR_DEVICE */

static struct tty_operations dpram_tty_ops = {
	.open 		= dpram_tty_open,
	.close 		= dpram_tty_close,
	.write 		= dpram_tty_write,
	.write_room = dpram_tty_write_room,
	.ioctl 		= dpram_tty_ioctl,
	.chars_in_buffer = dpram_tty_chars_in_buffer,

	/* TODO: add more operations */
};

#ifdef _ENABLE_ERROR_DEVICE

static void unregister_dpram_err_device(void)
{
	unregister_chrdev(DRIVER_MAJOR_NUM, DPRAM_ERR_DEVICE);
	class_destroy(dpram_class);
}

static int register_dpram_err_device(void)
{
	/* @LDK@ 1 = formatted, 2 = raw, so error device is '0' */
	struct device *dpram_err_dev_t;
	int ret = register_chrdev(DRIVER_MAJOR_NUM, DPRAM_ERR_DEVICE, &dpram_err_ops);

	if ( ret < 0 )
	{
		return ret;
	}

	dpram_class = class_create(THIS_MODULE, "err");

	if (IS_ERR(dpram_class))
	{
		unregister_dpram_err_device();
		return -EFAULT;
	}

	dpram_err_dev_t = device_create(dpram_class, NULL,
			MKDEV(DRIVER_MAJOR_NUM, 0), NULL, DPRAM_ERR_DEVICE);

	if (IS_ERR(dpram_err_dev_t))
	{
		unregister_dpram_err_device();
		return -EFAULT;
	}

	return 0;
}
#endif	/* _ENABLE_ERROR_DEVICE */

static int register_dpram_driver(void)
{
	int retval = 0;

	/* @LDK@ allocate tty driver */
	dpram_tty_driver = alloc_tty_driver(MAX_INDEX);

	if (!dpram_tty_driver) {
		return -ENOMEM;
	}

	/* @LDK@ initialize tty driver */
	dpram_tty_driver->owner = THIS_MODULE;
	dpram_tty_driver->driver_name = DRIVER_NAME;
	dpram_tty_driver->name = "dpram";
	dpram_tty_driver->major = DRIVER_MAJOR_NUM;
	dpram_tty_driver->minor_start = 1;
	dpram_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	dpram_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	dpram_tty_driver->flags = TTY_DRIVER_REAL_RAW;
	dpram_tty_driver->init_termios = tty_std_termios;
	dpram_tty_driver->init_termios.c_cflag =
		(B115200 | CS8 | CREAD | CLOCAL | HUPCL);

	tty_set_operations(dpram_tty_driver, &dpram_tty_ops);

	dpram_tty_driver->ttys = dpram_tty;
	dpram_tty_driver->termios = dpram_termios;
	dpram_tty_driver->termios_locked = dpram_termios_locked;

	/* @LDK@ register tty driver */
	retval = tty_register_driver(dpram_tty_driver);

	if (retval) {
		dprintk("tty_register_driver error\n");
		put_tty_driver(dpram_tty_driver);
		return retval;
	}

	return 0;
}

static void unregister_dpram_driver(void)
{
	tty_unregister_driver(dpram_tty_driver);
}

static void init_devices(void)
{
	int i;

	for (i = 0; i < MAX_INDEX; i++) {
		init_MUTEX(&dpram_table[i].serial.sem);

		dpram_table[i].serial.open_count = 0;
		dpram_table[i].serial.tty = NULL;
	}
}

static void init_hw_setting(void)
{
    u32 temp = 0;
	/* initial pin settings - dpram driver control */
	s3c_gpio_cfgpin(GPIO_PHONE_ACTIVE_AP, S3C_GPIO_SFN(GPIO_PHONE_ACTIVE_AP_AF));
	s3c_gpio_setpull(GPIO_PHONE_ACTIVE_AP, S3C_GPIO_PULL_DOWN); 
	set_irq_type(IRQ_PHONE_ACTIVE, IRQ_TYPE_EDGE_BOTH);
    //set gpio filter 
    temp = __raw_readl(S3C64XX_EINT0FLTCON0);
    printk("[ONEDRAM] init_hw_setting filter value : %x\n",temp);
    temp |= 0x80000000; 
    __raw_writel(temp,S3C64XX_EINT0FLTCON0);
    printk("[ONEDRAM] init_hw_setting filter value : %x\n",temp);

	s3c_gpio_cfgpin(GPIO_nONED_INT_AP, S3C_GPIO_SFN(GPIO_nONED_INT_AP_AF));
//	s3c_gpio_setpull(GPIO_nONED_INT_AP, S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(GPIO_nONED_INT_AP, S3C_GPIO_PULL_UP); 
//	set_irq_type(IRQ_nONED_INT_AP, IRQ_TYPE_EDGE_FALLING);
	set_irq_type(IRQ_nONED_INT_AP, IRQ_TYPE_LEVEL_LOW);

	s3c_gpio_cfgpin(GPIO_AP_FLM_RXD, S3C_GPIO_SFN(GPIO_AP_FLM_RXD_AF));
	s3c_gpio_setpull(GPIO_AP_FLM_RXD, S3C_GPIO_PULL_NONE); 
	s3c_gpio_cfgpin(GPIO_AP_FLM_TXD, S3C_GPIO_SFN(GPIO_AP_FLM_TXD_AF));
	s3c_gpio_setpull(GPIO_AP_FLM_TXD, S3C_GPIO_PULL_NONE); 

    s3c_gpio_setpin(GPIO_PHONE_ON, GPIO_LEVEL_LOW);
    s3c_gpio_setpin(GPIO_USIM_BOOT, GPIO_LEVEL_LOW);
    s3c_gpio_setpin(GPIO_FLM_SEL, GPIO_LEVEL_LOW); 
    s3c_gpio_setpin(GPIO_CP_RST, GPIO_LEVEL_LOW); 
    s3c_gpio_setpin(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);

}

static void kill_tasklets(void)
{
	tasklet_kill(&fmt_res_ack_tasklet);
	tasklet_kill(&raw_res_ack_tasklet);

	tasklet_kill(&fmt_send_tasklet);
	tasklet_kill(&raw_send_tasklet);
    tasklet_kill(&cmd_send_tasklet);
}

static int register_interrupt_handler(void)
{

	unsigned int dpram_irq, phone_active_irq;
	int retval = 0;
	
	dpram_irq = IRQ_nONED_INT_AP;
	phone_active_irq = IRQ_PHONE_ACTIVE;

	/* @LDK@ interrupt area read - pin level will be driven high. */
	dpram_clear();

	/* @LDK@ dpram interrupt */
	retval = request_irq(dpram_irq, dpram_irq_handler, IRQF_DISABLED, "dpram irq", NULL);

	if (retval) {
		dprintk("DPRAM interrupt handler failed.\n");
		unregister_dpram_driver();
		return -1;
	}

#if 1	/* kernel stop? -_-? */
	/* @LDK@ phone active interrupt */
	retval = request_irq(phone_active_irq, phone_active_irq_handler, IRQF_DISABLED, "Phone Active", NULL);

	if (retval) {
		dprintk("Phone active interrupt handler failed.\n");
		free_irq(phone_active_irq, NULL);
		unregister_dpram_driver();
		return -1;
	}
#endif
	return 0;
}

static void check_miss_interrupt(void)
{
	unsigned long flags;

	if (gpio_get_value(GPIO_PHONE_ACTIVE_AP) &&
			gpio_get_value(GPIO_nONED_INT_AP)) {
		dprintk("there is a missed interrupt. try to read it!\n");

		local_irq_save(flags);
		dpram_irq_handler(IRQ_nONED_INT_AP, NULL);
		local_irq_restore(flags);
	}
}

static int dpram_suspend(struct platform_device *dev, pm_message_t state)
{
    s3c_gpio_setpin(GPIO_PDA_ACTIVE, GPIO_LEVEL_LOW);
#if 0    
	gpio_set_value(GPIO_PDA_ACTIVE, 0);
#endif 
	return 0;
}

static int dpram_resume(struct platform_device *dev)
{
    s3c_gpio_setpin(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);
//	check_miss_interrupt();
	return 0;
}

static int __devinit dpram_probe(struct platform_device *dev)
{
	int retval;

	/* @LDK@ register dpram (tty) driver */
	retval = register_dpram_driver();
	if (retval) {
		dprintk("Failed to register dpram (tty) driver.\n");
		return -1;
	}

#ifdef _ENABLE_ERROR_DEVICE
	/* @LDK@ register dpram error device */
	retval = register_dpram_err_device();
	if (retval) {
		dprintk("Failed to register dpram error device.\n");

		unregister_dpram_driver();
		return -1;
	}

	memset((void *)dpram_err_buf, '\0', sizeof dpram_err_buf);
#endif /* _ENABLE_ERROR_DEVICE */

#ifdef _ENABLE_RAM_DUMP
    retval = register_modem_ram_dump_device();
    if (retval < 0){
        printk("[ONEDRAM] register modem ram dump device failed .. \n");
        unregister_modem_ram_dump_device();
        return retval; 
    }
#endif
	/* @LDK@ H/W setting */
	init_hw_setting();

	/* @LDK@ register interrupt handler */
	if ((retval = register_interrupt_handler()) < 0) {
		return -1;
	}

	/* @LDK@ initialize device table */
	init_devices();

#ifdef CONFIG_PROC_FS
	create_proc_read_entry(DRIVER_PROC_ENTRY, 0, 0, dpram_read_proc, NULL);
#endif	/* CONFIG_PROC_FS */

	/* @LDK@ check out missing interrupt from the phone */
//	check_miss_interrupt();

	printk("%s\n", DRIVER_ID);

	return 0;
}

static int __devexit dpram_remove(struct platform_device *dev)
{
	/* @LDK@ unregister dpram (tty) driver */
	unregister_dpram_driver();

	/* @LDK@ unregister dpram error device */
#ifdef _ENABLE_ERROR_DEVICE
	unregister_dpram_err_device();
#endif
	/* @LDK@ unregister irq handler */
	free_irq(IRQ_nONED_INT_AP, NULL);
//	free_irq(IRQ_PHONE_ACTIVE, NULL);

	kill_tasklets();

	return 0;
}

static struct platform_driver platform_dpram_driver = {
	.probe		= dpram_probe,
	.remove		= __devexit_p(dpram_remove),
	.suspend	= dpram_suspend,
	.resume 	= dpram_resume,
	.driver	= {
		.name	= "dpram",
	},
};

/* init & cleanup. */
static int __init dpram_init(void)
{
	return platform_driver_register(&platform_dpram_driver);
}

static void __exit dpram_exit(void)
{
	platform_driver_unregister(&platform_dpram_driver);
}

module_init(dpram_init);
module_exit(dpram_exit);

MODULE_AUTHOR("SAMSUNG ELECTRONICS CO., LTD");
MODULE_DESCRIPTION("Onedram Device Driver.");
MODULE_LICENSE("GPL");

