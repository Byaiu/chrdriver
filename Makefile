obj-m := ChangeLetter.o
KDIR := /lib/modules/$(shell uname -r)/build
SRCPWD := $(shell pwd)
all:
	make -C $(KDIR) M=$(SRCPWD) modules
clean:
	make -C $(KDIR) M=$(SRCPWD) clean	

test: test.c
	gcc test.c -o test
