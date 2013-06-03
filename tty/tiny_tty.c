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
#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/sched.h>


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
	struct timer_list *timer;

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
		if(tty_buffer_request_room(tty, i) > i)
			tty_flip_buffer_push(tty);
		tty_insert_flip_char(tty, data[i], TTY_NORMAL);
	}
	tiny->timer->expires = jiffies + DELAY_TIME;
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
		tiny->timer = NULL;
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

static void do_close(struct tiny_serial *tiny)
{
	down(&tiny->sem);
	if(!tiny->open_count)
		goto exit;
	--tiny->open_count;
	if(tiny->open_count <= 0)
	{
		del_timer(tiny->timer);

	}
exit:
	up(&tiny->sem);
}

static void tiny_close(struct tty_struct *tty, struct file *file)
{
	struct tiny_serial *tiny = tty->driver_data;
	if(tiny)
		do_close(tiny);
}

static int tiny_write(struct tty_struct *tty, const unsigned char *buffer, int count)
{
	struct tiny_serial *tiny = tty->driver_data;
	int i;
	int retval = -EINVAL;

	if(!tiny)
		return -ENODEV;
	down(&tiny->sem);
	if(!tiny->open_count)
		goto exit;
	printk(KERN_DEBUG"%s - ",__FUNCTION__);
	for(i = 0; i < count; i++)
	{
		printk("%02x ", buffer[i]);

	}
	printk("\n");
exit:
	up(&tiny->sem);
	return retval;
}

static int tiny_write_room(struct tty_struct *tty)
{
	struct tiny_serial *tiny = tty->driver_data;
	int room = -EINVAL;
	
	if(!tiny)
		return -ENODEV;

	down(&tiny->sem);
	if(!tiny->open_count)
		goto exit;
	room = 255;
exit:
	up(&tiny->sem);
	return room;
}

#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))
static void tiny_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
{
	unsigned int cflag;
	cflag = tty->termios->c_cflag;
	if(old_termios)
	{
		if((cflag == old_termios->c_cflag) &&
		(RELEVANT_IFLAG(tty->termios->c_iflag) ==
		RELEVANT_IFLAG(old_termios->c_iflag)))
		{
			printk(KERN_DEBUG" - nothing to change...\n");
			return;
		}
	}
	switch(cflag & CSIZE)
	{
		case CS5:
			printk(KERN_DEBUG" - data bits = 5\n");
			break;
		case CS6:
			printk(KERN_DEBUG" - data bits = 6\n");
			break;
		case CS7:
			printk(KERN_DEBUG" - data bits = 7\n");
			break;
		case CS8:
			printk(KERN_DEBUG" - data bits = 8\n");
			break;

	}
	if(cflag & PARENB)
		if(cflag & PARODD)
			printk(KERN_DEBUG" - parity = odd\n");
		else
			printk(KERN_DEBUG" - parity = even\n");
	else
		printk(KERN_DEBUG" - parity = none\n");

	if(cflag & CSTOPB)
		printk(KERN_DEBUG" - stop bits = 2\n");
	else
		printk(KERN_DEBUG" - stop bits = 1\n");

	if(cflag & CRTSCTS)
		printk(KERN_DEBUG" - RTS/CTS is enabled\n");
	else
		printk(KERN_DEBUG" - RTS/CTS is disabled\n");

	if(I_IXOFF(tty) || I_IXON(tty))
	{
		unsigned char stop_char = STOP_CHAR(tty);
		unsigned char start_char = START_CHAR(tty);
		if(I_IXOFF(tty))
			printk(KERN_DEBUG" - INBOUND XON/XOFF is enabled, " " XON = %2x, XOF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - INBOUND XON/XOFF is disabled");
		if(I_IXON(tty))
			printk(KERN_DEBUG" - INBOUND XON/XOFF is enabled, " " XON = %2x, XOF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - INBOUND XON/XOFF is disabled");
	}
	printk(KERN_DEBUG" - baud rate = %d", tty_get_baud_rate(tty));
}

