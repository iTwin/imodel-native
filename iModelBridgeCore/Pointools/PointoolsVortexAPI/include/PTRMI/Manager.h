#pragma once 

#include <PTRMI/ObjectManager.h>
#include <PTRMI/PipeProtocolManager.h>
#include <PTRMI/Dispatcher.h>

#include <PTRMI/ManagerClientInterface.h>
#include <PTRMI/ManagerServerInterface.h>

#include <PTRMI/GUID.h>
#include <PTRMI/Status.h>

#include <PTRMI/PipeManagerExt.h>

#include <PTRMI/HostManager.h>

#include <PTRMI/ManagerInfo.h>


#define MANAGER_CLASS_NAME	L"Manager"
#define	MANAGER_OBJECT_NAME	L"Manager"

#define MANAGER_LOCK_TIMEOUT	30 * 1000


namespace PTRMI
{

class Maanger;

Status		initialize			(void);
Manager &	getManager			(void);


class Manager
{
public:


protected:

//	Mutex											managerMutex;

	ManagerInfo									*	managerInfo;

	PTRMI::GUID										managerGUID;

	HostManager									*	hostManager;
	ObjectManager								*	objectManager;
	PipeProtocolManager							*	pipeProtocolManager;
	Dispatcher									*	dispatcher;

protected:
	
	void											setHostManager					(HostManager *manager);
	void											setObjectManager				(ObjectManager *manager);
	void											setPipeProtocolManager			(PipeProtocolManager *manager);
	void											setDispatcher					(Dispatcher *initDispatcher);

	HostManager									*	getHostManagerPtr				(void);
	ObjectManager								*	getObjectManagerPtr				(void);
	PipeProtocolManager							*	getPipeProtocolManagerPtr		(void);
	Dispatcher									*	getDispatcherPtr				(void);

	void											setManagerInfo					(ManagerInfo *initManagerInfo);

	Status											initializeHost					(InterfaceBase *interfaceBase, const ManagerInfo *remoteManagerInfo);

	
public:
													Manager							(void);
									   			   ~Manager							(void);

	Status											initialize						(void);

	HostManager									&	getHostManager					(void);
	ObjectManager								&	getObjectManager				(void);
	PipeProtocolManager							&	getPipeProtocolManager			(void);
	Dispatcher									&	getDispatcher					(void);

	Status											swapClientServerManagerInfo		(RemotePtr<Manager> &remoteManager, ClientInterfaceExtData *extData);

	const ManagerInfo							*	getManagerInfo					(void);
	void											updateManagerInfo				(unsigned int verboseLevel);

	bool											getHostFeatureMultiReadSet		(const Name &hostName, bool &result);

	const PTRMI::GUID							&	getManagerGUID					(void);

	template<typename Obj> void						newMetaInterface				(const Name &objectClassName);


	Host										*	lockHost						(const Name &hostName);
	Host										*	lockOrNewLockedHost				(const Name &hostName);
	Host										*	spinLockHost					(const Name &hostName);
	Status											releaseHost						(const Name &hostName);
	Status											releaseHost						(Host *host);

	template<typename Obj> RemotePtr<Obj>			newRemoteObject					(const Name &objectClass, const Name &objectName, ClientInterfaceExtData *extData = NULL, Status &status = Status(), RemotePtr<Obj> *clientInterfaceExisting = NULL);
	template<typename Obj> Status					discardRemoteObject				(RemotePtr<Obj> &clientInterface, ClientInterfaceExtData *extData = NULL);

	template<typename Obj> RemotePtr<Obj>			getRemoteObject					(const Name &objectName, Status &status = Status());

	template<typename Obj> Status					recoverRemoteObject				(RemotePtr<Obj> &clientInterface);
	Status											recoverRemoteManager			(RemotePtr<Manager> &managerInterface);

	RemotePtr<Manager>								getRemoteManager				(const Name &host, ClientInterfaceExtData *extData, RemotePtr<Manager> *managerClientInterfaceExisting = NULL);

																	// RMI Server Methods
public:

	Status											swapManagerInfo					(ServerInterfaceBase *serverInterface, const ManagerInfo *info, ManagerInfo *resultManagerInfo);

	Status											getCurrentManagerInfo			(unsigned int verboseLevel, ManagerInfo *resultManagerInfo);

	Status											newObject						(const Name *objectClass, const Name *objectName, const PTRMI::GUID *clientManager, const PTRMI::GUID *clientInterface, PTRMI::GUID *resultRemoteServerInterface);
	Status											discardObject					(const GUID *serverInterfaceGUID);

public:

