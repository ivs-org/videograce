/**
 * rw_lock.h : Defines the read/write lock primitive
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/mt
 */

#pragma once

#ifndef _WIN32
#include <pthread.h>
#else
#include <string>
#include <windows.h>
#include <Common/WindowsVersion.h>
#endif

#include <assert.h>
#include <memory>

namespace mt
{

#ifndef _WIN32
class rw_lock
{
public:
	rw_lock()
		: rw_lock_(PTHREAD_RWLOCK_INITIALIZER)
	{
		pthread_rwlock_init(&rw_lock_, NULL);
	}

	~rw_lock()
	{
		pthread_rwlock_destroy(&rw_lock_);
	}
	
	void read_lock()
	{
		pthread_rwlock_rdlock(&rw_lock_);
	}
	void read_unlock()
	{
		pthread_rwlock_unlock(&rw_lock_);
	}

	void write_lock()
	{
		pthread_rwlock_wrlock(&rw_lock_);
	}
	void write_unlock()
	{
		pthread_rwlock_unlock(&rw_lock_);
	}

private:
	pthread_rwlock_t rw_lock_;
};
#else

typedef struct _RTL_RWLOCK
{
	RTL_CRITICAL_SECTION rtlCS;

	HANDLE hSharedReleaseSemaphore;
	UINT   uSharedWaiters;

	HANDLE hExclusiveReleaseSemaphore;
	UINT   uExclusiveWaiters;

	INT    iNumberActive;
	HANDLE hOwningThreadId;
	DWORD  dwTimeoutBoost;
	PVOID  pDebugInfo;
} RTL_RWLOCK, *LPRTL_RWLOCK;

typedef void(__stdcall *RtlManagePtr)(LPRTL_RWLOCK);
typedef BYTE(__stdcall *RtlOperatePtr)(LPRTL_RWLOCK, BYTE);

class rw_lock_xp // Implementation for Windows XP
{
public:
	rw_lock_xp()
		: hGetProcIDDLL(NULL),
		RtlDeleteResource_func(NULL),
		RtlReleaseResource_func(NULL),
		RtlAcquireResourceExclusive_func(NULL),
		RtlAcquireResourceShared_func(NULL),
		rtlRWLock()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetSystemDirectory(path, sizeof(path) / sizeof(path[0]));
		std::wstring dllPath = std::wstring(path) + L"\\ntdll.dll";
		HINSTANCE hGetProcIDDLL = LoadLibrary(dllPath.c_str());
		if (hGetProcIDDLL)
		{
			RtlDeleteResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlDeleteResource");
			if (!RtlDeleteResource_func)
			{
				return;
			}
			RtlReleaseResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlReleaseResource");
			if (!RtlReleaseResource_func)
			{
				return;
			}
			RtlAcquireResourceExclusive_func = (RtlOperatePtr)GetProcAddress(hGetProcIDDLL, "RtlAcquireResourceExclusive");
			if (!RtlAcquireResourceExclusive_func)
			{
				return;
			}
			RtlAcquireResourceShared_func = (RtlOperatePtr)GetProcAddress(hGetProcIDDLL, "RtlAcquireResourceShared");
			if (!RtlAcquireResourceShared_func)
			{
				return;
			}

			RtlManagePtr RtlInitializeResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlInitializeResource");
			if (RtlInitializeResource_func)
			{
				RtlInitializeResource_func(&rtlRWLock);
			}
		}
	}
	
	~rw_lock_xp()
	{
		if (RtlDeleteResource_func)
		{
			RtlDeleteResource_func(&rtlRWLock);
		}
		if (hGetProcIDDLL)
		{
			FreeLibrary(hGetProcIDDLL);
		}
	}
	
	void read_lock()
	{
		if (RtlAcquireResourceShared_func)
		{
			RtlAcquireResourceShared_func(&rtlRWLock, TRUE);
		}
	}
	void read_unlock()
	{
		if (RtlReleaseResource_func)
		{
			RtlReleaseResource_func(&rtlRWLock);
		}
	}
	
	void write_lock()
    {
		if (RtlAcquireResourceExclusive_func)
		{
			RtlAcquireResourceExclusive_func(&rtlRWLock, TRUE);
		}
	}
	
	void write_unlock()
	{
		if (RtlReleaseResource_func)
		{
			RtlReleaseResource_func(&rtlRWLock);
		}
	}

private:
	HINSTANCE hGetProcIDDLL;
	RtlManagePtr RtlDeleteResource_func;
	RtlManagePtr RtlReleaseResource_func;
	RtlOperatePtr RtlAcquireResourceExclusive_func;
	RtlOperatePtr RtlAcquireResourceShared_func;

	RTL_RWLOCK rtlRWLock;
};

typedef void(__stdcall *SRWLock_fptr)(PSRWLOCK);

