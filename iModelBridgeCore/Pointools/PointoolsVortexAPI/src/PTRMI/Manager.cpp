#include "PointoolsVortexAPIInternal.h"
#include <PTRMI/Manager.h>

#include <PTRMI/ManagerClientInterface.h>
#include <PTRMI/ManagerServerInterface.h>

namespace PTRMI
{

Manager	*manager = NULL;

Status initialize(void)
{
	if(manager = new Manager)
	{
		return manager->initialize();		
	}

	return Status(Status::Status_Error_Memory_Allocation);
}

Manager &getManager(void)
{
	return *manager;
}

void initializeManager(void)
{
	if(manager == NULL)
	{
		manager = new Manager;
	}
}

Manager::Manager(void)
{
	setHostManager(NULL);

	setObjectManager(NULL);

	setPipeProtocolManager(NULL);

	setManagerInfo(NULL);
}

Manager::~Manager(void)
{
															// Delete all client and server related resources
	deleteAllHosts();

	if(objectManager)
		delete objectManager;

	if(pipeProtocolManager)
		delete pipeProtocolManager;

	if(managerInfo)
		delete managerInfo;
}


Status Manager::initialize(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Create Host Manager
	setHostManager(new HostManager());
	if(getHostManagerPtr() == NULL)
	{
		return PTRMI::Status(PTRMI::Status::Status_Error_Memory_Allocation);
	}
															// Create object manager
	setObjectManager(new ObjectManager());
	if(getObjectManagerPtr() == NULL)
	{
		return PTRMI::Status(PTRMI::Status::Status_Error_Memory_Allocation);
	}
															// Create pipe protocol manager
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	setPipeProtocolManager(new PipeProtocolManager());
	if(getPipeProtocolManagerPtr() == NULL)
	{
		return PTRMI::Status(PTRMI::Status::Status_Error_Memory_Allocation);
	}
#endif
															// Create message dispatcher
	setDispatcher(new Dispatcher());
	if(getDispatcherPtr() == NULL)
	{
		return PTRMI::Status(Status::Status_Error_Memory_Allocation);
	}

															// Generate the manager's session guid
	managerGUID.generate();
															// Initialize object manager
	getObjectManager().initialize(getManagerGUID());

	getObjectManager().newMetaInterface<Manager>(Name(MANAGER_CLASS_NAME));
															// Add this manager as an unmanaged object
	getObjectManager().addObject(Name(MANAGER_CLASS_NAME), Name(MANAGER_OBJECT_NAME), managerGUID, this, false);
															// Initialize ManagerInfo object

	setManagerInfo(new ManagerInfo(*this));

	if(getManagerInfo() == NULL)
	{
		return PTRMI::Status(Status::Status_Error_Memory_Allocation);
	}

	return Status();
}

void Manager::setHostManager(HostManager *manager)
{
	hostManager = manager;
}

HostManager &Manager::getHostManager(void)
{
	assert(getHostManagerPtr());

	return *getHostManagerPtr();
}


HostManager *Manager::getHostManagerPtr(void)
{
	return hostManager;
}


void Manager::setObjectManager(ObjectManager *manager)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/

	objectManager = manager;
}

ObjectManager &Manager::getObjectManager(void)
{
	assert(getObjectManagerPtr());
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/

	return *getObjectManagerPtr();
}


ObjectManager *Manager::getObjectManagerPtr(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return NULL;
	}
*/

	return objectManager;
}


void Manager::setPipeProtocolManager(PipeProtocolManager *manager)
{
	pipeProtocolManager = manager;
}


PipeProtocolManager &Manager::getPipeProtocolManager(void)
{
	assert(getPipeProtocolManagerPtr());

	return *getPipeProtocolManagerPtr();
}


PipeProtocolManager *Manager::getPipeProtocolManagerPtr(void)
{
	return pipeProtocolManager;
}


void Manager::setDispatcher(Dispatcher *initDispatcher)
{
	dispatcher = initDispatcher;
}


Dispatcher &Manager::getDispatcher(void)
{
	assert(getDispatcherPtr());

	return *getDispatcherPtr();
}


Dispatcher *Manager::getDispatcherPtr(void)
{
	return dispatcher;
}



