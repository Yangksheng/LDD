#include <linux/ioctl.h>
#include <linux/cdev.h>

#undef PDEBUG
#ifdef SCULLP_DEBUG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) printk(KERN_DEBUG "scullp: " fmt, ## args)
#else
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...)
#endif

#undef PDEBUG
#define PDEBUG(fmt, args...)
#define SCULLP_MAJOR	0
#define SCULLP_DEVS		4

#define SCULLP_ORDER	0
#define SCULLP_QSET		500

struct scullp_dev {
	void **data;
	struct scullp_dev	*next;
	int vmas;
	int order;
	int qset;
	size_t size;
	struct semaphore sem;
	struct cdev cdev;
};

extern struct scullp_dev *scullp_devices;
extern struct file_operations scullp_fops;

extern int scullp_major;
extern int scullp_devs;
extern int scullp_order;
extern int scullp_qset;

int scullp_trim(struct scullp_dev *dev);
struct scullp_dev *scullp_follow(struct scullp_dev *dev, int n);


#ifdef SCULLP_DEBUG
#define SCULLP_USER_PROC
#endif

#define SCULLP_IOC_MAGIC	'k'
#define SCULLP_IOCRESET		_IO(SCULLP_IOC_MAGIC, 0)

#define SCULLP_IOCSORDER	_IOW(SCULLP_IOC_MAGIC, 1, int)
#define SCULLP_IOCTORDER	_IO(SCULLP_IOC_MAGIC, 2)
#define SCULLP_IOCGORDER	_IOR(SCULLP_IOC_MAGIC, 3, int)
#define SCULLP_IOCQORDER	_IO(SCULLP_IOC_MAGIC, 4)
#define SCULLP_IOCXORDER	_IOWR(SCULLP_IOC_MAGIC, 5, int)
#define SCULLP_IOCHORDER	_IO(SCULLP_IOC_MAGIC, 6)
#define SCULLP_IOCSQSET		_IOW(SCULLP_IOC_MAGIC, 7, int)
#define SCULLP_IOCTQSET		_IO(SCULLP_IOC_MAGIC, 8)
#define SCULLP_IOCGQSET		_IOR(SCULLP_IOC_MAGIC, 9, int)
#define SCULLP_IOCQQSET		_IO(SCULLP_IOC_MAGIC, 10)
#define SCULLP_IOCXQSET		_IOWR(SCULLP_IOC_MAGIC, 11, int)
#define SCULLP_IOCHQSET		_IO(SCULLP_IOC_MAGIC, 12)

#define SCULLP_IOC_MAXNR	12


