








#include "SynaAbstractionLayer.h"



#include "UserI2C.h"
#include "SynaI2C.h"

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/sched.h>
#include "dprintk.h"
#include "../touchscreen.h"



struct RMI4FunctionDescriptor
{
  unsigned char QueryBase;
  unsigned char CommandBase;
  unsigned char ControlBase;
  unsigned char DataBase;
  unsigned char IntSourceCount;
  unsigned char ID;
};
struct RMI4FunctionDescriptor SynaPdtF34Flash;
struct RMI4FunctionDescriptor SynaPdtF01Common;

  
unsigned int   SynaRet;
unsigned short SynaBootloadID;
unsigned short SynabootloadImgID;
unsigned char  SynafirmwareImgVersion;
unsigned char *SynafirmwareImgData;
unsigned char *SynaconfigImgData;
unsigned short SynafirmwareBlockSize;
unsigned short SynafirmwareBlockCount;
unsigned short SynaconfigBlockSize;
unsigned short SynaconfigBlockCount;
unsigned char  SynauPageData[0x200];
unsigned char  SynauStatus;
unsigned long  SynafirmwareImgSize;
unsigned long  SynaconfigImgSize;
unsigned long  SynafileSize;


unsigned short SynauF01RMI_DataBase;
unsigned short SynauF34Reflash_BlockNum;
unsigned short SynauF34Reflash_BlockData;
unsigned short SynauF34Reflash_FlashControl;
unsigned short SynauF34ReflashQuery_BootID;
unsigned short SynauF34ReflashQuery_FirmwareBlockSize;
unsigned short SynauF34ReflashQuery_FirmwareBlockCount;
unsigned short SynauF34ReflashQuery_ConfigBlockSize;
unsigned short SynauF34ReflashQuery_ConfigBlockCount;
unsigned short SynauF34ReflashQuery_FlashPropertyQuery;

unsigned char SynaFirmware[11010]; 
EXPORT_SYMBOL(SynaFirmware);


void SynaSpecialCopyEndianAgnostic(unsigned char *dest, unsigned short src);
unsigned short SynaGetConfigSize(void);
unsigned short SynaGetFirmwareSize(void);

int SynaCheckIfFatalError(unsigned int errCode, char* func_name);
int SynaWaitATTN(int errorCount);
int SynaReadFirmwareInfo(void);
int SynaReadConfigInfo(void);
unsigned int SynaIsExpectedRegFormat(void);
void SynaSetFlashAddrForDifFormat(void);
int SynaReadPageDescriptionTable(void);
int SynaInitialize(void);
unsigned int SynaReadBootloadID(void);
unsigned int SynaWriteBootloadID(void);
int SynaEnableFlashing(void);
int SynaProgramConfiguration(void);
unsigned int SynaFinalizeFlash(void);
void SynaCalculateChecksum(unsigned short * data, unsigned short len, unsigned long * dataBlock);
void print_screen_touch_firmware_update_info(void);
int SynaFlashFirmwareWrite(void);
int SynaProgramFirmware(void);
unsigned long ExtractLongFromHeader(const unsigned char* SynaImage);
int SynaReadFirmwareHeader(void);



void SynaSpecialCopyEndianAgnostic(unsigned char *dest, unsigned short src) 
{
  dest[0] = src%0x100;
  dest[1] = src/0x100;  
}

unsigned short SynaGetConfigSize()
{ 
  return SynaconfigBlockSize*SynaconfigBlockCount;
}

unsigned short SynaGetFirmwareSize() 
{
  return SynafirmwareBlockSize*SynafirmwareBlockCount;
}







int SynaCheckIfFatalError(unsigned int errCode, char* func_name)
{


  if(errCode != ESuccess)
  {

	printk("\n[TSP][ERROR] %s() Fatal error : %d\n", func_name, errCode);






	return 1;
  }
  return 0;
}


