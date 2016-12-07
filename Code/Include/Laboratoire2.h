//===================================================
//
// AUTHOR  : JULIEN LEMAY     (LEMJ16059303)
//           THIERRY DESTIN   (DEST03099102)
//
// SCHOOL  : ECOLE DE TECHNOLOGIES SUPERIEURES
//
// CLASS   : ELE784 (FALL 2016)
//
// PROJECT : LABORATOIRE2
//
// FILE    : Laboratoire2.h
//
// DESCRIPTION : Header code : IOCTL command list
//               for Laboratoire2.c
//
// LAST MODIFICATION : Wednesday, December 7th 2016
//
//===================================================

#ifndef _LABORATOIRE2_H_
#define _LABORATOIRE2_H_

#include <linux/ioctl.h>

typedef enum CAM_MVT {CAM_UP, CAM_DOWN, CAM_LEFT, CAM_RIGHT} CAM_MVT;

#define LAB2_IOC_MAGIC 'L'

// Get register value from the camera
#define LAB2_IOCTL_GET             _IOR(LAB2_IOC_MAGIC, 0x10, int)

// Set register value to the camera
#define LAB2_IOCTL_SET             _IOW(LAB2_IOC_MAGIC, 0x20, int)

// Start picture acquisition 
#define LAB2_IOCTL_STREAMON        _IO(LAB2_IOC_MAGIC, 0x30)

// Stop picture acquisition
#define LAB2_IOCTL_STREAMOFF       _IO(LAB2_IOC_MAGIC, 0x40)

// Go grab data from the camera
#define LAB2_IOCTL_GRAB            _IOR(LAB2_IOC_MAGIC, 0x50, int)

// Set the position of the camera
#define LAB2_IOCTL_PANTILT         _IOW(LAB2_IOC_MAGIC, 0x60, CAM_MVT)

// Reset the position of the camera
#define LAB2_IOCTL_PANTILT_RESET   _IO(LAB2_IOC_MAGIC, 0x70)

// Maximum command ID
#define LAB2_IOC_MAX               0x70

#endif //_LABORATOIRE2_H_
