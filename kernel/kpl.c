/*
 *  linux/kernel/panic2disk.c
 */

#include <mach/kpl.h> 

#include <linux/rtc.h>
#include <asm/mach/time.h>

static struct KPL_log_fops *KPL_ops = NULL;

char *panic_dump = NULL;
int start_sector, start_block, current_sector=0;

extern int die_counter;
EXPORT_SYMBOL(panic_dump);

void KPL_register(struct KPL_log_fops *fops)
{
	KPL_ops=fops; 
}
	
EXPORT_SYMBOL(KPL_register);

int   KPL_Read(unsigned int  nVol,  unsigned int  nVsn,  unsigned int nNumOfScts, u8  *pMBuf, u8  *pSBuf, unsigned int nFlag)
{
	if  (KPL_ops && KPL_ops->KPL_read)
		return KPL_ops->KPL_read(nVol,nVsn,nNumOfScts,pMBuf,pSBuf,nFlag);
	else
		return BML_CRITICAL_ERROR; 
}

int   KPL_Write(unsigned int  nVol,  unsigned int  nVsn,  unsigned int nNumOfScts, u8  *pMBufs, u8  *pSBuf, unsigned int nFlag)
{
	if  (KPL_ops && KPL_ops->KPL_write)
		return KPL_ops->KPL_write(nVol,nVsn,nNumOfScts,pMBufs,pSBuf,nFlag);
	else	
		return BML_CRITICAL_ERROR; 
}

int   KPL_EraseBlk(unsigned int  nVol,  unsigned int  nVbn,  unsigned int nFlag)
{
	if  (KPL_ops && KPL_ops->KPL_eraseblk)
		return KPL_ops->KPL_eraseblk(nVol,nVbn,nFlag); 
	else
		return BML_CRITICAL_ERROR; 
}

int   KPL_IOCtl(unsigned int  nVol, unsigned int nCode, u8 *pBufIn, unsigned int nLenIn , u8 *pBufOut, unsigned int nLenOut, unsigned int *pByteReturned)
{
	if  (KPL_ops && KPL_ops->KPL_ioctl)
		return KPL_ops->KPL_ioctl(nVol,nCode,pBufIn,nLenIn,pBufOut,nLenOut,pByteReturned);
	else		
		return BML_CRITICAL_ERROR; 
}
int   KPL_Open(void)
{
	if  (KPL_ops && KPL_ops->KPL_open)
		return KPL_ops->KPL_open(0); 
	else
		return BML_CRITICAL_ERROR; 
}
int   KPL_Close(void)
{
	if  (KPL_ops && KPL_ops->KPL_open)
		return KPL_ops->KPL_close(0); 
	else
		return BML_CRITICAL_ERROR; 
}
BMLVolSpec	*KPL_get_vol_spec(void)
{
	if  (KPL_ops && KPL_ops->KPL_xsr_get_vol_spec)
		return KPL_ops->KPL_xsr_get_vol_spec(0); 
	else
		return NULL; 

}
XSRPartI	*KPL_get_part_spec(void)
{
	if  (KPL_ops && KPL_ops->KPL_xsr_get_part_spec)
		return KPL_ops->KPL_xsr_get_part_spec(0); 
	else
		return NULL; 
}

void free_panic_log_buffer(void)
{

	return;
}