#define MCR_DTR		0x01
#define MCR_RTS		0x02
#define MCR_LOOP	0x04
#define MSR_CTS		0x08
#define MSR_CD		0x10
#define MSR_RI		0x20
#define MSR_DSR		0x40

static int tiny_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct tiny_serial *tiny = tty->driver_data;
	unsigned int result = 0;
	unsigned int msr = tiny->msr;
	unsigned int mcr = tiny->mcr;

	result = ((mcr & MCR_DTR) ? TIOCM_DTR : 0) |
			 ((mcr & MCR_RTS) ? TIOCM_RTS : 0) |
			 ((mcr & MCR_LOOP)? TIOCM_LOOP: 0) |
			 ((msr & MSR_CTS) ? TIOCM_CTS : 0) |
			 ((msr & MSR_CD)  ? TIOCM_CAR : 0) |
			 ((msr & MSR_RI)  ? TIOCM_RI  : 0) |
			 ((msr & MSR_DSR) ? TIOCM_DSR : 0);

	return result;
}

static int tiny_tiocmset(struct tty_struct *tty, struct file *file, unsigned int set, unsigned int clear)
{
	struct tiny_serial *tiny = tty->driver_data;
	unsigned int mcr = tiny->mcr;

	if(set & TIOCM_RTS)
		mcr |= MCR_RTS;
	if(set & TIOCM_DTR)
		mcr |= MCR_RTS;

	if(clear & TIOCM_RTS)
		mcr &= ~MCR_RTS;
	if(clear & TIOCM_DTR)
		mcr &= ~MCR_RTS;

	tiny->mcr = mcr;
	return 0;
}

static int tiny_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct tiny_serial *tiny;
	off_t begin = 0;
	int length = 0;
	int i;
	length += sprintf(page, "tinyserinfo:1.0 driver:v1.0\n");
	for(i = 0; i < TINY_TTY_MINORS && length < PAGE_SIZE; i++)
	{
		tiny = tiny_table[i];
		if(tiny == NULL)
			continue;

		length += sprintf(page + length, "%d\n", i);
		if((length + begin) > (off + count))
			goto done;
		if((length + begin) < off)
		{
			begin += length;
			length = 0;
		}
	}
	*eof = 1;
done:
	if(off >= (length + begin))
		return 0;
	*start = page + (off-begin);
	return (count < begin+length-off) ? count : begin + length-off;
}

#define tiny_ioctl tiny_ioctl_tiocgserial
static int tiny_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct tiny_serial *tiny = tty->driver_data;
	if(cmd == TIOCGSERIAL)
	{
		struct serial_struct tmp;

		if(!arg)
			return -EFAULT;
		memset(&tmp, 0, sizeof(tmp));

		tmp.type		= tiny->serial.type;
		tmp.line		= tiny->serial.line;
		tmp.port		= tiny->serial.port;
		tmp.irq			= tiny->serial.irq;
		tmp.flags		= ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ;
		tmp.xmit_fifo_size = tiny->serial.xmit_fifo_size;
		tmp.baud_base	= tiny->serial.baud_base;
		tmp.close_delay = 5 * HZ;
		tmp.closing_wait	= 30*HZ;
		tmp.custom_divisor = tiny->serial.custom_divisor;
		tmp.hub6		= tiny->serial.hub6;
		tmp.io_type		= tiny->serial.io_type;

		if(copy_to_user((void __user *)arg, &tmp, sizeof(struct serial_struct)))
			return-EFAULT;

	}
	return -ENOIOCTLCMD;

}
#undef tiny_ioctl

#define tiny_ioctl tiny_ioctl_tiocmiwait
static int tiny_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct tiny_serial *tiny = tty->driver_data;

	if(cmd == TIOCMIWAIT)
	{
		DECLARE_WAITQUEUE(wait, current);
		struct async_icount cnow;
		struct async_icount cprev;
		cprev = tiny->icount;
		while(1)
		{
			add_wait_queue(&tiny->wait, &wait);
			set_current_state(TASK_INTERRUPTIBLE);
			schedule();
			remove_wait_queue(&tiny->wait, &wait);
			if(signal_pending(current))
				return -ERESTARTSYS;

			cnow = tiny->icount;
			if(cnow.rng == cprev.rng && cnow.dsr == cprev.dsr && cnow.dcd == cprev.dcd && cnow.cts == cprev.cts)
				return -EIO;
			if(((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
			   ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
			   ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
			   ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)))
			{
				return 0;
			}
			cprev = cnow;

		}
	}
	return -ENOIOCTLCMD;
}

