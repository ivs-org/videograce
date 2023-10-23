/**
 * URLEncode.cpp - Contains url string ecnoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Common/URLEncode.h>

namespace Common
{

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
	hex1 = c / 16;
	hex2 = c % 16;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string URLEncode(const std::string &input) noexcept
{
	const std::string ILLEGAL = "%<>{}|\\\"^`!*'()$,[]";

	std::string output;
	output.reserve(input.size());

	for (char c : input)
	{
		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			c == '-' || c == '_' ||
			c == '.' || c == '~')
		{
			output += c;
		}
		else if (c <= 0x20 || c >= 0x7F || ILLEGAL.find(c) != std::string::npos)
		{
			unsigned char d1, d2;
			hexchar(c, d1, d2);
			output += '%';
			output += d1;
			output += d2;
		}
		else
		{
			output += c;
		}
	}

	return output;
}

}
