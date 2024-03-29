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
// LAST MODIFICATION : Tuesday, December 13th 2016
//
//===================================================

#ifndef _LABORATOIRE2_H_
#define _LABORATOIRE2_H_

#include <linux/ioctl.h>
#include "usbvideo.h"

typedef enum CAM_MVT {CAM_UP, CAM_DOWN, CAM_LEFT, CAM_RIGHT} CAM_MVT;

typedef struct GetSetStruct
{
   uint8_t requestType;
   uint8_t processingUnitSelector;
   int16_t value; 
}
GetSetStruct;

#define LAB2_IOC_MAGIC 'L'

// Get register value from the camera
#define LAB2_IOCTL_GET             _IOWR(LAB2_IOC_MAGIC, 0x10, GetSetStruct)

// Set register value to the camera
#define LAB2_IOCTL_SET             _IOW(LAB2_IOC_MAGIC, 0x20, GetSetStruct)

// Start picture acquisition 
#define LAB2_IOCTL_STREAMON        _IO(LAB2_IOC_MAGIC, 0x30)

// Stop picture acquisition
#define LAB2_IOCTL_STREAMOFF       _IO(LAB2_IOC_MAGIC, 0x40)

// Go grab data from the camera
#define LAB2_IOCTL_GRAB            _IO(LAB2_IOC_MAGIC, 0x50)

// Set the position of the camera
#define LAB2_IOCTL_PANTILT         _IOW(LAB2_IOC_MAGIC, 0x60, CAM_MVT)

// Reset the position of the camera
#define LAB2_IOCTL_PANTILT_RESET   _IO(LAB2_IOC_MAGIC, 0x70)

// Maximum command ID
#define LAB2_IOC_MAX               0x70

#endif //_LABORATOIRE2_H_
