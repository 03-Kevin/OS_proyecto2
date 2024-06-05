obj-m += arduino_driver.o

all: module testapp

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm a.out

testapp: test.c
	gcc test.c

showDriverLogs:
	sudo dmesg | grep arduino_driver

showLogs:
	sudo dmesg | tail -10

installArduinoDriver:
	sudo insmod arduino_driver.ko
	sudo mknod /dev/mydev c 64 0

uninstallDefaultDriver:
	sudo rmmod ch341

uninstallArduinoDriver:
	sudo rmmod arduino_driver

showMods:
	lsmod | head -10