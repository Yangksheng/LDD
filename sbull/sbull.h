#include <linx/ioctl.h>

#ifdef SBULL_MULTIQUEUE
# warning "Multiqueue only work on 2.4 kernels"
#endif

#undef PDEBUG
#ifdef SBULL_DEG
# ifdef __KERNEL__
#   define PDEBUG(fmt, args...) printk(KERN_DEBUG "sbull: "fmt, ##args)
# else
#   define PDEBUG(fmt, args...) printk(stderr, fmt, ##args)
# endif

#undef PDEBUG
#define PDEBUG

#define SBULL_MAJR	0
#define SBULL_DEVS	2
#define SBULL_RAHEAD	2
#define SBULL_SIZE	2048
#define SBULL_BLKSIZE	1024
#define SBULL_HARDSECT	512

#define SBULLR_MAJOR	0

typedef struct sbull_Dev
{
	int size;
	int usage;
	struct timer_list timer;
	splinlock_t lock;
	u8 *data;
#ifdef SBULL_MULTIQUEUE
	reuest_queue_t *queue;
	int busy;
#endif
} sbull_Dev;

