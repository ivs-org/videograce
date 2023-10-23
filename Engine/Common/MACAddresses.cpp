/**
 * MacAddresses.cpp - Contains Ethernet MAC getter impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2020
 */

#include <Common/MACAddresses.h>
#include <Common/Common.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#ifdef __APPLE__
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#endif
#else
#include <windows.h>
#include <Iphlpapi.h>
#include <atlbase.h>
#endif

#include <boost/nowide/convert.hpp>

namespace Common
{
#ifndef _WIN32
void GetMacAddresses(std::vector<std::string> &vMacAddresses)
{
	vMacAddresses.clear();

	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
	{
		return;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
	{
		return;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it)
	{
#ifdef __APPLE__
		ifaddrs* iflist;
		if (getifaddrs(&iflist) == 0)
		{
			for (ifaddrs* cur = iflist; cur; cur = cur->ifa_next) {
				if ((cur->ifa_addr->sa_family == AF_LINK) &&
					(strcmp(cur->ifa_name, it->ifr_name) == 0) &&
					cur->ifa_addr)
				{
					sockaddr_dl* sdl = (sockaddr_dl*)cur->ifa_addr;

					char ret[25] = { 0 };

					const unsigned char* mac = (unsigned char*)LLADDR(sdl);
					sprintf(ret, "%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

					vMacAddresses.push_back(ret);

					break;
				}
			}

			freeifaddrs(iflist);
		}
#else
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
		{
			if (!(ifr.ifr_flags & IFF_LOOPBACK))
			{
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
				{
					char ret[25] = { 0 };

					const unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
					sprintf(ret, "%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

					vMacAddresses.push_back(ret);
				}
			}
		}
		else
		{
			return;
		}
#endif
	}
}
#else

bool IsPhysical(const char *adapterName)
{
	ATL::CRegKey key;

	std::wstring value;

	std::wstring section = L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" + boost::nowide::widen(adapterName) + L"\\Connection";

	if (key.Create(HKEY_LOCAL_MACHINE, section.c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		DWORD dwcbNeeded = 0;
		if (key.QueryStringValue(L"PnpInstanceID", (LPTSTR)NULL, &dwcbNeeded) == ERROR_SUCCESS)
		{
			if (dwcbNeeded > 2048)
			{
				return false;
			}

			wchar_t *val = (wchar_t*)calloc(dwcbNeeded, sizeof(wchar_t));

			if (key.QueryStringValue(L"PnpInstanceID", val, &dwcbNeeded) == ERROR_SUCCESS)
			{
				value.append(val, dwcbNeeded - 1);
			}

			free(val);
		}
	}

	key.Close();

	return value.find(L"PCI") != std::string::npos;
}

void GetMacAddresses(std::vector<std::string> &vMacAddresses)
{
	vMacAddresses.clear();

	IP_ADAPTER_INFO AdapterInfo[32];       // Allocate information for up to 32 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
		AdapterInfo,                       // [out] buffer to receive data
		&dwBufLen);                        // [in] size of receive data buffer

		//No network card? Other error?
	if (dwStatus != ERROR_SUCCESS)
		return;

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	char szBuffer[512];
	while (pAdapterInfo)
	{
		if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET && IsPhysical(pAdapterInfo->AdapterName))
		{
			sprintf_s(szBuffer, sizeof(szBuffer), "%02X:%02X:%02X:%02X:%02X:%02X"
				, pAdapterInfo->Address[0]
				, pAdapterInfo->Address[1]
				, pAdapterInfo->Address[2]
				, pAdapterInfo->Address[3]
				, pAdapterInfo->Address[4]
				, pAdapterInfo->Address[5]
			);
			vMacAddresses.push_back(szBuffer);
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
}
#endif

}
