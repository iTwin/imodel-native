
#pragma once

#include <PTRMI/Mutex.h>
#include <PTRMI/Status.h>

#include <assert.h>

namespace PTRMI
{

	const unsigned long REFERENCE_COUNTER_MUTEX_TIMEOUT	= 1000 * 10;


	template<typename T, unsigned int discardDeletionValue = 0, bool warnings = true> class ReferenceCounter
	{

	public:

		typedef	T				Counter;
		typedef unsigned long	TimeOut;

		enum DeleteAction
		{
			ActionDelete,
			ActionSignal
		};

	protected:

		Counter					referenceCounter;
		Mutex					referenceCounterMutex;
		TimeOut					mutexTimeOut;
		bool					referenceCounterDeleted;

	public:

		ReferenceCounter(TimeOut timeOut = REFERENCE_COUNTER_MUTEX_TIMEOUT)
		{
			setReferenceCounterMutexTimeOut(timeOut);

			clearReferenceCounter();

			setReferenceCounterDeleted(false);
		}

		virtual ~ReferenceCounter(void)
		{
			if(warnings)
			{
				assert(isReferenceCounterZero());
			}
		}

		void setReferenceCounterDeleted(bool deleted)
		{
			referenceCounterDeleted = deleted;
		}

		bool getReferenceCounterDeleted(void)
		{
			return referenceCounterDeleted;
		}

		Counter getDiscardDeletionValue(void)
		{
			return discardDeletionValue;
		}


		Status lockReferenceCounter(void)
		{
			if(referenceCounterMutex.wait(getReferenceCounterMutexTimeOut()))
			{
				return Status();
			}

			return Status(Status::Status_Error_Failed_To_Lock_Reference_Counter);
		}


		Status releaseReferenceCounter(void)
		{
			if(referenceCounterMutex.release())
			{
				return Status();
			}

			return Status(Status::Status_Error_Failed_To_Release_Reference_Counter);
		}


		Status discard(bool &deleted, DeleteAction action = ReferenceCounter::ActionDelete)
		{
															// Default result to true to minimize code running after 'delete this'
			deleted = true;
															// Lock for atomic decision
			if(lockReferenceCounter().isOK())
			{
															// Decrement the reference counter
				decrementReferenceCounter();
															// If reference count is the discard deletion value (usually zero)
				if(getReferenceCounter() == getDiscardDeletionValue())
				{
					if(action == ActionDelete)
					{
															// Delete this object now
						delete this;
					}
					else
					if(action == ActionSignal)
					{
															// Signal that this object should be deleted in future when suitable
						setReferenceCounterDeleted(true);
					}
															// Note: Reference Counter mutex will not exist after, so no need to release it
					return Status();
				}
															// Not deleted
				deleted = false;
															// Non zero, so release the mutex
				return releaseReferenceCounter();
			}
															// Not deleted
			deleted = false;
															// Return failed to lock
			return Status(Status::Status_Error_Failed_To_Lock_Reference_Counter);
		}

		void setReferenceCounterMutexTimeOut(TimeOut timeOut)
		{
			mutexTimeOut = timeOut;
		}

		TimeOut getReferenceCounterMutexTimeOut(void)
		{
			return mutexTimeOut;
		}

		void setReferenceCounter(Counter value)
		{
			referenceCounter = value;
		}

		Counter getReferenceCounter(void)
		{
			return referenceCounter;
		}

		void clearReferenceCounter(void)
		{
			setReferenceCounter(0);
		}

		bool isReferenceCounterValid(void)
		{
			return (getReferenceCounter() >= 0);
		}

		bool isReferenceCounterZero(void)
		{
			return (getReferenceCounter() == 0);
		}

		Status addReferenceCounter(Counter value)
		{
			if(lockReferenceCounter().isOK())
			{
				setReferenceCounter(getReferenceCounter() + value);

				if(isReferenceCounterValid() == false)
				{
					setReferenceCounter(0);

					if(warnings)
					{
						assert(false);
					}
				}

				return releaseReferenceCounter();
			}

			return Status(Status::Status_Error_Failed_To_Lock_Reference_Counter);
		}

		Status incrementReferenceCounter(void)
		{
			return addReferenceCounter(1);
		}

		Status decrementReferenceCounter(void)
		{
			return addReferenceCounter(-1);
		}
	};


} // End PTRMI
