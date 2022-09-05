#pragma once

#include <Windows.h>
#include <memory>

enum Availability
{
	DIRECT,
	PROGRAM,
	BOTH
};

namespace LASM
{
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

	BOOL ValidateReply(Command commandByte, BYTE* replyBuffer, UINT replyLength);

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

	CommandData ComposeCommand(Command lasmCommand);
	CommandData ComposeCommand(Command lasmCommand, BYTE* params, UINT paramsLength);

	enum class MotorAction : BYTE
	{
		FLOAT = 0,
		OFF = 1,
		ON = 2
	};

	namespace Motor
	{
		const BYTE A = 0b01;
		const BYTE B = 0b10;
		const BYTE C = 0b100;
	}

	//const BYTE MOTOR_A = 0b01;
	//const BYTE MOTOR_B = 0b10;
	//const BYTE MOTOR_C = 0b100;
	CommandData Cmd_OnOffFloat(BYTE motors, MotorAction action);

	enum class IRTransmissionRange : BYTE
	{
		SHORT = 0,
		LONG = 1
	};
	CommandData Cmd_PbTXPower(IRTransmissionRange range);

	enum class SystemSound : BYTE
	{
		KEY_CLICK = 0,
		BEEP = 1,
		SWEEP_DOWN = 2,
		SWEEP_UP = 3,
		ERROR_SOUND = 4,
		FAST_SWEEP_UP = 5
	};
	CommandData Cmd_PlaySystemSound(SystemSound sound);

	// task is number from 0 to 9
	CommandData Cmd_DeleteTask(BYTE task);
	CommandData Cmd_StartTask(BYTE task);
	CommandData Cmd_StopTask(BYTE task);

	// program is number from 0 to 5
	CommandData Cmd_SelectProgram(BYTE program);

	// timer is number from 0 to 3
	CommandData Cmd_ClearTimer(BYTE timer);

	CommandData Cmd_PBPowerDownTime(BYTE minutes);

	// subroutine is number from 0 to 7
	CommandData Cmd_DeleteSub(BYTE subroutine);

	// sensor is number from 0 to 3
	CommandData Cmd_ClearSensorValue(BYTE sensor);

	enum class MotorDirection : BYTE
	{
		BACKWARDS = 0,
		REVERSE = 1,
		FORWARDS = 2
	};
	CommandData Cmd_SetFwdSetRwdRewDir(BYTE motors, MotorDirection direction);

	// subroutine is number from 0 to 7
	CommandData Cmd_GoSub(BYTE subroutine);

	enum class JumpDirection : BYTE
	{
		FORWARDS = 0,
		BACKWARDS = 1
	};
	// distance is number from 0 to 6
	CommandData Cmd_SJump(BYTE distance, JumpDirection direction);
	CommandData Cmd_SCheckLoopCounter(BYTE distance);

	CommandData Cmd_ConnectDisconnect(BYTE motor, MotorAction action);

	// counter is number from 0 to 2
	CommandData Cmd_IncCounter(BYTE counter);
	CommandData Cmd_DecCounter(BYTE counter);
	CommandData Cmd_ClearCounter(BYTE counter);

	// priority is number from 0 to 255 (0 is highest priority)
	CommandData Cmd_SetPriority(BYTE priority);

	// dunno what irMessage is, page 48 of the Firmware Command Overview
	CommandData Cmd_InternMessage(BYTE irMessage);

	// duration is in 1/100 second
	CommandData Cmd_PlayToneVar(BYTE variableNumber, BYTE duration);

	// dunno what source and value are
	CommandData Cmd_Poll(BYTE source, BYTE value);

	// i assume 24 hour time, so hours is 0-23 and minutes is 0-59?
	CommandData Cmd_SetWatch(BYTE hours, BYTE minutes);

	enum class SensorType : BYTE
	{
		NO_SENSOR = 0,
		SWITCH = 1,
		TEMPERATURE = 2,
		REFLECTION = 3,
		ANGLE = 4
	};
	// sensorNumber is 0-2 i think
	CommandData Cmd_SetSensorType(BYTE sensorNumber, SensorType type);

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
	CommandData Cmd_SetSensorMode(BYTE sensorNumber, BYTE slope, SensorMode mode);

	CommandData Cmd_SetDataLog();
	CommandData Cmd_DataLogNext();

	CommandData Cmd_LJump(BYTE distance, JumpDirection direction);

	CommandData Cmd_SetLoopCounter();
	CommandData Cmd_LCheckLoopCounter();
	CommandData Cmd_SendPBMessage();
	CommandData Cmd_SendUARTData();
	CommandData Cmd_RemoteCommand();
	CommandData Cmd_SDecVarJumpLTZero();
	CommandData Cmd_DirectEvent();

	CommandData Cmd_SetPower(BYTE motorList, ParamSource powerSource, BYTE powerValue);

	CommandData Cmd_PlayTone();
	CommandData Cmd_SelectDisplay();
	CommandData Cmd_Wait();
	CommandData Cmd_UploadRAM();
	CommandData Cmd_EnterAccessControl();
	CommandData Cmd_SetEvent();
	CommandData Cmd_SetMaxPower();
	CommandData Cmd_LDecVarJumpLTZero();
	CommandData Cmd_CalibrateEvent();
	CommandData Cmd_SetVar();
	CommandData Cmd_SumVar();
	CommandData Cmd_SubVar();
	CommandData Cmd_DivVar();
	CommandData Cmd_MulVar();
	CommandData Cmd_SgnVar();
	CommandData Cmd_AbsVar();
	CommandData Cmd_AndVar();
	CommandData Cmd_OrVar();
	CommandData Cmd_Upload();
	CommandData Cmd_SEnterEventCheck();
	CommandData Cmd_SetSourceValue();
	CommandData Cmd_UnlockPBrick();
	CommandData Cmd_BeginOfTask();
	CommandData Cmd_BeginOfSub();
	CommandData Cmd_ContinueFirmwareDownload();
	CommandData Cmd_GoIntoBootMode();
	CommandData Cmd_BeginFirmwareDownload();
	CommandData Cmd_SCheckDo();
	CommandData Cmd_LCheckDo();
	CommandData Cmd_UnlockFirmware();
	CommandData Cmd_LEnterEventCheck();
	CommandData Cmd_ViewSourceValue();






#define Cmd(command, availabilityArg) \
inline CommandData Cmd_##command##() \
{ \
	return ComposeCommand(Command::##command##); \
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
