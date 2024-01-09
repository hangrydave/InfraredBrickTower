#pragma once

#include "TowerController.h"
#include <string>

#define BYTE unsigned char
#define WORD unsigned short

namespace LASM
{
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

#define COMMAND_REPLY_BUFFER_LENGTH 10

	/*enum Availability
	{
		DIRECT,
		PROGRAM,
		BOTH
	};*/

	enum class Command : BYTE
	{
		NO_COMMAND = 0,
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
		Bababooey = 0x4d
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
		SENSOR_bool =	13,
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

#define MAX_COMMAND_LENGTH 420 // I dunno what this *should* be, but I'm keeping it high so that I don't get memory errors later.

	struct CommandData
	{
		BYTE previousCommandByte = 0;
		BYTE commandByte = 0;
		BYTE data[MAX_COMMAND_LENGTH];
		unsigned int dataLength = 0;

		CommandData()
		{
			*data = {};
		}

		CommandData(Command command)
		{
			this->commandByte = (BYTE)command;
			dataLength = MAX_COMMAND_LENGTH;
			*data = {};
		}

		~CommandData() { }
	};

	bool ValidateReply(CommandData* command, BYTE* replyBuffer, unsigned int replyLength);
	bool SendCommand(
		CommandData* command,
		Tower::RequestData* towerData);
	bool SendCommand(
		CommandData* command,
		Tower::RequestData* towerData,
		BYTE* replyBuffer,
		bool skipReply = false,
		bool skipReplyValidation = false,
		bool preWriteFlush = true);

	LASMStatus ParseAndSendLASM(const char* lasmInput, Tower::RequestData* towerData);
	bool GetCommandFromCode(
		const char* code, 
		std::string parameters[],
		unsigned long paramCount,
		CommandData* command);

	void ComposeCommand(Command lasmCommand, BYTE* params, unsigned int paramsLength, CommandData& commandData);

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

	void Cmd_OnOffFloat(BYTE motors, MotorAction action, CommandData& commandData);

	enum class IRTransmissionRange : BYTE
	{
		SHORT = 0,
		LONG = 1
	};
	void Cmd_PbTXPower(IRTransmissionRange range, CommandData& commandData);

	enum class SystemSound : BYTE
	{
		KEY_CLICK = 0,
		BEEP = 1,
		SWEEP_DOWN = 2,
		SWEEP_UP = 3,
		ERROR_SOUND = 4,
		FAST_SWEEP_UP = 5
	};
	void Cmd_PlaySystemSound(SystemSound sound, CommandData& commandData);

	// task is number from 0 to 9
	void Cmd_DeleteTask(BYTE task, CommandData& commandData);
	void Cmd_StartTask(BYTE task, CommandData& commandData);
	void Cmd_StopTask(BYTE task, CommandData& commandData);

	// program is number from 0 to 5
	void Cmd_SelectProgram(BYTE program, CommandData& commandData);

	// timer is number from 0 to 3
	void Cmd_ClearTimer(BYTE timer, CommandData& commandData);

	void Cmd_PBPowerDownTime(BYTE minutes, CommandData& commandData);

	// subroutine is number from 0 to 7
	void Cmd_DeleteSub(BYTE subroutine, CommandData& commandData);

	// sensor is number from 0 to 3
	void Cmd_ClearSensorValue(BYTE sensor, CommandData& commandData);

	enum class MotorDirection : BYTE
	{
		BACKWARDS = 0,
		REVERSE = 1,
		FORWARDS = 2
	};
	void Cmd_SetFwdSetRwdRewDir(BYTE motors, MotorDirection direction, CommandData& commandData);

	// subroutine is number from 0 to 7
	void Cmd_GoSub(BYTE subroutine, CommandData& commandData);

	enum class JumpDirection : BYTE
	{
		FORWARDS = 0,
		BACKWARDS = 1
	};
	// distance is number from 0 to 6
	void Cmd_SJump(BYTE distance, JumpDirection direction, CommandData& commandData);
	void Cmd_SCheckLoopCounter(BYTE distance, CommandData& commandData);

	void Cmd_ConnectDisconnect(BYTE motor, MotorAction action, CommandData& commandData);

	// counter is number from 0 to 2
	void Cmd_IncCounter(BYTE counter, CommandData& commandData);
	void Cmd_DecCounter(BYTE counter, CommandData& commandData);
	void Cmd_ClearCounter(BYTE counter, CommandData& commandData);

	// priority is number from 0 to 255 (0 is highest priority)
	void Cmd_SetPriority(BYTE priority, CommandData& commandData);

	// dunno what irMessage is, page 48 of the Firmware Command Overview
	void Cmd_InternMessage(BYTE irMessage, CommandData& commandData);

