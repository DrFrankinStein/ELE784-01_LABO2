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
// LAST MODIFICATION : Friday, November 4th 2016
//
//===================================================

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
#include <linux/mutex.h>    //Mutex

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include "../Include/Laboratoire2.h"


#include <linux/usb.h>

MODULE_LICENSE("Dual BSD/GPL");

// Define these values to match your devices 
#define USB_SKEL_VENDOR_ID	0xfff0
#define USB_SKEL_PRODUCT_ID	0xfff0

// table of devices that work with this driver 
static struct usb_device_id skel_table [] = {
	{ USB_DEVICE(USB_SKEL_VENDOR_ID, USB_SKEL_PRODUCT_ID) },
	{ }					// Terminating entry 
};
MODULE_DEVICE_TABLE (usb, skel_table);


// Get a minor range for your devices from the usb maintainer 
#define USB_SKEL_MINOR_BASE	192

static int ele784_open (struct inode *inode, struct file *filp);
static int ele784_release (struct inode *inode, struct file *filp);
static ssize_t ele784_read (struct file *filp, char __user *ubuf, size_t count,loff_t *f_ops);
static long ele784_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);
static int lab2_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void lab2_disconnect(struct usb_interface *interface);

struct Camera_Dev 
{
    dev_t           dev;    //Device informations
    struct cdev     cdev;   //
    struct class    *class; //
} CamDev;

// Structure to hold all of our device specific stuff 
struct usb_skel 
{
	struct usb_device      *udev;			   // the usb device for this device
	struct usb_interface   *interface;	   // the interface for this device 
	unsigned char          *bulk_in_buffer;// the buffer to receive data 
	size_t			        bulk_in_size;   // the size of the receive buffer 
	__u8			bulk_in_endpointAddr;	   // the address of the bulk in endpoint 
	__u8			bulk_out_endpointAddr;	   // the address of the bulk out endpoint 
	struct kref		kref;
};
#define to_skel_dev(d) container_of(d, struct usb_skel, kref)

struct file_operations ele784_fops = 
{
    .owner    =   THIS_MODULE,
    .open     =   ele784_open,
    .release  =   ele784_release, //"close"
    .read     =   ele784_read,
    .unlocked_ioctl = ele784_ioctl,
};

static struct usb_driver ele784_usb_driver = 
{
    .name        = "lab2_usb",
    .probe       = lab2_probe,
    .disconnect  = lab2_disconnect,
 //   .fops        = &lab2_fops,
//	 .minor       = USB_SKEL_MINOR_BASE,
    .id_table    = skel_table,
};
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
long ele784_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
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



