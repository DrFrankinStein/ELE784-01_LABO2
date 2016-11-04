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

MODULE_LICENSE("Dual BSD/GPL");

static int ele784_open (struct inode *inode, struct file *filp);
static int ele784_release (struct inode *inode, struct file *filp);
static ssize_t ele784_read (struct file *filp, char __user *ubuf, size_t count,loff_t *f_ops);
static long ele784_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

struct Camera_Dev 
{
    dev_t           dev;    //Device informations
    struct cdev     cdev;   //
    struct class    *class; //
} CamDev;

struct file_operations ele784_fops = 
{
    .owner  =   THIS_MODULE,
    .open     =   buf_open,
    .release  =   buf_release, //"close"
    .read     =   buf_read,
    .unlocked_ioctl = buf_ioctl,
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

    printk(KERN_ALERT"Laboratoire2_exit (%s:%u) => Goodbye, cruel world!!!\n", __FUNCTION__, __LINE__);
}


//Module initiation function
module_init(ele784_init);

//Module exiting (disinstalling) function
module_exit(ele784_exit);

