#pragma once

#ifdef PBRICKLOGIC_EXPORTS
#define PBRICKLOGIC_API __declspec(dllexport)
#else
#define PBRICKLOGIC_API __declspec(dllimport)
#endif

#include "pch.h"
#include <Windows.h>
#include <memory>

namespace LASM
{
	/*enum Availability
	{
		DIRECT,
		PROGRAM,
		BOTH
	};*/

	enum class Command : BYTE
	{
		PBAliveOrNot =				0x10,
		MemMap =					0x20,
		PBBattery =					0x30,
		DeleteAllTasks =			0x40,
		StopAllTasks =				0x50,
		PBTurnOff =					0x60,
		DeleteAllSubs =				0x70,
		ClearSound =				0x80,
		ClearPBMessage =			0x90,
		ExitAccessControl =			0xA0,
		ExitEventCheck =			0xB0,
		MuteSound =					0xD0,
		UnmuteSound =				0xE0,
		ClearAllEvents =			0x06,
		EndOfSub =					0xF6,
		OnOffFloat =				0x21,
		PbTXPower =					0x31,
		PlaySystemSound =			0x51,
		DeleteTask =				0x61,
		StartTask =					0x71,
		StopTask =					0x81,
		SelectProgram =				0x91,
		ClearTimer =				0xA1,
		PBPowerDownTime =			0xB1,
		DeleteSub =					0xC1,
		ClearSensorValue =			0xD1,
		SetFwdSetRwdRewDir =		0xE1,
		Gosub =						0x17,
		SJump =						0x27,
		SCheckLoopCounter =			0x37,
		ConnectDisconnect =			0x67,
		SetNormSetInvAltDir =		0x77,
		IncCounter =				0x97,
		DecCounter =				0xA7,
		ClearCounter =				0xB7,
		SetPriority =				0xD7,
		InternMessage =				0xF7,
		PlayToneVar =				0x02,
		Poll =						0x12,
		SetWatch =					0x22,
		SetSensorType =				0x32,
		SetSensorMode =				0x42,
		SetDataLog =				0x52,
		DataLogNext =				0x62,
		LJump =						0x72,
		SetLoopCounter =			0x82,
		LCheckLoopCounter =			0x92,
		SendPBMessage =				0xB2,
		SendUARTData =				0xC2,
		RemoteCommand =				0xD2,
		SDecVarJumpLTZero =			0xF2,
		DirectEvent =				0x03,
		SetPower =					0x13,
		PlayTone =					0x23,
		SelectDisplay =				0x33,
		Wait =						0x43,
		UploadRam =					0x63,
		EnterAccessControl =		0x73,
		SetEvent =					0x93,
		SetMaxPower =				0xA3,
		LDecVarJumpLTZero =			0xF3,
		CalibrateEvent =			0x04,
		SetVar =					0x14,
		SumVar =					0x24,
		SubVar =					0x34,
		DivVar =					0x44,
		MulVar =					0x54,
		SgnVar =					0x64,
		AbsVar =					0x74,
		AndVar =					0x84,
		OrVar =						0x94,
		Upload =					0xA4,
		SEnterEventCheck =			0xB4,
		SetSourceValue =			0x05,
		UnlockPBrick =				0x15,
		BeginOfTask =				0x25,
		BeginOfSub =				0x35,
		ContinueFirmwareDownLoad =	0x45,
		GoIntoBootMode =			0x65,
		BeginFirmwareDownLoad =		0x75,
		SCheckDo =					0x85,
		LCheckDo =					0x95,
		UnlockFirmware =			0xA5,
		LEnterEventCheck =			0xB5,
		ViewSourceValue =			0xE5,
	};

	enum class ParamSource : BYTE
	{
		VARIABLE =		0,
		TIMER =			1,
		CONSTANT =		2,
		MOTOR_STATUS =	3,
		RANDOM =		4,
		PROGRAM_SLOT =	8,
		SENSOR_VALUE =	9,
		SENSOR_TYPE =	10,
		SENSOR_MODE =	11,
		SENSOR_RAW =	12,
		SENSOR_BOOL =	13,
		WATCH =			14,
		MESSAGE =		15,
		GLOBAL_MOTOR_STATUS = 17,
		COUNTER =		21,
		TASK_EVENTS =	23,
		EVENT_STATE =	25,
		TEN_MS_TIMER =	26,
		CLICK_COUNTER =	27,
		UPPER_THRESHOLD = 28,
		LOWER_THRESHOLD = 29,
		HYSTERESIS =	30,
		DURATION =		31,
		UART_SETUP =	33,
		BATTERY_LEVEL =	34,
		FIRMWARE_VERSION = 35,
		INDIRECT_VARIABLE = 36
	};

