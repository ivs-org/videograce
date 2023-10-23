/**
 * HTMLSymbolsScreener.cpp - Contains HTML symbols screener impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <string>

namespace Common
{

namespace HTML
{

std::string Screen(const std::string &value)
{
	std::string buffer;
	buffer.reserve(value.size());
	for (size_t pos = 0; pos != value.size(); ++pos)
	{
		switch (value[pos])
		{
			case '&':  buffer.append("&amp;");       break;
			case '\"': buffer.append("&quot;");      break;
			case '\'': buffer.append("&apos;");      break;
			case '<':  buffer.append("&lt;");        break;
			case '>':  buffer.append("&gt;");        break;
			default:   buffer.append(&value[pos], 1); break;
		}
	}
	
	return buffer;
}

}

}
