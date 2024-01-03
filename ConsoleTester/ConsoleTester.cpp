#include "../IBTDriverWin/WinUsbTowerInterface.h"
#include "../TowerLogic/TowerController.h"
#include "../PBrickLogic/VLL.h"
#include "../PBrickLogic/LASM.h"
#include "../PBrickLogic/PBrick.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>

BOOL StringsAreEqual(char* strOne, char* strTwo);
VOID MicroScoutCLI(Tower::RequestData* towerData);

VOID TestTower(Tower::RequestData* data);
VOID DriveMotors(Tower::RequestData* towerData);
VOID BeepRCX(Tower::RequestData* towerData);
VOID BeepMicroScout(Tower::RequestData* towerData);
VOID BeepRCXAndMicroScout(Tower::RequestData* towerData);

int main()
{
	WinUsbTowerInterface* usbTowerInterface;
	BOOL gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
	if (!gotInterface)
	{
		printf("Error getting WinUSB interface!\n");
		system("pause");
		return 0;
	}

	Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	BYTE replyBuffer[64];
	ULONG lengthRead = 0;

	ULONG lengthWritten = 0;
	/*BOOL writeSuccess = Tower::WriteData(
		command.data,
		command.dataLength,
		lengthWritten,
		towerData);

	BOOL readSuccess = Tower::ReadData(
		replyBuffer,
		21,
		lengthRead,
		towerData);*/

	/*int rom_major = replyBuffer[7];
	int rom_minor = replyBuffer[10];

	int ram_major = replyBuffer[14];
	int ram_minor = replyBuffer[16] * 10 + replyBuffer[17];*/

	Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
	usbTowerInterface->Flush();

	RCX::DownloadFirmware("C:\\Users\\david\\source\\repos\\LegoInfraredUSBTower\\ConsoleTester\\x64\\Debug\\firm0332.lgo", towerData);


	//assert(RCX::DownloadProgram("drive_until_button.rcx", 0, towerData));
	//assert(RCX::DownloadProgram("example_program.rcx", 1, towerData));
	//assert(RCX::DownloadProgram("beep.rcx", 2, towerData));
	//DriveMotors(towerData));
	BeepRCX(towerData);
	//MicroScoutCLI(towerData);

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

	usbTowerInterface->Flush();

	delete usbTowerInterface;
	delete towerData;

	system("pause");
	return 0;
}
