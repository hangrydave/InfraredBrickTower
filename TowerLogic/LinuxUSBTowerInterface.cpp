#if defined(__linux)

#include "LinuxUSBTowerInterface.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define READ_TIMEOUT 300
#define WRITE_TIMEOUT 300

// current reference is RCX_USBTowerPipe_linux.cpp from nqc

#define USB_NAME_1 "/dev/legousbtower0"
#define USB_NAME_2 "/dev/usb/legousbtower0"
#ifndef DEFAULT_USB_NAME
#define DEFAULT_USB_NAME USB_NAME_1
#endif

BOOL OpenLinuxUSBTowerInterface(HostTowerCommInterface*& towerInterface)
{
    const char* name = "short";
    struct stat stFileInfo;

    if ((0 != name) && (0 != *name))
    {
        // a specific name was passed; attempt to use it
    }
    else if (0 == stat(DEFAULT_USB_NAME, &stFileInfo))
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

    int fileDescriptor = open(name, O_RDWR);

    if (fileDescriptor < 0) { return false; }

    towerInterface = new LinuxUSBTowerInterface(fileDescriptor);
    return true;
}

LinuxUSBTowerInterface::LinuxUSBTowerInterface(int fileDescriptor)
{
    this->fileDescriptor = fileDescriptor;
}

LinuxUSBTowerInterface::~LinuxUSBTowerInterface()
{
}

bool LinuxUSBTowerInterface::ControlTransfer(
        BYTE request,
        WORD value,
        WORD index,
        WORD bufferLength,
        BYTE* buffer,
        unsigned long &lengthTransferred) const
{
    ioctl(fileDescriptor, request, value);
    return true;
}

bool LinuxUSBTowerInterface::Write(
        BYTE* buffer,
        unsigned long bufferLength,
        unsigned long &lengthWritten) const
{
    sleep(WRITE_TIMEOUT / 1000);

    lengthWritten = write(fileDescriptor, buffer, bufferLength);
    return lengthWritten != -1;
}

bool LinuxUSBTowerInterface::Read(
        BYTE* buffer,
        unsigned long bufferLength,
        unsigned long &lengthRead) const
{
    sleep(READ_TIMEOUT / 1000);

    lengthRead = read(fileDescriptor, buffer, bufferLength);
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
