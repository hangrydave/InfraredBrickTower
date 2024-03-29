#include "ControllerUI.h"

#if defined(WIN32)
#include "WinUsbTowerInterface.h"

#elif defined(__linux)
#include "LinuxUsbTowerInterface.h"

#endif

#include "PBrick.h"
#include <iostream>

namespace IBTUI
{
	static RCXRemoteData rcxRemoteData;
	static VLLData vllData;
	static LASM::CommandData lasmCommand;
	static unsigned long towerLengthWritten = 0;

	static bool isDownloadingSomething = false;

	static Tower::VersionData towerVersionData;
	static char* towerCredits = new char[1000];
	static int towerCreditsLen = 0;
	static char* towerCopyright = new char[1000];
	static int towerCopyrightLen = 0;

#define WINDOW_MAX_WIDTH 550
#define WINDOW_MAX_HEIGHT 550
#define WINDOW_GAP 25
#define RCX_X WINDOW_GAP
#define RCX_Y WINDOW_GAP
#define RCX_WIDTH 300
#define RCX_HEIGHT 300
#define VLL_X RCX_X + RCX_WIDTH + WINDOW_GAP
#define VLL_Y WINDOW_GAP
#define VLL_WIDTH 320
#define VLL_HEIGHT 300
#define LASM_X WINDOW_GAP
#define LASM_Y RCX_Y + RCX_HEIGHT + WINDOW_GAP
#define LASM_WIDTH 300
#define LASM_HEIGHT 300
#define INFO_X LASM_X + LASM_WIDTH + WINDOW_GAP
#define INFO_Y RCX_Y + RCX_HEIGHT + WINDOW_GAP
#define INFO_WIDTH 450
#define INFO_HEIGHT 370

	static char lasmInput[32] = "";
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
		LASM::Cmd_PBAliveOrNot(lasmCommand);
		LASM::SendCommand(&lasmCommand, towerData);

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

	void SendVLL(BYTE* data, Tower::RequestData* towerData)
	{
		// todo: store current comm mode
		Tower::SetCommMode(Tower::CommMode::VLL, towerData);
		Tower::WriteData(
			data,
			VLL_PACKET_LENGTH,
			towerLengthWritten,
			towerData,
			false);
	}

