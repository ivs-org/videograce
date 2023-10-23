/**
 * PhysicalMAC.cpp - Contains PhysicalMAC getter impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Common/ToDigit.h>

#include <string>

namespace Common
{

uint32_t toDigit(const char* inStr)
{
	std::string str = inStr;
	std::string outStr;
	for (uint32_t i = 0; i != str.size(); ++i)
	{
		char c = str[i];
		switch (c)
		{
		case ' ': break;
		case '0': outStr += "0"; break;
		case '1': outStr += "1"; break;
		case '2': outStr += "2"; break;
		case '3': outStr += "3"; break;
		case '4': outStr += "4"; break;
		case '5': outStr += "5"; break;
		case '6': outStr += "6"; break;
		case '7': outStr += "7"; break;
		case '8': outStr += "8"; break;
		case '9': outStr += "9"; break;
		case 'A': outStr += "10"; break;
		case 'B': outStr += "11"; break;
		case 'C': outStr += "12"; break;
		case 'D': outStr += "13"; break;
		case 'E': outStr += "14"; break;
		case 'F': outStr += "15"; break;
		case 'G': outStr += "16"; break;
		case 'H': outStr += "17"; break;
		case 'I': outStr += "18"; break;
		case 'J': outStr += "19"; break;
		case 'K': outStr += "20"; break;
		case 'L': outStr += "21"; break;
		case 'M': outStr += "22"; break;
		case 'N': outStr += "23"; break;
		case 'O': outStr += "24"; break;
		case 'P': outStr += "25"; break;
		case 'Q': outStr += "26"; break;
		case 'R': outStr += "27"; break;
		case 'S': outStr += "28"; break;
		case 'T': outStr += "29"; break;
		case 'U': outStr += "30"; break;
		case 'V': outStr += "31"; break;
		case 'W': outStr += "32"; break;
		case 'X': outStr += "33"; break;
		case 'Y': outStr += "34"; break;
		case 'Z': outStr += "35"; break;
		}
	}

	if (outStr.size() > 9)
	{
		outStr.resize(9);
	}

	return atoi(outStr.c_str());
}

}
