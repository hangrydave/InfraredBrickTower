#pragma once

#include "imgui.h"
#include "imfilebrowser.h"

#include "VLL.h"
#include "LASM.h"
#include "TowerController.h"

namespace IBTUI
{
    struct RCXRemoteData
    {
        bool motorAFwd,
            motorBFwd,
            motorCFwd,
            motorABwd,
            motorBBwd,
            motorCBwd = false;

        int message1,
            message2,
            message3 = 0;

        int program1,
            program2,
            program3,
            program4,
            program5 = 0;

        int stop = 0;
        int sound = 0;

        std::string* downloadFilePath;

        WORD request = 0;
    };

    struct VLLData
    {
        int beep1Immediate,
            beep2Immediate,
            beep3Immediate,
            beep4Immediate,
            beep5Immediate = 0;
        BYTE beep1ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_IMMEDIATE };
        BYTE beep2ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_IMMEDIATE };
        BYTE beep3ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_IMMEDIATE };
        BYTE beep4ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_IMMEDIATE };
        BYTE beep5ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_IMMEDIATE };

        bool forwardImmediate = false;
        bool backwardImmediate = false;
        BYTE fwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_IMMEDIATE };
        BYTE bwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_IMMEDIATE };

        int beep1Program,
            beep2Program,
            beep3Program,
            beep4Program,
            beep5Program = 0;
        BYTE beep1ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_PROGRAM };
        BYTE beep2ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_PROGRAM };
        BYTE beep3ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_PROGRAM };
        BYTE beep4ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_PROGRAM };
        BYTE beep5ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_PROGRAM };

        int forwardHalf,
            forwardOne,
            forwardTwo,
            forwardFive = 0;
        BYTE forwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_HALF };
        BYTE forwardOneByte[VLL_PACKET_LENGTH]{ VLL_FORWARD_ONE };
        BYTE forwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_TWO };
        BYTE forwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_FIVE };

        int backwardHalf,
            backwardOne,
            backwardTwo,
            backwardFive = 0;
        BYTE backwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_HALF };
        BYTE backwardOneByte[VLL_PACKET_LENGTH]{ VLL_BACKWARD_ONE };
        BYTE backwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_TWO };
        BYTE backwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_FIVE };

        int waitLight,
            seekLight,
            code,
            keepAlive = 0;
        BYTE waitLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
        BYTE seekLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
        BYTE codeBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
        BYTE keepAliveBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

        int run = 0;
        BYTE runBytes[VLL_PACKET_LENGTH]{ VLL_RUN };

        int stop = 0;
        BYTE stopBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

        int deleteProgram = 0;
        BYTE deleteBytes[VLL_PACKET_LENGTH]{ VLL_DELETE_PROGRAM };
    };

    void SendVLL(BYTE* data, Tower::RequestData* towerData);
    void RunTowerThread(bool& couldNotAccessTower, bool& programIsDone);

    void Init();
    void BuildMicroScoutRemote(const ImGuiViewport* mainViewport);
    void BuildRCXRemote(const ImGuiViewport* mainViewport);
}