static int panic_dump_len=0;
void  KPL_PRINTK(const char* fmt, ...)
{
	va_list args;
	char dump_buf[1024]={0,};
	int dump_len;

	if(panic_dump==NULL)
		return;

	va_start(args, fmt);
	//r = vprintk(fmt, args);
	dump_len = vscnprintf(dump_buf, 1024, fmt, args);
	memcpy(panic_dump + panic_dump_len, dump_buf, dump_len);

	panic_dump_len+=dump_len;
	va_end(args);

	return;
}
EXPORT_SYMBOL(KPL_PRINTK);
static int panic_count=0;
static int panic_initialize=0;
void alloc_panic_log_buffer(void)
{
	struct timeval dump_time;
	struct rtc_time tm;

	if(!panic_dump) {
		panic_dump = (void *)__get_free_pages(GFP_ATOMIC, 0);
	}
	if(KPL_ops!=NULL)
		panic_initialize=1;

	panic_dump_len=0;
	memset(panic_dump,0x00,4096);
	do_gettimeofday(&dump_time);
	rtc_time_to_tm(dump_time.tv_sec, &tm);
	
	tm.tm_year+=1900;
	tm.tm_mon+=1;

	KPL_PRINTK("==============================================================\n");
	KPL_PRINTK("   %dth oops     TIME :", panic_count++); //%9i.%06i\n",++die_counter,(int)tv.tv_sec, (int)tv.tv_usec);
	KPL_PRINTK("  [%04d-%02d-%02d] %02d:%02d:%0d\n",tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	KPL_PRINTK("==============================================================\n");
}


int write_join_buffer(void *write_buf)
{
	int i=0, ret=0, j=0;

	if(panic_initialize==0)
		return -1;

	//int write_buf_len=strlen(write_buf);
#if 0
	printk(KERN_ERR"[%s/%d] current sector=%d\n", __FUNCTION__, __LINE__, current_sector);
	printk(KERN_ERR"[%s/%d] wrtie_buf=%d\n", __FUNCTION__, __LINE__, write_buf_len);
#endif

	//memset(block_buf, 0x00, 4096);
#if 0
	for(i=start_sector ; i < (start_sector + current_sector) ; i+=SPP, j++)
	{
		memset(block_buf + SIZE_PER_PAGE*j, 0x00, SIZE_PER_PAGE);
		ret = KPL_Read(0, i, SPP, block_buf + SIZE_PER_PAGE*j, NULL, BML_FLAG_ECC_ON);
		if(ret)
		{
			ret=-1;
			printk(KERN_ERR "BML_Read[%d]: error = 0x%x\n", __LINE__, ret);
			break;
		}
	}
	memcpy(block_buf + j*SIZE_PER_PAGE , write_buf, 4096);

	if(j> 62)
	{
		printk(KERN_ERR"[%s] Panic Log Buffer is Full\n", __FUNCTION__);
		return 0;
	}
#endif
	if(current_sector==0)
	{
		ret = KPL_EraseBlk(0, start_block, BML_FLAG_SYNC_OP);
		if(ret)
			printk(KERN_ERR "BML_EraseBlk: error = 0x%x\n", ret);
	}


	//printk(KERN_ERR"[%s/%d] current sector=%d\n", __FUNCTION__, __LINE__, current_sector);

	for(i=start_sector+current_sector,j=0 ; i< start_sector + current_sector + 8 ; i+=SPP, j++)
		ret = KPL_Write(0, i , SPP,  write_buf + PAGE_SIZE*j, NULL, BML_FLAG_SYNC_OP | BML_FLAG_ECC_ON);

	current_sector+=8;

	if(ret != BML_SUCCESS)
		printk(KERN_ERR "BML_Write: error = 0x%x\n", ret);

	return ret;
}


int panic_log_write(void)
{
	int ret=0, i;
	int nblock;
	BMLVolSpec *vs;
	XSRPartI *pi;
//	int nBytesReturned;

	if(panic_initialize==0)
		return -1;

	vs = KPL_get_vol_spec();
	
	pi = KPL_get_part_spec();

	for (i = 0 ; i < pi->nNumOfPartEntry ; i++) {
		if (pi->stPEntry[i].nID == PANIC_PARTITON_ID) {
			break;
		}
	}
	start_sector = (pi->stPEntry[i].n1stVbn) * SPB;
	start_block = (pi->stPEntry[i].n1stVbn);
	nblock = pi->stPEntry[i].nNumOfBlks;


	ret = KPL_Open();
	if(ret != BML_SUCCESS)
		printk(KERN_ERR "BML Open: error = 0x%x\n", ret);
#if  0
	ret = KPL_IOCtl(0, BML_IOCTL_UNLOCK_WHOLEAREA, NULL, 0, NULL, 0, &nBytesReturned);
	if(ret != BML_SUCCESS) {
		printk(KERN_ERR "BML IOCTL: error = 0x%x\n", ret);
	}
#endif
	 write_join_buffer(panic_dump);

	KPL_Close();

	printk("[LINUSYS] : Complete Kernel Panic Log\n");

	return ret;
}
