#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define DELAY_TIME		HZ * 2
#define TINY_DATA_CHARACTER		't'
#define TINY_TTY_MAJOR			240
#define TINY_TTY_MINORS			4

struct tiny_serial
{
	struct tty_struct *tty;
	int		open_count;
	struct semaphore	sem;
	struct timer_list *time;

	int msr;
	int mcr;
	struct serial_struct serial;
	wait_queue_head_t wait;
	struct async_icount icount;
};

static struct tiny_serial *tiny_table[TINY_TTY_MINORS];
static void tiny_timer(unsigned long timer_data)
{
	struct tiny_serial *tiny = (struct tiny_serial *)timer_data;
	struct tty_struct *tty;
	int i;
	char data[1] = {TINY_DATA_CHARACTER};
	int data_size = 1;
	if(!tiny)
		return;

	tty = tiny->tty;
	for(i = 0; i < data_size; i++)
	{
		if(tty->flip.count >= TTY_FLIPBUF_SIZE)
			tty_flip_buffer_push(tty);
		tty_insert_flip_char(tty, data[i], TTY_NORMAL);
	}
	tty->timer->expires = jiffies + DELAY_TIME:
	add_timer(tiny->timer);
}

static int tiny_open(struct tty_struct *tty, struct file *file)
{
	struct tiny_serial *tiny;
	struct timer_list  *timer;
	int index;

	tty->driver_data = NULL;
	index = tty->index;
	tiny = tiny_table[index];
	if(tiny == NULL)
	{
		tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
		if(!tiny)
			return -ENOMEM;
		init_MUTEX(&tiny->sem);
		tiny->open_count = 0;
		tiny_timer = NULL;
		tiny_table[index] = tiny;
	}
	down(&tiny->sem);
	tty->driver_data = tiny;
	tiny->tty = tty;
	++tiny->open_count;

	if(tiny->open_count == 1)
	{
		if(!tiny->timer)
		{
			timer = kmalloc(sizeof(*timer), GFP_KERNEL);
			if(!timer)
			{
				up(&tiny->sem);
				return -ENOMEM;
			}
			tiny->timer = timer;
		}
		tiny->timer->data = (unsigned long)tiny;
		tiny->timer->expires = jiffies + DELAY_TIME;
		tiny->timer->function = tiny_timer;
		add_timer(tiny->timer);
	}
	up(&tiny->sem);
	return 0;
}
