#include <linux/fs.h>               
#include <linux/module.h>
#include <asm/uaccess.h>            
#include <linux/init.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DJC");
#define DEV_NAME "ChangeLetter"
static ssize_t LetterRead(struct file *, char *, size_t, loff_t *);
static ssize_t LetterWrite(struct file *, char *, size_t, loff_t *);

// user defined function to reverse char.
char reverseLetter(char ch);

static int char_major = 0;                  
static char kbuf[32] = {0};
static const struct file_operations ChangeLetter_fops =
{
	.read = LetterRead,
	.write = LetterWrite
};

static int __init ChangeLetter_init(void)
{
	int ret;
	ret = register_chrdev(char_major, DEV_NAME, &ChangeLetter_fops);
	if (ret < 0)
	{
		printk(KERN_ALERT "ChangeLetter Reg Fail ! \n");
	}
	else
	{
		printk(KERN_ALERT "ChangeLetter Reg Success ! \n");
		char_major = ret;
		printk(KERN_ALERT "Major = %d \n",char_major);
	}
	return ret;
}

static void __exit ChangeLetter_exit(void)
{
	unregister_chrdev(char_major, DEV_NAME);
	return;
}

static ssize_t LetterRead(struct file *filp, char *buf, size_t len, loff_t *off)
{
	int ret = 0;
	if (len > 30)
		len = 30;


	if(copy_to_user(buf, kbuf, len))
	{
		printk(KERN_ALERT "Letter Write Error\n");
		return -EFAULT;
	}
	printk(KERN_ALERT "Letter Write %s\n", kbuf);
	return ret;
}

static ssize_t LetterWrite(struct file *filp,  char *buf, size_t len, loff_t *off)
{
	int i = 0;
	if (len > 30)
		len = 30;

	if(copy_from_user(kbuf , buf, len))
	{
		printk(KERN_ALERT "Letter Write Error\n");
		return -EFAULT;
	}
	/*
	 *  write your code here
	 */
	for (i = 0; i < len; i++) {
		kbuf[i] = reverseLetter(kbuf[i]);
	}

	printk(KERN_ALERT "Letter Write %s\n", kbuf);
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
