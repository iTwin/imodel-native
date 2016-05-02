#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/InterfaceBase.h>
#include <PTRMI/Stream.h>


namespace PTRMI
{


InterfaceBase::InterfaceBase(void)
{
															// Generate a default GUID
	interfaceGUID.generate();
}

InterfaceBase::~InterfaceBase(void)
{
															// Delete send stream if present
	deleteStream();

}

const PTRMI::GUID &InterfaceBase::getInterfaceGUID(void) const
{
	return interfaceGUID;
}


void InterfaceBase::setObjectClass(const Name &initObjectClass)
{
	objectClass = initObjectClass;
}


const Name &InterfaceBase::getObjectClass(void)
{
	return objectClass;
}

void InterfaceBase::setObjectName(const Name &initObjectName)
{
	objectName = initObjectName;
}

const Name &InterfaceBase::getObjectName(void)
{
	return objectName;
}


void InterfaceBase::setHostName(const Name &initHostName)
{
	hostName = initHostName;
}


const Name &InterfaceBase::getHostName(void)
{
	return hostName;
}


bool InterfaceBase::updateHostName(const Name &newName)
{
	return hostName.update(newName);
}


void InterfaceBase::setRemoteInterface(const Name &objectNameOrInterfacecGUID)
{
	remoteInterface = objectNameOrInterfacecGUID;
}

const PTRMI::Name & InterfaceBase::getRemoteInterface(void) const
{
	return remoteInterface;
}

void InterfaceBase::setStream(Stream *initStream)
{
	stream = initStream;
}

Stream * InterfaceBase::getStream(void) const
{
	return stream;
}

Status InterfaceBase::createStream(void)
{
	setStream(new Stream(this));

	if(getStream() == NULL)
	{
		return Status(Status::Status_Error_Memory_Allocation);
	}

	return Status();
}

Status InterfaceBase::deleteStream(void)
{
	if(getStream() == NULL)
	{
		return Status(Status::Status_Error_Stream_Invalid);
	}

	delete getStream();

	setStream(NULL);

	return Status();
}

void InterfaceBase::setTimeLastUsed(void)
{
															// Restart last use timer
	timeLastUsed.start();
}


double InterfaceBase::getIdleTimeSeconds(void)
{
															// Return time since object last used
	return timeLastUsed.getEllapsedTimeSeconds();
}


PTRMI::GUID InterfaceBase::getHostGUID(void)
{
	Stream			*	s;
															// If Stream and Stream's Pipe are defined
	if(s = getStream())
	{
		return s->getHostGUID();
	}
															// Not found, so return invalid GUID
	return PTRMI::GUID();
}




} // End PTRMI namespace