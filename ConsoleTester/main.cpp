#if defined(WIN32)
#include "WinUsbTowerInterface.h"

#elif defined(__linux)
#include "LinuxUsbTowerInterface.h"

#endif
#include "TowerController.h"
#include "VLL.h"
#include "LASM.h"
#include "PBrick.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>

static char* lasmInput = "";

int main(int argc, char* argv[])
{
	HostTowerCommInterface* usbTowerInterface;

#if defined(WIN32)
		bool gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
#elif defined(__linux)
		bool gotInterface = OpenLinuxUSBTowerInterface(usbTowerInterface);
#endif

	if (!gotInterface)
	{
		printf("Error getting USB interface!\n");
		return 1;
	}

	Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	BYTE replyBuffer[64];
	unsigned long lengthRead = 0;
	unsigned long lengthWritten = 0;

	Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
	usbTowerInterface->Flush();

	LASM::CommandData commandData;

	LASM::Cmd_PBAliveOrNot(commandData);
	LASM::SendCommand(&commandData, towerData);

	// RCX::DownloadFirmware(argv[1], towerData);

	//while (true)
	//{

		/*
		This was code testing the ability to read input from various lego IR remotes, and the results are promising!
		I was able to get input from both a Manas remote and (to my surprise) a Power Functions remote.
		*/
		//if (usbTowerInterface->Read(remoteReadBuffer, remoteBufferLen, remoteReadLength))
		//{
		//    if (remoteReadLength > 0)
		//    {
		//        printf("remote input, size %d: ", remoteReadLength);
		//        for (int i = 0; i < remoteReadLength; i++)
		//        {
		//            //printf("%#x", remoteReadBuffer[i]);
		//            std::cout << (int) (remoteReadBuffer[i]);
		//        }
		//        printf("\n");
		//    }
		//}
	//}

	lasmInput = "plays 1";
	LASM::ParseAndSendLASM(lasmInput, towerData);

	usbTowerInterface->Flush();
	usbTowerInterface->Close();

	delete usbTowerInterface;
	delete towerData;

	return 0;
}
