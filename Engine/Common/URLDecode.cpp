/**
 * URLDecode.cpp - Contains url string denoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Common/URLEncode.h>

namespace Common
{

std::string URLDecode(std::string_view input) noexcept
{
	std::string output;
	output.reserve(input.size());

	bool inQuery = false;
	auto it = input.begin();
	auto end = input.end();
	while (it != end)
	{
		char c = *it++;
		if (c == '?')
		{
			inQuery = true;
		}
		// spaces may be encoded as plus signs in the query
		if (inQuery && c == '+')
		{
			c = ' ';
		}
		else if (c == '%')
		{
			if (it == end) // No hex digit following percent sign
			{
				return "";
			}
			char hi = *it++;
			if (it == end) // two hex digits must follow percent sign
			{
				return "";
			}
			char lo = *it++;
			if (hi >= '0' && hi <= '9')
			{
				c = hi - '0';
			}
			else if (hi >= 'A' && hi <= 'F')
			{
				c = hi - 'A' + 10;
			}
			else if (hi >= 'a' && hi <= 'f')
			{
				c = hi - 'a' + 10;
			}
			else // not a hex digit
			{
				return "";
			}
			c *= 16;
			if (lo >= '0' && lo <= '9')
			{
				c += lo - '0';
			}
			else if (lo >= 'A' && lo <= 'F')
			{
				c += lo - 'A' + 10;
			}
			else if (lo >= 'a' && lo <= 'f')
			{
				c += lo - 'a' + 10;
			}
			else // not a hex digit
			{
				return "";
			}
		}
		output += c;
	}

	return output;
}

}