int SynaWaitATTN(int errorCount)
{
  int uErrorCount = 0;

  if (SynaWaitForATTN(DefaultMillisecondTimeout))   
  {
    if(SynaCheckIfFatalError(EErrorTimeout, "SynaWaitATTN") != 0)
      return 1;
  }

  



  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    
    if(SynaRet && uErrorCount < errorCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    if(SynaCheckIfFatalError(SynaRet, "SynaWaitATTN") != 0)
      return 1;
    
    
    SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);

    if(SynaCheckIfFatalError(SynaRet, "SynaWaitATTN") != 0 )
      return 1;
  } while( SynauPageData[0] != 0x80 && uErrorCount < errorCount); 

  return 0;
}


int SynaReadFirmwareInfo()
{
  unsigned char uData[2];

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FirmwareBlockSize, &uData[0], 2);
  if(SynaCheckIfFatalError(SynaRet, "SynaReadFirmwareInfo")!=0)
	return 1;

  SynafirmwareBlockSize = uData[0] | (uData[1] << 8);

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FirmwareBlockCount, &uData[0], 2);
  if(SynaCheckIfFatalError(SynaRet, "SynaReadFirmwareInfo") !=0)
	return 1;

  SynafirmwareBlockCount = uData[0] | (uData[1] << 8);
 
  SynafirmwareImgSize = SynafirmwareBlockCount*SynafirmwareBlockSize;
  return 0;
}


int SynaReadConfigInfo()
{
  unsigned char uData[2];
  
  SynaRet = SynaReadRegister(SynauF34ReflashQuery_ConfigBlockSize, &uData[0], 2);
  if(SynaCheckIfFatalError(SynaRet, "SynaReadConfigInfo")!=0) {
	return 1;
  }
  
  SynaconfigBlockSize = uData[0] | (uData[1] << 8);

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_ConfigBlockCount, &uData[0], 2);
  if(SynaCheckIfFatalError(SynaRet, "SynaReadConfigInfo") != 0) {
	return 1;
  }
  
  SynaconfigBlockCount = uData[0] | (uData[1] << 8);

  SynaconfigImgSize = SynaconfigBlockSize*SynaconfigBlockCount;
  return 0;
}





unsigned int SynaIsExpectedRegFormat()
{ 
  


  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FlashPropertyQuery, &SynauPageData[0], 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaIsExpectedRegFormat") != 0) {
	printk("SynaIsExpectedRegFormat fail\n");
  }

  return ((unsigned int)((SynauPageData[0] & 0x01) == 0x01));
}

void SynaSetFlashAddrForDifFormat()
{
  if (SynaIsExpectedRegFormat())
  {
    SynauF34Reflash_FlashControl = SynaPdtF34Flash.DataBase + SynafirmwareBlockSize + 2;
    SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase;
    SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 2;
  }
  else {
    SynauF34Reflash_FlashControl = SynaPdtF34Flash.DataBase;
    SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase + 1;
    SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 3;
  }
}



int SynaReadPageDescriptionTable() 
{
  struct RMI4FunctionDescriptor Buffer;
  unsigned short uAddress;

  SynaPdtF01Common.ID = 0;
  SynaPdtF34Flash.ID = 0;

  for (uAddress = 0xe9; uAddress > 0xbf; uAddress -= sizeof(struct RMI4FunctionDescriptor))
  {
    SynaRet = SynaReadRegister(uAddress, (unsigned char*)&Buffer, sizeof(Buffer));
    if(SynaCheckIfFatalError(SynaRet, "SynaReadPageDescriptionTable") != 0)
      return 1;

    if(Buffer.ID == 0)
	break;
    switch(Buffer.ID)
    {
      case 0x34:
        SynaPdtF34Flash = Buffer;
        break;
      case 0x01:
        SynaPdtF01Common = Buffer;
        break;
    }

  }

  
  SynauF01RMI_DataBase = SynaPdtF01Common.DataBase;

  SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase;
  SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 2;
  SynauF34ReflashQuery_BootID = SynaPdtF34Flash.QueryBase;

  SynauF34ReflashQuery_FlashPropertyQuery = SynaPdtF34Flash.QueryBase + 2;
  SynauF34ReflashQuery_FirmwareBlockSize = SynaPdtF34Flash.QueryBase + 3;
  SynauF34ReflashQuery_FirmwareBlockCount = SynaPdtF34Flash.QueryBase + 5;

  SynauF34ReflashQuery_ConfigBlockSize = SynaPdtF34Flash.QueryBase + 3;
  SynauF34ReflashQuery_ConfigBlockCount = SynaPdtF34Flash.QueryBase + 7;

  SynaSetFlashAddrForDifFormat();
  return 0;
}

