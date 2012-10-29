#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

unsigned long long movinand_workload;
static int movinand_workload_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "TotalWorkload:       %llu Bytes\n", movinand_workload);
	return 0;
}

static int movinand_workload_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, movinand_workload_proc_show, NULL);
}

static const struct file_operations movinand_workload_proc_fops = {
	.open		= movinand_workload_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_movinand_workload_init(void)
{
	proc_create("movinand_workload", 0, NULL, &movinand_workload_proc_fops);
	return 0;
}
module_init(proc_movinand_workload_init);
