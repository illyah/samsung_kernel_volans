/****************************************************************************
**
** COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR		: KyoungHOON Kim (khoonk)
**
*****************************************************************************
** 2006/07/13. khoonk	1.add kernel_thread for periodic i2c comunication
**                    	2.http://www.scs.ch/%7Efrey/linux/kernelthreads.html
*****************************************************************************/
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/signal.h>

#include <asm/semaphore.h>
#include <linux/smp_lock.h>
#include "dcm_thread.h"
static void kthread_launcher( struct work_struct* work )
{
    kthread_t *kthread = ( void* ) work;
    kernel_thread((int (*)(void *))kthread->function, (void *)kthread, 0);
}

void start_kthread(void (*func)(kthread_t *), kthread_t *kthread)
{
	init_MUTEX_LOCKED(&kthread->startstop_sem);

	kthread->function=func;

	INIT_WORK( &kthread->wq, kthread_launcher );
	schedule_work(&kthread->wq);
	down(&kthread->startstop_sem);
               
}
void stop_kthread(kthread_t *kthread)
{
	if (kthread->thread == NULL)
	{
		printk("stop_kthread: killing non existing thread!\n");
		return;
	}

	lock_kernel();

	init_MUTEX_LOCKED(&kthread->startstop_sem);

	mb();

	kthread->terminate = 1;

	mb();
	kill_proc(kthread->thread->pid, SIGKILL, 1);

	down(&kthread->startstop_sem);

	unlock_kernel();

	kill_proc(2, SIGCHLD, 1);

}
void init_kthread(kthread_t *kthread, char *name)
{
	lock_kernel();

	kthread->thread = current;

	siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));

	init_waitqueue_head(&kthread->queue);

	kthread->terminate = 0;

	sprintf(current->comm, name);

	unlock_kernel();

	up(&kthread->startstop_sem);

}
void exit_kthread(kthread_t *kthread)
{

	    lock_kernel();
	    kthread->thread = NULL;
	    mb();

	up(&kthread->startstop_sem);

}
