
#include <PTRMI/ManagerClientInterface.h>
#include <PTRMI/Manager.h>


namespace PTRMI
{
	
PTRMI::Status ManagerClientInterface::swapManagerInfo(const ManagerInfo *managerInfo, ManagerInfo *resultManagerInfo)
{
	return sendAuto<Status, PC<In<const ManagerInfo *>>, PC<Out<ManagerInfo *>>>(Status(Status::Status_Error_RMI_Invoke_Failed, false), L"swapManagerInfo", managerInfo, resultManagerInfo);
}


PTRMI::Status ManagerClientInterface::getCurrentManagerInfo(unsigned int verboseLevel, ManagerInfo *resultManagerInfo)
{
	return sendAuto<Status, PC<In<unsigned int>>, PC<Out<ManagerInfo *>>>(Status(Status::Status_Error_RMI_Invoke_Failed, false), L"getCurrentManagerInfo", verboseLevel, resultManagerInfo);
}


PTRMI::Status ManagerClientInterface::newObject(Name *objectClass, Name *objectName, const PTRMI::GUID *clientManager, PTRMI::GUID *clientInterface, PTRMI::GUID *resultRemoteServerInterface)
{
	return sendAuto<Status, PC<In<const Name *>>, PC<In<const Name *>>, PC<In<const PTRMI::GUID *>>, PC<In<const PTRMI::GUID *>>, PC<Out<PTRMI::GUID *>>>(Status(Status::Status_Error_RMI_Invoke_Failed, false), L"newObject", objectClass, objectName, clientManager, clientInterface, resultRemoteServerInterface);
}


PTRMI::Status ManagerClientInterface::discardObject(const PTRMI::GUID &serverInterfaceGUID)
{
	return sendAuto<Status, PC<In<const PTRMI::GUID *>>>(Status(Status::Status_Error_RMI_Invoke_Failed, false), L"discardObject", &serverInterfaceGUID);
}


Status ManagerClientInterface::recoverRemoteObject(void)
{
	Status	status;

	RemotePtr<Manager>	managerClientInterface(this);

	status = getManager().recoverRemoteManager(managerClientInterface);

	return status;
}



} // End PTRMI namespace
