#pragma once

#include <PTRMI/Status.h>

namespace PTRMI
{

class Mutex
{
protected:

	HANDLE		mutex;

public:

	Mutex(void)
	{
		mutex = NULL;
	}

	~Mutex(void)
	{
		if(mutex)
		{
			CloseHandle(mutex);
		}
	}

	bool wait(unsigned long timeout = INFINITE)
	{
		DWORD	result;

		if(mutex == NULL)
		{
			if((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
			{
				Status status(Status::Status_Error_Failed_To_Create_Mutex);
				return false;
			}
		}

		result = WaitForSingleObject(mutex, timeout);

		switch(result)
		{
		case WAIT_OBJECT_0:

			return true;

		case WAIT_TIMEOUT:

			wchar_t m[256];
			swprintf(m, L"%d", timeout);
			Status::log(L"Warning: PTRMI::Mutex::wait() timed out. Timeout (ms) : ", m);
			break;

		case WAIT_ABANDONED:

			Status::log(L"Warning: PTRMI::Mutex::wait() abandoned", L"");
			break;

		default: ;

		}

		return false;
	}

	bool release(void)
	{
		if(mutex)
		{
			if(ReleaseMutex(mutex) == TRUE)
			{
				return true;
			}

			Status::log(L"Warning: PTRMI::Mutex::release() failed to release", L"");

			return false;
		}

		Status::log(L"Warning: PTRMI::Mutex::release() mutex not defined", L"");

		return false;
	}
};


class MutexScope
{

protected:

	Mutex *		mutex;

	Mutex		mutexInternal;

	bool		locked;

public:

	MutexScope(unsigned long timeOut = INFINITE)
	{
		setLocked(false);
															// Set internal mutex
		mutex = &mutexInternal;
															// Wait on mutex
		setLocked(mutex->wait(timeOut));
	}

	MutexScope(Mutex &initMutex, unsigned long timeOut = INFINITE)
	{
		setLocked(false);
															// Set external mutex
		mutex = &initMutex;
															// Wait on mutex
		setLocked(mutex->wait(timeOut));
	}

	MutexScope(Mutex *initMutex, unsigned long timeOut = INFINITE)
	{
		setLocked(false);

		mutex = initMutex;

		setLocked(false);

		if(initMutex)
		{
			setLocked(mutex->wait(timeOut));
		}
	}

	~MutexScope(void)
	{
															// If mutex defined, release mutex
		if(mutex)
		{
			mutex->release();			
		}
	}

	void setLocked(bool lock)
	{
		locked = lock;
	}

	bool isLocked(void)
	{
															// If mutex is defined, return the real result
		if(mutex != NULL)
		{
			return locked;
		}
															// If not defined, return true to allow 
		return false;
	}



};



} // End PTRMI namespace
