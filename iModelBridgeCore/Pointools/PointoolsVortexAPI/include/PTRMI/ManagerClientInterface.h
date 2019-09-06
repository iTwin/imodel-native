#pragma once 

#include <PTRMI/ClientInterface.h>
//#include <PTRMI/Manager.h>

#include <PTRMI/ManagerInfo.h>

namespace PTRMI
{
	class Manager;
}

namespace PTRMI
{


PTRMI_DECLARE_CLIENT_INTERFACE(ManagerClientInterface)
{
public:

	Status			swapManagerInfo			(const ManagerInfo *managerInfo, ManagerInfo *resultManagerInfo);

	Status			getCurrentManagerInfo	(unsigned int verboseLevel, ManagerInfo *resultManagerInfo);

	Status			newObject				(Name *objectClass, Name *objectName, const PTRMI::GUID *clientManager, PTRMI::GUID *clientInterface, PTRMI::GUID *resultRemoteServerInterface);
	Status			discardObject			(const PTRMI::GUID &serverInterfaceGUID);
	Status			deleteObject			(const Name &objectName)												{return Status();}

	Status			getObject				(const Name &objectName, PTRMI::GUID &resultRemoteServerInterface)		{return Status();}

	Status			recoverRemoteObject		(void);


};

PTRMI_USE_CLIENT_INTERFACE(PTRMI::ManagerClientInterface, PTRMI::Manager);

} // End PTRMI namespace
