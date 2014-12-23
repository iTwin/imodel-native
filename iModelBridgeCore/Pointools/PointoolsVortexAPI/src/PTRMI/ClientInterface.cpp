
#include <PTRMI/ClientInterface.h>

	
namespace PTRMI
{

ClientInterfaceBase::ClientInterfaceBase(void)
{
															// Initially not shared
	setSharedCounter(0);
															// Initially no Ext data
	setExtData(NULL);
}

ClientInterfaceBase::~ClientInterfaceBase(void)
{
															// Delete the client interface's Ext data if present
	if(getExtData())
	{
		delete getExtData();
	}
}

Mutex & ClientInterfaceBase::getInterfaceMutex(void)
{
	return interfaceMutex;
}

void ClientInterfaceBase::setExtData(ClientInterfaceExtData *initExtData)
{
															// Set new Ext data
	extData = initExtData;
}

ClientInterfaceExtData * ClientInterfaceBase::getExtData(void)
{
	return extData;
}


PTRMI::Status ClientInterfaceBase::lock(void)
{
	if(getInterfaceMutex().wait())
	{
		return Status();
	}

	return Status(Status::Status_Error_Client_Interface_Lock_Timeout);
}


PTRMI::Status ClientInterfaceBase::unlock()
{
	if(getInterfaceMutex().release())
		return Status();

	return Status(Status::Status_Error_Client_Interface_Not_Locked);
}

PTRMI::Status ClientInterfaceBase::beginShared(ClientInterfaceExtData *extData)
{
	Status	status;
															// If usage lock on interface is obtained
	if((status = lock()).isOK())
	{
															// Set any specified Ext Data
		setExtData(extData);

		setSharedCounter(getSharedCounter() + 1);
	}
															// Reurn status
	return status;
}

PTRMI::Status ClientInterfaceBase::endShared(void)
{
															// Decrement shared counter
	setSharedCounter(getSharedCounter() - 1);
															// Only clear extData if recursive lock level is zero, i.e. unlocked
	if(getSharedCounter() == 0)
	{
															// Clear Ext data
		setExtData(NULL);
	}
															// Unlock ready for next use
	return unlock();
}


void ClientInterfaceBase::setStatus(Status initStatus)
{
	status = initStatus;
}


PTRMI::Status ClientInterfaceBase::getStatus(void) const
{
	return status;
}


void ClientInterfaceBase::resetStatus(void)
{
	setStatus(Status());
}


PTRMI::Status ClientInterfaceBase::releaseStream(void)
{
	Stream *stream;
															// If Stream is defined
	if(stream = getStream())
	{
															// Clear the stream, releasing the pipe
		getStream()->clear();

		return Status();
	}
															// Stream not set, so return error
	return Status(Status::Status_Client_Stream_Not_Set);
}

void ClientInterfaceBase::setSharedCounter(int counter)
{
	sharedCounter = counter;

	assert(sharedCounter >= 0);
}

int ClientInterfaceBase::getSharedCounter(void)
{
	return sharedCounter;
}




} // End PTRMI namespace