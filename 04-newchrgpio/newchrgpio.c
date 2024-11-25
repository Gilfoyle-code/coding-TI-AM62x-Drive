/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名       : newchrgpio.c
作者          : 正点原子
版本          : V1.0
描述          : gpio驱动文件。
其他          : 无
论坛          : www.openedv.com
日志          : 初版V1.0 2022/12/02 正点原子团队创建
***************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRGPIO_CNT          1                   /* 设备号个数  */
#define NEWCHRGPIO_NAME         "newchrgpio"        /* 名字       */
#define GPIO_HIGH               1                   /* 低电平        */
#define GPIO_LOW                0                   /* 高电平        */

#define PADCFG_CTRL0_CFG0_PADCONFIG0    (0xF4034)

#define GPIO_DIR01  (0x600010)
#define GPIO_OUT_DATA01 (0x600014)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR;
static void __iomem *GPIO_DIR01_ADDR;
static void __iomem *GPIO_OUT_DATA01_ADDR;

/* newchrgpio设备结构体 */
struct newchrgpio_dev{
    dev_t devid;                /* 设备号        */
    struct cdev cdev;           /* cdev         */
    struct class *class;        /* 类              */
    struct device *device;      /* 设备           */
    int major;                  /* 主设备号     */
    int minor;                  /* 次设备号     */
};

struct newchrgpio_dev newchrgpio; /* gpio设备 */

/*
 * @description     : gpio打开/关闭
 * @param - sta     : gpioON(0) 打开gpio，gpioOFF(1) 关闭gpio
 * @return          : 无
 */
void gpio_switch(u8 sta)
{
    u32 val = 0;
    if(sta == GPIO_HIGH) {
        val = readl(GPIO_OUT_DATA01_ADDR);
        val |= (0X1 << 13); /* bit0 置1*/
        writel(val, GPIO_OUT_DATA01_ADDR);
    }else if(sta == GPIO_LOW) { 
        val = readl(GPIO_OUT_DATA01_ADDR);
        val &= ~(0X1 << 13); /* bit0 清零*/
        writel(val, GPIO_OUT_DATA01_ADDR);
    }   
}

/*
 * @description     : 物理地址映射
 * @return          : 无
 */
void gpio_remap(void)
{
    PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR = ioremap(PADCFG_CTRL0_CFG0_PADCONFIG0, 4);
    GPIO_DIR01_ADDR = ioremap(GPIO_DIR01, 4);
    GPIO_OUT_DATA01_ADDR = ioremap(GPIO_OUT_DATA01, 4);
}

/*
 * @description     : 取消映射
 * @return          : 无
 */
void gpio_unmap(void)
{
    /* 取消映射 */
    iounmap(PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR);
    iounmap(GPIO_DIR01_ADDR);
    iounmap(GPIO_OUT_DATA01_ADDR);
}

/*
 * @description     : 打开设备
 * @param – inode : 传递给驱动的inode
 * @param - filp    : 设备文件，file结构体有个叫做private_data的成员变量
 *                    一般在open的时候将private_data指向设备结构体。
 * @return          : 0 成功;其他 失败
 */
static int gpio_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &newchrgpio; /* 设置私有数据 */
    return 0;
}

/*
 * @description     : 从设备读取数据 
 * @param - filp    : 要打开的设备文件(文件描述符)
 * @param - buf     : 返回给用户空间的数据缓冲区
 * @param - cnt     : 要读取的数据长度
 * @param - offt    : 相对于文件首地址的偏移
 * @return          : 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t gpio_read(struct file *filp, char __user *buf, 
size_t cnt, loff_t *offt)
{
    return 0;
}

/*
 * @description     : 向设备写数据 
 * @param – filp  : 设备文件，表示打开的文件描述符
 * @param - buf     : 要写给设备写入的数据
 * @param - cnt     : 要写入的数据长度
 * @param - offt    : 相对于文件首地址的偏移
 * @return          : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t gpio_write(struct file *filp, const char __user *buf, 
size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char gpiostat;

    retvalue = copy_from_user(databuf, buf, cnt);
    if(retvalue < 0) {
        printk("kernel write faigpio!\r\n");
        return -EFAULT;
    }

    gpiostat = databuf[0];       /* 获取状态值 */

    if(gpiostat == GPIO_HIGH) {  
        gpio_switch(GPIO_HIGH);      /* 设置GPIO高电平 */
    } else if(gpiostat == GPIO_LOW) {
        gpio_switch(GPIO_LOW);     /* 设置GPIO低电平 */
    }
    return 0;
}

