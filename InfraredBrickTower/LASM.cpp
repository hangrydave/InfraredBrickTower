#include "LASM.h"

namespace LASM
{
	BOOL IsReplyByteGood(LASMCommandByte commandByte, BYTE reply)
	{
		return (~commandByte & 0xff) == reply;
	}

	VOID BuildCommand(Command* command)
	{
		BYTE* baseCommandPointer = command->dataToTransmit;
		BYTE* dataPtr = command->dataToTransmit;

		// preamble
		*(dataPtr++) = 0x55;
		*(dataPtr++) = 0xFF;
		*(dataPtr++) = 0x00;

		UINT dataSum = 0;

		// command, reply, and repeat both
		*(dataPtr++) = command->commandByte;
		*(dataPtr++) = ~command->commandByte;

		dataSum += command->commandByte;

		for (UINT i = 0; i < command->paramsLength; i++)
		{
			BYTE paramByte = command->params[i];
			*(dataPtr++) = paramByte;
			*(dataPtr++) = ~paramByte;

			dataSum += paramByte;
		}

		// checksum for the RCX is just the data sum, so...
		*(dataPtr++) = dataSum;
		*(dataPtr++) = ~dataSum;

		command->transmissionLength = (dataPtr - baseCommandPointer);
	}

	//VOID BuildCmd_PlaySystemSound(const Command* command, BYTE sound)
	//{
	//	//command()
	//}
}