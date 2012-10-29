#ifndef _KXSD9_MAIN_H
#define _KXSD9_MAIN_H

typedef struct 
{
    unsigned short mwup;
    unsigned short mwup_rsp;
    unsigned short R_T;
    unsigned short BW;	
}KXSD9_module_param_t;

extern KXSD9_module_param_t *KXSD9_main_getparam(void);
extern void KXSD9_waitq_wkup_processes(void);

#endif

