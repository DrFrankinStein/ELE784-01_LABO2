# Variables pointing to different paths
#KERNEL_DIR    	:= /lib/modules/$(shell uname -r)/build
KERNEL_DIR  := /usr/src/linux-headers-3.16.0
PWD        	:= $(shell pwd)
OUTPUT		:= $(PWD)/output

ccflags-y	:= -std=gnu11 -Wno-declaration-after-statement -I../Include
obj-m       := Laboratoire2.o

all: ele784_driver

# We build our module in this section
ele784_driver:
	@echo "Building the ELE784 LAB2: CAM DRIVER"
	@make -C $(KERNEL_DIR) M=$(PWD) modules
	-@mkdir $(OUTPUT)
	-@mv -f -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order .tmp_versions $(OUTPUT)/

# It's always a good thing to offer a way to cleanup our development directory
clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order
	-rm -rf .tmp_versions
	-rm -rf $(OUTPUT)/

# Clean and rebuild. Simple as that!
remake: clean all
