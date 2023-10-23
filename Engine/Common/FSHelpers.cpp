/**
 * FSHelpers.cpp - Contains helpers for file system manipulation impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Common/FSHelpers.h>

#include <iostream>
#include <fstream>
#include <stdio.h>

#include <boost/nowide/convert.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Common
{

#ifdef _WIN32
bool CheckAllowFileWrite(const std::string &path)
{
	bool yes = false;

	DWORD length = 0;
	if (!::GetFileSecurity(boost::nowide::widen(path).c_str(), OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
		| DACL_SECURITY_INFORMATION, NULL, NULL, &length) &&
		ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
	{
		PSECURITY_DESCRIPTOR security = static_cast<PSECURITY_DESCRIPTOR>(::malloc(length));
		if (security && ::GetFileSecurity(boost::nowide::widen(path).c_str(), OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
			| DACL_SECURITY_INFORMATION, security, length, &length))
		{
			HANDLE hToken = NULL;
			if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY |
				TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken))
			{
				HANDLE hImpersonatedToken = NULL;
				if (::DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken))
				{
					GENERIC_MAPPING mapping = { 0xFFFFFFFF };
					PRIVILEGE_SET privileges = { 0 };
					DWORD grantedAccess = 0, privilegesLength = sizeof(privileges);
					BOOL result = FALSE;

					mapping.GenericRead = FILE_GENERIC_READ;
					mapping.GenericWrite = FILE_GENERIC_WRITE;
					mapping.GenericExecute = FILE_GENERIC_EXECUTE;
					mapping.GenericAll = FILE_ALL_ACCESS;

					DWORD genericAccessRights = FILE_GENERIC_WRITE;

					::MapGenericMask(&genericAccessRights, &mapping);
					if (::AccessCheck(security, hImpersonatedToken, genericAccessRights,
						&mapping, &privileges, &privilegesLength, &grantedAccess, &result))
					{
						yes = (result == TRUE);
					}
					::CloseHandle(hImpersonatedToken);
				}
				::CloseHandle(hToken);
			}
		}
		free(security);
	}

	return yes;
}
#else
bool CheckAllowFileWrite(const std::string &path)
{
	return false;
}
#endif

std::string DirNameOf(const std::string& fileNameWithPath)
{
	size_t pos = fileNameWithPath.find_last_of("\\/");
	return (std::string::npos == pos)
		? ""
		: fileNameWithPath.substr(0, pos);
}

std::string FileNameOf(const std::string& fileNameWithPath)
{
	size_t pos = fileNameWithPath.find_last_of("\\/");
	if (std::string::npos != pos)
	{
		std::string tmp(fileNameWithPath);
		tmp.erase(0, pos + 1);
		return tmp;
	}
	return "";
}

}