Status Manager::initializeHost(InterfaceBase *interfaceBase, const ManagerInfo *remoteManagerInfo)
{
	Stream				*	stream;
	Status					status;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Make sure Client or Server Interface has been specified
	if(interfaceBase == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Get the interface (Client or Server) stream
	if((stream = interfaceBase->getStream()) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Initialize_Host);
	}
															// Get remote host GUID
	const Name &remoteHostName = stream->getHostName();

															// Make sure remote host name is valid
	if(remoteHostName.isPartiallyValid() == false)
	{
		return Status(Status::Status_Error_Failed_To_Initialize_Host);
	}

	Host *host;
															// Add a locked host or lock host if it already exists
	if((host = getHostManager().lockOrNewLockedHost(remoteHostName, remoteManagerInfo)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Host);
	}
															// If ManagerInfo has been specified for the remote host
	if(remoteManagerInfo)
	{
															// Resolve and set Message version to be used on this Pipe
		host->setMessageVersionUsed(Message::resolveMessageVersions(Message::getCurrentMessageVersion(), remoteManagerInfo->getMessageVersion()));
	}
															// Release the Host
	status = getHostManager().releaseHost(remoteHostName);

															// Return status
	return status;
}


Status Manager::swapClientServerManagerInfo(RemotePtr<Manager> &remoteManager, ClientInterfaceExtData *extData)
{
	Status			status;
	ManagerInfo		remoteManagerInfo;
	ManagerInfo *	managerInfo;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Makes sure remote pointer is valid
	if(remoteManager.isValid() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

															// Begin potentially shared access to remote Manager object (not really needed here)
	if((status = remoteManager->beginShared(extData)).isFailed())
	{
		return status;										// Manager not obtained, so return NULL object
	}

	try
	{
															// Try to swap ManagerInfo between the two hosts
		status = remoteManager->swapManagerInfo(getManagerInfo(), &remoteManagerInfo);

// Pip Option
/*
remoteManagerInfo.writeFile(L"C:\\users\\Lee.Bull\\ServerManagerInfo.txt");

const ManagerInfo *clientManagerInfo;
clientManagerInfo = PTRMI::getManager().getManagerInfo();
if(clientManagerInfo)
{
	clientManagerInfo->writeFile(L"C:\\users\\Lee.Bull\\ClientManagerInfo.txt");
}
ManagerInfo testRemoteManagerInfo;
remoteManager->getCurrentManagerInfo(1, &testRemoteManagerInfo);
testRemoteManagerInfo.writeFile(L"C:\\users\\Lee.Bull\\ServerManagerInfoVerbose.txt");
*/
															// If RMI call was made successfully
		if(remoteManager.getStatus().isOK())
		{
															// If RMI method result was successful
			if(status.isOK())
			{
															// Use the returned ManagerInfo
				managerInfo = &remoteManagerInfo;
			}
			else
			{
				throw Status(Status::Status_Error_Failed_To_Swap_Manager_Info);
			}
		}
		else
		{
															// RMI method call failed
															// If class method wasn't found, Manager is older than v2 Message, so system stays same
			if(remoteManager.getStatus().is(Status::Status_Server_Class_Method_Not_Found))
			{
															// Reset Manager ClientInterface status so it can continue to be used
				remoteManager.resetStatus();
															// Initialize host without ManagerInfo
				managerInfo = NULL;
			}
			else
			{
				throw remoteManager.getStatus();
			}
		}
															// Initialize host
		if((status = initializeHost(remoteManager.getObjectClientInterface(), managerInfo)).isFailed())
		{
			throw status;
		}
	}
	catch(Status s)
	{
		status = s;
	}

	remoteManager->endShared();

															// Return Status
	return status;
}

RemotePtr<Manager> Manager::getRemoteManager(const Name &objectName, ClientInterfaceExtData *extData, RemotePtr<Manager> *managerClientInterfaceExisting)
{
	RemotePtr<Manager>				remoteManager;
	Name							remoteManagerName;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return remoteManager;
	}
*/
															// Get name of remote manager from remote object name
	objectName.getRemoteManagerName(remoteManagerName);
															// If an existing ClientInterface isn't given (as needed for recovery)
	if(managerClientInterfaceExisting == NULL)
	{
															// If remote host's RMI Manager has a client already, use it
		if((remoteManager = getObjectManager().getFirstObjectClientInterface<Manager>(remoteManagerName)).isValid())
		{
															// Return existing remote manager
			return remoteManager;
		}
															// Create an object Client Interface for the remote manager
															// Note: Remote manager's server interface is unknown at this time
															// so the full URL of the manager is used
		remoteManager = getObjectManager().newObjectClientInterface<Manager>(Name(L"Manager"), remoteManagerName);
	}
	else
	{
															// Use the ManagerClientInterface provided (used for recovery)
		remoteManager = *managerClientInterfaceExisting;
	}

															// Swap ManagerInfo between Client and Server
	swapClientServerManagerInfo(remoteManager, extData);

															// If ManagerClientInterface was created now, register with serverManagerClientInterface
	if(managerClientInterfaceExisting == NULL)
	{
		getObjectManager().addServerManagerClientInterface(&remoteManager.getObjectClientInterface()->getHostGUID(), remoteManager.getObjectClientInterface());
	}

															// Return the result
	return remoteManager;
}


