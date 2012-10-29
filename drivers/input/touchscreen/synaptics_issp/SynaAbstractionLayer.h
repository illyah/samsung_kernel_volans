
#ifdef _cplusplus
extern "C"
  {
#endif

#ifndef _SYNA_ABSTRACTION_LAYER_H
#define _SYNA_ABSTRACTION_LAYER_H

#define DefaultErrorRetryCount      300
#define DefaultMillisecondTimeout   300
#define DefaultWaitPeriodAfterReset 200 

#define ESuccess                     0
#define EErrorTimeout               -1
#define EErrorFlashFailed           -2
#define EErrorBootID                -3
#define EErrorFunctionNotSupported  -4
#define EErrorFlashImageCorrupted   -5

void
SynaSleep(unsigned long MilliSeconds);

unsigned int
SynaWaitForATTN(unsigned long MilliSeconds);

unsigned int
SynaWriteRegister(unsigned short uRmiAddress, unsigned char * data,
    unsigned int length);

unsigned int
SynaReadRegister(unsigned short uRmiAddress, unsigned char * data,
    unsigned int length);

unsigned int
SynaDoReflash(void);

#endif 

#ifdef _cplusplus
}
#endif
