#pragma once

#include <PTRMI/Mutex.h>

namespace PTRMI
{


class Thread
{

public:

	typedef	LPTHREAD_START_ROUTINE	Function;

	typedef DWORD					ThreadID;

	Mutex							threadLock;

protected:

	HANDLE				thread;
	ThreadID			threadID;

	bool				stopRequest;
	bool				suspended;

protected:

	void				setStop					(bool initStop);
	void				setSuspended			(bool initSuspended);

	Status				lock					(void);
	Status				unlock					(void);

public:

						Thread					(void);
					   ~Thread					(void);

	ThreadID			getThreadID				(void) const;

	Status				start					(LPTHREAD_START_ROUTINE function, void *params);
	Status				stop					(void);

	Status				suspend					(void);
	Status				resume					(void);

	Status				join					(unsigned long timeOut = INFINITE);

	Status				exit					(void);
	Status				terminate				(void);

	bool				getStop					(void) const;
	bool				getSuspended			(void) const;



};



inline Thread::Thread(void)
{
	setStop(false);
	setSuspended(false);
}


inline Thread::~Thread(void)
{
	terminate();
}

inline Status Thread::exit(void)
{
	ExitThread(0);
}

inline Status Thread::terminate(void)
{
	if(TerminateThread(thread, 0))
		return Status(Status::Status_OK);

	return Status(Status::Status_Error_Failed_To_Terminate_Thread);
}

inline Status Thread::start(Function function, void *params)
{
	Status	status;

	lock();

	if((thread = CreateThread(NULL, 0, function, params, 0, &threadID)) == NULL)
	{
		status = Status(Status::Status_Error_Failed_To_Create_Thread);
	}

	unlock();

	return status;
}

inline PTRMI::Status Thread::stop(void)
{
	lock();
															// Request thread termination
	setStop(true);

	unlock();

	return Status();
}

inline void Thread::setStop(bool initStop)
{
	lock();

	stopRequest = initStop;

	unlock();
}

inline bool Thread::getStop(void) const
{
	return stopRequest;
}

inline PTRMI::Status Thread::suspend(void)
{
	Status	status;

	lock();

	if(getSuspended() == false)
	{
		SuspendThread(thread);
		setSuspended(true);
	}

	unlock();

	return status;
}

inline PTRMI::Status Thread::resume(void)
{
	Status	status;

	lock();

	if(getSuspended())
	{
		if(ResumeThread(thread) == -1)
		{
			return Status(Status::Status_Error_Thread_Resume);
		}

		setSuspended(false);
	}

	unlock();

	return Status();

}

inline PTRMI::Status Thread::join(unsigned long timeOut)
{
															// Don't allow thread to wait for itself
	if(GetCurrentThreadId() == GetThreadId(thread))
		return Status();

	if(WaitForSingleObject(thread, timeOut) == WAIT_OBJECT_0)
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Terminate_Thread);
}

inline PTRMI::Status Thread::lock(void)
{
	threadLock.wait();

	return Status();
}

inline PTRMI::Status Thread::unlock(void)
{
	threadLock.release();

	return Status();
}

inline void Thread::setSuspended(bool initSuspended)
{
	lock();

	suspended = initSuspended;

	unlock();
}

inline bool Thread::getSuspended(void) const
{
	return suspended;
}

} // End PTRMI namespace
