#pragma once

#include <Windows.h>

enum Availability
{
	DIRECT,
	PROGRAM,
	BOTH
};

//struct Params
//{
//};

//struct NoParam : Params
//{
//	NoParam()
//	{
//		this->length = 0;
//	}
//};
//
//struct NumParam : Params
//{
//	NumParam()
//	{
//		this->length = 1;
//	}
//};


//#define ParamCmd(name, command, reply, availability, paramsStructBody, paramLength) \
//_Pragma("pack(push, 1)") \
//struct name##Params : Params paramsStructBody##; \
//_Pragma("pack(pop)") \
//VOID BuildCmd_##name##(##name##Params paramStruct, BYTE* commandData, UINT& commandLength) \
//{ \
//	BuildCommand(##command##, paramStruct, paramLength, commandData, commandLength); \
//}



//expectedReply = reply##; \
//BuildCommand(##command##, nullptr, 0, commandData, commandLength); \

namespace LASM
{
	enum LASMCommandByte : BYTE
	{
		PBAliveOrNot = 0x10,
		MemMap = 0x20,
		PBBattery = 0x30,
		DeleteAllTasks = 0x40,
		StopAllTasks = 0x50,
		PBTurnOff = 0x60,
		DeleteAllSubs = 0x70,
		ClearSound = 0x80,
		ClearPBMessage = 0x90,
		ExitAccessControl = 0xA0,
		ExitEventCheck = 0xB0,
		MuteSound = 0xD0,
		UnmuteSound = 0xE0,
		ClearAllEvents = 0x06,
		EndOfSub = 0xF6,
		OnOffFloat = 0x21,
		PbTXPower = 0x31,
		PlaySystemSound = 0x51,
		DeleteTask = 0x61,
		StartTask = 0x71,
		StopTask = 0x81,
		SelectProgram = 0x91,
		ClearTimer = 0xA1,
		PBPowerDownTime = 0xB1,
		DeleteSub = 0xC1,
		ClearSensorValue = 0xD1,
		SetFwdSetRwdRewDir = 0xE1,
		Gosub = 0x17,
		SJump = 0x27,
		SCheckLoopCounter = 0x37,
		ConnectDisconnect = 0x67,
		SetNormSetInvAltDir = 0x77,
		IncCounter = 0x97,
		DecCounter = 0xA7,
		ClearCounter = 0xB7,
		SetPriority = 0xD7,
		InternMessage = 0xF7,
		PlayToneVar = 0x02,
		Poll = 0x12,
		SetWatch = 0x22,
		SetSensorType = 0x32,
		SetSensorMode = 0x42,
		SetDataLog = 0x52,
		DataLogNext = 0x62,
		LJump = 0x72,
		SetLoopCounter = 0x82,
		LCheckLoopCounter = 0x92,
		SendPBMessage = 0xB2,
		SendUARTData = 0xC2,
		RemoteCommand = 0xD2,
		SDecVarJumpLTZero = 0xF2,
		DirectEvent = 0x03,
		SetPower = 0x13,
		PlayTone = 0x23,
		SelectDisplay = 0x33,
		Wait = 0x43,
		UploadRam = 0x63,
		EnterAccessControl = 0x73,
		SetEvent = 0x93,
		SetMaxPower = 0xA3,
		LDecVarJumpLTZero = 0xF3,
		CalibrateEvent = 0x04,
		SetVar = 0x14,
		SumVar = 0x24,
		SubVar = 0x34,
		DivVar = 0x44,
		MulVar = 0x54,
		SgnVar = 0x64,
		AbsVar = 0x74,
		AndVar = 0x84,
		OrVar = 0x94,
		Upload = 0xA4,
		SEnterEventCheck = 0xB4,
		SetSourceValue = 0x05,
		UnlockPBrick = 0x15,
		BeginOfTask = 0x25,
		BeginOfSub = 0x35,
		ContinueFirmwareDownLoad = 0x45,
		GoIntoBootMode = 0x65,
		BeginFirmwareDownLoad = 0x75,
		SCheckDo = 0x85,
		LCheckDo = 0x95,
		UnlockFirmware = 0xA5,
		LEnterEventCheck = 0xB5,
		ViewSourceValue = 0xE5,
	};

	BOOL IsReplyByteGood(LASMCommandByte commandByte, BYTE reply);

	struct MessageData
	{
		LASMCommandByte commandByte;
		BYTE params[16];
		UINT paramsLength;
		BYTE replyByte;
		Availability availability;

		BYTE composedData[16];
		UINT composedLength;
		/*Command(LASMCommandByte commandByte, BYTE* params, UINT paramsLength, Availability availability)
		{
			this->commandByte = commandByte;
			this->replyByte = ~commandByte & 0xf7;
			this->params = params;
			this->paramsLength = paramsLength;
			this->availability = availability;
		}*/
	};

	enum class SystemSound
	{
		KEY_CLICK = 0,
		BEEP = 1,
		SWEEP_DOWN = 2,
		SWEEP_UP = 3,
		ERROR_SOUND = 4,
		FAST_SWEEP_UP = 5
	};

	class LASMBuilder
	{
	private:
		const UINT messageCount = 16;
		MessageData messages[16];
		UINT currentMessageIndex = 0;
	public:
		VOID PlaySystemSound(SystemSound sound);
	};

	VOID ComposeMessage(MessageData* messageData);

	inline VOID InitCommand(MessageData* messageData, LASMCommandByte commandByte)
	{
		messageData->commandByte = commandByte;
		messageData->replyByte = ~commandByte & 0xf7;
		messageData->availability = BOTH;

		ComposeMessage(messageData);
	}

