KERNELDIR := /home/alientek/ATK-AM62x-SDK/board-support/atk-linux-kernel-6.1.46
CURRENT_PATH := $(shell pwd)

obj-m := keyinput.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean