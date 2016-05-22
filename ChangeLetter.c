#include <linux/fs.h>               
#include <linux/module.h>
#include <asm/uaccess.h>            
#include <linux/cdev.h>
#include <linux/init.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DJC");
#define DEV_NAME "ChangeLetter"
static ssize_t LetterRead(struct file *, char *, size_t, loff_t *);
static ssize_t LetterWrite(struct file *, char *, size_t, loff_t *);

// user defined function to reverse char.
char reverseLetter(char ch);

#define KBUF_LEN	32

static int char_major = 250; 

struct changeletter_dev {
	struct cdev cdev;
	char kbuf[KBUF_LEN];
};
struct changeletter_dev dev;

static const struct file_operations ChangeLetter_fops =
{
	.read = LetterRead,
	.write = LetterWrite
};

static int __init ChangeLetter_init(void)
{
	int ret, err;
	dev_t devno = MKDEV(char_major, 0);

	if (char_major)
		ret = register_chrdev_region(devno, 1, "changechar");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "changechar");
		char_major = MAJOR(devno);
	}

	if (ret < 0)
		return ret;

	printk(KERN_NOTICE "Error adding changeletter");

	cdev_init(&dev.cdev, &ChangeLetter_fops);
	dev.cdev.owner = THIS_MODULE;
	err = cdev_add(&dev.cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding changeletter", err);
	return 0;
}

static void __exit ChangeLetter_exit(void)
{
	cdev_del(&dev.cdev);
	unregister_chrdev_region(MKDEV(char_major, 0), 1);
	return;
}

static ssize_t LetterRead(struct file *filp, char *buf, size_t len, loff_t *off)
{
	int ret = 0;
	if (len > 30)
		len = 30;


	if(copy_to_user(buf, dev.kbuf, len))
	{
		printk(KERN_ALERT "Letter Write Error\n");
		return -EFAULT;
	}
	return ret;
}

static ssize_t LetterWrite(struct file *filp,  char *buf, size_t len, loff_t *off)
{
	int i = 0;
	if (len > 30)
		len = 30;

	if(copy_from_user(dev.kbuf , buf, len))
	{
		printk(KERN_ALERT "Letter Write Error\n");
		return -EFAULT;
	}
	/*
	 *  write your code here
	 */
	for (i = 0; i < len; i++) {
		dev.kbuf[i] = reverseLetter(dev.kbuf[i]);
	}

	printk(KERN_ALERT "Letter Write %s\n", dev.kbuf);
	return len;
}

char reverseLetter(char ch)
{
	if (ch == 0)
		return 0;

	char ret;

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
