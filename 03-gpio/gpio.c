/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名       : gpio.c
作者      : 正点原子
版本      : V1.0
描述      : gpio驱动文件。
其他      : 无
论坛      : www.openedv.com
日志      : 初版V1.0 2022/12/02 正点原子团队创建
***************************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define GPIO_MAJOR      200     /* 主设备号 */
#define GPIO_NAME       "gpio"  /* 设备名字 */

#define GPIO_LOW    0               /* 关灯 */
#define GPIO_HIGH   1               /* 开灯 */

#define PADCFG_CTRL0_CFG0_PADCONFIG0    (0xF4034)

#define GPIO_DIR01  (0x600010)
#define GPIO_OUT_DATA01 (0x600014)

/* 映像后的寄存器虚拟地址指针 */
static void __iomem *PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR;
static void __iomem *GPIO_DIR01_ADDR;
static void __iomem *GPIO_OUT_DATA01_ADDR;

/*
 * @description     : GPIO高电平/GPIO低电平
 * @param - sta     : GPIO_HIGH(0) 设置GPIO高电平，GPIO_LOW(1) 设置GPIO低电平
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
 * @description     : 物理地址映像
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
 * @param - inode   : 传递给驱动的inode
 * @param - filp    : 设备文件，file结构体有个叫做private_data的成员变量
 *                    一般在open的时候将private_data指向设备结构体。
 * @return          : 0 成功;其他 失败
 */
static int gpio_open(struct inode *inode, struct file *filp)
{
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
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

/*
 * @description     : 向设备写数据 
 * @param - filp    : 设备文件，表示打开的文件描述符
 * @param - buf     : 要写给设备写入的数据
 * @param - cnt     : 要写入的数据长度
 * @param - offt    : 相对于文件首地址的偏移
 * @return          : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t gpio_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char gpiostat;

    retvalue = copy_from_user(databuf, buf, cnt);
    if(retvalue < 0) {
        printk("kernel write faigpio!\r\n");
        return -EFAULT;
    }

    gpiostat = databuf[0];      /* 获取状态值 */

    if(gpiostat == GPIO_HIGH) { 
        gpio_switch(GPIO_HIGH);     /* 打开gpio灯 */
    } else if(gpiostat == GPIO_LOW) {
        gpio_switch(GPIO_LOW);      /* 关闭gpio灯 */
    }
    return 0;
}

/*
 * @description     : 关闭/释放设备
 * @param - filp    : 要关闭的设备文件(文件描述符)
 * @return          : 0 成功;其他 失败
 */
static int gpio_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* 设备操作函数 */
static struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .read = gpio_read,
    .write = gpio_write,
    .release =  gpio_release,
};

/*
 * @description : 驱动出口函数
 * @param       : 无
 * @return      : 无
 */
static int __init gpio_init(void)
{
    int retvalue = 0;
    u32 val = 0;

    /* 初始化gpio */
    /* 1、寄存器地址映像 */
    gpio_remap();

    /* 2、设置GPIO0_0为GPIO功能。*/
    val = readl(PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR);
    val &= ~(0XFFFFFFFF << 0);/* all 清零*/
    val |= (((0 << 18) | (1 << 16)) | 7); //bit14 置1,

    writel(val, PADCFG_CTRL0_CFG0_PADCONFIG0_ADDR);

    /* 3、设置GPIO0_0为输出 */
    val = readl(GPIO_DIR01_ADDR);
    val &= ~(0X1 << 13); /* bit0 清零*/
    writel(val, GPIO_DIR01_ADDR);

    /* 4、设置GPIO0_0为低电平，关闭gpio灯。*/
    val = readl(GPIO_OUT_DATA01_ADDR);
    val &= ~(0X1 << 13); /* bit0 清零*/
    writel(val, GPIO_OUT_DATA01_ADDR);

    /* 5、注册字符设备驱动 */
    retvalue = register_chrdev(GPIO_MAJOR, GPIO_NAME, &gpio_fops);
    if(retvalue < 0) {
        printk("register chrdev faigpio!\r\n");
        goto fail_map;
    }
    return 0;
    
fail_map:
    gpio_unmap();
    return -EIO;
}

/*
 * @description : 驱动出口函数
 * @param       : 无
 * @return      : 无
 */
static void __exit gpio_exit(void)
{
    /* 取消映射 */
    gpio_unmap();

    /* 注销字符设备驱动 */
    unregister_chrdev(GPIO_MAJOR, GPIO_NAME);
}

module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
