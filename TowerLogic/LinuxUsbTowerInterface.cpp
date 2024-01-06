#if defined(__linux)

#include "LinuxUsbTowerInterface.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define READ_TIMEOUT 300
#define WRITE_TIMEOUT 300

// current reference is RCX_USBTowerPipe_linux.cpp from nqc

#define USB_NAME_1 "/dev/legousbtower0"
#define USB_NAME_2 "/dev/usb/legousbtower0"
#define USB_NAME_3 "/dev/usb/legousbtower1"
#ifndef DEFAULT_USB_NAME
#define DEFAULT_USB_NAME USB_NAME_1
#endif

bool OpenLinuxUSBTowerInterface(HostTowerCommInterface*& towerInterface)
{
    const char* name;
    struct stat stFileInfo;

    if (0 == stat(DEFAULT_USB_NAME, &stFileInfo))
    {
        name = DEFAULT_USB_NAME;
    }
    else if (0 == stat(USB_NAME_1, &stFileInfo))
    {
        name = USB_NAME_1;
    }
    else if (0 == stat(USB_NAME_2, &stFileInfo))
    {
        name = USB_NAME_2;
    }
    else if (0 == stat(USB_NAME_3, &stFileInfo))
    {
        name = USB_NAME_3;
    }

    int fileDescriptor = open(name, O_RDWR);
    if (fileDescriptor < 0) { return false; }

    int r = libusb_init(NULL);

    libusb_device_handle* deviceHandle;
    // r = libusb_wrap_sys_device(NULL, fileDescriptor, &deviceHandle);

#define VID 0x0694
#define PID 0x0001
    deviceHandle = libusb_open_device_with_vid_pid(NULL, 0x0694, 0x0001);
    if (deviceHandle == NULL)
    {
        printf("Failed to access USB device.");
        return false;
    }
    // fileDescriptor = open(name, O_RDWR);

    towerInterface = new LinuxUSBTowerInterface(fileDescriptor, deviceHandle);
    return true;
}

LinuxUSBTowerInterface::LinuxUSBTowerInterface(int fileDescriptor, libusb_device_handle* deviceHandle)
{
    this->fileDescriptor = fileDescriptor;
    this->deviceHandle = deviceHandle;
//    this->device = libusb_get_device(deviceHandle);
}

// LinuxUSBTowerInterface::~LinuxUSBTowerInterface()
// {
// }

bool LinuxUSBTowerInterface::ControlTransfer(
        BYTE request,
        WORD value,
        WORD index,
        WORD bufferLength,
        BYTE* buffer,
        unsigned long &lengthTransferred) const
{
    lengthTransferred = libusb_control_transfer(
            this->deviceHandle,
            0xc0,
            request,
            value,
            index,
            buffer,
            bufferLength,
            WRITE_TIMEOUT);

    return lengthTransferred > 0;
}

bool LinuxUSBTowerInterface::Write(
        BYTE* buffer,
        unsigned long bufferLength,
        unsigned long &lengthWritten) const
{
    // sleep(WRITE_TIMEOUT / 1000);

    lengthWritten = write(fileDescriptor, buffer, bufferLength);
    return lengthWritten != -1;
}

bool LinuxUSBTowerInterface::Read(
        BYTE* buffer,
        unsigned long bufferLength,
        unsigned long &lengthRead)
{
	usleep(20 * 1000 / 2);
    ioctl(fileDescriptor, 200, 20);

    long signedLen = read(fileDescriptor, buffer, bufferLength);
    if (signedLen < 0)
        signedLen = 0;
    
    lengthRead = signedLen;
    return lengthRead >= 0;
}

bool LinuxUSBTowerInterface::Flush() const
{
    return true;
}

bool LinuxUSBTowerInterface::Close() const
{
    if (fileDescriptor < 0)
        return true;

    if (close(fileDescriptor) == 0)
        return true;

    return false;
}

#endif