Status Manager::recoverRemoteManager(RemotePtr<Manager> &managerInterface)
{
	// Manager ServerInterface on remote host has been deleted
	// Force remote system to create another one.
	// getRemoteManager() type call to create another one that is bound to the given ManagerClientInterface
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Clear previous error status
	managerInterface.resetStatus();
															// Clear remote ServerInterface binding beause it's out of date
	managerInterface->setRemoteInterface(Name());

	getRemoteManager(managerInterface->getObjectName(), managerInterface->getExtData(), &managerInterface);

	return Status();
}


const PTRMI::GUID & Manager::getManagerGUID(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
	return managerGUID;
}


Status Manager::swapManagerInfo(ServerInterfaceBase *serverInterface, const ManagerInfo *clientManagerInfo, ManagerInfo *resultManagerInfo)
{
	const ManagerInfo *	serverManagerInfo;
	Status				status;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
	if(serverInterface == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	if(clientManagerInfo != NULL)
	{
		if((status = getManager().initializeHost(serverInterface, clientManagerInfo)).isOK())
		{
															// Get this host's ManagerInfo
			if(serverManagerInfo = getManagerInfo())
			{
															// Copy to given return parameter
				*resultManagerInfo = *serverManagerInfo;
			}
		}
	}

	return status;
}


Status Manager::getCurrentManagerInfo(unsigned int verboseLevel, ManagerInfo *resultManagerInfo)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
	if(resultManagerInfo == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	resultManagerInfo->update(*this, verboseLevel);

	return Status();
}



PTRMI::Status Manager::newObject(const Name *objectClass, const Name *objectName, const PTRMI::GUID *clientManager, const PTRMI::GUID *clientInterface, PTRMI::GUID *resultRemoteServerInterface)
{
	Status					status;
	ServerInterfaceBase	*	serverInterface;


	if(objectClass == NULL || objectName == NULL || clientManager == NULL || clientInterface == NULL || resultRemoteServerInterface == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Make sure client manager's GUID is valid
	if(clientManager->isValidGUID() == false)
	{
		resultRemoteServerInterface->clear();
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Create new object and server interface.
	if((serverInterface = getObjectManager().newObjectAndServerInterface(*objectClass, *objectName, *clientManager)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Create_Remote_Object);
	}
															// Lock server interface for access
	if((getObjectManager().lockServerInterface(serverInterface).isFailed()))
	{
															// Failed to lock, so delete server interface (and object)
		getObjectManager().deleteObjectServerInterface(serverInterface);
															// Return error
		return Status(Status::Status_Error_Failed_To_Create_Remote_Object);
	}
															// Return GUID for the new server interface 
	*resultRemoteServerInterface = serverInterface->getInterfaceGUID();

															// Release server interface
	getObjectManager().releaseServerInterface(serverInterface);

															// Return status
	return status;
}


PTRMI::Status Manager::discardObject(const GUID *serverInterfaceGUID)
{
															// Make sure GUID is given
	if(serverInterfaceGUID == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Delete the server interface and potentially the object too
	return getObjectManager().deleteObjectServerInterface(*serverInterfaceGUID);
}


PTRMI::Status Manager::setSendExternalCallFunction(PipeManagerExt::ExternalCallFunction function)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	PipeManagerExt *pipeManagerExt;
	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Ext_Manager);
	}

	pipeManagerExt->setSendExternalCallFunction(function);

	return Status();
#else
    Status status;
    return status;
#endif
}


PipeManagerExt::ExternalCallFunction Manager::getSendExternalCallFunction(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return NULL;
	}
*/

#ifdef NEEDS_WORK_VORTEX_DGNDB
	PipeManagerExt *pipeManagerExt;
	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return NULL;
	}

	return pipeManagerExt->getSendExternalCallFunction();
#else
    return nullptr;
