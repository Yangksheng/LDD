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

