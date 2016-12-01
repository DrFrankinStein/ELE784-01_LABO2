//===================================================
//
// AUTHOR : JULIEN LEMAY    (LEMJ16059303)
//          THIERRY DESTIN  (DEST03099102)
//
// SCHOOL : ECOLE DE TECHNOLOGIES SUPERIEURES
//
// CLASS : ELE784 (FALL 2016)
//
// PROJECT : LABORATOIRE2
//
// FILE : Laboratoire2.c
//
// DESCRIPTION : Main code for a linux 
//               camera char device
//
// LAST MODIFICATION : Thursday, November 24th 2016
//
//===================================================

// Define these values to match your devices 
#define USB_CAM_VENDOR_ID		0x046d
#define USB_CAM_PRODUCT_ID_A	0x08cc	
#define USB_CAM_PRODUCT_ID_B	0x0994
#define USB_CAM_MINOR_BASE		0

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>    //Error list
#include <linux/fcntl.h>    //File access ex : R/W
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/usb.h>
//#include <linux/kref.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include "../Include/Laboratoire2.h"
#include "../Include/usbvideo.h"

MODULE_LICENSE("Dual BSD/GPL");

static int ele784_open (struct inode *inode, struct file *filp);
static int ele784_release (struct inode *inode, struct file *filp);
static ssize_t ele784_read (struct file *filp, char __user *ubuf, size_t count,loff_t *f_ops);
static long ele784_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);
static int ele784_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void ele784_disconnect(struct usb_interface *interface);

// table of devices that work with this driver 
static struct usb_device_id camera_id [] = 
{
	{ USB_DEVICE(USB_CAM_VENDOR_ID, USB_CAM_PRODUCT_ID_A) },
	{ USB_DEVICE(USB_CAM_VENDOR_ID, USB_CAM_PRODUCT_ID_B) },
	{ }					// Terminating entry 
};
MODULE_DEVICE_TABLE (usb, camera_id);

// Implemented function for USB driver
static struct usb_driver ele784_usb_driver = 
{
	//.owner    	= THIS_MODULE,
	.name       = "OrbiteCam",
	.id_table   = camera_id,
	.probe      = ele784_probe,
	.disconnect = ele784_disconnect,
};

static struct file_operations ele784_fops = 
{
	.owner    			= THIS_MODULE,
	.open     			= ele784_open,
	.release  			= ele784_release, //"close"
	.read     			= ele784_read,
	.unlocked_ioctl 	= ele784_ioctl,
};

static struct usb_class_driver ele784_class = {
	//.name = "etsele_cdev%d",
	.name = "etsele_cdev",
	.fops = &ele784_fops,
	.minor_base = USB_CAM_MINOR_BASE,
};

// Structure to hold all of our device specific stuff 
struct usb_ele784
{
	struct usb_device    	*dev;			   				// the usb device for this device
	struct usb_interface 	*interface;	   				// the interface for this device 
	unsigned char        	*bulk_in_buffer;				// the buffer to receive data 
	size_t			      	bulk_in_size;   				// the size of the receive buffer 
	//__u8							bulk_in_endpointAddr;	   // the address of the bulk in endpoint 
	//__u8							bulk_out_endpointAddr;	   // the address of the bulk out endpoint 
	//struct kref					kref;
};
//#define to_ele784_dev(d) container_of(d, struct usb_ele784, kref)


/*static void ele784_delete(struct kref *kref)
{	
	struct usb_ele784 *dev = to_ele784_dev(kref);

	usb_put_dev(dev->dev);
	kfree (dev->bulk_in_buffer);
	kfree (dev);
}*/

//===================================================
//
// Open the driver
//
// inode : 
// filp  : File pointer
//
// Return : 0 if successful, -1 if error 
//
//===================================================
static int ele784_open (struct inode *inode, struct file *filp)
{
	printk(KERN_WARNING"Calling : %s\n",__FUNCTION__);

	struct usb_interface *intf;
	int subminor;

	subminor = iminor(inode);

	intf = usb_find_interface(&ele784_usb_driver, subminor);
	if (!intf)
	{
		printk(KERN_WARNING"%s : Cannot open driver", __FUNCTION__);
		return -ENODEV;
	}	
	
	filp->private_data = intf;
	return 0;
}



//===================================================
//
// Close the driver
//
// inode : 
// filp  : File pointer
//
// Return : 0 if successful, -1 if error 
//
//===================================================
static int ele784_release (struct inode *inode, struct file *filp)
{
    printk(KERN_WARNING"Calling : %s\n",__FUNCTION__);

    return 0;
}



