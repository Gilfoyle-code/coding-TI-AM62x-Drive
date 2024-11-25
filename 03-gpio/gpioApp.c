/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名       : gpioApp.c
作者      : 正点原子
版本      : V1.0
描述      : chrdevbase驱测试APP。
其他      : 无
使用方法     ：./gpioApp /dev/gpio  0 关闭gpio
             ./gpioApp /dev/gpio  1 打开gpio      
论坛      : www.openedv.com
日志      : 初版V1.0 2022/12/02 正点原子团队创建
***************************************************************/
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define gpioOFF     0
#define gpioON  1

/*
 * @description     : main主程序
 * @param - argc    : argv数组元素个数
 * @param - argv    : 具体参数
 * @return          : 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    unsigned char databuf[1];
    
    if(argc != 3){
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    /* 打开gpio驱动 */
    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("file %s open faigpio!\r\n", argv[1]);
        return -1;
    }

    databuf[0] = atoi(argv[2]); /* 要执行的操作：打开或关闭 */

    /* 向/dev/gpio文件写入数据 */
    retvalue = write(fd, databuf, sizeof(databuf));
    if(retvalue < 0){
        printf("gpio Control Faigpio!\r\n");
        close(fd);
        return -1;
    }

    retvalue = close(fd); /* 关闭文件 */
    if(retvalue < 0){
        printf("file %s close faigpio!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