#undef tiny_ioctl
#define tiny_ioctl tiny_ioctl_tiocgicount
static int tiny_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct tiny_serial *tiny = tty->driver_data;
	if(cmd == TIOCGICOUNT)
	{
		struct async_icount cnow = tiny->icount;
		struct serial_icounter_struct icount;

		icount.cts	= cnow.cts;
		icount.dsr	= cnow.dsr;
		icount.rng	= cnow.rng;
		icount.dcd	= cnow.dcd;
		icount.rx	= cnow.rx;
		icount.tx	= cnow.tx;
		icount.frame	= cnow.frame;
		icount.overrun	= cnow.overrun;
		icount.parity	= cnow.parity;
		icount.brk		= cnow.brk;
		icount.buf_overrun	= cnow.buf_overrun;

		if(copy_to_user((void __user *)arg, &icount, sizeof(icount)))
			return -EFAULT;
		return 0;
	}
	return -ENOIOCTLCMD;
}
#undef tiny_ioctl
static int tiny_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case TIOCGSERIAL:
			return tiny_ioctl_tiocgserial(tty, file, cmd, arg);
		case TIOCMIWAIT:
			return tiny_ioctl_tiocmiwait(tty, file, cmd, arg);
		case TIOCGICOUNT:
			return tiny_ioctl_tiocgicount(tty, file, cmd, arg);
	}
	return -ENOIOCTLCMD;
}


static struct tty_operations serial_ops = 
{
	.open	= tiny_open,
	.close	= tiny_close,
	.write	= tiny_write,
	.write_room = tiny_write_room,
	.set_termios = tiny_set_termios,
	.tiocmget	= tiny_tiocmget,
	.tiocmset	= tiny_tiocmset,
	.ioctl		= tiny_ioctl,
//	.proc_fops	= &serial_proc_ops
	
};

static struct tty_driver *tiny_tty_driver;

static int __init tiny_init(void)
{
	int retval ;
	int i;

	tiny_tty_driver = alloc_tty_driver(TINY_TTY_MINORS);
	if(!tiny_tty_driver)
		return -ENOMEM;

	tiny_tty_driver->owner = THIS_MODULE;
	tiny_tty_driver->driver_name = "tiny_tty";
	tiny_tty_driver->name = "ttty";
//	tiny_tty_driver->dev_name = "tts/ttty%d";
	tiny_tty_driver->major = TINY_TTY_MAJOR;
	tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW ;
	tiny_tty_driver->init_termios = tty_std_termios;
	tiny_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_set_operations(tiny_tty_driver, &serial_ops);
	retval = tty_register_driver(tiny_tty_driver);
	if(retval)
	{
		printk(KERN_ERR"failed to register tiny tty driver");
		put_tty_driver(tiny_tty_driver);
		return retval;
	}

	for(i = 0; i < TINY_TTY_MINORS; i++)
		tty_register_device(tiny_tty_driver, i, NULL);
	printk(KERN_INFO " tiny serila v1.0 by ksheng");
	return retval;

}

static void __exit tiny_exit(void)
{
	struct tiny_serial *tiny;
	int i;
	for(i = 0; i < TINY_TTY_MINORS; i++)
		tty_unregister_device(tiny_tty_driver, i);
	tty_unregister_driver(tiny_tty_driver);

	for(i = 0; i < TINY_TTY_MINORS; i++)
	{
		tiny = tiny_table[i];
		if(tiny)
		{
			while(tiny->open_count)
				do_close(tiny);
			del_timer(tiny->timer);
			kfree(tiny->timer);
			kfree(tiny);
			tiny_table[i] = NULL;
		}
	}
}

module_init(tiny_init);
module_exit(tiny_exit);