static int lab2_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    /* called when a USB device is connected to the computer. */

	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_skel *skeldev = NULL;
	size_t buffer_size;
	int i;
	int retval = -ENODEV;

	if (!udev)
	{
		printk(KERN_WARNING"udev est null");
		return -1;
	}

	// allocate memory for our device state and initialize it 
	skeldev = kzalloc(sizeof(struct usb_skel), GFP_KERNEL);
	if (!skeldev)
	{
		printk(KERN_WARNING"skeldev est null");
		return -1;
	}

	memset(skeldev, 0x00, sizeof (*skeldev));
	kref_init(&skeldev->kref);

	skeldev->udev = usb_get_dev(udev);
	skeldev->interface = interface;

	// set up the endpoint information 
	// use only the first bulk-in and bulk-out endpoints 
	interface = intf->cur_altsetting;
	for (i = 0; i < interface->desc.bNumEndpoints; ++i) 
	{
		endpoint = &interface->endpoint[i].desc;

		if (!skeldev->bulk_in_endpointAddr &&
		    (endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) 
		{
			// we found a bulk in endpoint 
			buffer_size = endpoint->wMaxPacketSize;
			skeldev->bulk_in_size = buffer_size;
			skeldev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			skeldev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!skeldev->bulk_in_buffer) 
			{
				printk(KERN_WARNING"Could not allocate bulk_in_buffer");
				return -1;
			}
		}

		if (!skeldev->bulk_out_endpointAddr &&
		    !(endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) 
		{
			// we found a bulk out endpoint 
			skeldev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(skeldev->bulk_in_endpointAddr && skeldev->bulk_out_endpointAddr)) 
	{
		printk(KERN_WARNING"Could not find both bulk-in and bulk-out endpoints");
		return -1;
	}

	// save our data pointer in this interface device 
	usb_set_intfdata(intf, skeldev);

	// we can register the device now, as it is ready 
	retval = usb_register_dev(intf, &ele784_usb_driver);
	if (retval) 
	{
		// something prevented us from registering this driver 
		printk(KERN_WARNING"Not able to get a minor for this device.");
		usb_set_intfdata(intf, NULL);
		return -1;
	}

	// let the user know what node this device is now attached to 
	printk(KERN_WARNING"USB Skeleton device now attached to USBSkel-%d", intf->minor);
	return 0;
/*
error:
	if (dev)
		kref_put(&dev->kref, skel_delete);
	return 0;*/
}

static void lab2_disconnect(struct usb_interface *interface)
{
    /* called when unplugging a USB device. */
}

//===================================================
//
// Char driver initialization
// 
// Return : 0 if successful, 
//          Negative value if an error occured 
//
//===================================================
static int __init ele784_init(void)
{
    int result;
	 int usb_result;

    printk(KERN_ALERT"Laboratoire2_init (%s:%u) => Let's hope this will not crash! MOM'S SPAGHETTI!!!\n", __FUNCTION__, __LINE__);

    // Allocate a version number *** MAYBE CHANGE THAT TO GET MAJOR = 250
    result = alloc_chrdev_region(&CamDev.dev, 0, 1, "Laboratoire2");

    if (result < 0)
   {
        printk(KERN_WARNING"Laboratoire2_init ERROR IN alloc_chrdev_region (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
   }
    else
   {
        printk(KERN_WARNING"Laboratoire2_init : MAJOR = %u MINOR = %u\n", MAJOR(CamDev.dev), MINOR(CamDev.dev));
   }
    
    // Initialize class that will be used by device_create()
    CamDev.class = class_create(THIS_MODULE, "Labo2Class");

    // Create a device and register it with sysfs
    device_create(CamDev.class, NULL, CamDev.dev, &CamDev, "Laboratoire2");

    // Initialize char device structure
    cdev_init(&CamDev.cdev, &ele784_fops);

    // Set owner to this current module
    CamDev.cdev.owner = THIS_MODULE;
    
    // INIT EVERYTHING HERE!-----------------------------------------------
   

    // register this driver with the USB subsystem 
    usb_result = usb_register(&ele784_usb_driver);
    if (usb_result < 0) 
    {
       printk(KERN_WARNING"usb_register failed for the "__FILE__ "driver." "Error number %d",usb_result);
                
       return -EAGAIN;
    }


    // Add the char device to the system, initialize everything before here!
    if (cdev_add(&CamDev.cdev, CamDev.dev, 1) < 0)
   {
        printk(KERN_WARNING"Laboratoire2 ERROR IN cdev_add (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
   }
    
    // Return 0 for success
    return 0;
}



//===================================================
//
// Char driver deinitialization
// 
//===================================================
static void __exit ele784_exit (void) 
{
    //Remove char device from system
    cdev_del(&CamDev.cdev);

    //Unregister a range of devicestarting from number BDev.dev
    unregister_chrdev_region(CamDev.dev, 1);

    //Remove a device created before with device_create()
    device_destroy (CamDev.class, CamDev.dev);

    //Destroy the class created before with class_create()
    class_destroy(CamDev.class);

    //UNINIT EVERYTHING HERE!
    usb_deregister(&ele784_usb_driver);

    printk(KERN_ALERT"Laboratoire2_exit (%s:%u) => Goodbye, cruel world!!!\n", __FUNCTION__, __LINE__);
}


//Module initiation function
module_init(ele784_init);

//Module exiting (disinstalling) function
module_exit(ele784_exit);

// https://www.kernel.org/doc/htmldocs/writing_usb_driver/basics.html
// http://matthias.vallentin.net/blog/2007/04/writing-a-linux-kernel-driver-for-an-unknown-usb-device/
// http://www.makelinux.net/ldd3/chp-13-sect-4
