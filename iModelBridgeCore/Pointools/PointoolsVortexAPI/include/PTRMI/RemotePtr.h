#pragma once

#include <assert.h>
#include <vector>

#include <PTRMI/Status.h>


namespace PTRMI
{

template<typename T> class ObjClientInfo {public: typedef void I; std::vector<void *> Set;};	// Unknown types resolve to void to induce compile time error

template<typename T> class ObjServerInfo {public: typedef void I; std::vector<void *> Set;};	// Unknown types resolve to void to induce compile time error


template<typename Obj> class RemotePtr
{

public:

	typedef typename ObjClientInfo<Obj>::I	ObjectClientInterface;


protected:

	ObjectClientInterface	*	objectClientInterface;

public:

	RemotePtr(void)
	{
		setObjectClientInterface(NULL);
	}

	RemotePtr(ObjectClientInterface *i)
	{
		setObjectClientInterface(i);
	}

	~RemotePtr(void)
	{

	}

	Status setStatus(Status initStatus)
	{
		if(getObjectClientInterface())
		{
			getObjectClientInterface()->setStatus(initStatus);

			return Status();
		}

		return Status(Status::Status_Error_Failed);
	}

	Status getStatus(void)
	{
		if(getObjectClientInterface())
		{
			return getObjectClientInterface()->getStatus();
		}

		return Status(Status::Status_Error_Failed);
	}

	void resetStatus(void)
	{
															// Reset status to recover after an error
		setStatus(Status::Status_OK);
	}

	void setObjectClientInterface(ObjectClientInterface *i)
	{
		objectClientInterface = i;
	}

	ObjectClientInterface *getObjectClientInterface(void)
	{
		return objectClientInterface;
	}

	ObjectClientInterface *operator->(void)
	{
		assert(getObjectClientInterface() != NULL);

		return getObjectClientInterface();
	}

	bool isValid(void)
	{
		return (getObjectClientInterface() != NULL);
	}

	void invalidate(void)
	{
		setObjectClientInterface(NULL);
	}
};



}
