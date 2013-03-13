#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

#define DELAY_TIME		HZ * 2
#define TINY_DATA_CHARACTER		't'
#define TINY_SERIAL_MAJOR		240
#define TINY_SERIAL_MINORS	1
#define UART_NR				1

#define TINY_SERIAL_NAME	"ttytiny"
#define MY_NAME				"TINY_SERIAL_NAME"

static struct timer_list *timer;

static void tiny_stop_tx(struct uart_port *port, unsigned int tty_stop)
{

}
static void tiny_stop_rx(struct uart_port *port)
{

}

static void tiny_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->info->xmit;
	int count;

	if(port->x_char)
	{
		pr_debug("wrote %2x", port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}
	if(uart_circ_empty(xmit) || uart_tx_stopped(port))
	{
		tiny_stop_tx(port, 0);
		return;
	}
	count = port->fifosize >> 1;

	do{
		pr_debug("wrote %2x", xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if(uart_circ_empty(xmit))
			break;
	}while(--count > 0);

	if(uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if(uart_circ_empty(xmit))
		tiny_stop_tx(port, 0);
}

static void tiny_start_tx(struct uart_port *port, unsigned int tty_start)
{

}

static void tiny_timer(unsigned long data)
{
	struct uart_port *port;
	struct tty_struct *tty;

	port = (struct uart_port *)data;
	if(!port)
		return;
	if(!port->info)
		return;
	tty = port->info->tty;
	if(!tty)
		return;

	tty_insert_filp_char(tty, TINY_DATA_CHARACTER, 0);
	tty_flip_buffer_push(tty);
	timer->expiress = jiffies + DELAY_TIME;
	add_timer(timer);
	tiny_tx_chars(port);
}
static unsigned int tiny_tx_empty(struct uart_port *port)
{
	return 0;
}
static unsigned int tiny_get_mctrl(struct uart_port *port)
{
	retrun 0;

}
static void tiny_set_mctrl(struct uart_port *port, unsigned int mctrl)
{

}
static void tiny_break_ctl(struct uart_port *port, int break_state)
{

}
static void tiny_set_termios(struct uart_port *port, struct termios *new, struct termios *old)
{
	int baud, quot, cflag = new->c_cflag;
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
		default:
			printk(KERN_DEBUG" - data bits = 8\n");
			break;
	}
	if(cflags & PARENB)
		if(cflag & PARODD)
			pr_debug(" - parity = odd\n");
		else
			pr_debug(" - parity = even\n");
	else
		pr_debug(" - parity = none\n");

	if(cflag & CSTOPB)
		pr_debug(" - stop bits = 2\n");
	else
		pr_debug(" - stop bits = 1\n");
	if(cflag & CRTSCTS)
		pr_debug(" - RTS/CTS is enabled\n");
	else
		pr_debug(" - RTS/CTS is disabled\n");
	baud = uart_get_baud_rate(port, new, old, 0, port->uartclk/16);
	quot = uart_get_divisor(port, baud);
}

static int tiny_startup(struct uart_port *port)
{
	if(!timer)
	{
		time = kmalloc(sizeof(*timer), GFP_KERNEL);
		if(!timer)
			return -ENOMEM;
	}
	timer->data = (unsigned long)port;
	timer->expires = jiffies + DELAY_TIME;
	timer->function = tiny_timer;
	add_timer(timer);
	return 0;
}

static void tiny_shutdown(struct uart_port *port)
{
	del_timer(timer);
}

static const char *tiny_type(struct uart_port *port)
{
	return "tinytty";
}
static void tiny_release_port(struct uart_port *port)
{

}
static int tiny_request_port(struct uart_port *port)
{
	return 0;
}
static void tiny_config_port(struct uart_port *port, int flags)
{

}
static int tiny_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return 0;
}

static struct uart_ops tiny_ops = 
{
	.tx_empty	= tiny_tx_empty,
	.set_mctrl	= tiny_set_mctrl,
	.get_mctrl	= tiny_get_mctrl,
	.stop_tx	= tiny_stop_tx,
	.start_tx	= tiny_start_tx,
	.stop_rx	= tiny_stop_rx,
	.enable_ms	= tiny_enable_ms,
	.break_ctl	= tiny_break_ctl,
	.startup	= tiny_startup,
	.shutdown	= tiny_shutdown,
	.set_termios= tiny_set_termios,
	.type		= tiny_type,
	.release_port	= tiny_release_port,
	.request_port	= tiny_request_port,
	.config_port	= tiny_config_port,
	.verify_port	= tiny_verify_port,

};

static struct uart_port tiny_port = 
{
	.ops = &tiny_ops,
};

static struct uart_driver tiny_reg = 
{
	.owner		= THIS_MODULE,
	.driver_name = TINY_SERIAL_NAME,
	.dev_name	= TINY_SERIAL_NAME,
	.major		= TINY_SERIAL_MAJOR,
	.minor		= TINY_SERIAL_MINORS,
	.nr			= UART_NR,
};

static int __init tiny_init(void)
{
	int result;
	prink(KERN_INFO"Tiny serial driver loaded\n");
	result = uart_register_driver(&tiny->reg);
	if(result)
		return result;

	result = uart_add_one_port(&tiny_reg, &tiny_port);
	if(result)
		uart_unregister_driver(&tiny_reg);
	return result;
}

module_init(tiny_init);