int SynaInitialize()
{  
  unsigned char uPage = 0x00;

  
  SynaRet = SynaWriteRegister(0xff, &uPage, 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaInitialize")!=0) {
	return 1;
  }

  do
  {
    SynaRet = SynaReadRegister(0, &SynauStatus, 1);
    if(SynaCheckIfFatalError(SynaRet, "SynaInitialize")!=0) {
	return 1;
    }

    if(SynauStatus & 0x80)
    {
      
      break;
    }

  } while(SynauStatus & 0x40);

  
  SynaReadPageDescriptionTable();

  
  if(SynaPdtF34Flash.ID == 0)
  {

	printk("Function $34 is not supported\n");

	return 1;
  }
  
  if(SynaPdtF01Common.ID == 0)
  {

	printk("Function $01 is not supported\n");

	return 1;
  }

  
  SynafirmwareImgData = 0;
  SynaconfigImgData = 0;
  return 0;
}


unsigned int SynaReadBootloadID()
{
  unsigned char uData[2];

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_BootID, &uData[0], 2);

  SynaBootloadID = (unsigned int)uData[0] + (unsigned int)uData[1]*0x100;

  return SynaRet;
}


unsigned int SynaWriteBootloadID()
{
  unsigned char uData[2];

  SynaSpecialCopyEndianAgnostic(uData, SynaBootloadID);

  SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, &uData[0], 2);

  return SynaRet;
}


int SynaEnableFlashing()
{
  int uErrorCount = 0;
  unsigned char uData[2];

  
  SynaRet = SynaReadBootloadID();
  if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
  }

  
  SynaRet = SynaWriteBootloadID();
  if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
  }

  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    
    if(SynaRet && uErrorCount < DefaultErrorRetryCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
    }

    
    SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);
    if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
    }

  } while(((SynauPageData[0] & 0x0f) != 0x00) && (uErrorCount < DefaultErrorRetryCount)); 
  
  
  uData[0] = 0x0f;
  SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
  }

  mdelay(10);
  
  SynaReadPageDescriptionTable();

  mdelay(10);
  
  SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaEnableFlashing") != 0) {
	return 1;
  }
  
  if ( uData[0] != 0x80 ) 
  {

	printk("\nFlash failed\n");

	return 1;
  }
  return 0;
}




int SynaProgramConfiguration()
{
  unsigned char uData[2];
  unsigned char *puData = SynaconfigImgData;
  unsigned short blockNum;

  for (blockNum = 0; blockNum < SynaconfigBlockCount; blockNum++)
  {
    SynaSpecialCopyEndianAgnostic(&uData[0], blockNum);

    
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockNum, &uData[0], 2);
    if(SynaCheckIfFatalError(SynaRet, "SynaProgramConfiguration") != 0) {
	return 1;
    }
    
    
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, puData, SynaconfigBlockSize);
    if(SynaCheckIfFatalError(SynaRet, "SynaProgramConfiguration") != 0) {
	return 1;
    }
    
    puData += SynaconfigBlockSize;
    
    
    uData[0] = 0x06;
    SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
    if(SynaCheckIfFatalError(SynaRet, "SynaProgramConfiguration") != 0) {
	return 1;
    }

    
    if(SynaWaitATTN(DefaultErrorRetryCount) != 0)
      return 1;
  }
  return 0;
}

