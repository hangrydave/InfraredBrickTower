#include "ControllerUI.h"

#if defined(WIN64)
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

	void SendVLL(BYTE* data, Tower::RequestData* towerData)
	{
		// todo: store current comm mode
		Tower::SetCommMode(Tower::CommMode::VLL, towerData);
		Tower::WriteData(data, VLL_PACKET_LENGTH, towerLengthWritten, towerData);
	}

	void RunTowerThread(bool& couldNotAccessTower, bool& programIsDone)
	{
		HostTowerCommInterface* usbTowerInterface;

#if defined(WIN64)
		bool gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
#elif defined(__linux)
		bool gotInterface = OpenLinuxUSBTowerInterface(usbTowerInterface);
#endif

		if (!gotInterface)
		{
			printf("Error getting USB interface!\n");
			couldNotAccessTower = true;
			programIsDone = true;
			return;
		}

		Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

		Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
		towerData->commInterface->Flush();

		LASM::Cmd_PBAliveOrNot(lasmCommand);
		bool success = LASM::SendCommand(
			&lasmCommand,
			towerData,
			towerData->replyBuffer,
			0,
			true);

		LASM::Cmd_PBAliveOrNot(lasmCommand);
		success = LASM::SendCommand(
			&lasmCommand,
			towerData,
			towerData->replyBuffer,
			0,
			true);

		// flush read
		unsigned long lengthRead = -1;
		BYTE* readBuffer = new BYTE[512];

		while (lengthRead != 0)
		{
			towerData->commInterface->Read(readBuffer, 512, lengthRead);
		}

		while (!programIsDone)
		{
			// RCX
			{
				// remote
				rcxRemoteData.request = 0;

				if (rcxRemoteData.message1-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_1;
				if (rcxRemoteData.message2-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_2;
				if (rcxRemoteData.message3-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PB_MESSAGE_3;

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

				if (rcxRemoteData.program1-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_1;
				if (rcxRemoteData.program2-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_2;
				if (rcxRemoteData.program3-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_3;
				if (rcxRemoteData.program4-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_4;
				if (rcxRemoteData.program5-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::PROGRAM_5;

				if (rcxRemoteData.stop-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::STOP_PROGRAM_AND_MOTORS;
				if (rcxRemoteData.sound-- > 0)
					rcxRemoteData.request |= LASM::RemoteCommandRequest::REMOTE_SOUND;

				if (rcxRemoteData.request != 0)
				{
					Tower::SetCommMode(Tower::CommMode::IR, towerData);
					if (rcxRemoteData.request & LASM::RemoteCommandRequest::REMOTE_SOUND)
					{
						// TODO: grabbed this from bricxcc wireshark. what is this
						lasmCommand.commandByte = 0xd2;
						lasmCommand.data[0] = 0x55;
						lasmCommand.data[1] = 0xff;
						lasmCommand.data[2] = 0x00;
						lasmCommand.data[3] = 0xd2;
						lasmCommand.data[4] = 0x2d;
						lasmCommand.data[5] = 0x00;
						lasmCommand.data[6] = 0xff;
						lasmCommand.data[7] = 0x00;
						lasmCommand.data[8] = 0xff;
						lasmCommand.data[9] = 0xd2;
						lasmCommand.data[10] = 0x2d;
						lasmCommand.dataLength = 11;
						LASM::SendCommand(
							&lasmCommand,
							towerData,
							towerData->replyBuffer,
							true,
							true,
							false);
					}

					LASM::Cmd_RemoteCommand(rcxRemoteData.request, lasmCommand);
					LASM::SendCommand(
						&lasmCommand,
						towerData,
						towerData->replyBuffer,
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
				if (vllData.beep1Immediate-- > 0)
					SendVLL(vllData.beep1ImmediateBytes, towerData);
				if (vllData.beep2Immediate-- > 0)
					SendVLL(vllData.beep2ImmediateBytes, towerData);
				if (vllData.beep3Immediate-- > 0)
					SendVLL(vllData.beep3ImmediateBytes, towerData);
				if (vllData.beep4Immediate-- > 0)
					SendVLL(vllData.beep4ImmediateBytes, towerData);
				if (vllData.beep5Immediate-- > 0)
					SendVLL(vllData.beep5ImmediateBytes, towerData);

				// program
				// sound
				if (vllData.beep1Program-- > 0)
					SendVLL(vllData.beep1ProgramBytes, towerData);
				if (vllData.beep2Program-- > 0)
					SendVLL(vllData.beep2ProgramBytes, towerData);
				if (vllData.beep3Program-- > 0)
					SendVLL(vllData.beep3ProgramBytes, towerData);
				if (vllData.beep4Program-- > 0)
					SendVLL(vllData.beep4ProgramBytes, towerData);
				if (vllData.beep5Program-- > 0)
					SendVLL(vllData.beep5ProgramBytes, towerData);

				// motor
				if (vllData.forwardHalf-- > 0)
					SendVLL(vllData.forwardHalfBytes, towerData);
				if (vllData.forwardOne-- > 0)
					SendVLL(vllData.forwardOneByte, towerData);
				if (vllData.forwardTwo-- > 0)
					SendVLL(vllData.forwardTwoBytes, towerData);
				if (vllData.forwardFive-- > 0)
					SendVLL(vllData.forwardFiveBytes, towerData);
				if (vllData.backwardHalf-- > 0)
					SendVLL(vllData.backwardHalfBytes, towerData);
				if (vllData.backwardOne-- > 0)
					SendVLL(vllData.backwardOneByte, towerData);
				if (vllData.backwardTwo-- > 0)
					SendVLL(vllData.backwardTwoBytes, towerData);
				if (vllData.backwardFive-- > 0)
					SendVLL(vllData.backwardFiveBytes, towerData);

				// preset programs
				if (vllData.waitLight-- > 0)
					SendVLL(vllData.waitLightBytes, towerData);
				if (vllData.seekLight-- > 0)
					SendVLL(vllData.seekLightBytes, towerData);
				if (vllData.code-- > 0)
					SendVLL(vllData.codeBytes, towerData);
				if (vllData.keepAlive-- > 0)
					SendVLL(vllData.keepAliveBytes, towerData);

				// etc functionality
				if (vllData.run-- > 0)
					SendVLL(vllData.runBytes, towerData);
				if (vllData.stop-- > 0)
					SendVLL(vllData.stopBytes, towerData);
				if (vllData.deleteProgram-- > 0)
					SendVLL(vllData.deleteBytes, towerData);
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
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300));

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
				vllData.deleteProgram = 1;
		}

		ImGui::End();
	}

	void BuildRCXRemote(const ImGuiViewport* mainViewport)
	{
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 350), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300));

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
}
