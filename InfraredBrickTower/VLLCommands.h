#pragma once

#ifndef VLLCOMMANDS_H
#define VLLCOMMANDS_H

#include <Windows.h>
#include "TowerController.h"

#define PACKET_LENGTH 25

/*

REVELATION

there is a pattern!

0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a
0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14
...
0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a
0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14
0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a
0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14
0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a
...
0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a
...
0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a
0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14
0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a
0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14
0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a
0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14
0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a
0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14
...
0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a
0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14
0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a
0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14
0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a
...
0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14
0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a
0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14
0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a
...
0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14
0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a

every 0x0b is followed by a 0x14,
and every 0x15 is followed by a 0x0a!

what does it mean?
no idea!

analysis time!

utilizing search and replace, I'll replace the "0x0b, 0x14"s with 1, and the "0x15, 0x0a"s with 0...

(i got the indices from the PDF in the Scout SDK zip linked here: https://pbrick.info/scout-software-developers-kit/index.html)

0	1110000000
1	1100000001
...
4	0100000100
5	0010000101
6	0000000110
7	1110000111
8	1010001000
...
10	0110001010
...
16	0100010000
17	0010010001
18	0000010010
19	1110010011
20	1010010100
21	1000010101
22	0110010110
23	0100010111
24	0000011000
25	1110011001
26	1100011010
27	1010011011
28	0110011100
29	0100011101
30	0010011110
31	0000011111
32	1010100000
33	1000100001
34	0110100010

bingo!

so I know that the messages are:
	1 start bit
	3 checksum bits
	7 data bits
	1 stop bit (light off)
(source: https://www.eurobricks.com/forum/index.php?/forums/topic/188584-mulpi-a-multiple-lego-remote-protocol-interface/&tab=comments#comment-3475050)

i'll look at a small chunk:

    checksum
	 |	data
	[-][-----] 
4	0100000100
5	0010000101
6	0000000110
7	1110000111
8	1010001000

i know from the aforementioned Scout SDK document that there are 128 VLL codes (0-127), which lines up with knowing that there are 7 data bits since 127 in binary is 1111111.
the checksum is calculated using the below equation (source: https://www.elecbrick.com/vll/)

	checksum(n) = 7 - ((n + (n >> 2) + (n >> 4)) & 7)

let's confirm this.
the checksum on the data above for index 4 is 010, so:

	checksum(4) = 7 - ((4 + (4 >> 2) + (4 >> 4)) & 7) = 2

in binary, 2 is 10. mission accomplished!

so, i've got the checksum and the data. what about the start bit and the stop bit?

*/



// I grabbed these by monitoring USB traffic through WireShark from the VLL Sample program provided in the Mindstorms SDK (download here: https://www.philohome.com/sdk25/sdk25.htm)
// (I ran this on a Windows XP VM)
// 
						//	     start                checksum                                                  data                                                   stop
						//	  [--------]  [--------------------------------]  [--------------------------------------------------------------------------------]  [--------------]
#define FORWARD_IMMEDIATE	{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BACKWARD_IMMEDIATE	{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }

#define BEEP_1IMMEDIATE		{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BEEP_2IMMEDIATE		{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BEEP_3IMMEDIATE		{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BEEP_4IMMEDIATE		{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BEEP_5IMMEDIATE		{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }

#define STOP				{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }

#define FORWARD_HALF		{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define FORWARD_ONE			{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define FORWARD_TWO			{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define FORWARD_FIVE		{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BACKWARD_HALF		{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BACKWARD_ONE		{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BACKWARD_TWO		{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BACKWARD_FIVE		{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }

#define BEEP_1				{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BEEP_2				{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BEEP_3				{ 0x51, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define BEEP_4				{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define BEEP_5				{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }

#define WAIT_LIGHT			{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define SEEK_LIGHT			{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }
#define CODE				{ 0x51, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define KEEP_ALIVE			{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }

#define RUN					{ 0x51, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x0b, 0x00 }
#define DELETE_PROGRAM		{ 0x51, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x0b, 0x14, 0x15, 0x0a, 0x0b, 0x0b, 0x00 }

#define GenerateVLLFunction(commandName, commandData) \
inline BOOL VLL_##commandName##(Tower::RequestData* towerData) \
{ \
	BYTE bytes[] = commandData; \
	ULONG lengthWritten; \
	Tower::WriteData(bytes, PACKET_LENGTH, lengthWritten, towerData); \
	return lengthWritten == PACKET_LENGTH; \
}

GenerateVLLFunction(Beep1Immediate, BEEP_1IMMEDIATE)
GenerateVLLFunction(Beep2Immediate, BEEP_2IMMEDIATE)
GenerateVLLFunction(Beep3Immediate, BEEP_3IMMEDIATE)
GenerateVLLFunction(Beep4Immediate, BEEP_4IMMEDIATE)
GenerateVLLFunction(Beep5Immediate, BEEP_5IMMEDIATE)
GenerateVLLFunction(ForwardImmediate, FORWARD_IMMEDIATE)
GenerateVLLFunction(BackwardImmediate, BACKWARD_IMMEDIATE)
GenerateVLLFunction(Stop, STOP)
GenerateVLLFunction(Run, RUN)
GenerateVLLFunction(Delete, DELETE_PROGRAM)
GenerateVLLFunction(Beep1, BEEP_1)
GenerateVLLFunction(Beep2, BEEP_2)
GenerateVLLFunction(Beep3, BEEP_3)
GenerateVLLFunction(Beep4, BEEP_4)
GenerateVLLFunction(Beep5, BEEP_5)
GenerateVLLFunction(ForwardHalf, FORWARD_HALF)
GenerateVLLFunction(ForwardOne, FORWARD_ONE)
GenerateVLLFunction(ForwardTwo, FORWARD_TWO)
GenerateVLLFunction(ForwardFive, FORWARD_FIVE)
GenerateVLLFunction(BackwardHalf, BACKWARD_HALF)
GenerateVLLFunction(BackwardOne, BACKWARD_ONE)
GenerateVLLFunction(BackwardTwo, BACKWARD_TWO)
GenerateVLLFunction(BackwardFive, BACKWARD_FIVE)
GenerateVLLFunction(WaitLight, WAIT_LIGHT)
GenerateVLLFunction(SeekLight, SEEK_LIGHT)
GenerateVLLFunction(Code, CODE)
GenerateVLLFunction(KeepAlive, KEEP_ALIVE)

#endif VLLCOMMANDS_H