	Status											setSendExternalCallFunction		(PipeManagerExt::ExternalCallFunction function);
	PipeManagerExt::ExternalCallFunction			getSendExternalCallFunction		(void);

	Status											setReleaseExternalBufferFunction(PipeManagerExt::ExternalReleaseBufferFunction function);
	PipeManagerExt::ExternalReleaseBufferFunction	getReleaseExternalBufferFunction(void);

	Status											receiveExternalCall				(void *receive, unsigned int receiveSize, void **send, unsigned int *sendSize, PTRMI::GUID *clientManager = NULL);
	Status											receiveExternalCallCB			(void *receive, unsigned int receiveSize, PTRMI::GUID *clientManager, PipeManagerExt::ExternalProcessRequestCallback externalProcessRequestCallback);

	Status											deleteHost						(const PTRMI::GUID &hostGUID);
	Status											deleteAllHosts					(void);

	Status											deleteAll						(void);

	Status											pingInactiveHosts				(void);
};



template<typename Obj>
inline void Manager::newMetaInterface(const Name &objectClassName)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return;
	}
*/
	getManager().getObjectManager().newMetaInterface<Obj>(objectClassName);
}



template<typename Obj>
inline typename PTRMI::RemotePtr<Obj> Manager::newRemoteObject(const Name &objectClass, const Name &objectName, ClientInterfaceExtData *extData, Status &status, RemotePtr<Obj> *clientInterfaceExisting)
{
	RemotePtr<Obj>		clientInterface;
	PTRMI::GUID			remoteServerInterfaceGUID;
	RemotePtr<Manager>	remoteManager;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		status = Status(Status::Status_Error_Failed_To_Lock_Manager);
		return clientInterface;
	}
*/
															// Get remote manager holding named item (host is in the name)
	if((remoteManager = getRemoteManager(objectName, extData)).isValid() == false)
	{
		status = Status(Status::Status_Error_Remote_Manager_Not_Found);
		return clientInterface;								// Manager not obtained, so return NULL object
	}

															// If ext data specified for client messaging, set it
// 	if(remoteManager.getObjectClientInterface())
// 	{
// 		remoteManager.getObjectClientInterface()->setExtData(extData);
// 	}
															// Begin potentially shared access to remote Manager object
	if((status = remoteManager->beginShared(extData)).isFailed())
	{
															// End potentially shared access to Manager
		remoteManager->endShared();

		return clientInterface;								// Manager not obtained, so return NULL object
	}
															// If no existing ClientInterface is provided (normal case)
	if(clientInterfaceExisting == NULL)
	{
															// Create local Client Interface
		if((clientInterface = getObjectManager().newObjectClientInterface<Obj>(objectClass, objectName)).isValid() == false)
		{
			Status status(Status::Status_Error_Failed_To_Create_Client_Interface);
															// End potentially shared access to Manager
			remoteManager->endShared();

			return clientInterface;
		}
	}
	else
	{
															// Copy RemotePtr to provided ClientInterface
		clientInterface = *clientInterfaceExisting;
	}
															// Create a new remote object of specified class, with specified name
	if((status = remoteManager->newObject(const_cast<Name *>(&objectClass), const_cast<Name *>(&objectName), &(getManagerGUID()), const_cast<PTRMI::GUID *>(&clientInterface->getInterfaceGUID()), &remoteServerInterfaceGUID)).isFailed())
	{
															// Delete client interface and invalidate result
		getObjectManager().deleteObjectClientInterface(clientInterface.getObjectClientInterface());
		clientInterface.invalidate();

															// End potentially shared access to Manager
		remoteManager->endShared();

		return clientInterface;								// Return NULL object
	}

	Name remoteServerInterface(remoteServerInterfaceGUID);
															// Bind client interface to server interface
	clientInterface->setRemoteInterface(remoteServerInterface);
															// Add client interface now that remote host GUID is known
	PTRMI::Name	remoteHostName = remoteManager->getHostName();
	clientInterface->setHostName(remoteHostName);
	getObjectManager().addServerManagerClientInterface(&(remoteHostName.getGUID()), clientInterface.getObjectClientInterface());

															// Set ExtData back to NULL as manager doesn't own it
// Pip Option
// 	if(remoteManager.getObjectClientInterface())
// 	{
// 		remoteManager.getObjectClientInterface()->setExtData(NULL);
// 	}
															// End potentially shared use of remote manager
	remoteManager->endShared();
															// Return pointer to new object's Client Interface
	return clientInterface;
}



