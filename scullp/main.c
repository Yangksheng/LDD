#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <asm/uaccess.h>
#include "scullp.h"

int scullp_major	= SCULLP_MAJOR;
int scullp_devs		= SCULLP_DEVS;
int scullp+qset 	= SCULLP_QSET;

module_param(scullp_major, int, 0);
module_param(scullp_devs, int, 0);
module_param(scullp_qset, int, 0);
module_param(scullp_order, int, 0);
MODULE_LICENSE("Dual BSD/GPL");

struct scullp_dev *scullp_devices;

int scullp_trim(struct scullp_dev *dev);
void scullp_cleanup(void);

