/*
 * linux/drivers/memnoti/xo_mem_notify.c
 *
 * Memory notifier.
 *
 * Author:  InKi Dae  <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/err.h>

#define DEFAULT_VALUE 512

#define OOM_KILL_NONE 0
#define OOM_KILL_ONE 1
#define OOM_KILL_ALL 2

#define DEBUG
#if 0//def DEBUG
#define mdebug	printk
#else
#define mdebug
#endif

extern struct task_struct *select_bad_process(unsigned long *ppoints,struct mem_cgroup *mem, int *wait_pid);
extern int hardlimit_kill_process(struct task_struct *p, gfp_t gfp_mask, int order, unsigned long points, const char *message);

static unsigned long threshold_value = DEFAULT_VALUE;
static unsigned long other_free_value = 0;
static unsigned long hardlimit_value = 0;
unsigned long hardlimit_value_backup = 0;
static unsigned long oom_option_value = 0; //oom option: 0=no hardlimit 1=1 process kill when hardlimit 2=all process kill when hardlimit
static pid_t foreground_pid = 0;
static unsigned long initial_threshold_value = 0;
static unsigned char initial_value_flag = 0;

static struct work_struct	mem_noti_work;

unsigned long get_memnotify_hardlimit(void)
{
	return hardlimit_value;
}

void set_memnotify_hardlimit(unsigned long value)
{
	hardlimit_value = value;
}

int set_memnotify_other_free(unsigned long value)
{
	other_free_value = value;

	return 0;
}

static ssize_t oom_option_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	return sprintf(buf, "%ld\n", oom_option_value);
}

static ssize_t oom_option_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	oom_option_value = simple_strtoul(buf, NULL, 10);

	return count;
}

static DEVICE_ATTR(oom_option, S_IRUGO | S_IWUSR, oom_option_show, oom_option_store);

static ssize_t hardlimit_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	return sprintf(buf, "%ld\n", hardlimit_value);
}

static ssize_t hardlimit_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	hardlimit_value = simple_strtoul(buf, NULL, 10);
	hardlimit_value_backup = hardlimit_value;

	return count;
}

static DEVICE_ATTR(hardlimit, S_IRUGO | S_IWUSR, hardlimit_show, hardlimit_store);

static ssize_t foreground_process_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	return sprintf(buf, "%d\n", foreground_pid);
}

static ssize_t foreground_process_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	foreground_pid = (pid_t)simple_strtoul(buf, NULL, 10);

	return count;
}

static DEVICE_ATTR(foreground_pid, S_IRUGO | S_IWUSR, foreground_process_show, foreground_process_store);

static ssize_t other_free_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	return sprintf(buf, "%ld\n", other_free_value);
}

static DEVICE_ATTR(other_free, S_IRUGO, other_free_show, NULL);

struct class *mem_notify_class;

static int memnotify_probe(struct platform_device *dev)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	if (device_create_file(&dev->dev, &dev_attr_foreground_pid)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}
	
	if (device_create_file(&dev->dev, &dev_attr_other_free)) {
		printk(KERN_ERR "failed to create sysfs file for other free\n");
		return -1;
	}
	
	if (device_create_file(&dev->dev, &dev_attr_oom_option)) {
		printk(KERN_ERR "failed to create sysfs file for oom option\n");
		return -1;
	}
	
	if (device_create_file(&dev->dev, &dev_attr_hardlimit)) {
		printk(KERN_ERR "failed to create sysfs file for hardlimit\n");
		return -1;
	}

	return 0;
}
	

static struct platform_driver memnotify_driver = {
	.probe			= memnotify_probe,
	.driver			= {
		.name 	=	"xo_mem_notify",
		.owner	=	THIS_MODULE,
	},
};

static struct platform_device memnotify_device = {
	.name	=	"xo_mem_notify",
	.id		=	0,
	.dev	=	{
		.platform_data = NULL,
	},
};

static void send_mem_nofify(struct work_struct *data)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	kobject_uevent(&memnotify_device.dev.kobj, KOBJ_CHANGE); 
}

int xo_direct_notifier(void)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	threshold_value = DEFAULT_VALUE;
	schedule_work(&mem_noti_work);

	return 0;
}

int background_turn = 0;

int is_foreground_process(pid_t pid, pid_t reader_pid)
{
	if(background_turn == 0 && (pid == foreground_pid || reader_pid == foreground_pid))
		return 1;
	else 
		return 0;
}

unsigned long jiffies_before = 0;
void xo_oom_process_killer(unsigned long current_other_free)
{
	unsigned long points = 0;
	int wait_pid;
	struct task_struct *p;
	unsigned long jiffies_spent;

	
	if(jiffies_before == 0)
		jiffies_before = jiffies;

	jiffies_spent = jiffies - jiffies_before;
	
	current_other_free = global_page_state(NR_FREE_PAGES) 
		+ global_page_state(NR_FILE_PAGES);

		
	if(hardlimit_value_backup < current_other_free || jiffies_spent < msecs_to_jiffies(800))
	{
		return;
	}

	jiffies_before = jiffies;
	
	printk("Hardlimit oom occured!! current other free = %lu\n",current_other_free);

	if(oom_option_value == OOM_KILL_NONE)
		return;	

	hardlimit_value = 0;
	
	read_lock(&tasklist_lock);
		
retry:	
	p = select_bad_process(&points,NULL,&wait_pid);

	if (PTR_ERR(p) == -1UL)
	{
		read_unlock(&tasklist_lock);
		background_turn = 0;
		hardlimit_value = hardlimit_value_backup;
		printk("The process of PID=%d is being terminated...\n", wait_pid);
		return;
	}


	if (!p) {
		if(background_turn == 0)
		{
			background_turn = 1;
			goto retry;
		}
		printk("=====================================================\n");
		printk("= Hardlimit occured but no more killable processes. =\n");
		printk("=====================================================\n");
		read_unlock(&tasklist_lock);
		background_turn = 0;
		return;
	
	}

	if(p->pid == foreground_pid)
	{
		printk("kill foreground process. Send notify.\n");
		xo_direct_notifier();
	}

	if (hardlimit_kill_process(p, (int)NULL, (int)NULL, points,"Hardlimit occured"))
		goto retry;

	
	if(background_turn == 1)
		background_turn = 0;


	current_other_free = global_page_state(NR_FREE_PAGES) 
		+ global_page_state(NR_FILE_PAGES);
	
	if(hardlimit_value_backup > current_other_free || oom_option_value == OOM_KILL_ALL)
	{
		printk("Need more memory. Kill more process.(hardlimit=%lu , ofree=%lu)\n", hardlimit_value_backup, current_other_free);
	}

	read_unlock(&tasklist_lock);

	hardlimit_value = hardlimit_value_backup;

	background_turn = 0;

}


static int memnotify_init(void)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	if (platform_driver_register(&memnotify_driver)) {
		printk(KERN_ERR "failed to register memnotify driver\n");
		return -ENODEV;
	}

	if (platform_device_register(&memnotify_device)) {
		printk(KERN_ERR "failed to register memnotify device\n");
		return -ENODEV;
	}

	INIT_WORK(&mem_noti_work, send_mem_nofify);
	return 0;
}


static void __exit memnotify_exit(void)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	flush_scheduled_work(); /* flush any pending tx tasks */

	platform_driver_unregister(&memnotify_driver);
	platform_device_unregister(&memnotify_device);
}

module_init(memnotify_init);
module_exit(memnotify_exit);
