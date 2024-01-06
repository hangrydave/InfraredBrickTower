#pragma once

#include "Types.h"

#include <string>

namespace Utilities
{
    BYTE ReadByteFromTwoHexChars(const char a, const char b);

	template <typename T>
	T ReadNumFromString(const std::string* s)
    {
        T result = 0;

		size_t s_len = s->length();
        for (int i = 0; i < s_len; i++)
        {
            char c = s->at(s_len - i - 1);
            short multiplier;
            if (i > 0)
                multiplier = 10 * i;
            else
                multiplier = 1;

            result += (c - 48) * multiplier;
        }

        return (result);
    }
}