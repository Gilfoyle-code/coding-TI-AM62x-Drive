#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
//#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/fcntl.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/* 寄存器物理地址 */
#define OSPI0_CSn2    (0xF4034)

#define GPIO_DIR01  (0x600010)
#define GPIO_OUT_DATA01 (0x600014)

#define REGISTER_LENGTH         	4

/* @description  	: 释放flatform设备模块的时候此函数会执行 
 * @param - dev  	: 要释放的设备 
 * @return        	: 无
 */
static void led_release(struct device *dev)
{
    printk("led device released!\r\n"); 
}

/*  
 * 设备资源信息，也就是LED0所使用的所有寄存器
 */
static struct resource led_resources[] = {
    [0] = {
        .start  = OSPI0_CSn2,
        .end    = (OSPI0_CSn2 + REGISTER_LENGTH - 1),
        .flags  = IORESOURCE_MEM,
    },  
    [1] = {
        .start  = GPIO_DIR01,
        .end    = (GPIO_DIR01 + REGISTER_LENGTH - 1),
        .flags  = IORESOURCE_MEM,
    },
    [2] = {
        .start  = GPIO_OUT_DATA01,
        .end    = (GPIO_OUT_DATA01 + REGISTER_LENGTH - 1),
        .flags  = IORESOURCE_MEM,
    },
};


/*
 * platform设备结构体 
 */
static struct platform_device leddevice = {
    .name = "AM62x-led",
    .id = -1,
    .dev = {
        .release = &led_release,
    },
    .num_resources = ARRAY_SIZE(led_resources),
    .resource = led_resources,
};
        
/*
 * @description 	: 设备模块加载 
 * @param       	: 无
 * @return      	: 无
 */
static int __init leddevice_init(void)
{
    return platform_device_register(&leddevice);
}

/*
 * @description 	: 设备模块注销
 * @param       	: 无
 * @return      	: 无
 */
static void __exit leddevice_exit(void)
{
    platform_device_unregister(&leddevice);
}

module_init(leddevice_init);
module_exit(leddevice_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
MODULE_INFO(intree, "Y");
