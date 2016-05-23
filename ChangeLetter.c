#include <linux/fs.h>               
#include <linux/errno.h>               
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>            
#include <asm/system.h>            
#include <asm/uaccess.h>            

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DJC");
#define DEV_NAME "ChangeLetter"
static ssize_t LetterRead(struct file *, char *, size_t, loff_t *);
static ssize_t LetterWrite(struct file *, char *, size_t, loff_t *);
int LetterOpen(struct inode *inode, struct file *filp);
int LetterRelease(struct inode *inode, struct file *filp);



// user defined function to reverse char.
char reverseLetter(char ch);

#define KBUF_LEN	32

static int char_major = 250; 

struct changeletter_dev {
	struct cdev cdev;
	unsigned int current_len;
	char kbuf[KBUF_LEN];
	struct semaphore sem;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
};
struct changeletter_dev *changeletter_devp;

static const struct file_operations ChangeLetter_fops =
{
	.open = LetterOpen,
	.release = LetterRelease,
	.read = LetterRead,
	.write = LetterWrite,
};

static void changeletter_setup_cdev(struct changeletter_dev *dev, int index) {
	int err, devno = MKDEV(char_major, index);

	cdev_init(&dev->cdev, &ChangeLetter_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if(err)
		printk(KERN_NOTICE "Error %d adding changeletter %d", err, index);
}

static int __init ChangeLetter_init(void)
{
	int ret;
	dev_t devno = MKDEV(char_major, 0);

	if (char_major)
		ret = register_chrdev_region(devno, 1, "changechar");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "changechar");
		char_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;

	changeletter_devp = kmalloc(sizeof(struct changeletter_dev), GFP_KERNEL);
	if (!changeletter_devp) {
		goto fail_malloc;
	}

	memset(changeletter_devp, 0, sizeof(struct changeletter_dev));

	changeletter_setup_cdev(changeletter_devp, 0);
	//init_MUTEX(&changeletter_devp->sem);
	sema_init(&changeletter_devp->sem, 1);
	init_waitqueue_head(&changeletter_devp->r_wait);
	init_waitqueue_head(&changeletter_devp->w_wait);
	return 0;
fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static void __exit ChangeLetter_exit(void)
{
	cdev_del(&changeletter_devp->cdev);
	kfree(changeletter_devp);
	unregister_chrdev_region(MKDEV(char_major, 0), 1);
	return;
}

int LetterOpen(struct inode *inode, struct file *filp)
{
	filp->private_data = changeletter_devp;
	printk(KERN_ALERT "File Opened!\n");
	return 0;
}

int LetterRelease(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "File Closed!\n");
	return 0;
}

static ssize_t LetterRead(struct file *filp, char *buf, size_t len, loff_t *off)
{
	unsigned long p = *off;
	struct changeletter_dev *dev = filp->private_data;
	int ret = 0;

	DECLARE_WAITQUEUE(wait, current);

	printk(KERN_ALERT "LetterRead1\n");
	down(&dev->sem);
	add_wait_queue(&dev->r_wait, &wait);

	while (dev->current_len == 0) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}
		printk(KERN_ALERT "LetterRead2\n");

		__set_current_state(TASK_INTERRUPTIBLE);
		up(&dev->sem);

		printk(KERN_ALERT "LetterRead3!\n");
		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto out2;
		}
		down(&dev->sem);
	}

	if (len > dev->current_len)
		len = dev->current_len;

	printk(KERN_ALERT "LetterRead4\n");
	if(copy_to_user(buf,(void *) (dev->kbuf + p), len))
	{
		printk(KERN_ALERT "LetterRead5\n");
		ret = -EFAULT;
		goto out;
	}
	else 
	{
		printk(KERN_ALERT "LetterRead6\n");
		memcpy(dev->kbuf, dev->kbuf + len, dev->current_len - len);
		dev->current_len -= len;

		wake_up_interruptible(&dev->w_wait);
		ret = len;
	}
out:
	up(&dev->sem);
out2: 
	remove_wait_queue(&dev->w_wait, &wait);
	set_current_state(TASK_RUNNING);
	return ret;
}

static ssize_t LetterWrite(struct file *filp,  char *buf, size_t len, loff_t *off)
{
	int i, ret = 0;
	struct changeletter_dev *dev = filp->private_data;
	DECLARE_WAITQUEUE(wait, current);

	down(&dev->sem);
	add_wait_queue(&dev->w_wait, &wait);

	while (dev->current_len == KBUF_LEN)  {
		printk(KERN_ALERT "Write1\n");
		if (filp->f_flags & O_NONBLOCK) {
			goto out;
		}
		printk(KERN_ALERT "Write2\n");
		__set_current_state(TASK_INTERRUPTIBLE);
		up(&dev->sem);
		schedule();
		if (signal_pending(current)) {
			printk(KERN_ALERT "Write3\n");
			ret = -ERESTARTSYS;
			goto out2;
		}
		down(&dev->sem);


		if (len > KBUF_LEN - dev->current_len)
			len = KBUF_LEN - dev->current_len;

		if(copy_from_user(dev->kbuf +dev->current_len, buf, len))
		{
			printk(KERN_ALERT "Write4\n");
			ret = -EFAULT;
			goto out;
		} else {
			printk(KERN_ALERT "Write5\n");
			dev->current_len += len;

			printk(KERN_ALERT "Write6\n");
			wake_up_interruptible(&dev->r_wait);
			ret = len;
		}
	}
	/*
	 *  write your code here
	 */
	for (i = 0; i < len; i++) {
		changeletter_devp->kbuf[i] = reverseLetter(changeletter_devp->kbuf[i]);
	}

	printk(KERN_ALERT "Write7\n");
out:
	up(&dev->sem);
out2:
	remove_wait_queue(&dev->r_wait, &wait);
	set_current_state(TASK_RUNNING);
	return ret;
}

char reverseLetter(char ch)
{
	char ret;
	if (ch == 0)
		return 0;

	if (('a' <= ch) && (ch <= 'z')) {
		ret = ch + 'A' - 'a';
	}
	else if (('A' <= ch) && (ch <= 'Z')) {
		ret = ch - 'A' + 'a';
	}
	else {
		ret = ch;
	}

	return ret;
}

module_init(ChangeLetter_init);
module_exit(ChangeLetter_exit);
