#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>

#define MAX_LEN 100
struct dentry *demo_debugfs_root;

u8 p0 = 100;
char str[MAX_LEN] = "Hello DebugFs!\n";
struct debugfs_blob_wrapper p1;

static int p2_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t p2_read(struct file *filp, char __user *buffer,
		size_t count, loff_t *ppos)
{
	if (*ppos >= MAX_LEN)
		return 0;
	if (*ppos + count > MAX_LEN)
		count = MAX_LEN - *ppos;
	
	// memset(buffer, 0, MAX_LEN);
	if (copy_to_user(buffer, str + *ppos, count))
		return -EFAULT;	
	
	*ppos += count;
	
	return count;
}

static ssize_t p2_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{	
	if (*ppos >= MAX_LEN)
		return 0;	
	if (*ppos + count > MAX_LEN)
		count = MAX_LEN - *ppos;
	
	// memset(str, 0, MAX_LEN);
	if (copy_from_user(str + *ppos, buffer, count))
		return -EFAULT;	
	
	*ppos += count;
	
	return count;
}

struct file_operations p2_fops = {
	.owner = THIS_MODULE,
	.open = p2_open,
	.read = p2_read,
	.write = p2_write,
};

static int __init demo_debugfs_init(void)
{
	struct dentry *sub_dir, *p0_dentry, *p1_dentry, *p2_dentry;
	
	demo_debugfs_root = debugfs_create_dir("debugfs-demo", NULL);
	if (!demo_debugfs_root)
		return -ENOENT;

	p0_dentry = debugfs_create_u8("p0", 0644, demo_debugfs_root, &p0);
	if (!p0_dentry)
		goto Fail;

	p1.data = (void *)str;
	p1.size = strlen(str) + 1;
	p1_dentry = debugfs_create_blob("p1", 0644, demo_debugfs_root, &p1);
	if (!p1_dentry)
		goto Fail;

	sub_dir = debugfs_create_dir("subdir", demo_debugfs_root);
	if (!sub_dir)
		goto Fail;

	p2_dentry = debugfs_create_file("p2", 0644, sub_dir, NULL, &p2_fops);
	if (!p2_dentry)
		goto Fail;
	        
	return 0;

Fail:
	debugfs_remove_recursive(demo_debugfs_root);
	demo_debugfs_root = NULL;
	return -ENOENT;
}

static void __exit demo_debugfs_exit(void)
{
	debugfs_remove_recursive(demo_debugfs_root);
	return;
}

module_init(demo_debugfs_init);
module_exit(demo_debugfs_exit);

MODULE_LICENSE("GPL");

