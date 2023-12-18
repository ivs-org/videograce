/**
 * URI.h - Contains URI praser impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Transport/URI/URI.h>

#include <algorithm>

namespace Transport
{

void ParseURI(std::string_view url, std::string &proto, std::string &host, std::string &port)
{
	proto.clear();
	host.clear();
	port.clear();

	bool protoComlete = false;
	bool hostComlete = false, hostReady = false;
	int pos = 0, lastColonPos = -1;

	for (const auto c : url)
	{
		if (!protoComlete)
		{
			if (c != ':')
			{
				proto += c;
			}
			else
			{
				protoComlete = true;
				continue;
			}
		}

		if (protoComlete && !hostComlete)
		{
			if (!hostReady && c != '/')
			{
				hostReady = true;
			}

			if (hostReady)
			{
				host += c;

				if (c == ':')
				{
					lastColonPos = pos;
				}
			}

			++pos;
		}
	}

	if (lastColonPos > 1)
	{
		port = host.substr(lastColonPos - 1, host.size() - lastColonPos + 1);

		if (std::all_of(port.begin(), port.end(), ::isdigit))
		{
			host.erase(lastColonPos - 2);
		}
		else
		{
			port.erase();
		}
	}
}

}
