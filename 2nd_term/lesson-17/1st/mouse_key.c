#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static int usbmouse_key_probe(struct usb_interface *intf,const struct usb_device_id *id)
{
	printk("usbmouse probe success!");
	return 0;
}

static void usbmouse_key_disconnect(struct usb_interface *intf)
{
	printk("usbmouse disconnected!");
}

static const struct usb_device_id usbmouse_key_dev_table[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_MOUSE) }
};


static struct usb_driver usbmouse_key_drv = {
	.name       = "wzh-usbmouse_key_drv",
	.probe      = usbmouse_key_probe,
	.disconnect = usbmouse_key_disconnect,
	.id_table   = usbmouse_key_dev_table
};

static int usbmouse_key_init(void)
{
	usb_register(&usbmouse_key_drv);
	return 1;
}

static void usbmouse_key_exit(void)
{
	usb_deregister(&usbmouse_key_drv);
}

module_init(usbmouse_key_init);
module_exit(usbmouse_key_exit);
MODULE_LICENSE("GPL");

