
#include <PTRMI/ServerInterface.h>


void PTRMI::ServerInterfaceBase::setClientManagerGUID(const PTRMI::GUID &guid)
{
	hostName = guid;
}

const PTRMI::GUID PTRMI::ServerInterfaceBase::getClientManagerGUID(void) const
{
	return hostName.getGUID();
}