	//VOID ComposePlaySystemSound(MessageData* messageData, SystemSound sound);

	//VOID ComposeOnOffFloat

//#define Params

#define Cmd(name, availabilityArg) \
inline VOID Compose##name##(MessageData* messageData) \
{ \
	messageData->commandByte = name##; \
	messageData->paramsLength = 0; \
	ComposeMessage(messageData); \
}

#define ParamCmd(name, commandByteArg, replyByteArg, availability, ...) \
VOID Compose_##name##(MessageData* messageData, ##__VA_ARGS__);
//{ \
//	commandStruct->commandByte = ##commandByteArg; \
//	commandStruct->params = ##params; \
//	commandStruct->paramsLength = ##paramsLength; \
//	commandStruct->replyByte = ##replyByte; \
//	commandStruct->availability = ##availability; \
//}
// VOID BuildCmd_##name##(BYTE* commandData, UINT& commandLength, BYTE& expectedReply ##__VA_ARGS__);

#define COMMA ,
#define NO_PARAM NoParam
#define HAS_PARAMS NumParam
#define NO_REPLY -1

	// dumped from the LASM pdf from the RCX 2.0 sdk using a python script i wrote that's included in the root folder of this repo
	Cmd(PBAliveOrNot, BOTH);
	Cmd(MemMap, BOTH);
	Cmd(PBBattery, BOTH);
	Cmd(DeleteAllTasks, BOTH);
	Cmd(StopAllTasks, BOTH);
	Cmd(PBTurnOff, BOTH);
	Cmd(DeleteAllSubs, BOTH);
	Cmd(ClearSound, BOTH);
	Cmd(ClearPBMessage, BOTH);
	Cmd(ExitAccessControl, BOTH);
	Cmd(ExitEventCheck, BOTH);
	Cmd(MuteSound, BOTH);
	Cmd(UnmuteSound, BOTH);
	Cmd(ClearAllEvents, BOTH);
	Cmd(EndOfSub, BOTH);
	Cmd(OnOffFloat, BOTH);
	Cmd(PbTXPower, BOTH);
	//Cmd(PlaySystemSound, BOTH);
	Cmd(DeleteTask, BOTH);
	Cmd(StartTask, BOTH);
	Cmd(StopTask, BOTH);
	Cmd(SelectProgram, BOTH);
	Cmd(ClearTimer, BOTH);
	Cmd(PBPowerDownTime, BOTH);
	Cmd(DeleteSub, BOTH);
	Cmd(ClearSensorValue, BOTH);
	Cmd(SetFwdSetRwdRewDir, BOTH);
	Cmd(Gosub, BOTH);
	Cmd(SJump, BOTH);
	Cmd(SCheckLoopCounter, BOTH);
	Cmd(ConnectDisconnect, BOTH);
	Cmd(SetNormSetInvAltDir, BOTH);
	Cmd(IncCounter, BOTH);
	Cmd(DecCounter, BOTH);
	Cmd(ClearCounter, BOTH);
	Cmd(SetPriority, BOTH);
	Cmd(InternMessage, BOTH);
	Cmd(PlayToneVar, BOTH);
	Cmd(Poll, BOTH);
	Cmd(SetWatch, BOTH);
	Cmd(SetSensorType, BOTH);
	Cmd(SetSensorMode, BOTH);
	Cmd(SetDataLog, BOTH);
	Cmd(DataLogNext, BOTH);
	Cmd(LJump, BOTH);
	Cmd(SetLoopCounter, BOTH);
	Cmd(LCheckLoopCounter, BOTH);
	Cmd(SendPBMessage, BOTH);
	Cmd(SendUARTData, BOTH);
	Cmd(RemoteCommand, BOTH);
	Cmd(SDecVarJumpLTZero, BOTH);
	Cmd(DirectEvent, BOTH);
	Cmd(SetPower, BOTH);
	Cmd(PlayTone, BOTH);
	Cmd(SelectDisplay, BOTH);
	Cmd(Wait, BOTH);
	Cmd(UploadRam, BOTH);
	Cmd(EnterAccessControl, BOTH);
	Cmd(SetEvent, BOTH);
	Cmd(SetMaxPower, BOTH);
	Cmd(LDecVarJumpLTZero, BOTH);
	Cmd(CalibrateEvent, BOTH);
	Cmd(SetVar, BOTH);
	Cmd(SumVar, BOTH);
	Cmd(SubVar, BOTH);
	Cmd(DivVar, BOTH);
	Cmd(MulVar, BOTH);
	Cmd(SgnVar, BOTH);
	Cmd(AbsVar, BOTH);
	Cmd(AndVar, BOTH);
	Cmd(OrVar, BOTH);
	Cmd(Upload, BOTH);
	Cmd(SEnterEventCheck, BOTH);
	Cmd(SetSourceValue, BOTH);
	Cmd(UnlockPBrick, BOTH);
	Cmd(BeginOfTask, BOTH);
	Cmd(BeginOfSub, BOTH);
	Cmd(ContinueFirmwareDownLoad, BOTH);
	Cmd(GoIntoBootMode, BOTH);
	Cmd(BeginFirmwareDownLoad, BOTH);
	Cmd(SCheckDo, BOTH);
	Cmd(LCheckDo, BOTH);
	Cmd(UnlockFirmware, BOTH);
	Cmd(LEnterEventCheck, BOTH);
	Cmd(ViewSourceValue, BOTH);
}
