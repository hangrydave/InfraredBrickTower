#if defined(__linux)

#ifndef INFRAREDBRICKTOWER_LINUXUSBTOWERINTERFACE_H
#define INFRAREDBRICKTOWER_LINUXUSBTOWERINTERFACE_H

#define BYTE unsigned char
#define WORD unsigned short

#include "HostTowerCommInterface.h"
#include <libusb.h>

#include <sys/ioctl.h>

class LinuxUSBTowerInterface : public HostTowerCommInterface
{
private:
    int fileDescriptor;
    libusb_device_handle* deviceHandle;
    libusb_device* device;
public:
    LinuxUSBTowerInterface(int fileDescriptor, libusb_device_handle* deviceHandle);
//     ~LinuxUSBTowerInterface() override;

    bool ControlTransfer(
            BYTE request,
            WORD value,
            WORD index,
            WORD bufferLength,
            BYTE* buffer,
            unsigned long& lengthTransferred) const override;

    bool Write(
            unsigned char* buffer,
            unsigned long bufferLength,
            unsigned long& lengthWritten) const override;

    bool Read(
            unsigned char* buffer,
            unsigned long bufferLength,
            unsigned long& lengthRead) override;

    bool Flush() const override;

    bool Close() const override;
};

bool OpenLinuxUSBTowerInterface(HostTowerCommInterface*& towerInterface);

#endif

#endif