//===================================================
//
// Read the driver
// 
// filp  : File pointer
// ubuf  : User buffer to transfer data to user
// count : Number of data to read
// f_ops : Offset to read data
//
// Return : Positive value that represent the number of data read, 
//          Negative value if an error occured 
//
//===================================================
static ssize_t ele784_read (struct file *filp, char __user *ubuf, size_t count, loff_t *f_ops)
{
    printk(KERN_WARNING"Calling : %s\n",__FUNCTION__);  
    
    ssize_t retval = 0;

    //return the number of data transfer
    return retval;
}


//===================================================
//
// Send an I/O control command to the driver
// 
// filp : File pointer
// cmd  : Command send from user
// arg  : Set of get a value depending of the command
//
// Return : 0 if successful, 
//          Negative value if an error occured 
//
//===================================================
static long ele784_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0;
    int retval = 0;

    // Checking if IOC command has magic number type
    if (_IOC_TYPE(cmd) != LAB2_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    // Checking if IOC command number is valid
    if (_IOC_NR(cmd) > LAB2_IOC_MAX)
    {
        return -ENOTTY;
    }
    
    // Check if user have access to write or read mode 
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
    }
    
    // User doesn't have access in read and or write mode... Send error
    if (err)
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        // Get register value from the camera
        case LAB2_IOCTL_GET:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x10);
            break;

        // Set register value to the camera
        case LAB2_IOCTL_SET:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x20);
            break;
            
        // Start picture acquisition
        case LAB2_IOCTL_STREAMON:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x30);
            break;
            
        // Stop picture acquisition
        case LAB2_IOCTL_STREAMOFF:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x40);
            break;

        // Go grab data from the camera
        case LAB2_IOCTL_GRAB:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x50);
            break;

        // Set the position of the camera
        case LAB2_IOCTL_PANTILT:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x60);
            break;

        // Reset the position of the camera
        case LAB2_IOCTL_PANTILT_RESET:
            printk(KERN_WARNING"Calling : %s(%X)\n",__FUNCTION__, 0x70);
            break;
        
        // Data is not a know command : send an error
        default :
            return -ENOTTY;
            break;
    }

    return retval;
}


/*
static int ele784_probe_old(struct usb_interface *intf, const struct usb_device_id *id)
{
	// called when a USB device is connected to the computer.

	//struct usb_host_interface *iface_desc;
	//struct usb_endpoint_descriptor *endpoint;
	struct usb_ele784 *dev = NULL;
	//size_t buffer_size;
	int retval = 0;
	//int i;

	// allocate memory for our device state and initialize it 
	dev = kzalloc(sizeof(struct usb_ele784), GFP_KERNEL);
	if (dev == NULL)
	{
		printk(KERN_WARNING"Out of memory (%s:%u)\n",__FUNCTION__, __LINE__);
		retval = -ENOMEM;
	}

	if (retval == 0)
	{
		kref_init(&dev->kref);
		dev->dev = usb_get_dev(interface_to_usbdev(intf));
		dev->interface = intf;
		//iface_desc = intf->cur_altsetting;

		// set up the endpoint information 
		// use only the first bulk-in and bulk-out endpoints 
		//for (i = 0; (i < (retval == 0) && iface_desc->desc.bNumEndpoints); ++i) 
		{
			endpoint = &iface_desc->endpoint[i].desc;

			if (!dev->bulk_in_endpointAddr &&
				 (endpoint->bEndpointAddress & USB_DIR_IN) &&
				 ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
						== USB_ENDPOINT_XFER_BULK)) 
			{
				// we found a bulk in endpoint 
				buffer_size = endpoint->wMaxPacketSize;
				dev->bulk_in_size = buffer_size;
				dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
				dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
				if (dev->bulk_in_buffer == NULL) 
				{
					printk(KERN_WARNING"Could not allocate bulk_in_buffer");
					retval = -ENOMEM;
				}
				else
				{
					printk(KERN_WARNING"dev->bulk_in_endpointAddr = %u", dev->bulk_in_endpointAddr);
				}
			}
			else
			{
				printk(KERN_WARNING"We didn't found a bulk in endpoint");
			}

			if ((retval == 0) &&
				 !dev->bulk_out_endpointAddr &&
				 !(endpoint->bEndpointAddress & USB_DIR_IN) &&
				 ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
						== USB_ENDPOINT_XFER_BULK)) 
			{
				// we found a bulk out endpoint 
				dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
				printk(KERN_WARNING"dev->bulk_out_endpointAddr = %u", dev->bulk_out_endpointAddr);
			}
			else
			{
				printk(KERN_WARNING"We didn't found a bulk out endpoint");
			}
		}*/

