#pragma once

#include <PTRMI/Mutex.h>
#include <PTRMI/ReferenceCounter.h>
#include <map>

namespace PTRMI
{

	template<typename Obj> class ObjectLockManagerLock : public PTRMI::ReferenceCounter<long>
	{

	protected:

		PTRMI::Mutex			mutex;

	public:

		ObjectLockManagerLock()
		{

		}

		bool wait(unsigned long timeout)
		{
			return mutex.wait(timeout);
		}

		bool release()
		{
			return mutex.release();
		}

	};

	template<typename Obj> class ObjectLockManager
	{

	public:

		typedef ObjectLockManagerLock<Obj>		ObjectLock;

	protected:

		typedef	std::map<Obj *, ObjectLock>		ObjectLockMap;

	protected:

		ObjectLockMap	objectLocks;

		Mutex			lockManagerMutex;

	protected:

		ObjectLock *getObjectLock(Obj *object)
		{
			ObjectLockMap::iterator		it;
															// Find object lock from object address
			if((it = objectLocks.find(object)) != objectLocks.end())
			{
				return &(it->second);
			}
															// Return not found
			return NULL;
		}

		Status deleteObjectLock(Obj *object)
		{
			ObjectLockMap::iterator		it;
															// Find object lock from object address
			if((it = objectLocks.find(object)) != objectLocks.end())
			{
															// Erase the object lock
				objectLocks.erase(it);
															// Return OK
				return Status();
			}
															// Return failed to find lock
			return Status(Status::Status_Error_Failed_To_Find_Object_Lock);
		}


		ObjectLock *getLockAndReferenceCount(Obj *object, int value, unsigned long timeOut = INFINITE, PTRMI::Mutex *externalMutex = NULL)
		{
															// Lock main mutex if specified
			MutexScope mutexExternal(externalMutex, timeOut);
			if(externalMutex && mutexExternal.isLocked() == false)
			{
				Status status(Status::Status_Warning_Failed_To_Lock_Object);
				return NULL;
			}
															// Lock this object manager mutex
			MutexScope mutexLockManager(lockManagerMutex, timeOut);
			if(mutexLockManager.isLocked() == false)
			{
				Status status(Status::Status_Warning_Failed_To_Lock_Object);

				return NULL;
			}

			ObjectLock *objectLock;
															// Get the object lock
			if((objectLock = getObjectLock(object)) != NULL)
			{
															// Alter reference counter as specified by given value (+ve increment or -ve decrement)
				objectLock->addReferenceCounter(value);
			}

			return objectLock;
		}
		

	public:

		Status newLock(Obj *object, unsigned long timeOut = INFINITE, PTRMI::Mutex *externalMutex = NULL, bool immediateLock = false)
		{
															// Lock main mutex if specified
			MutexScope mutexExternal(externalMutex, timeOut);
			if(externalMutex && mutexExternal.isLocked() == false)
			{
				return Status(Status::Status_Warning_Failed_To_Lock_Object);
			}

															// Mutex this lock manager
			MutexScope mutexLockManager(lockManagerMutex, timeOut);
			if(mutexLockManager.isLocked() == false)
			{
				return Status(Status::Status_Warning_Failed_To_Lock_Object);
			}

															// If lock for object already exists, return error
			if(getObjectLock(object) != NULL)
			{
				return Status(Status::Status_Error_Object_Lock_Exists);
			}
															// Otherwise, add new lock
			objectLocks[object] = ObjectLock();

															// If an immediate lock is requested
			if(immediateLock)
			{
				ObjectLock *objectLock;
															// Get the new lock
				if((objectLock = getObjectLock(object)) == NULL)
				{
					return Status(Status::Status_Warning_Failed_To_Lock_Object);
				}
															// Set reference count to 1
				objectLock->setReferenceCounter(1);
															// Obtain the lock's mutex. This should always be permitted immediately.
				if(objectLock->wait(timeOut) == false)
				{
					return Status(Status::Status_Warning_Failed_To_Lock_Object);
				}
			}
															// Return OK
			return Status();
		}


		Status lock(Obj *object, unsigned long timeOut = INFINITE, PTRMI::Mutex *externalMutex = NULL)
		{
			ObjectLock *objectLock;
															// If object not specified, return error
			if(object == NULL)
			{
				return Status(Status::Status_Error_Bad_Parameter);
			}
															// Note: Data structure mutexes are not locked in this scope to allow
															// the objectLock to wait() outside of those locks, protected from deletion by the lock's reference count

															// Increment lock reference counter to prevent deletion when main Mutex is released
															// addCounter() uses main Mutex
			if((objectLock = getLockAndReferenceCount(object, 1, timeOut, externalMutex)) == NULL)
			{
				return Status(Status::Status_Error_Failed_To_Find_Object_Lock);
			}
															// External and ObjectManager mutexes are now released so the next wait() on the object lock does not block them

															// Wait for lock
			if(objectLock->wait(timeOut))
			{
															// Lock granted, so return OK
				return Status();
			}

			if(timeOut > 0)
			{
															// Return failed to lock
				return Status(Status::Status_Warning_Failed_To_Lock_Object);
			}
															// Return failed to spin lock
			return Status(Status::Status_Warning_Failed_To_Spin_Lock_Object);
		}


		Status release(Obj *object, unsigned long timeOut = INFINITE, PTRMI::Mutex *externalMutex = NULL)
		{
			ObjectLock *objectLock;
															// If object not specified, return error
			if(object == NULL)
			{
				return Status(Status::Status_Error_Bad_Parameter);
			}
															// Decrement lock reference counter to prevent deletion when main Mutex is released
															// addCounter() uses main Mutex
			if((objectLock = getLockAndReferenceCount(object, -1, timeOut, externalMutex)) == NULL)
			{
				return Status(Status::Status_Error_Failed_To_Find_Object_Lock);
			}
															// Release the object lock's mutex
			if(objectLock->release() == false)
			{
				return Status(Status::Status_Error_Failed_To_Release_Object_Lock);
			}
															// Return OK
			return Status();
		}


		Status deleteLock(Obj *object, bool preLocked = false, unsigned long timeOut = INFINITE, PTRMI::Mutex *externalMutex = NULL, unsigned int numRetries = 60, unsigned long retrySleep = 1000)
		{
			Status			status;
			ObjectLock *	objectLock;
			unsigned int	t;

															// Retry deleting a number of times. This is necessary to handle other threads having the 'intention of locking, without having locked
															// This is reflected in the reference count for the object lock
			for(t = 0; t < numRetries; t++)
			{
															// Lock object lock to obtain queue based access
															// Note: Because lock() is blocking, it can not be called from within the mutexes
															// If already locked, don't lock again
				if(preLocked == false)
				{
					if((status = lock(object, timeOut, externalMutex)).isFailed())
					{
						return status;
					}
				}
															// Create scope to allow scoped mutexes to destruct before sleeping on retry
				{
															// Lock main mutex if specified
					MutexScope mutexExternal(externalMutex, timeOut);
					if(externalMutex && mutexExternal.isLocked() == false)
					{
						return Status(Status::Status_Warning_Failed_To_Lock_Object);
					}

															// Mutex this lock manager
					MutexScope mutexLockManager(lockManagerMutex, timeOut);
					if(mutexLockManager.isLocked() == false)
					{
						return Status(Status::Status_Warning_Failed_To_Lock_Object);
					}
				
															// Get the object lock
					if((objectLock = getObjectLock(object)) == NULL)
					{
						return Status(Status::Status_Warning_Failed_To_Delete_Object_Lock);
					}
															// If only this function has access to the object lock
															// and therefore no other lock() call has checked out the object lock
					if(objectLock->getReferenceCounter() == 1)
					{
															// Release the object lock
						if((status = release(object, timeOut, externalMutex)).isFailed())
						{
							return status;
						}
															// Delete the object lock entry
						return deleteObjectLock(object);
					}
				}
															// Release the object lock to allow other thread(s) to decrement reference counter
				if((status = release(object, timeOut, externalMutex)).isFailed())
				{
					return status;
				}
															// Sleep for a short period of time and lock again
                BeThreadUtilities::BeSleep(retrySleep);
			}
															// Release object lock
			if((status = release(object, timeOut, externalMutex)).isFailed())
			{
				return status;
			}

															// Return failed to delete object lock
			return Status(Status::Status_Warning_Failed_To_Delete_Object_Lock);
		}

	};


} // End PTRMI
