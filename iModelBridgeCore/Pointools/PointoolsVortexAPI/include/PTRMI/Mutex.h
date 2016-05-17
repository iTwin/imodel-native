#pragma once

#include <PTRMI/Status.h>

namespace PTRMI
{

class Mutex
{
protected:

#if defined (ANDROID)
    // Android doesn't provide std::timed_mutex 
    // I'm reluctant to use BeSharedMutex on all platforms since it is an homemade implementation and no one seems to use it.
    BeSharedMutex mutex;
#else
    std::timed_mutex mutex;
#endif

public:

	Mutex(void){}

	~Mutex(void){}

    bool wait()
        {
        mutex.lock();
        return true;
        }

    bool wait(unsigned long timeMs)
        {
        return mutex.try_lock_for(std::chrono::milliseconds(timeMs));
        }

    bool release(void)
        {
        mutex.unlock();
        return true;
        }
};


class MutexScope
{

protected:

	Mutex *		mutex;

	bool		locked;

public:

    MutexScope(Mutex &initMutex)
        {
        setLocked(false);
        mutex = &initMutex;
        setLocked(mutex->wait());
        }


	MutexScope(Mutex &initMutex, unsigned long timeOut)
	{
		setLocked(false);
															// Set external mutex
		mutex = &initMutex;
															// Wait on mutex
		setLocked(mutex->wait(timeOut));
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
