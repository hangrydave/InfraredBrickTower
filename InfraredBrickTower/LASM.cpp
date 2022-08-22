#include "LASM.h"

namespace LASM
{
	BOOL IsReplyByteGood(LASMCommandByte commandByte, BYTE reply)
	{
		return (~commandByte & 0xff) == reply;
	}

	/*VOID ComposePlaySystemSound(MessageData* messageData, SystemSound sound)
	{
		messageData->commandByte = PlaySystemSound;
		messageData->params[0] = (BYTE)sound;
		messageData->paramsLength = 1;
		ComposeMessage(messageData);
	}*/

	VOID ComposeMessage(MessageData* messageData)
	{
		BYTE* baseCommandPointer = messageData->composedData;
		BYTE* dataPtr = messageData->composedData;

		// preamble
		*(dataPtr++) = 0x55;
		*(dataPtr++) = 0xFF;
		*(dataPtr++) = 0x00;

		UINT dataSum = 0;

		// command, reply, and repeat both
		*(dataPtr++) = messageData->commandByte;
		*(dataPtr++) = ~messageData->commandByte;

		dataSum += messageData->commandByte;

		for (UINT i = 0; i < messageData->paramsLength; i++)
		{
			BYTE paramByte = messageData->params[i];
			*(dataPtr++) = paramByte;
			*(dataPtr++) = ~paramByte;

			dataSum += paramByte;
		}

		// checksum for the RCX is just the data sum, so...
		*(dataPtr++) = dataSum;
		*(dataPtr++) = ~dataSum;

		messageData->composedLength = (dataPtr - baseCommandPointer);
	}

	//VOID BuildCmd_PlaySystemSound(const Command* command, BYTE sound)
	//{
	//	//command()
	//}
}