unsigned int SynaFinalizeFlash()
{
  unsigned char uData[2];
  unsigned int uErrorCount = 0;

  
  uData[0] = 1;
  SynaRet = SynaWriteRegister(SynaPdtF01Common.CommandBase, &uData[0], 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaFinalizeFlash") != 0) {
	return 1;
  }
  
  
  SynaSleep(DefaultWaitPeriodAfterReset);

  
  if (SynaWaitForATTN(DefaultMillisecondTimeout))
  {
    if(SynaCheckIfFatalError(SynaRet, "SynaFinalizeFlash") != 0) {
	return 1;
    }
  }

  do {

    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    
    if(SynaRet && uErrorCount < DefaultErrorRetryCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

  } while(((SynauPageData[0] & 0x0f) != 0x00) && (uErrorCount < DefaultErrorRetryCount));
    if(SynaCheckIfFatalError(SynaRet, "SynaFinalizeFlash") != 0) {
	return 1;
    }
  
  
  SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaFinalizeFlash") != 0) {
	return 1;
  }
  
  
  uData[0] = 0;
  do
  {
    SynaRet = SynaReadRegister(SynauF01RMI_DataBase, &uData[0], 1);
    if(SynaCheckIfFatalError(SynaRet, "SynaFinalizeFlash") != 0) {
	return 1;
    }
  } while((uData[0] & 0x40)!= 0);

  
  SynaReadPageDescriptionTable();

  return ESuccess;
}

void SynaCalculateChecksum(unsigned short * data, unsigned short len, unsigned long * dataBlock)
{
  unsigned long temp;
  unsigned long sum1 = 0xffffffff & 0xFFFF;
  unsigned long sum2 = 0xffffffff >> 16;

  *dataBlock = 0xffffffff;

  while (len--)
  {
    temp = *data++;
    sum1 += temp;
    sum2 += sum1;
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16); 
  }

  *dataBlock = sum2 << 16 | sum1;
}

static int g_touch_firmware_update_count=0;

