#if defined(__linux)

#ifndef INFRAREDBRICKTOWER_LINUXUSBTOWERINTERFACE_H
#define INFRAREDBRICKTOWER_LINUXUSBTOWERINTERFACE_H

#define BYTE unsigned char
#define WORD unsigned short

#include "HostTowerCommInterface.h"

class LinuxUSBTowerInterface : public HostTowerCommInterface
{
private:
    int fileDescriptor;
public:
    LinuxUSBTowerInterface(int fileDescriptor);

    ~LinuxUSBTowerInterface() override;

    bool ControlTransfer(
            BYTE request,
            WORD value,
            WORD index,
            WORD bufferLength,
            BYTE* buffer,
            unsigned long& lengthTransferred) const override;

    bool Write(
            BYTE* buffer,
            unsigned long bufferLength,
            unsigned long& lengthWritten) const override;

    bool Read(
            BYTE* buffer,
            unsigned long bufferLength,
            unsigned long& lengthRead) const override;

    bool Flush() const override;

    bool Close() const override;
};

bool OpenLinuxUSBTowerInterface(HostTowerCommInterface*& towerInterface);

#endif

#endif