/*
 * @description   : 关闭/释放设备
 * @param - filp    : 要关闭的设备文件(文件描述符)
 * @return          : 0 成功;其他 失败
 */
static int gpio_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* 设备操作函数 */
static struct file_operations newchrgpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .read = gpio_read,
    .write = gpio_write,
    .release =  gpio_release,
};

/*
 * @description     : 驱动出口函数
 * @param           : 无
 * @return          : 无
 */
static int __init gpio_init(void)
{
    u32 val = 0;
    int ret;

    /* 初始化gpio */
    /* 1、寄存器地址映射 */
    gpio_remap();

    /* 2、设置GPIO0_0为GPIO功能。*/
    val = readl(PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR);
    val |= (0X1 << 16)| 7; //bit14 置1,

    writel(val, PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR);

    /* 3、设置GPIO0_0为输出 */
    val = readl(GPIO_DIR01_ADDR);
    val &= ~(0X1 << 13); /* bit0 清零*/
    writel(val, GPIO_DIR01_ADDR);

    /* 4、设置GPIO0_0为低电平，关闭gpio灯。*/
    val = readl(GPIO_OUT_DATA01_ADDR);
    val &= ~(0X1 << 13); /* bit0 清零*/
    writel(val, GPIO_OUT_DATA01_ADDR);

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (newchrgpio.major) {      /*  定义了设备号 */
        newchrgpio.devid = MKDEV(newchrgpio.major, 0);
        ret = register_chrdev_region(newchrgpio.devid, NEWCHRGPIO_CNT, 
NEWCHRGPIO_NAME);
         if(ret < 0) {
             pr_err("cannot register %s char driver [ret=%d]\n",NEWCHRGPIO_NAME, NEWCHRGPIO_CNT);
             goto fail_map;
         }
     } else {                        /* 没有定义设备号 */
         ret = alloc_chrdev_region(&newchrgpio.devid, 0, NEWCHRGPIO_CNT, 
NEWCHRGPIO_NAME);  /* 申请设备号 */
         if(ret < 0) {
             pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", 
NEWCHRGPIO_NAME, ret);
             goto fail_map;
         }
         newchrgpio.major = MAJOR(newchrgpio.devid);/* 获取主设备号 */
         newchrgpio.minor = MINOR(newchrgpio.devid);/* 获取次设备号 */
     }
     printk("newchegpio major=%d,minor=%d\r\n",newchrgpio.major, 
newchrgpio.minor); 
     
     /* 2、初始化cdev */
     newchrgpio.cdev.owner = THIS_MODULE;
     cdev_init(&newchrgpio.cdev, &newchrgpio_fops);
     
     /* 3、添加一个cdev */
     ret = cdev_add(&newchrgpio.cdev, newchrgpio.devid, NEWCHRGPIO_CNT);
     if(ret < 0)
         goto del_unregister;
         
     /* 4、创建类 */
     newchrgpio.class = class_create(THIS_MODULE, NEWCHRGPIO_NAME);
     if (IS_ERR(newchrgpio.class)) {
         goto del_cdev;
     }
 
     /* 5、创建设备 */
     newchrgpio.device = device_create(newchrgpio.class, NULL, 
newchrgpio.devid, NULL, NEWCHRGPIO_NAME);
     if (IS_ERR(newchrgpio.device)) {
         goto destroy_class;
     }
     
     return 0;
 
destroy_class:
    class_destroy(newchrgpio.class);
del_cdev:
    cdev_del(&newchrgpio.cdev);
del_unregister:
    unregister_chrdev_region(newchrgpio.devid, NEWCHRGPIO_CNT);
fail_map:
    gpio_unmap();
    return -EIO;
}

/*
 * @description     : 驱动出口函数
 * @param           : 无
 * @return          : 无
 */
static void __exit gpio_exit(void)
{
    /* 取消映射 */
    gpio_unmap();
   
    /* 注销字符设备驱动 */
    cdev_del(&newchrgpio.cdev);/*  删除cdev */
    unregister_chrdev_region(newchrgpio.devid, NEWCHRGPIO_CNT);

    device_destroy(newchrgpio.class, newchrgpio.devid);
    class_destroy(newchrgpio.class);
}

module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
