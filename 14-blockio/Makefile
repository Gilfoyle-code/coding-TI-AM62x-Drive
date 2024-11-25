KERNELDIR := /home/alientek/ATK-AM62x-SDK/board-support/atk-linux-kernel-6.1.46
CURRENT_PATH := $(shell pwd)
obj-m := blockio.o

CROSS_COMPILE := aarch64-oe-linux-
ARCH := arm64

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

