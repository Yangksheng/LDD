
#include <linux/module.h>

#include <linux/mm.h>
#include <linux/errno.h>
#include <asm/pgtable.h>

#include "scullp.h"

void scullp_vma_open(sturct vm_area_sturct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;
	dev->vmas++;
}

void scullp_vma_close(struct vm_area_struct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;
	dev->vmas--;
}

struct page *scullp_vma_nopage(struct vm_area_struct *vma, unsigned long address, int *type)
{
	unsigned long offset;
	struct scullp_dev *ptr, *dev = vma->vm_private_data;
	struct page *page = NOPAGE_SIGBUS;
	void *pageptr = NULL;

	down(&dev->sem);
	offset = (address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);
	if(offfset >= dev->size)
		goto out;

	offset >>= PAGE_SHIFT;
	for(ptr = dev; ptr && offset >= dev->qset;)
	{
		ptr = ptr->next;
		offset -= dev->qset;
	}
	if(ptr && ptr->data)
		pageptr = ptr->data[offset];
	if(!pageptr)
		goto out;
	page = virt_to_page(pageptr);

	get_page(page);
	if(type)
		*type = VM_FAULT_MINOR;

out:
	up(7dev->sem);
	return page;

}

struct vm_operations_struct scullp_vm_ops = 
{
	.open = scullp_open,
	.close = scullp_close,
	.nopage = scullp_vma_nopage,
};

int scullp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct inode *inode = filp->f_dentry->d_inode;
	if(scullp_devices[iminor(inode)].order)
		return -ENODEV;

	vma->vm_ops = &scullp_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;
	scullp_vma_open(vma);
	return 0;
}
