#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/io.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4GNU_Linux");
MODULE_DESCRIPTION("A driver for Arduino UNO with CH340 chip based on Johannes 4 GNU/Linux driver");

#define VENDOR_ID 0x1a86
#define PRODUCT_ID 0x7523

#define MYMAJOR 64
#define DEVNAME "mydev"

static void *my_data;

static struct usb_device *usb_dev;

/**
 * @brief Read data out of the buffer
 */
static ssize_t my_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offs)
{
	int not_copied, to_copy = (len > PAGE_SIZE) ? PAGE_SIZE : len;
	not_copied = copy_to_user(user_buffer, my_data, to_copy);
	return to_copy - not_copied;
}

/**
 * @brief Write data to buffer
 */
static ssize_t my_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offs)
{
	int not_copied, to_copy = (len > PAGE_SIZE) ? PAGE_SIZE : len;
	not_copied = copy_from_user(my_data, user_buffer, to_copy);
	return to_copy - not_copied;
}

static int my_mmap(struct file *file, struct vm_area_struct *vma)
{
	int status;

	vma->vm_pgoff = virt_to_phys(my_data) >> PAGE_SHIFT;

	status = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, 
				 vma->vm_end - vma->vm_start, vma->vm_page_prot);
	if(status) {
		printk("arduino_driver - Error remap_pfn_range: %d\n", status);
		return -EAGAIN;
	}
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.mmap = my_mmap,
};

static struct usb_device_id usb_dev_table [] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{},
};
MODULE_DEVICE_TABLE(usb, usb_dev_table);

static int my_usb_probe(struct usb_interface *intf, const struct usb_device_id *id) {
	printk("arduino_driver: Probe Function\n");

	usb_dev = interface_to_usbdev(intf);
	if(usb_dev == NULL) {
		printk("arduino_driver: Error getting device from interface\n");
		return -1;
	}

	return 0;
}

static void my_usb_disconnect(struct usb_interface *intf) {
	printk("arduino_driver: Disconnect Function\n");
}

static struct usb_driver my_usb_driver = {
	.name = "arduino_driver",
	.id_table = usb_dev_table,
	.probe = my_usb_probe,
	.disconnect = my_usb_disconnect,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	int result;
	printk("arduino_driver: - Init Function\n");
	result = usb_register(&my_usb_driver);
	if(result) {
		printk("arduino_driver: Error during register!\n");
		return -result;
	}
		int status;

	printk("arduino_driver - Hello!\n");
	my_data = kzalloc(PAGE_SIZE, GFP_DMA);
	if(!my_data)
		return -ENOMEM;

	printk("arduino_driver - I have allocated a page (%ld Bytes)\n", PAGE_SIZE);
	printk("arduino_driver - PAGESHIFT: %d\n", PAGE_SHIFT);

	status = register_chrdev(MYMAJOR, DEVNAME, &fops);
	if(status < 0) {
		printk("arduino_driver - Error registering device number!\n");
		kfree(my_data);
		return status;
	}

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("arduino_driver: Exit Function\n");
	if(my_data)
		kfree(my_data);
	unregister_chrdev(MYMAJOR, DEVNAME);
	usb_deregister(&my_usb_driver);
}

module_init(my_init);
module_exit(my_exit);


