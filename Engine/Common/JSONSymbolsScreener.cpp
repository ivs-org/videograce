/**
 * JSONSymbolsScreener.cpp - Contains JSON symbols screener impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#include <string>

namespace Common
{

namespace JSON
{

std::string Screen(const std::string &value)
{
	std::string buffer;
	buffer.reserve(value.size());
	for (size_t pos = 0; pos != value.size(); ++pos)
	{
		switch (value[pos])
		{
			case '\b': buffer.append("\\b");  break;
			case '\f': buffer.append("\\f");  break;
			case '\n': buffer.append("\\n");  break;
			case '\r': buffer.append("\\r");  break;
			case '\t': buffer.append("\\t");  break;
			case '\"': buffer.append("\\\""); break;
			case '\\': buffer.append("\\\\"); break;
			case '/': buffer.append("\\/"); break;
			case 0: buffer.append("\\u0000"); break;
			case 1: buffer.append("\\u0001"); break;
			case 2: buffer.append("\\u0002"); break;
			case 3: buffer.append("\\u0003"); break;
			case 4: buffer.append("\\u0004"); break;
			case 5: buffer.append("\\u0005"); break;
			case 6: buffer.append("\\u0006"); break;
			case 7: buffer.append("\\u0007"); break;
			case 11: buffer.append("\\u0011"); break;
			case 14: buffer.append("\\u0014"); break;
			case 15: buffer.append("\\u0015"); break;
			case 16: buffer.append("\\u0016"); break;
			case 17: buffer.append("\\u0017"); break;
			case 18: buffer.append("\\u0018"); break;
			case 19: buffer.append("\\u0019"); break;
			case 20: buffer.append("\\u0020"); break;
			case 21: buffer.append("\\u0021"); break;
			case 22: buffer.append("\\u0022"); break;
			case 23: buffer.append("\\u0023"); break;
			case 24: buffer.append("\\u0024"); break;
			case 25: buffer.append("\\u0025"); break;
			case 26: buffer.append("\\u0026"); break;
			case 27: buffer.append("\\u0027"); break;
			case 28: buffer.append("\\u0028"); break;
			case 29: buffer.append("\\u0029"); break;
			case 30: buffer.append("\\u0030"); break;
			case 31: buffer.append("\\u0031"); break;
			default:   buffer.append(&value[pos], 1); break;
		}
	}

	return buffer;
}

}

}
