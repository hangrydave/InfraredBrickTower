#include "Utilities.h"

namespace Utilities
{
    BYTE ReadByteFromTwoHexChars(const char a, const char b)
    {
		BYTE result = 1;

		if (a >= '0' && a <= '9')
		{
			result = (a - 48) * 16;
		}
		else if (a >= 'A' && a <= 'F')
		{
			result = 160 + ((a - 65) * 16);
		}

		if (b >= '0' && b <= '9')
		{
			result += (b - 48);
		}
		else if (b >= 'A' && b <= 'F')
		{
			result += (b - 65) + 10;
		}

		return result;
	}
}