	PBRICKLOGIC_API BOOL ValidateReply(Command commandByte, BYTE* replyBuffer, UINT replyLength);

#define MAX_COMMAND_LENGTH 30 // I dunno what this *should* be, but I'm keeping it high so that I don't get memory errors later.

	struct CommandData
	{
		Command command;
		std::shared_ptr<BYTE[]> data;
		UINT dataLength;

		CommandData(Command command)
		{
			this->command = command;
			dataLength = MAX_COMMAND_LENGTH;
			data = std::make_unique<BYTE[]>(MAX_COMMAND_LENGTH);
		}

		~CommandData()
		{
			data.reset();
		}
	};

	PBRICKLOGIC_API CommandData ComposeCommand(Command lasmCommand, BYTE* params, UINT paramsLength);

	enum class MotorAction : BYTE
	{
		FLOAT = 0,
		OFF = 1,
		ON = 2
	};

	namespace Motor
	{
		// note to future me: this is not an enum because if it were an enum i wouldn't be able to do ORs on them
		const BYTE A = 0b01;
		const BYTE B = 0b10;
		const BYTE C = 0b100;
	}

	PBRICKLOGIC_API CommandData Cmd_OnOffFloat(BYTE motors, MotorAction action);

	enum class IRTransmissionRange : BYTE
	{
		SHORT = 0,
		LONG = 1
	};
	PBRICKLOGIC_API CommandData Cmd_PbTXPower(IRTransmissionRange range);

	enum class SystemSound : BYTE
	{
		KEY_CLICK = 0,
		BEEP = 1,
		SWEEP_DOWN = 2,
		SWEEP_UP = 3,
		ERROR_SOUND = 4,
		FAST_SWEEP_UP = 5
	};
	PBRICKLOGIC_API CommandData Cmd_PlaySystemSound(SystemSound sound);

	// task is number from 0 to 9
	PBRICKLOGIC_API CommandData Cmd_DeleteTask(BYTE task);
	PBRICKLOGIC_API CommandData Cmd_StartTask(BYTE task);
	PBRICKLOGIC_API CommandData Cmd_StopTask(BYTE task);

	// program is number from 0 to 5
	PBRICKLOGIC_API CommandData Cmd_SelectProgram(BYTE program);

	// timer is number from 0 to 3
	PBRICKLOGIC_API CommandData Cmd_ClearTimer(BYTE timer);

	PBRICKLOGIC_API CommandData Cmd_PBPowerDownTime(BYTE minutes);

	// subroutine is number from 0 to 7
	PBRICKLOGIC_API CommandData Cmd_DeleteSub(BYTE subroutine);

	// sensor is number from 0 to 3
	PBRICKLOGIC_API CommandData Cmd_ClearSensorValue(BYTE sensor);

	enum class MotorDirection : BYTE
	{
		BACKWARDS = 0,
		REVERSE = 1,
		FORWARDS = 2
	};
	PBRICKLOGIC_API CommandData Cmd_SetFwdSetRwdRewDir(BYTE motors, MotorDirection direction);

	// subroutine is number from 0 to 7
	PBRICKLOGIC_API CommandData Cmd_GoSub(BYTE subroutine);

	enum class JumpDirection : BYTE
	{
		FORWARDS = 0,
		BACKWARDS = 1
	};
	// distance is number from 0 to 6
	PBRICKLOGIC_API CommandData Cmd_SJump(BYTE distance, JumpDirection direction);
	PBRICKLOGIC_API CommandData Cmd_SCheckLoopCounter(BYTE distance);

	PBRICKLOGIC_API CommandData Cmd_ConnectDisconnect(BYTE motor, MotorAction action);

	// counter is number from 0 to 2
	PBRICKLOGIC_API CommandData Cmd_IncCounter(BYTE counter);
	PBRICKLOGIC_API CommandData Cmd_DecCounter(BYTE counter);
	PBRICKLOGIC_API CommandData Cmd_ClearCounter(BYTE counter);

	// priority is number from 0 to 255 (0 is highest priority)
	PBRICKLOGIC_API CommandData Cmd_SetPriority(BYTE priority);

	// dunno what irMessage is, page 48 of the Firmware Command Overview
	PBRICKLOGIC_API CommandData Cmd_InternMessage(BYTE irMessage);

	// duration is in 1/100 second
	PBRICKLOGIC_API CommandData Cmd_PlayToneVar(BYTE variableNumber, BYTE duration);

	// dunno what source and value are
	PBRICKLOGIC_API CommandData Cmd_Poll(BYTE source, BYTE value);