//		if (retval == 0)
//		{
			/*if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) 
			{
				printk(KERN_WARNING"Could not find both bulk-in and bulk-out endpoints");
				retval = -1;
			}*/
			
			// save our data pointer in this interface device 
/*			usb_set_intfdata(intf, dev);

			// we can register the device now, as it is ready 
			retval = usb_register_dev(intf, &ele784_class);
			if (retval) 
			{
				// something prevented us from registering this driver 
				printk(KERN_WARNING"Not able to get a minor for this device.");
				usb_set_intfdata(intf, NULL);
				retval = -1;
			}
			else
			{
				// let the user know what node this device is now attached to 
				printk(KERN_ALERT"Laboratoire2_probe (%s:%u) => USB CONNECTED to USB_ELE784-%d\n", __FUNCTION__, __LINE__, intf->minor);
			}
		}
	}

	if (retval != 0)
	{
		if (dev)
		{
			kref_put(&dev->kref, ele784_delete);
		}
	}

	return retval;
}*/

static int ele784_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	// called when a USB device is connected to the computer.

	struct usb_host_interface *iface_desc;
	struct usb_ele784 *dev = NULL;
	int retval = 0;

	// allocate memory for our device state and initialize it 
	dev = kzalloc(sizeof(struct usb_ele784), GFP_KERNEL);
	if (dev == NULL)
	{
		printk(KERN_WARNING"Out of memory (%s:%u)\n",__FUNCTION__, __LINE__);
		retval = -ENOMEM;
	}

	if (retval == 0)
	{
		//kref_init(&dev->kref);
		dev->dev = usb_get_dev(interface_to_usbdev(intf));
		dev->interface = intf;
		iface_desc = intf->cur_altsetting;
		if(iface_desc->desc.bInterfaceClass == CC_VIDEO &&
			iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING)
		{
			// save our data pointer in this interface device 
			usb_set_intfdata(intf, dev);

			// we can register the device now, as it is ready 
			retval = usb_register_dev(intf, &ele784_class);
			if (retval) 
			{
				// something prevented us from registering this driver 
				printk(KERN_WARNING"Not able to get a minor for this device.");
				usb_set_intfdata(intf, NULL);
				retval = -1;
			}
			else
			{
				// let the user know what node this device is now attached to 
				printk(KERN_ALERT"Laboratoire2_probe (%s:%u) => USB CONNECTED to USB_ELE784-%d\n", __FUNCTION__, __LINE__, intf->minor);
			}
		}
		else if (iface_desc->desc.bInterfaceClass == CC_VIDEO &&
			iface_desc->desc.bInterfaceSubClass == SC_VIDEOCONTROL)
		{
			retval = 0;
		}
		else
		{
			retval = -1;
		}
	}

	/*if (retval != 0)
	{
		if (dev)
		{
			kref_put(&dev->kref, ele784_delete);
		}
	}*/

	return retval;
}

static void ele784_disconnect(struct usb_interface *interface)
{
	// called when unplugging a USB device.

	struct usb_ele784 *dev;
	//int minor = interface->minor;

	/* prevent ele784_open() from racing ele784_disconnect() */
	//lock_kernel();
	
	/* give back our minor */
	usb_deregister_dev(interface, &ele784_class);

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	//unlock_kernel();

	kfree(dev);

	/* decrement our usage count */
	//kref_put(&dev->kref, ele784_delete);

	printk(KERN_ALERT"Laboratoire2_disconnect (%s:%u) => USB DISCONNECTED\n", __FUNCTION__, __LINE__);
}

//===================================================
//
// Char driver initialization
// 
// Return : 0 if successful, 
//          Negative value if an error occured 
//
//===================================================
/*static int __init ele784_init(void)
{
	int usb_result = 0;

	printk(KERN_ALERT"Laboratoire2_init (%s:%u) => Let's hope this will not crash! MOM'S SPAGHETTI!!!\n", __FUNCTION__, __LINE__);

	// register this driver with the USB subsystem 
	usb_result = usb_register(&ele784_usb_driver);
	if (usb_result < 0) 
	{
		printk(KERN_WARNING"usb_register failed for the %s driver." "Error number %d",__FILE__,usb_result);
	}

	return usb_result;
}



//===================================================
//
// Char driver deinitialization
// 
//===================================================
static void __exit ele784_exit (void) 
{
	usb_deregister(&ele784_usb_driver);

	printk(KERN_ALERT"Laboratoire2_exit (%s:%u) => Goodbye, cruel world!!!\n", __FUNCTION__, __LINE__);
}*/

module_usb_driver(ele784_usb_driver);

