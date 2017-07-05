#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

char *usb_buffer;

static void usbmouse_key_irq(struct urb *usbmouse_urb)
{
	unsigned int i;
	for(i = 0; i < sizeof(usb_buffer); i++)
		printk("0x%x ", usb_buffer[i]);
	printk("\n");
	usb_submit_urb(usbmouse_urb, GFP_ATOMIC);
}

static int usbmouse_key_probe(struct usb_interface *intf,const struct usb_device_id *id)
{
	struct usb_device *usbmouse_dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct urb *usbmouse_urb;
	struct input_dev *usbmouse_input_dev;
	static dma_addr_t usb_phy_addr;
	unsigned int pipe;
	__le16 len;
	
	/* 1.分配地址*/
	usbmouse_input_dev = input_allocate_device();
	/* 2.设置事件类*/
	usbmouse_input_dev->evbit[EV_KEY/32] = 1 << EV_KEY; //应该没问题
	//set_bit(EV_KEY, usbmouse_input_dev->evbit);   
	set_bit(EV_REP, usbmouse_input_dev->evbit);
	/* 3.设置时间类下的具体事件*/
	set_bit(KEY_L,     usbmouse_input_dev->keybit);
	set_bit(KEY_S,     usbmouse_input_dev->keybit);
	set_bit(KEY_ENTER, usbmouse_input_dev->keybit);
	/* 4.注册驱动*/
	input_register_device(usbmouse_input_dev);
	/* 5.usb中断相关操作*/
	usbmouse_urb = usb_alloc_urb(0, GFP_KERNEL);
	interface = intf->cur_altsetting;
	endpoint  = &interface->endpoint[0].desc;
	/* USB数据源*/
	pipe = usb_rcvintpipe(usbmouse_dev, endpoint->bEndpointAddress);
	/* 数据长度*/
	len = endpoint->wMaxPacketSize;
	/* USB目的*/
	usb_buffer = usb_buffer_alloc(usbmouse_dev, len, GFP_ATOMIC, &usb_phy_addr);
	/* USB中断设置*/
	usb_fill_int_urb(usbmouse_urb, usbmouse_dev, pipe, usb_buffer, len, usbmouse_key_irq, NULL, endpoint->bInterval);
	usbmouse_urb->transfer_dma = usb_phy_addr;
	usbmouse_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_submit_urb(usbmouse_urb, GFP_ATOMIC);
	
	printk("bcdUSB    = %x\n",   usbmouse_dev->descriptor.bcdUSB);
	printk("idVendor  = 0x%x\n", usbmouse_dev->descriptor.idVendor);
	printk("idProduct = 0x%x\n", usbmouse_dev->descriptor.idProduct);
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

