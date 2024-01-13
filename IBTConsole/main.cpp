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
#include <string.h>

using namespace LASM;

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
		fprintf(stderr, "Error getting USB interface!\n");
		return 1;
	}

	Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);
	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
	usbTowerInterface->Flush();

	CommandData lasmCommand;
	Cmd_PBAliveOrNot(lasmCommand);
	SendCommand(&lasmCommand, towerData);

	if (argc < 2)
	{
		printf("Not enough parameters provided.\n");
		return 1;
	}

	bool verbose = false;
	char* firmwarePath = NULL;
	char* prgmToCompilePath = NULL;
	char* prgmToDownloadPath = NULL;
	bool lasm = false;

	int result = 0;
	for (int i = 1; i < argc; i++)
	{
		char* prior = argv[i - 1];
		char* arg = argv[i];
		if (strcmp(prior, "-v") == 0)
		{
			verbose = true;
		}
		else if (strcmp(prior, "--firmware") == 0)
		{
			firmwarePath = arg;
		}
		else if (strcmp(prior, "--download") == 0)
		{
			prgmToDownloadPath = arg;
		}
		else if (strcmp(prior, "--compile") == 0)
		{
			prgmToCompilePath = arg;
		}
		else if (strcmp(arg, "--lasm") == 0 && i == argc - 1)
		{
			printf("Enter LASM commands or exit to exit:\n");

			LASMStatus status;
			char* input;
			size_t len = 0;
			size_t read = 0;
			while (read != -1)
			{
				printf(">");
				read = getline(&input, &len, stdin);
				input[read - 1] = 0; // To account for the newline added at the end

				if (strncmp(input, "exit", read - 1) == 0)
				{
					break;
				}
				else
				{
					status = ParseAndSendLASM(input, towerData);
					switch (status)
					{
					case LASM_BAD_PARAMS:
						printf("Incorrect parameters provided.");
						break;
					case LASM_NO_REPLY:
						printf("No reply received from brick.");
						break;
					case LASM_WRITE_FAILED:
						printf("Failed to send command.");
						break;
					case LASM_FAILED_OTHER:
						printf("Command failed.");
						break;
					case LASM_COMMAND_NOT_FOUND:
						printf("Unknown LASM command.");
						break;
					}
					printf("\n");
				}
			}
			break;
		}
	}

	if (verbose)
	{
		// idk
	}

	if (firmwarePath)
	{
		result = RCX::DownloadFirmware(firmwarePath, towerData) ? 0 : 1;
	}

	if (prgmToDownloadPath)
	{
		result = RCX::DownloadProgram(prgmToDownloadPath, 1, towerData) ? 0 : 1;
	}

	if (prgmToCompilePath)
	{
		// call nqc and compile???
	}

	usbTowerInterface->Close();

	delete usbTowerInterface;
	delete towerData;

	return 0;
}