	// duration is in 1/100 second
	void Cmd_PlayToneVar(BYTE variableNumber, BYTE duration, CommandData& commandData);

	// dunno what source and value are
	void Cmd_Poll(BYTE source, BYTE value, CommandData& commandData);

	// i assume 24 hour time, so hours is 0-23 and minutes is 0-59?
	void Cmd_SetWatch(BYTE hours, BYTE minutes, CommandData& commandData);

	enum class SensorType : BYTE
	{
		NO_SENSOR = 0,
		SWITCH = 1,
		TEMPERATURE = 2,
		REFLECTION = 3,
		ANGLE = 4
	};
	// sensorNumber is 0-2 i think
	void Cmd_SetSensorType(BYTE sensorNumber, SensorType type, CommandData& commandData);

	enum class SensorMode : BYTE
	{
		RAW = 0,
		boolEAN = 1,
		TRANSITION_COUNT = 2,
		PERIOD_COUNTER = 3,
		PCT_FULL_SCALE = 4,
		CELSIUS = 5,
		FAHRENHEIT = 6,
		ANGLE_STEPS = 7
	};
	// slope is number from 0 to 31
	void Cmd_SetSensorMode(BYTE sensorNumber, BYTE slope, SensorMode mode, CommandData& commandData);

	void Cmd_SetDataLog();
	void Cmd_DataLogNext();

	void Cmd_LJump(BYTE distance, JumpDirection direction, CommandData& commandData);

	void Cmd_SetLoopCounter();
	void Cmd_LCheckLoopCounter();
	void Cmd_SendPBMessage();
	void Cmd_SendUARTData();

	enum RemoteCommandRequest : WORD
	{
		MOTOR_C_BACKWARDS = 0x01,
		PROGRAM_1 = 0x02,
		PROGRAM_2 = 0x04,
		PROGRAM_3 = 0x08,
		PROGRAM_4 = 0x10,
		PROGRAM_5 = 0x20,
		STOP_PROGRAM_AND_MOTORS = 0x40,
		REMOTE_SOUND = 0x80,
		PB_MESSAGE_1 = 0x0100,
		PB_MESSAGE_2 = 0x0200,
		PB_MESSAGE_3 = 0x0400,
		MOTOR_A_FORWARDS = 0x0800,
		MOTOR_B_FORWARDS = 0x1000,
		MOTOR_C_FORWARDS = 0x2000,
		MOTOR_A_BACKWARDS = 0x4000,
		MOTOR_B_BACKWARDS = 0x8000
	};
	void Cmd_RemoteCommand(WORD request, CommandData& commandData);

	void Cmd_SDecVarJumpLTZero();
	void Cmd_DirectEvent();

	void Cmd_SetPower(BYTE motorList, ParamSource powerSource, BYTE powerValue, CommandData& commandData);
	void Cmd_PlayTone(WORD frequency, BYTE duration, CommandData& commandData);

	void Cmd_SelectDisplay();
	void Cmd_Wait();
	void Cmd_UploadRAM();
	void Cmd_EnterAccessControl();
	void Cmd_SetEvent();
	void Cmd_SetMaxPower();
	void Cmd_LDecVarJumpLTZero();
	void Cmd_CalibrateEvent();
	void Cmd_SetVar();
	void Cmd_SumVar();
	void Cmd_SubVar();
	void Cmd_DivVar();
	void Cmd_MulVar();
	void Cmd_SgnVar();
	void Cmd_AbsVar();
	void Cmd_AndVar();
	void Cmd_OrVar();
	void Cmd_Upload();
	void Cmd_SEnterEventCheck();
	void Cmd_SetSourceValue();
	void Cmd_UnlockPBrick(CommandData& commandData);

	void Cmd_BeginOfTask(BYTE taskNumber, BYTE taskSize, CommandData& commandData);
	void Cmd_BeginOfSub(BYTE subNumber, BYTE subSize, CommandData& commandData);
	
	void Cmd_Download(BYTE* data, BYTE blockCount, BYTE byteCount, CommandData& commandData);

	void Cmd_GoIntoBootMode(CommandData& commandData);

	void Cmd_BeginFirmwareDownload(int checksum, CommandData& commandData);
	
	void Cmd_SCheckDo();
	void Cmd_LCheckDo();
	void Cmd_UnlockFirmware(CommandData& commandData);
	void Cmd_LEnterEventCheck();
	void Cmd_ViewSourceValue();

#define Cmd(command, availabilityArg) \
inline void Cmd_##command(CommandData& commandData) \
{ \
	ComposeCommand(Command::command, nullptr, 0, commandData); \
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