template<typename Obj>
inline Status Manager::discardRemoteObject(RemotePtr<Obj> &clientInterface, ClientInterfaceExtData *extData)
{
	Name				remoteServerInterface;
	RemotePtr<Manager>	remoteManager;
	Status				status;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Get remote manager holding named item (host is in the name)
	if((remoteManager = getRemoteManager(clientInterface->getObjectName(), extData)).isValid() == false)
	{
															// Manager not obtained, so return NULL object
		return Status(Status::Status_Error_Remote_Manager_Not_Found);
	}
															// Begin potentially shared access to remote Manager object
	if((status = remoteManager->beginShared(extData)).isFailed())
	{
		return status;										// Manager not obtained, so return NULL object
	}

															// If this client interface is in a failed state
	if(clientInterface.getStatus().isOK())
	{
															// If ext data specified for client messaging, set it
		if(remoteManager.getObjectClientInterface())
		{
			remoteManager.getObjectClientInterface()->setExtData(extData);
		}
															// If fully bound to remote server interface (i.e. GUID is known)
		const PTRMI::GUID &remoteInterfaceGUID = clientInterface->getRemoteInterface().getGUID();
		if(remoteInterfaceGUID.isValidGUID())
		{
															// Create a new remote object of specified class, with specified name
			if((status = remoteManager->discardObject(remoteInterfaceGUID)).isFailed())
			{
															// End potentially shared access to remote Manager object
				remoteManager->endShared();

				return status;
			}
		}
															// Set ExtData back to NULL as manager doesn't own it
// 		if(remoteManager.getObjectClientInterface())
// 		{
// 			remoteManager.getObjectClientInterface()->setExtData(NULL);
// 		}
	}

															// Delete the client interface
	getObjectManager().deleteObjectClientInterface(clientInterface.getObjectClientInterface());
															// The RemotePtr Pointer is now invalid
	clientInterface.invalidate();
															// End potentially shared access to remote Manager object
	remoteManager->endShared();


	return status;
}


template<typename Obj>
inline typename Status Manager::recoverRemoteObject(RemotePtr<Obj> &clientInterface)
{
	Name				remoteServerInterface;
	RemotePtr<Manager>	remoteManager;
	RemotePtr<Obj>		newObject;
	Status				status;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Make sure given ClientInterface is valid
	if(clientInterface.isValid() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

															// Delete ClientInterface to remote Manager as it is now out of date
	if((remoteManager = getRemoteManager(clientInterface.getObjectClientInterface()->getObjectName(), clientInterface->getExtData())).isValid())
	{
		getObjectManager().deleteObjectClientInterface(remoteManager.getObjectClientInterface());
	}

															// Create a new object bound to the given client interface
	newObject = newRemoteObject<Obj>(clientInterface->getObjectClass(), clientInterface->getObjectName(), clientInterface->getExtData(), status, &clientInterface);
	if(newObject.isValid() == false)
	{
		return clientInterface.getStatus();
	}
															// Reset client interface's status to clear the error status
	clientInterface.resetStatus();
															// Return status
	return status;
}



/*

template<typename Obj> typename PTRMI::RemotePtr<Obj> Manager::getRemoteObject(const Name &objectName, Status &status)
{
	Status				result;
	RemotePtr<Manager>	remoteManager;
	PTRMI::GUID			remoteServerInterfaceGUID;
	Status				status;
															// Get remote manager
	if((remoteManager = getRemoteManager(objectName)).isValid() == false)
	{
		return Status(Status::Status_Error_Remote_Manager_Not_Found);
	}

	if((status = remoteManager->getObject(objectName, remoteServerInterfaceGUID, PTRMI::GUID &remoteServerInterfaceGUID)).isFailed())
	{
		return status;
	}

	if(remoteServerInterfaceGUID.isValid() == false)
	{
		return Status(Status::Status_Error_Get_Remote_Object_Bad_GUID);
	}

	Name remoteServerInterface(remoteServerInterfaceGUID);

															// Create a new client interface for the remote object
	if((result = getObjectManager().newObjectClientInterface<Obj>(objectName, remoteServerInterface)).isValid() == false)
	{
															// Delete remote object as an error has occured
		status = remoteManager->discardObject(objectName, remoteServerInterface.getGUID());
															// Return NULL object
		return result;
	}


	return result;
}

*/

} // End PTRMI namespace