	// i assume 24 hour time, so hours is 0-23 and minutes is 0-59?
	PBRICKLOGIC_API CommandData Cmd_SetWatch(BYTE hours, BYTE minutes);

	enum class SensorType : BYTE
	{
		NO_SENSOR = 0,
		SWITCH = 1,
		TEMPERATURE = 2,
		REFLECTION = 3,
		ANGLE = 4
	};
	// sensorNumber is 0-2 i think
	PBRICKLOGIC_API CommandData Cmd_SetSensorType(BYTE sensorNumber, SensorType type);

	enum class SensorMode : BYTE
	{
		RAW = 0,
		BOOLEAN = 1,
		TRANSITION_COUNT = 2,
		PERIOD_COUNTER = 3,
		PCT_FULL_SCALE = 4,
		CELSIUS = 5,
		FAHRENHEIT = 6,
		ANGLE_STEPS = 7
	};
	// slope is number from 0 to 31
	PBRICKLOGIC_API CommandData Cmd_SetSensorMode(BYTE sensorNumber, BYTE slope, SensorMode mode);

	PBRICKLOGIC_API CommandData Cmd_SetDataLog();
	PBRICKLOGIC_API CommandData Cmd_DataLogNext();

	PBRICKLOGIC_API CommandData Cmd_LJump(BYTE distance, JumpDirection direction);

	PBRICKLOGIC_API CommandData Cmd_SetLoopCounter();
	PBRICKLOGIC_API CommandData Cmd_LCheckLoopCounter();
	PBRICKLOGIC_API CommandData Cmd_SendPBMessage();
	PBRICKLOGIC_API CommandData Cmd_SendUARTData();
	PBRICKLOGIC_API CommandData Cmd_RemoteCommand();
	PBRICKLOGIC_API CommandData Cmd_SDecVarJumpLTZero();
	PBRICKLOGIC_API CommandData Cmd_DirectEvent();

	PBRICKLOGIC_API CommandData Cmd_SetPower(BYTE motorList, ParamSource powerSource, BYTE powerValue);
	PBRICKLOGIC_API CommandData Cmd_PlayTone(WORD frequency, BYTE duration);

