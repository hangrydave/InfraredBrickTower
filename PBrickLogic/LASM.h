#pragma once

#include <Windows.h>
#include <memory>

#include "TowerController.h"

namespace LASM
{
#define COMMAND_REPLY_BUFFER_LENGTH 10

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
		Download =					0x45, // original name is ContinueFirmwareDownLoad
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

#define MAX_COMMAND_LENGTH 45 // I dunno what this *should* be, but I'm keeping it high so that I don't get memory errors later.

	struct CommandData
	{
		Command command;
		//std::shared_ptr<BYTE[]> data;
		BYTE data[MAX_COMMAND_LENGTH];
		UINT dataLength;

		CommandData() {}

		CommandData(Command command)
		{
			this->command = command;
			dataLength = MAX_COMMAND_LENGTH;
			//data = std::make_unique<BYTE[]>(MAX_COMMAND_LENGTH);
		}

		~CommandData()
		{
			//data.reset();
			//delete[] data;
		}
	};

	BOOL ValidateReply(Command commandByte, BYTE* replyBuffer, UINT replyLength);
	BOOL SendCommand(CommandData* command, Tower::RequestData* towerData);

	VOID ComposeCommand(Command lasmCommand, BYTE* params, UINT paramsLength, CommandData& commandData);

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

	VOID Cmd_OnOffFloat(BYTE motors, MotorAction action, CommandData& commandData);

	enum class IRTransmissionRange : BYTE
	{
		SHORT = 0,
		LONG = 1
	};
	VOID Cmd_PbTXPower(IRTransmissionRange range, CommandData& commandData);

	enum class SystemSound : BYTE
	{
		KEY_CLICK = 0,
		BEEP = 1,
		SWEEP_DOWN = 2,
		SWEEP_UP = 3,
		ERROR_SOUND = 4,
		FAST_SWEEP_UP = 5
	};
	VOID Cmd_PlaySystemSound(SystemSound sound, CommandData& commandData);

	// task is number from 0 to 9
	VOID Cmd_DeleteTask(BYTE task, CommandData& commandData);
	VOID Cmd_StartTask(BYTE task, CommandData& commandData);
	VOID Cmd_StopTask(BYTE task, CommandData& commandData);

	// program is number from 0 to 5
	VOID Cmd_SelectProgram(BYTE program, CommandData& commandData);

	// timer is number from 0 to 3
	VOID Cmd_ClearTimer(BYTE timer, CommandData& commandData);

	VOID Cmd_PBPowerDownTime(BYTE minutes, CommandData& commandData);

	// subroutine is number from 0 to 7
	VOID Cmd_DeleteSub(BYTE subroutine, CommandData& commandData);

	// sensor is number from 0 to 3
	VOID Cmd_ClearSensorValue(BYTE sensor, CommandData& commandData);

	enum class MotorDirection : BYTE
	{
		BACKWARDS = 0,
		REVERSE = 1,
		FORWARDS = 2
	};
	VOID Cmd_SetFwdSetRwdRewDir(BYTE motors, MotorDirection direction, CommandData& commandData);

	// subroutine is number from 0 to 7
	VOID Cmd_GoSub(BYTE subroutine, CommandData& commandData);

	enum class JumpDirection : BYTE
	{
		FORWARDS = 0,
		BACKWARDS = 1
	};
	// distance is number from 0 to 6
	VOID Cmd_SJump(BYTE distance, JumpDirection direction, CommandData& commandData);
	VOID Cmd_SCheckLoopCounter(BYTE distance, CommandData& commandData);

	VOID Cmd_ConnectDisconnect(BYTE motor, MotorAction action, CommandData& commandData);

	// counter is number from 0 to 2
	VOID Cmd_IncCounter(BYTE counter, CommandData& commandData);
	VOID Cmd_DecCounter(BYTE counter, CommandData& commandData);
	VOID Cmd_ClearCounter(BYTE counter, CommandData& commandData);

	// priority is number from 0 to 255 (0 is highest priority)
	VOID Cmd_SetPriority(BYTE priority, CommandData& commandData);

	// dunno what irMessage is, page 48 of the Firmware Command Overview
	VOID Cmd_InternMessage(BYTE irMessage, CommandData& commandData);

	// duration is in 1/100 second
	VOID Cmd_PlayToneVar(BYTE variableNumber, BYTE duration, CommandData& commandData);

	// dunno what source and value are
	VOID Cmd_Poll(BYTE source, BYTE value, CommandData& commandData);

	// i assume 24 hour time, so hours is 0-23 and minutes is 0-59?
	VOID Cmd_SetWatch(BYTE hours, BYTE minutes, CommandData& commandData);

	enum class SensorType : BYTE
	{
		NO_SENSOR = 0,
		SWITCH = 1,
		TEMPERATURE = 2,
		REFLECTION = 3,
		ANGLE = 4
	};
	// sensorNumber is 0-2 i think
	VOID Cmd_SetSensorType(BYTE sensorNumber, SensorType type, CommandData& commandData);

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
	VOID Cmd_SetSensorMode(BYTE sensorNumber, BYTE slope, SensorMode mode, CommandData& commandData);

	VOID Cmd_SetDataLog();
	VOID Cmd_DataLogNext();

	VOID Cmd_LJump(BYTE distance, JumpDirection direction, CommandData& commandData);

	VOID Cmd_SetLoopCounter();
	VOID Cmd_LCheckLoopCounter();
	VOID Cmd_SendPBMessage();
	VOID Cmd_SendUARTData();
	VOID Cmd_RemoteCommand();
	VOID Cmd_SDecVarJumpLTZero();
	VOID Cmd_DirectEvent();

	VOID Cmd_SetPower(BYTE motorList, ParamSource powerSource, BYTE powerValue, CommandData& commandData);
	VOID Cmd_PlayTone(WORD frequency, BYTE duration, CommandData& commandData);

	VOID Cmd_SelectDisplay();
	VOID Cmd_Wait();
	VOID Cmd_UploadRAM();
	VOID Cmd_EnterAccessControl();
	VOID Cmd_SetEvent();
	VOID Cmd_SetMaxPower();
	VOID Cmd_LDecVarJumpLTZero();
	VOID Cmd_CalibrateEvent();
	VOID Cmd_SetVar();
	VOID Cmd_SumVar();
	VOID Cmd_SubVar();
	VOID Cmd_DivVar();
	VOID Cmd_MulVar();
	VOID Cmd_SgnVar();
	VOID Cmd_AbsVar();
	VOID Cmd_AndVar();
	VOID Cmd_OrVar();
	VOID Cmd_Upload();
	VOID Cmd_SEnterEventCheck();
	VOID Cmd_SetSourceValue();
	VOID Cmd_UnlockPBrick();

	VOID Cmd_BeginOfTask(BYTE taskNumber, BYTE taskSize, CommandData& commandData);
	VOID Cmd_BeginOfSub(BYTE subNumber, BYTE subSize, CommandData& commandData);
	
	VOID Cmd_Download(BYTE* data, BYTE blockCount, BYTE byteCount, CommandData& commandData);

	VOID Cmd_GoIntoBootMode();
	VOID Cmd_BeginFirmwareDownload();
	VOID Cmd_SCheckDo();
	VOID Cmd_LCheckDo();
	VOID Cmd_UnlockFirmware();
	VOID Cmd_LEnterEventCheck();
	VOID Cmd_ViewSourceValue();

#define Cmd(command, availabilityArg) \
inline VOID Cmd_##command##(CommandData& commandData) \
{ \
	ComposeCommand(Command::##command##, nullptr, 0, commandData); \
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
}