void print_screen_touch_firmware_update_info()
{
	if(g_touch_firmware_update_count == 0)
		display_dbg_msg("touch firmware update.    ", KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
	else if(g_touch_firmware_update_count == 1)
		display_dbg_msg("touch firmware update..   ", KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
	else if(g_touch_firmware_update_count == 2)
		display_dbg_msg("touch firmware update...  ", KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
	else if(g_touch_firmware_update_count == 3)
		display_dbg_msg("touch firmware update.... ", KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
	else if(g_touch_firmware_update_count == 4)
	{
		display_dbg_msg("touch firmware update.....", KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
		g_touch_firmware_update_count = -1;
	}
	g_touch_firmware_update_count ++;
}



int SynaFlashFirmwareWrite()
{
  unsigned char *puFirmwareData = SynafirmwareImgData;
  unsigned char uData[2];
  unsigned short uBlockNum;
  printk("SynafirmwareBlockCount : %d\n",SynafirmwareBlockCount);
  for (uBlockNum = 0; uBlockNum < SynafirmwareBlockCount; ++uBlockNum)
  {
    uData[0] = uBlockNum & 0xff;
    uData[1] = (uBlockNum & 0xff00) >> 8;
    
    
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockNum, &uData[0], 2);
    if(SynaCheckIfFatalError(SynaRet, "SynaFlashFirmwareWrite")!=0)
      return 1;

    
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, puFirmwareData, SynafirmwareBlockSize);
    if(SynaCheckIfFatalError(SynaRet, "SynaFlashFirmwareWrite")!=0)
      return 1;

    
    puFirmwareData += SynafirmwareBlockSize;

    
    uData[0] = 2;
    SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
    if(SynaCheckIfFatalError(SynaRet, "SynaFlashFirmwareWrite")!=0)
      return 1;

    
    if(SynaWaitATTN(DefaultErrorRetryCount) != 0)
      return 1;

    printk("Firmware writing : %d\n",uBlockNum);

    if(uBlockNum%10 == 0)
	print_screen_touch_firmware_update_info();
    schedule();
  }
  return 0;
}

int SynaProgramFirmware()
{
  unsigned char uData[1];

printk("\nSynaProgramFirmware start\n");
  SynaRet = SynaReadBootloadID();
  if(SynaCheckIfFatalError(SynaRet, "SynaProgramFirmware") != 0) {
	return 1;
  }

  
  SynaRet = SynaWriteBootloadID();
  if(SynaCheckIfFatalError(SynaRet, "SynaProgramFirmware") != 0) {
	return 1;
  }

  
  uData[0] = 3;
  SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  if(SynaCheckIfFatalError(SynaRet, "SynaProgramFirmware") != 0) {
	return 1;
  }

  
  if(SynaWaitATTN(DefaultErrorRetryCount)!=0)
    return 1;

  
  if(SynaFlashFirmwareWrite() != 0)
    return 1;

printk("SynaFlashFirmwareWrite Done\n");
  return 0;
}






unsigned long ExtractLongFromHeader(const unsigned char* SynaImage)  
{
  return((unsigned long)SynaImage[0] +
         (unsigned long)SynaImage[1]*0x100 +
         (unsigned long)SynaImage[2]*0x10000 +
         (unsigned long)SynaImage[3]*0x1000000);
}

int SynaReadFirmwareHeader()
{
  unsigned long firmwareImgFileCheckSum = 0;
  unsigned long checkSumCode = 0;


  
  SynafileSize = sizeof(SynaFirmware) - 2;
 









 
	checkSumCode           = ExtractLongFromHeader(&(SynaFirmware[0]));
  SynabootloadImgID      = (unsigned int)SynaFirmware[4] + (unsigned int)SynaFirmware[5]*0x100;
  SynafirmwareImgVersion = SynaFirmware[7];	
  SynafirmwareImgSize    = ExtractLongFromHeader(&(SynaFirmware[8]));
  SynaconfigImgSize      = ExtractLongFromHeader(&(SynaFirmware[12]));		





	SynaCalculateChecksum((unsigned short*)(&SynaFirmware[4]), (unsigned short)((SynafileSize - 4) >> 1),
                        (unsigned long *)(&firmwareImgFileCheckSum));

	if (firmwareImgFileCheckSum != checkSumCode)
  { 

	printk("\nError:  SynaFirmwareImage invalid checksum.\n");

	return 1;
  }

	if (SynafileSize != (0x100+SynafirmwareImgSize+SynaconfigImgSize))
  {

	printk("Error: SynaFirmwareImage actual size not expected size.\n");

	return 1;
  }

	if (SynafirmwareImgSize != SynaGetFirmwareSize())
  {

	printk("\nFirmware image size verfication failed.\n");

	return 1;
  }
		
	if (SynaconfigImgSize != SynaGetConfigSize())
  {

	printk("Configuration size verfication failed.\n");

	return 1;
  }

  SynafirmwareImgData = (unsigned char *)((&SynaFirmware[0])+0x100);
  SynaconfigImgData   = (unsigned char *)((&SynaFirmware[0])+0x100+SynafirmwareImgSize);

	SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);
  return 0;
}




unsigned int SynaDoReflash()
{
printk("SynaDoReflash start\n");
  if(SynaInitialize()) {
	printk("[TSP][ERROR]SynaInitialize fail\n");
	return 1;
  }
printk("SynaInitialize Done\n");
 
  
  if(SynaReadConfigInfo()) {
	printk("[TSP][ERROR]SynaReadConfigInfo fail\n");
	return 1;
  }
printk("SynaReadConfigInfo Done\n");

  if(SynaReadFirmwareInfo()) {
	printk("[TSP][ERROR]SynaReadFirmwareInfo fail\n");
	return 1;
  }
printk("SynaReadFirmwareInfo Done\n");

  
  SynaSetFlashAddrForDifFormat();
printk("SynaSetFlashAddrForDifFormat Done\n");

  
  if(SynaEnableFlashing()) {
	printk("[TSP][ERROR]SynaEnableFlashing fail\n");
	return 1;
  }
printk("SynaEnableFlashing Done\n");

  
  if(SynaReadFirmwareHeader() != 0) {
	printk("[TSP][ERROR]SynaEnableFlashing fail\n");
	return 1;
  }
printk("SynaReadFirmwareHeader Done\n");

  
  if(SynaProgramFirmware()) {
	printk("[TSP][ERROR]SynaProgramFirmware fail\n");
	return 1;
  }
printk("SynaProgramFirmware Done\n");

  
  if(SynaProgramConfiguration()) {
	printk("[TSP][ERROR]SynaProgramConfiguration fail\n");
	return 1;
  }
printk("SynaProgramConfiguration Done\n");

  
  if(SynaFinalizeFlash()) {
	printk("[TSP][ERROR]SynaFinalizeFlash fail\n");
	return 1;
  }
printk("SynaFinalizeFlash Done\n");
  
  return SynaRet;
}
EXPORT_SYMBOL(SynaDoReflash);
