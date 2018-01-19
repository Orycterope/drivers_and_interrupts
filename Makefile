TARGET= keylogger

KDIR = /lib/modules/$(shell uname -r)/build

obj-m += $(TARGET).o
$(TARGET)-y := module_main.o dev_file.o interrupt_handler.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

