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
#include <unistd.h>

bool StringsAreEqual(char* strOne, char* strTwo);
void MicroScoutCLI(Tower::RequestData* towerData);

void TestTower(Tower::RequestData* data);
void DriveMotors(Tower::RequestData* towerData);
void BeepRCX(Tower::RequestData* towerData);
void BeepMicroScout(Tower::RequestData* towerData);
void BeepRCXAndMicroScout(Tower::RequestData* towerData);
bool ParseAndSendLASM(Tower::RequestData* towerData);

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

	LASM::Cmd_PlaySystemSound((LASM::SystemSound) 1, commandData);
	LASM::SendCommand(&commandData, towerData);
	sleep(2);
	LASM::Cmd_PBAliveOrNot(commandData);
	// LASM::SendCommand(&commandData, towerData);
	LASM::Cmd_PlaySystemSound((LASM::SystemSound) 2, commandData);
	LASM::SendCommand(&commandData, towerData);
	sleep(2);
	LASM::Cmd_PBAliveOrNot(commandData);
	LASM::SendCommand(&commandData, towerData);
	LASM::Cmd_PlaySystemSound((LASM::SystemSound) 3, commandData);
	LASM::SendCommand(&commandData, towerData);
	sleep(2);
	LASM::Cmd_PBAliveOrNot(commandData);
	LASM::SendCommand(&commandData, towerData);
	LASM::Cmd_PlaySystemSound((LASM::SystemSound) 4, commandData);
	LASM::SendCommand(&commandData, towerData);
	sleep(2);
	LASM::Cmd_PBAliveOrNot(commandData);
	LASM::SendCommand(&commandData, towerData);
	LASM::Cmd_PlaySystemSound((LASM::SystemSound) 5, commandData);
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
	ParseAndSendLASM(towerData);

	usbTowerInterface->Flush();

	delete usbTowerInterface;
	delete towerData;

	system("pause");
	return 0;
}

	enum LASMStatus
	{
		LASM_DEFAULT,
		LASM_SUCCESS,
		LASM_BAD_PARAMS,
		LASM_NO_REPLY,
		LASM_WRITE_FAILED,
		LASM_FAILED_OTHER,
		LASM_COMMAND_NOT_FOUND,
		LASM_IN_PROGRESS
	};
	static LASMStatus lasmStatus = LASM_DEFAULT;
	static bool lasmEntered = false;

bool ParseAndSendLASM(Tower::RequestData* towerData)
{
	lasmStatus = LASM_IN_PROGRESS;

	std::string inputString(lasmInput);
	std::string inputParts[5];
	int partIndex = 0;

	size_t partLength = 0;
	while ((partLength = inputString.find(" ")) != std::string::npos) {
		inputParts[partIndex++] = inputString.substr(0, partLength);
		inputString.erase(0, partLength + 1);
	}
	inputParts[partIndex] = inputString;
	
	static LASM::CommandData lasmCommand;
	if (LASM::GetCommandFromCode(inputParts[0].c_str(), inputParts + 1, partIndex, &lasmCommand))
	{
		if (LASM::SendCommand(&lasmCommand, towerData))
		{
			lasmStatus = LASM_SUCCESS;
			return true;
		}
		else
		{
			lasmStatus = LASM_FAILED_OTHER;
		}
	}
	else
	{
		lasmStatus = LASM_COMMAND_NOT_FOUND;
	}

	return false;
}
