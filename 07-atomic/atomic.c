#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/semaphore.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名  		: semaphore.c
作者      	: 正点原子Linux团队
版本      	: V1.0
描述      	: 信号量实验，使用信号量来实现对实现设备的互斥访问。
其他      	: 无
论坛      	: www.openedv.com
日志      	: 初版V1.0 2022/12/08 正点原子Linux团队创建
***************************************************************/
#define GPIOLED_CNT      		1           	/* 设备号个数 	*/
#define GPIOLED_NAME       	"gpioled" 	/* 名字 		*/
#define LEDOFF              	0           	/* 关灯 		*/
#define LEDON               	1           	/* 开灯 		*/

/* gpioled设备结构体 */
struct gpioled_dev{
    dev_t devid;            	/* 设备号     	*/
    struct cdev cdev;       	/* cdev     	*/
    struct class *class;    	/* 类     	 	*/
    struct device *device;  	/* 设备    		*/
    int major;              	/* 主设备号   	*/
    int minor;              	/* 次设备号   	*/
    struct device_node  *nd;	/* 设备节点 		*/
    int led_gpio;           	/* led所使用的GPIO编号        */
    struct semaphore sem;   	/* 信号量 		*/
};

static struct gpioled_dev gpioled;  /* led设备 */


/*
 * @description   	: 打开设备
 * @param - inode   	: 传递给驱动的inode
 * @param - filp    	: 设备文件，file结构体有个叫做private_data的成员变
 *                         量一般在open的时候将private_data指向设备结构体。
 * @return          	: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &gpioled; /* 设置私有数据 */

    /* 获取信号量 */
    if (down_interruptible(&gpioled.sem)) { 
        return -ERESTARTSYS;
    }
#if 0
    down(&gpioled.sem);     /* 不能被信号打断 */
#endif

    return 0;
}
..
static int led_release(struct inode *inode, struct file *filp)
{
    struct gpioled_dev *dev = filp->private_data;

    up(&dev->sem);  /* 释放信号量，信号量count值加1 */
    return 0;
}

/* 设备操作函数 */
static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release =  led_release,
};

/*
 * @description 	: 驱动出口函数
 * @param       	: 无
 * @return      	: 无
 */
static int __init led_init(void)
{
    int ret = 0;
    const char *str;

    /* 初始化信号量 */
    sema_init(&gpioled.sem, 1);
..
}

/*
 * @description 	: 驱动出口函数
 * @param       	: 无
 * @return      	: 无
 */
static void __exit led_exit(void)
{
    /* 注销字符设备驱动 */
    cdev_del(&gpioled.cdev);/*  删除cdev */
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
    device_destroy(gpioled.class, gpioled.devid);/* 注销设备 */
    class_destroy(gpioled.class);/* 注销类 		*/
    gpio_free(gpioled.led_gpio); /* 释放GPIO 	*/
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
