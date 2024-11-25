#include <linux/init.h>
#include <linux/module.h>

static int __init hello_kernel_module_init(void)
{
	printk(KERN_INFO "Hello from the kernel module!\n");
	return 0;
}

static void __exit hello_kernel_module_exit(void)
{
	printk(KERN_INFO "Goodbye from the kernel module!\n");
}

module_init(hello_kernel_module_init);
module_exit(hello_kernel_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("alientek");
