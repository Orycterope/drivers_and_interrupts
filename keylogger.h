#ifndef HELLO_WORLD_H
# define HELLO_WORLD_H

#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/list.h>

/* A simple list of keylogger_entries and a lock protecting it */
struct	keylogger {
	struct mutex		lock;
	struct list_head	entries;
};

#define ASCII_UNDEFINED		'?'
#define KEYNAME_LEN		16

/* A struct describing a keyboard key */
struct key_info {
	char	name[KEYNAME_LEN];
	char	ascii;
	char	ascii_uppercase;
};

/* A struct describing a key event */
struct	keylogger_entry {
	struct key_info		key;
	struct tm		time;
	uint64_t		scancode;
	bool			released;
	struct list_head	list;
};

/* dev_file.c */
int	ft_create_dev_file(void);
void	ft_delete_dev_file(void);

/* interrupt_handler.c */
int	init_keylogger_irq(void *dev_id);
void	free_keylogger_irq(void *dev_id);

#endif
