/****************************************************************************
**
** COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR		: KyoungHOON Kim (khoonk)
**
*****************************************************************************/
#ifndef _DCM_THREAD_H
#define _DCM_THREAD_H
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>

#include <linux/workqueue.h>

#include <linux/wait.h>

#include <asm/unistd.h>
#include <asm/semaphore.h>

typedef struct kthread_struct
{
	struct task_struct *thread;
	struct work_struct wq;
	void (*function) (struct kthread_struct *kthread);
	struct semaphore startstop_sem;

	wait_queue_head_t queue;
	int terminate;
	void *arg;
} kthread_t;


void start_kthread(void (*func)(kthread_t *), kthread_t *kthread);

void stop_kthread(kthread_t *kthread);
#endif