	void RunTowerThread(bool& couldNotAccessTower, bool* programIsDone)
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
			couldNotAccessTower = true;
			*programIsDone = true;
			return;
		}

		Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

		Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
		towerData->commInterface->Flush();

		towerVersionData = Tower::GetVersion(towerData);

		Tower::GetCopyright(towerData);
		memcpy(towerCopyright, towerData->stringBuffer, towerData->stringLength);
		
		Tower::GetCredits(towerData);
		memcpy(towerCredits, towerData->stringBuffer, towerData->stringLength);

		LASM::Cmd_PBAliveOrNot(lasmCommand);
		bool success = LASM::SendCommand(
			&lasmCommand,
			towerData);

		LASM::Cmd_PBAliveOrNot(lasmCommand);
		success = LASM::SendCommand(
			&lasmCommand,
			towerData);

		// flush read
		unsigned long lengthRead = -1;
		BYTE* readBuffer = new BYTE[512];

		while (lengthRead != 0)
		{
			towerData->commInterface->Read(readBuffer, 512, lengthRead);
		}

		while (!*programIsDone)
		{
			// RCX
			{
				// remote
				rcxRemoteData.request = 0;

				if (rcxRemoteData.message1 > 0) { rcxRemoteData.message1--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_1; }
				if (rcxRemoteData.message2 > 0) { rcxRemoteData.message2--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_2; }
				if (rcxRemoteData.message3 > 0) { rcxRemoteData.message3--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_3; }

				if (rcxRemoteData.motorAFwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_A_FORWARDS;
				if (rcxRemoteData.motorABwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_A_BACKWARDS;

				if (rcxRemoteData.motorBFwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_B_FORWARDS;
				if (rcxRemoteData.motorBBwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_B_BACKWARDS;

				if (rcxRemoteData.motorCFwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_C_FORWARDS;
				if (rcxRemoteData.motorCBwd)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::MOTOR_C_BACKWARDS;

				if (rcxRemoteData.program1 > 0) { rcxRemoteData.program1--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_1; }
				if (rcxRemoteData.program2 > 0) { rcxRemoteData.program2--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_2; }
				if (rcxRemoteData.program3 > 0) { rcxRemoteData.program3--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_3; }
				if (rcxRemoteData.program4 > 0) { rcxRemoteData.program4--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_4; }
				if (rcxRemoteData.program5 > 0) { rcxRemoteData.program5--; rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_5; }

				if (rcxRemoteData.stop > 0) { rcxRemoteData.stop--; rcxRemoteData.request |= LASM::RemoteCommandRequest::STOP_PROGRAM_AND_MOTORS; }
				if (rcxRemoteData.sound > 0) { rcxRemoteData.sound--; rcxRemoteData.request |= LASM::RemoteCommandRequest::REMOTE_SOUND; }

				if (rcxRemoteData.request != 0)
				{
					Tower::SetCommMode(Tower::CommMode::IR, towerData);

					// LASM documentation tells us that a remote command with 0 should be sent between multiple remote commands to clear internal buffers
					LASM::Cmd_RemoteCommand(0, lasmCommand);
					LASM::SendCommand(
						&lasmCommand,
						towerData,
						NULL,
						true,
						true,
						false);

					LASM::Cmd_RemoteCommand(rcxRemoteData.request, lasmCommand);
					LASM::SendCommand(
						&lasmCommand,
						towerData,
						NULL,
						true,
						true,
						false);
				}

				if (rcxRemoteData.downloadFilePath != nullptr)
				{
					Tower::SetCommMode(Tower::CommMode::IR, towerData);
					isDownloadingSomething = true;
					if (rcxRemoteData.downloadFirmware)
					{
						RCX::DownloadFirmware(rcxRemoteData.downloadFilePath->c_str(), towerData);
					}
					else
					{
						RCX::DownloadProgram(rcxRemoteData.downloadFilePath->c_str(), 0, towerData);
					}
					isDownloadingSomething = false;

					rcxRemoteData.downloadFilePath = nullptr;
				}
			}

			// VLL
			{
				// immediate
				// motor
				if (vllData.forwardImmediate)
					SendVLL(vllData.fwdImmediateBytes, towerData);
				if (vllData.backwardImmediate)
					SendVLL(vllData.bwdImmediateBytes, towerData);

				// sound
				if (vllData.beep1Immediate > 0) { vllData.beep1Immediate--; SendVLL(vllData.beep1ImmediateBytes, towerData); }
				if (vllData.beep2Immediate > 0) { vllData.beep2Immediate--; SendVLL(vllData.beep2ImmediateBytes, towerData); }
				if (vllData.beep3Immediate > 0) { vllData.beep3Immediate--; SendVLL(vllData.beep3ImmediateBytes, towerData); }
				if (vllData.beep4Immediate > 0) { vllData.beep4Immediate--; SendVLL(vllData.beep4ImmediateBytes, towerData); }
				if (vllData.beep5Immediate > 0) { vllData.beep5Immediate--; SendVLL(vllData.beep5ImmediateBytes, towerData); }

				// program
				// sound
				if (vllData.beep1Program > 0) { vllData.beep1Program--; SendVLL(vllData.beep1ProgramBytes, towerData); }
				if (vllData.beep2Program > 0) { vllData.beep2Program--; SendVLL(vllData.beep2ProgramBytes, towerData); }
				if (vllData.beep3Program > 0) { vllData.beep3Program--; SendVLL(vllData.beep3ProgramBytes, towerData); }
				if (vllData.beep4Program > 0) { vllData.beep4Program--; SendVLL(vllData.beep4ProgramBytes, towerData); }
				if (vllData.beep5Program > 0) { vllData.beep5Program--; SendVLL(vllData.beep5ProgramBytes, towerData); }

				// motor
				if (vllData.forwardHalf > 0) { vllData.forwardHalf--; SendVLL(vllData.forwardHalfBytes, towerData); }
				if (vllData.forwardOne > 0) { vllData.forwardOne--; SendVLL(vllData.forwardOneByte, towerData); }
				if (vllData.forwardTwo > 0) { vllData.forwardTwo--; SendVLL(vllData.forwardTwoBytes, towerData); }
				if (vllData.forwardFive > 0) { vllData.forwardFive--; SendVLL(vllData.forwardFiveBytes, towerData); }
				if (vllData.backwardHalf > 0) { vllData.backwardHalf--; SendVLL(vllData.backwardHalfBytes, towerData); }
				if (vllData.backwardOne > 0) { vllData.backwardOne--; SendVLL(vllData.backwardOneByte, towerData); }
				if (vllData.backwardTwo > 0) { vllData.backwardTwo--; SendVLL(vllData.backwardTwoBytes, towerData); }
				if (vllData.backwardFive > 0) { vllData.backwardFive--; SendVLL(vllData.backwardFiveBytes, towerData); }

				// preset programs				
				if (vllData.waitLight > 0) { vllData.waitLight--; SendVLL(vllData.waitLightBytes, towerData); }
				if (vllData.seekLight > 0) { vllData.seekLight--; SendVLL(vllData.seekLightBytes, towerData); }
				if (vllData.code > 0) { vllData.code--; SendVLL(vllData.codeBytes, towerData); }
				if (vllData.keepAlive > 0) { vllData.keepAlive--; SendVLL(vllData.keepAliveBytes, towerData); }

				// etc functionality
				if (vllData.run > 0) { vllData.run--; SendVLL(vllData.runBytes, towerData); }
				if (vllData.stop > 0) { vllData.stop--; SendVLL(vllData.stopBytes, towerData); }
				if (vllData.deletePrgm > 0) { vllData.deletePrgm--; SendVLL(vllData.deleteBytes, towerData); }

			}
		
			// LASM
			{
				if (lasmEntered)
				{
					bool success = ParseAndSendLASM(towerData);
					lasmEntered = false;
				}
			}
		}

		delete towerData;
		delete usbTowerInterface;
	}

	static ImGui::FileBrowser fileDialog;
	void Init()
	{
	}

	void BuildMicroScoutRemote(const ImGuiViewport* mainViewport)
	{
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + VLL_X, mainViewport->WorkPos.y + VLL_Y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(VLL_WIDTH, VLL_HEIGHT), ImGuiCond_FirstUseEver);

		// VLL window
		ImGui::Begin("MicroScout Remote");

#define VLL_IMMEDIATE_MODE 0
#define VLL_PROGRAM_MODE 1
		static int commandMode = 0;
		ImGui::Text("Command mode");
		ImGui::RadioButton("immediate", &commandMode, VLL_IMMEDIATE_MODE); ImGui::SameLine();
		ImGui::RadioButton("program", &commandMode, VLL_PROGRAM_MODE);

		if (commandMode == VLL_IMMEDIATE_MODE)
		{
			// sounds
			ImGui::Separator();
			if (ImGui::Button("1"))
				vllData.beep1Immediate = 1;
			ImGui::SameLine();
			if (ImGui::Button("2"))
				vllData.beep2Immediate = 1;
			ImGui::SameLine();
			if (ImGui::Button("3"))
				vllData.beep3Immediate = 1;
			ImGui::SameLine();
			if (ImGui::Button("4"))
				vllData.beep4Immediate = 1;
			ImGui::SameLine();
			if (ImGui::Button("5"))
				vllData.beep5Immediate = 1;

			ImGui::SameLine();
			ImGui::Text("Sound");

			// motors
			ImGui::Separator();
			ImGui::Checkbox("motor forwards", &vllData.forwardImmediate);
			if (vllData.forwardImmediate)
				vllData.backwardImmediate = false;

			ImGui::SameLine();
			ImGui::Checkbox("motor backwards", &vllData.backwardImmediate);
			if (vllData.backwardImmediate)
				vllData.forwardImmediate = false;
		}
		else
		{
			// sounds
			ImGui::Separator();
			ImGui::Text("Sound");
			if (ImGui::Button("1"))
				vllData.beep1Program = 1;
			ImGui::SameLine();
			if (ImGui::Button("2"))
				vllData.beep2Program = 1;
			ImGui::SameLine();
			if (ImGui::Button("3"))
				vllData.beep3Program = 1;
			ImGui::SameLine();
			if (ImGui::Button("4"))
				vllData.beep4Program = 1;
			ImGui::SameLine();
			if (ImGui::Button("5"))
				vllData.beep5Program = 1;

			// motors
			// forward
			ImGui::Separator();
			ImGui::Text("Motor");
			if (ImGui::Button("fwd 0.5 sec"))
				vllData.forwardHalf = 1;
			ImGui::SameLine();
			if (ImGui::Button("fwd 1 sec"))
				vllData.forwardOne = 1;
			ImGui::SameLine();
			if (ImGui::Button("fwd 2 sec"))
				vllData.forwardTwo = 1;
			ImGui::SameLine();
			if (ImGui::Button("fwd 5 sec"))
				vllData.forwardFive = 1;

			// backward
			if (ImGui::Button("bwd 0.5 sec"))
				vllData.backwardHalf = 1;
			ImGui::SameLine();
			if (ImGui::Button("bwd 1 sec"))
				vllData.backwardOne = 1;
			ImGui::SameLine();
			if (ImGui::Button("bwd 2 sec"))
				vllData.backwardTwo = 1;
			ImGui::SameLine();
			if (ImGui::Button("bwd 5 sec"))
				vllData.backwardFive = 1;

			// program presets
			// wait seek code keep
			ImGui::Separator();
			ImGui::Text("Program presets");
			if (ImGui::Button("wait light"))
				vllData.waitLight = 1;
			ImGui::SameLine();
			if (ImGui::Button("seek light"))
				vllData.seekLight = 1;
			ImGui::SameLine();
			if (ImGui::Button("code"))
				vllData.code = 1;
			ImGui::SameLine();
			if (ImGui::Button("keep alive"))
				vllData.keepAlive = 1;

			// run/delete/stop
			ImGui::Separator();
			ImGui::Text("Management");
			if (ImGui::Button("run"))
				vllData.run = 1;
			ImGui::SameLine();
			if (ImGui::Button("stop"))
				vllData.stop = 1;
			ImGui::SameLine();
			if (ImGui::Button("delete program"))
				vllData.deletePrgm = 1;
		}

		ImGui::End();
	}

	void BuildRCXRemote(const ImGuiViewport* mainViewport)
	{
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + RCX_X, mainViewport->WorkPos.y + RCX_Y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(RCX_WIDTH, RCX_HEIGHT), ImGuiCond_FirstUseEver);

		// RCX remote
		ImGui::Begin("RCX Remote");

		// messages
		ImGui::Text("Message");
		if (ImGui::Button("Msg 1"))
			rcxRemoteData.message1 = true;
		ImGui::SameLine();
		if (ImGui::Button("Msg 2"))
			rcxRemoteData.message2 = true;
		ImGui::SameLine();
		if (ImGui::Button("Msg 3"))
			rcxRemoteData.message3 = true;

		// motors
		ImGui::Separator();
		ImGui::Text("Motors");

		if (ImGui::BeginTable("motorControlTable", 3))
		{
			ImVec2 buttonSize;
			buttonSize.x = -1;
			buttonSize.y = 0;

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Checkbox("A Fwd", &rcxRemoteData.motorAFwd);
			if (rcxRemoteData.motorAFwd) rcxRemoteData.motorABwd = false;
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("B Fwd", &rcxRemoteData.motorBFwd);
			if (rcxRemoteData.motorBFwd) rcxRemoteData.motorBBwd = false;
			ImGui::TableSetColumnIndex(2);
			ImGui::Checkbox("C Fwd", &rcxRemoteData.motorCFwd);
			if (rcxRemoteData.motorCFwd) rcxRemoteData.motorCBwd = false;

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Checkbox("A Bwd", &rcxRemoteData.motorABwd);
			if (rcxRemoteData.motorABwd) rcxRemoteData.motorAFwd = false;
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("B Bwd", &rcxRemoteData.motorBBwd);
			if (rcxRemoteData.motorBBwd) rcxRemoteData.motorBFwd = false;
			ImGui::TableSetColumnIndex(2);
			ImGui::Checkbox("C Bwd", &rcxRemoteData.motorCBwd);
			if (rcxRemoteData.motorCBwd) rcxRemoteData.motorCFwd = false;
		}
		ImGui::EndTable();

		// programs
		ImGui::Separator();
		ImGui::Text("Program");
		if (ImGui::Button("Prgm 1"))
			rcxRemoteData.program1 = 1;
		ImGui::SameLine();
		if (ImGui::Button("Prgm 2"))
			rcxRemoteData.program2 = 1;
		ImGui::SameLine();
		if (ImGui::Button("Prgm 3"))
			rcxRemoteData.program3 = 1;
		ImGui::SameLine();
		if (ImGui::Button("Prgm 4"))
			rcxRemoteData.program4 = 1;
		ImGui::SameLine();
		if (ImGui::Button("Prgm 5"))
			rcxRemoteData.program5 = 1;

		ImGui::Separator();
		if (ImGui::Button("Stop"))
			rcxRemoteData.stop = 1;
		ImGui::SameLine();
		if (ImGui::Button("Sound"))
			rcxRemoteData.sound = 1;

		ImGui::Separator();
		if (ImGui::Button("Download Program"))
		{
			rcxRemoteData.downloadFirmware = false;

			fileDialog.SetTitle("Select an RCX program");
			fileDialog.SetTypeFilters({ ".rcx" });
			fileDialog.Open();
		}

		if (ImGui::Button("Download Firmware"))
		{
			rcxRemoteData.downloadFirmware = true;

			fileDialog.SetTitle("Select an RCX firmware file");
			fileDialog.SetTypeFilters({ ".lgo" });
			fileDialog.Open();
		}

		if (isDownloadingSomething)
		{
			ImGui::Separator();
			// from https://github.com/ocornut/imgui/issues/1901#issuecomment-400563921
			ImGui::Text("Downloading to RCX %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
		}

		ImGui::End();

		fileDialog.Display();
		if (fileDialog.HasSelected())
		{
			rcxRemoteData.downloadFilePath = new std::string(fileDialog.GetSelected().string());
			fileDialog.ClearSelected();
		}
	}

	void BuildInfoWindow(const ImGuiViewport* mainViewport)
	{
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + INFO_X, mainViewport->WorkPos.y + INFO_Y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(INFO_WIDTH, INFO_HEIGHT), ImGuiCond_FirstUseEver);

		ImGui::Begin("Tower Info");

		ImGui::Text(towerCopyright);
		ImGui::Separator();
		ImGui::Text(towerCredits);
		ImGui::Separator();
		ImGui::Text("Build number: %d", towerVersionData.buildNumber);
		ImGui::Text("Version: %d.%d", towerVersionData.majorVersion, towerVersionData.minorVersion);

		ImGui::End();
	}

	void BuildLASMWindow(const ImGuiViewport* mainViewport)
	{
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + LASM_X, mainViewport->WorkPos.y + LASM_Y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(LASM_WIDTH, LASM_HEIGHT), ImGuiCond_FirstUseEver);

		ImGui::Begin("LASM Console");

		if (ImGui::InputText("LASM", lasmInput, IM_ARRAYSIZE(lasmInput), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			lasmEntered = true;
		}

		ImGui::Separator();

		switch (lasmStatus)
		{
		case LASM_IN_PROGRESS:
			ImGui::Text("Sending LASM %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
			break;
		case LASM_SUCCESS:
			ImGui::Text("Sent LASM successfully");
			break;
		case LASM_BAD_PARAMS:
			ImGui::Text("Bad parameters supplied");
			break;
		case LASM_NO_REPLY:
			ImGui::Text("No reply received");
			break;
		case LASM_WRITE_FAILED:
			ImGui::Text("Write failed");
			break;
		case LASM_FAILED_OTHER:
			ImGui::Text("Failed to send LASM");
			break;
		case LASM_COMMAND_NOT_FOUND:
			ImGui::Text("Command %s could not be parsed", lasmInput);
			break;
		case LASM_DEFAULT:
		default:
			break;
		}

		ImGui::End();
	}
}
