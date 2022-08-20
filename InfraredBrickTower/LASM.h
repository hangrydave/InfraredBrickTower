#pragma once

#include <Windows.h>

#define NO_REPLY -1

enum Availability
{
	DIRECT,
	PROGRAM,
	BOTH
};

struct Command
{
	BYTE command;
	BYTE reply;
	Availability availability;

	Command(BYTE command, BYTE reply, Availability availability)
	{
		this->command = command;
		this->reply = reply;
		this->availability = availability;
	}
};

#define Cmd(name, command, reply, availability) \
Command name##(##command##, reply##, availability##)

namespace LASM
{
	// dumped from the LASM pdf from the RCX 2.0 sdk using a python script i wrote that's included in the root folder of this repo
	Cmd(PBAliveOrNot, 0x10, 0xE7, BOTH);
	Cmd(MemMap, 0x20, 0xD7, DIRECT);
	Cmd(PBBattery, 0x30, 0xC7, DIRECT);
	Cmd(DeleteAllTasks, 0x40, 0xB7, DIRECT);
	Cmd(StopAllTasks, 0x50, 0xA7, BOTH);
	Cmd(PBTurnOff, 0x60, 0x97, BOTH);
	Cmd(DeleteAllSubs, 0x70, 0x87, DIRECT);
	Cmd(ClearSound, 0x80, 0x77, BOTH);
	Cmd(ClearPBMessage, 0x90, 0x67, BOTH);
	Cmd(ExitAccessControl, 0xA0, NO_REPLY, PROGRAM);
	Cmd(ExitEventCheck, 0xB0, NO_REPLY, PROGRAM);
	Cmd(MuteSound, 0xD0, 0x27, BOTH);
	Cmd(UnmuteSound, 0xE0, 0x17, BOTH);
	Cmd(ClearAllEvents, 0x06, 0xF1, BOTH);
	Cmd(EndOfSub, 0xF6, NO_REPLY, PROGRAM);
	Cmd(OnOffFloat, 0x21, 0xD6, BOTH);
	Cmd(PbTXPower, 0x31, 0xC6, BOTH);
	Cmd(PlaySystemSound, 0x51, 0xA6, BOTH);
	Cmd(DeleteTask, 0x61, 0x96, DIRECT);
	Cmd(StartTask, 0x71, 0x86, BOTH);
	Cmd(StopTask, 0x81, 0x76, BOTH);
	Cmd(SelectProgram, 0x91, 0x66, BOTH);
	Cmd(ClearTimer, 0xA1, 0x56, BOTH);
	Cmd(PBPowerDownTime, 0xB1, 0x46, BOTH);
	Cmd(DeleteSub, 0xC1, 0x36, DIRECT);
	Cmd(ClearSensorValue, 0xD1, 0x26, BOTH);
	Cmd(SetFwdSetRwdRewDir, 0xE1, 0x16, BOTH);
	Cmd(Gosub, 0x17, NO_REPLY, PROGRAM);
	Cmd(SJump, 0x27, NO_REPLY, PROGRAM);
	Cmd(SCheckLoopCounter, 0x37, NO_REPLY, PROGRAM);
	Cmd(ConnectDisconnect, 0x67, 0x90, BOTH);
	Cmd(SetNormSetInvAltDir, 0x77, 0x80, BOTH);
	Cmd(IncCounter, 0x97, 0x60, BOTH);
	Cmd(DecCounter, 0xA7, 0x50, BOTH);
	Cmd(ClearCounter, 0xB7, 0x40, BOTH);
	Cmd(SetPriority, 0xD7, NO_REPLY, PROGRAM);
	Cmd(InternMessage, 0xF7, NO_REPLY, BOTH);
	Cmd(PlayToneVar, 0x02, 0xF5, BOTH);
	Cmd(Poll, 0x12, 0xE5, DIRECT);
	Cmd(SetWatch, 0x22, 0xD5, BOTH);
	Cmd(SetSensorType, 0x32, 0xC5, BOTH);
	Cmd(SetSensorMode, 0x42, 0xB5, BOTH);
	Cmd(SetDataLog, 0x52, 0xA5, BOTH);
	Cmd(DataLogNext, 0x62, 0x95, BOTH);
	Cmd(LJump, 0x72, NO_REPLY, PROGRAM);
	Cmd(SetLoopCounter, 0x82, NO_REPLY, PROGRAM);
	Cmd(LCheckLoopCounter, 0x92, NO_REPLY, PROGRAM);
	Cmd(SendPBMessage, 0xB2, NO_REPLY, PROGRAM);
	Cmd(SendUARTData, 0xC2, NO_REPLY, BOTH);
	Cmd(RemoteCommand, 0xD2, NO_REPLY, DIRECT);
	Cmd(SDecVarJumpLTZero, 0xF2, NO_REPLY, PROGRAM);
	Cmd(DirectEvent, 0x03, 0xF4, BOTH);
	Cmd(SetPower, 0x13, 0xE4, BOTH);
	Cmd(PlayTone, 0x23, 0xD4, BOTH);
	Cmd(SelectDisplay, 0x33, 0xC4, BOTH);
	Cmd(Wait, 0x43, NO_REPLY, PROGRAM);
	Cmd(UploadRam, 0x63, 0x94, DIRECT);
	Cmd(EnterAccessControl, 0x73, NO_REPLY, PROGRAM);
	Cmd(SetEvent, 0x93, 0x64, BOTH);
	Cmd(SetMaxPower, 0xA3, 0x54, BOTH);
	Cmd(LDecVarJumpLTZero, 0xF3, NO_REPLY, PROGRAM);
	Cmd(CalibrateEvent, 0x04, 0xF3, BOTH);
	Cmd(SetVar, 0x14, 0xE3, BOTH);
	Cmd(SumVar, 0x24, 0xD3, BOTH);
	Cmd(SubVar, 0x34, 0xC3, BOTH);
	Cmd(DivVar, 0x44, 0xB3, BOTH);
	Cmd(MulVar, 0x54, 0xA3, BOTH);
	Cmd(SgnVar, 0x64, 0x93, BOTH);
	Cmd(AbsVar, 0x74, 0x83, BOTH);
	Cmd(AndVar, 0x84, 0x73, BOTH);
	Cmd(OrVar, 0x94, 0x63, BOTH);
	Cmd(Upload, 0xA4, 0x53, DIRECT);
	Cmd(SEnterEventCheck, 0xB4, NO_REPLY, PROGRAM);
	Cmd(SetSourceValue, 0x05, 0xF2, BOTH);
	Cmd(UnlockPBrick, 0x15, 0xE2, DIRECT);
	Cmd(BeginOfTask, 0x25, 0xD2, DIRECT);
	Cmd(BeginOfSub, 0x35, 0xC2, DIRECT);
	Cmd(ContinueFirmwareDownLoad, 0x45, 0xB2, DIRECT);
	Cmd(GoIntoBootMode, 0x65, 0x92, DIRECT);
	Cmd(BeginFirmwareDownLoad, 0x75, 0x82, DIRECT);
	Cmd(SCheckDo, 0x85, NO_REPLY, PROGRAM);
	Cmd(LCheckDo, 0x95, NO_REPLY, PROGRAM);
	Cmd(UnlockFirmware, 0xA5, 0x52, DIRECT);
	Cmd(LEnterEventCheck, 0xB5, NO_REPLY, PROGRAM);
	Cmd(ViewSourceValue, 0xE5, 0x12, BOTH);


}