class rw_lock_srw // For Windows Vista+ based on Slim rw_lock
{
public:
	rw_lock_srw()
		: hGetProcIDDLL(NULL), 
		AcquireSRWLockShared_func(NULL),
		ReleaseSRWLockShared_func(NULL),
		AcquireSRWLockExclusive_func(NULL),
		ReleaseSRWLockExclusive_func(NULL),
		srwLock()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetSystemDirectory(path, sizeof(path));
		std::wstring dllPath = std::wstring(path) + L"\\kernel32.dll";
		HINSTANCE hGetProcIDDLL = LoadLibrary(dllPath.c_str());
		if (hGetProcIDDLL)
		{
			AcquireSRWLockShared_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "AcquireSRWLockShared");
			if (!AcquireSRWLockShared_func)
			{
				return;
			}
			ReleaseSRWLockShared_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "ReleaseSRWLockShared");
			if (!ReleaseSRWLockShared_func)
			{
				return;
			}
			AcquireSRWLockExclusive_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "AcquireSRWLockExclusive");
			if (!AcquireSRWLockExclusive_func)
			{
				return;
			}
			ReleaseSRWLockExclusive_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "ReleaseSRWLockExclusive");
			if (!ReleaseSRWLockExclusive_func)
			{
				return;
			}

			SRWLock_fptr InitializeSRWLock_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "InitializeSRWLock");
			if (InitializeSRWLock_func)
			{
				InitializeSRWLock_func(&srwLock);
			}
		}
	}

	~rw_lock_srw()
	{
		if (hGetProcIDDLL)
		{
			FreeLibrary(hGetProcIDDLL);
		}
	}
	
	void read_lock()
	{
		if (AcquireSRWLockShared_func)
		{
			AcquireSRWLockShared_func(&srwLock);
		}
	}
	void read_unlock()
	{
		if (ReleaseSRWLockShared_func)
		{
			ReleaseSRWLockShared_func(&srwLock);
		}
	}

	void write_lock()
	{
		if (AcquireSRWLockExclusive_func)
		{
			AcquireSRWLockExclusive_func(&srwLock);
		}
	}
	void write_unlock()
	{
		if (ReleaseSRWLockExclusive_func)
		{
			ReleaseSRWLockExclusive_func(&srwLock);
		}
	}

private:
	HINSTANCE hGetProcIDDLL;

	SRWLock_fptr AcquireSRWLockShared_func;
	SRWLock_fptr ReleaseSRWLockShared_func;
	SRWLock_fptr AcquireSRWLockExclusive_func;
	SRWLock_fptr ReleaseSRWLockExclusive_func;

	RTL_SRWLOCK srwLock;
};

class rw_lock // Wrapper
{
    static bool is_windows_vista_or_greater()
    {
        OSVERSIONINFOEX osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.dwMajorVersion = 6;

        DWORDLONG dwlConditionMask = 0;
        int op = VER_GREATER_EQUAL;

        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
        VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
        VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);
        VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, op);

        return VerifyVersionInfo(
            &osvi,
            VER_MAJORVERSION | VER_MINORVERSION |
            VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
            dwlConditionMask);
    }
public:
	rw_lock()
		: is_vista_plus(is_windows_vista_or_greater()), rw_lock_xp_(!is_vista_plus ? new rw_lock_xp() : nullptr), rw_lock_srw_(is_vista_plus ? new rw_lock_srw() : nullptr) {}

	void read_lock()
	{
		if (is_vista_plus)
		{
			rw_lock_srw_->read_lock();
		}
		else
		{
			rw_lock_xp_->read_lock();
		}
	}
	void read_unlock()
	{
		if (is_vista_plus)
		{
			rw_lock_srw_->read_unlock();
		}
		else
		{
			rw_lock_xp_->read_unlock();
		}
	}

	void write_lock()
	{
		if (is_vista_plus)
		{
			rw_lock_srw_->write_lock();
		}
		else
		{
			rw_lock_xp_->write_lock();
		}
	}
	void write_unlock()
	{
		if (is_vista_plus)
		{
			rw_lock_srw_->write_unlock();
		}
		else
		{
			rw_lock_xp_->write_unlock();
		}
	}

private:
	bool is_vista_plus;
	std::unique_ptr<rw_lock_xp> rw_lock_xp_;
	std::unique_ptr<rw_lock_srw> rw_lock_srw_;
};
#endif

class scoped_rw_lock
{
public:
	scoped_rw_lock(rw_lock *lc_, bool write_ = false)
		: lc(*lc_), write(write_), locked(false)
	{
		if (write)
		{
			lc.write_lock();
		}
		else
		{
			lc.read_lock();
		}
	}

	~scoped_rw_lock()
	{
		if (write)
		{
			lc.write_unlock();
		}
		else
		{
			lc.read_unlock();
		}
	}

	static void *operator new(size_t) = delete;
	static void operator delete(void *) = delete;
	scoped_rw_lock(const scoped_rw_lock&) = delete;
	void operator=(const scoped_rw_lock&) = delete;
private:
	rw_lock &lc;
	bool write, locked;
};

}