	PBRICKLOGIC_API CommandData Cmd_SelectDisplay();
	PBRICKLOGIC_API CommandData Cmd_Wait();
	PBRICKLOGIC_API CommandData Cmd_UploadRAM();
	PBRICKLOGIC_API CommandData Cmd_EnterAccessControl();
	PBRICKLOGIC_API CommandData Cmd_SetEvent();
	PBRICKLOGIC_API CommandData Cmd_SetMaxPower();
	PBRICKLOGIC_API CommandData Cmd_LDecVarJumpLTZero();
	PBRICKLOGIC_API CommandData Cmd_CalibrateEvent();
	PBRICKLOGIC_API CommandData Cmd_SetVar();
	PBRICKLOGIC_API CommandData Cmd_SumVar();
	PBRICKLOGIC_API CommandData Cmd_SubVar();
	PBRICKLOGIC_API CommandData Cmd_DivVar();
	PBRICKLOGIC_API CommandData Cmd_MulVar();
	PBRICKLOGIC_API CommandData Cmd_SgnVar();
	PBRICKLOGIC_API CommandData Cmd_AbsVar();
	PBRICKLOGIC_API CommandData Cmd_AndVar();
	PBRICKLOGIC_API CommandData Cmd_OrVar();
	PBRICKLOGIC_API CommandData Cmd_Upload();
	PBRICKLOGIC_API CommandData Cmd_SEnterEventCheck();
	PBRICKLOGIC_API CommandData Cmd_SetSourceValue();
	PBRICKLOGIC_API CommandData Cmd_UnlockPBrick();
	PBRICKLOGIC_API CommandData Cmd_BeginOfTask();
	PBRICKLOGIC_API CommandData Cmd_BeginOfSub();
	PBRICKLOGIC_API CommandData Cmd_ContinueFirmwareDownload();
	PBRICKLOGIC_API CommandData Cmd_GoIntoBootMode();
	PBRICKLOGIC_API CommandData Cmd_BeginFirmwareDownload();
	PBRICKLOGIC_API CommandData Cmd_SCheckDo();
	PBRICKLOGIC_API CommandData Cmd_LCheckDo();
	PBRICKLOGIC_API CommandData Cmd_UnlockFirmware();
	PBRICKLOGIC_API CommandData Cmd_LEnterEventCheck();
	PBRICKLOGIC_API CommandData Cmd_ViewSourceValue();






#define Cmd(command, availabilityArg) \
PBRICKLOGIC_API inline CommandData Cmd_##command##() \
{ \
	return ComposeCommand(Command::##command##, nullptr, 0); \
}

#define COMMA ,
#define NO_PARAM NoParam
#define HAS_PARAMS NumParam
#define NO_REPLY -1

	// dumped from the LASM pdf from the RCX 2.0 sdk using a python script i wrote that's included in the root folder of this repo
	Cmd(PBAliveOrNot, BOTH)
	Cmd(MemMap, BOTH)
	Cmd(PBBattery, BOTH)
	Cmd(DeleteAllTasks, BOTH)
	Cmd(StopAllTasks, BOTH)
	Cmd(PBTurnOff, BOTH)
	Cmd(DeleteAllSubs, BOTH)
	Cmd(ClearSound, BOTH)
	Cmd(ClearPBMessage, BOTH)
	Cmd(ExitAccessControl, BOTH)
	Cmd(ExitEventCheck, BOTH)
	Cmd(MuteSound, BOTH)
	Cmd(UnmuteSound, BOTH)
	Cmd(ClearAllEvents, BOTH)
	Cmd(EndOfSub, BOTH)
	//Cmd(OnOffFloat, BOTH);
	//Cmd(PbTXPower, BOTH);
	//Cmd(PlaySystemSound, BOTH);
	//Cmd(DeleteTask, BOTH);
	//Cmd(StartTask, BOTH);
	//Cmd(StopTask, BOTH);
	//Cmd(SelectProgram, BOTH);
	//Cmd(ClearTimer, BOTH);
	//Cmd(PBPowerDownTime, BOTH);
	//Cmd(DeleteSub, BOTH);
	//Cmd(ClearSensorValue, BOTH);
	//Cmd(SetFwdSetRwdRewDir, BOTH);
	//Cmd(Gosub, BOTH);
	//Cmd(SJump, BOTH);
	//Cmd(SCheckLoopCounter, BOTH);
	//Cmd(ConnectDisconnect, BOTH);
	//Cmd(SetNormSetInvAltDir, BOTH);
	//Cmd(IncCounter, BOTH);
	//Cmd(DecCounter, BOTH);
	//Cmd(ClearCounter, BOTH);
	//Cmd(SetPriority, BOTH);
	//Cmd(InternMessage, BOTH);
	//Cmd(PlayToneVar, BOTH);
	//Cmd(Poll, BOTH);
	//Cmd(SetWatch, BOTH);
	//Cmd(SetSensorType, BOTH);
	//Cmd(SetSensorMode, BOTH);
	//Cmd(SetDataLog, BOTH);
	//Cmd(DataLogNext, BOTH);
	//Cmd(LJump, BOTH);
	//Cmd(SetLoopCounter, BOTH);
	//Cmd(LCheckLoopCounter, BOTH);
	//Cmd(SendPBMessage, BOTH);
	//Cmd(SendUARTData, BOTH);
	//Cmd(RemoteCommand, BOTH);
	//Cmd(SDecVarJumpLTZero, BOTH);
	//Cmd(DirectEvent, BOTH);
	//Cmd(SetPower, BOTH);
	//Cmd(PlayTone, BOTH);
	//Cmd(SelectDisplay, BOTH);
	//Cmd(Wait, BOTH);
	//Cmd(UploadRam, BOTH);
	//Cmd(EnterAccessControl, BOTH);
	//Cmd(SetEvent, BOTH);
	//Cmd(SetMaxPower, BOTH);
	//Cmd(LDecVarJumpLTZero, BOTH);
	//Cmd(CalibrateEvent, BOTH);
	//Cmd(SetVar, BOTH);
	//Cmd(SumVar, BOTH);
	//Cmd(SubVar, BOTH);
	//Cmd(DivVar, BOTH);
	//Cmd(MulVar, BOTH);
	//Cmd(SgnVar, BOTH);
	//Cmd(AbsVar, BOTH);
	//Cmd(AndVar, BOTH);
	//Cmd(OrVar, BOTH);
	//Cmd(Upload, BOTH);
	//Cmd(SEnterEventCheck, BOTH);
	//Cmd(SetSourceValue, BOTH);
	//Cmd(UnlockPBrick, BOTH);
	//Cmd(BeginOfTask, BOTH);
	//Cmd(BeginOfSub, BOTH);
	//Cmd(ContinueFirmwareDownLoad, BOTH);
	//Cmd(GoIntoBootMode, BOTH);
	//Cmd(BeginFirmwareDownLoad, BOTH);
	//Cmd(SCheckDo, BOTH);
	//Cmd(LCheckDo, BOTH);
	//Cmd(UnlockFirmware, BOTH);
	//Cmd(LEnterEventCheck, BOTH);
	//Cmd(ViewSourceValue, BOTH);*/
}