#endif
}


PTRMI::Status Manager::setReleaseExternalBufferFunction(PipeManagerExt::ExternalReleaseBufferFunction function)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	PipeManagerExt *pipeManagerExt;
	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Ext_Manager);
	}

	pipeManagerExt->setReleaseExternalBufferFunction(function);

	return Status();
#else
    Status status;
    return status;
#endif
}


PipeManagerExt::ExternalReleaseBufferFunction Manager::getReleaseExternalBufferFunction(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return NULL;
	}
*/

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
    PipeManagerExt *pipeManagerExt;
	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return NULL;
	}

	return pipeManagerExt->getReleaseExternalBufferFunction();
#else
    return nullptr;
#endif
}


PTRMI::Status Manager::receiveExternalCall(void *receive, unsigned int receiveSize, void **send, unsigned int *sendSize, PTRMI::GUID *clientManager)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	PipeManagerExt *pipeManagerExt;

	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Ext_Manager);
	}

	return pipeManagerExt->receiveExternalCall(receive, receiveSize, send, sendSize, clientManager);
#else
    Status status;
    return status;
#endif
}



PTRMI::Status Manager::receiveExternalCallCB(void *receive, unsigned int receiveSize, PTRMI::GUID *clientManager, PipeManagerExt::ExternalProcessRequestCallback externalProcessRequestCallback)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	PipeManagerExt *pipeManagerExt;

	if((pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")))) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Ext_Manager);
	}

	return pipeManagerExt->receiveExternalCallCB(receive, receiveSize, clientManager, externalProcessRequestCallback);
#else
    Status status;
    return status;
#endif
}


Status Manager::deleteAllHosts(void)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
	return getObjectManager().deleteAllHosts();
}


Status Manager::deleteHost(const PTRMI::GUID &hostGUID)
{
	Host			*	host;
	Status				status;

	Name				hostName(hostGUID);
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
	if(hostGUID.isValidGUID() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Lock access to the host to prevent host's resource being deleted while potentially being used.
	if((host = lockHost(hostName)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Host);
	}
															// Delete all Host related objects including Client & Server Interfaces
	status = getObjectManager().deleteHost(hostGUID);

															// Delete the Host and it's Pipe
	getHostManager().deleteHost(hostName, true);

															// Unlock host just in case deleteHost failed to delete it
	releaseHost(hostName);
															// Return status
	return status;
}


PTRMI::Status Manager::deleteAll(void)
{
	Status				status;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Manager);
	}
*/
															// Delete all PTRMI objects (Managed only)
	getObjectManager().deleteAll();
															// Delete all Host entries
	getHostManager().deleteAll();

	return Status();
}

void Manager::setManagerInfo(ManagerInfo *initManagerInfo)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return;
	}
*/
	managerInfo = initManagerInfo;
}


void Manager::updateManagerInfo(unsigned int verboseLevel)
{
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return;
	}
*/
	if(managerInfo)
	{
		managerInfo->update(*this, verboseLevel);
	}
}


const ManagerInfo *Manager::getManagerInfo(void)
{
	return managerInfo;
}

Host *Manager::lockHost(const Name &hostName)
{
	return getHostManager().lockHost(hostName);
}


Host *Manager::spinLockHost(const Name &hostName)
{
	return getHostManager().spinLockHost(hostName);
}


Host *Manager::lockOrNewLockedHost(const Name &hostName)
{
	return getHostManager().lockOrNewLockedHost(hostName);
}


Status Manager::releaseHost(const Name &hostName)
{
	return getHostManager().releaseHost(hostName);
}


Status Manager::releaseHost(Host *host)
{
	return getHostManager().releaseHost(host);
}


bool Manager::getHostFeatureMultiReadSet(const Name &hostName, bool &result)
{
	Host				*	host;
/*
	MutexScope mutexScope(managerMutex, MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Manager);
		return false;
	}
*/
	result = false;
															// Lock host for access
	if(host = lockHost(hostName))
	{
															// Get remote host's ManagerInfo (if version 1.6.0.0 or above)
		const ManagerInfo &hostManagerInfo = host->getManagerInfo();
															// See if feature is supported
		result = hostManagerInfo.getFeatureMultiReadSet();
															// Release the host
		releaseHost(host);
															// Return OK
		return true;
	}
															// Return failure
	return false;
}


Status Manager::pingInactiveHosts(void)
{
	return getObjectManager().pingInactiveHosts();
}


} // End Namespace

