#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/ObjectManager.h>

#include <PTRMI/Manager.h>

#include <PTRMI/KeyKeyMap.h>


#define PTRMI_ERROR_HANDLER_OBJECT_NAME		L"ErrorHandler"

															// Main timeout is 30 seconds
const unsigned long OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT			= 1000 * 30;
															// Spin retries 60 times every 1 second (for deleteLock() )
const unsigned int OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_SPIN_RETRIES		= 60;
const unsigned int	OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_SPIN_TIMEOUT	= 1000;
 
#include <ptds/DataSourceServer.h>

namespace PTRMI
{


ObjectManager::ObjectManager(void)
{
															// Initially no Error Handler
	setErrorHandlerServerInterface(NULL);
}


Status ObjectManager::initialize(const PTRMI::GUID &managerGUID)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	status = createErrorHandlerServerInterface(managerGUID);

	return status;
}


Status ObjectManager::createErrorHandlerServerInterface(const PTRMI::GUID &managerGUID)
{
	ServerInterfaceBase *	errorHandler;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}


	if(getErrorHandlerServerInterface() != NULL)
		return Status(Status::Status_Error_Failed);
															// Register NULL object class type
	newMetaInterface<PTRMI::NullObject>(Name(NULL_OBJECT_CLASS_NAME));
															// Add unmanaged error handler object owned by the manager GUID as a client
	if(addObject(Name(NULL_OBJECT_CLASS_NAME), Name(PTRMI_ERROR_HANDLER_OBJECT_NAME), managerGUID, &errorHandlerObject, false) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Create_Error_Handler);
	}
															// Create server interface for Error Handler object
															// Error Handler is created with this system's Manager as the client manager
	if((errorHandler = newObjectServerInterface(Name(NULL_OBJECT_CLASS_NAME), Name(PTRMI_ERROR_HANDLER_OBJECT_NAME), managerGUID)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Create_Error_Handler);
	}

	setErrorHandlerServerInterface(errorHandler);
															// Return OK
	return Status();
}


void ObjectManager::setErrorHandlerServerInterface(ServerInterfaceBase *serverInterface)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return;
	}

	errorHandlerServerInterface = serverInterface;
}


ServerInterfaceBase	*ObjectManager::getErrorHandlerServerInterface(void)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}

	return errorHandlerServerInterface;
}


ObjectManager::~ObjectManager(void)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return;
	}
															// Make sure all interfaces are deleted
	deleteAll();
}


bool ObjectManager::addObjectClientInterface(ClientInterfaceBase *clientInterface)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return false;
	}

	if(clientInterface == NULL)
	{
//		return Status(Status::Status_Error_Bad_Parameter);
		return false;
	}
															// Make sure client interface is specified
															// Add client interface
	objectClientInterfacesNamed.insert(NameClientInterfaceMap::value_type(clientInterface->getObjectName(), clientInterface));

	objectClientInterfacesGUID.insert(GUIDClientInterfaceMap::value_type(clientInterface->getInterfaceGUID(), clientInterface));

															// Record mapping between the remote Manager and the client interface

// Pip Option
/*
	PTRMI::GUID serverManagerGUID = clientInterface->getHostGUID();
	if(serverManagerGUID.isValidGUID())
	{
		if((status = addServerManagerClientInterface(&serverManagerGUID, clientInterface)).isFailed())
		{
			return false;
		}
	}
*/
															// Return OK
	return true;
}


Status ObjectManager::removeObjectClientInterface(ClientInterfaceBase *clientInterface)
{
	NameClientInterfaceMap::iterator	it;
	GUIDClientInterfaceMap::iterator	i;
	unsigned int						items = 0;

	if(clientInterface == NULL)
		return Status(Status::Status_Error_Bad_Parameter);


	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

															// Remove ServerManagerClientInterfaces index entry
	removeServerManagerClientInterface(clientInterface->getHostGUID(), clientInterface->getInterfaceGUID());

															// Erase GUID index entry
	if((i = objectClientInterfacesGUID.find(clientInterface->getInterfaceGUID())) != objectClientInterfacesGUID.end())
	{
		objectClientInterfacesGUID.erase(i);
	}

															// Get object's name
	const Name &objectName = clientInterface->getObjectName();
															// Find the first client interface for the named object
	if((it = objectClientInterfacesNamed.find(objectName)) != objectClientInterfacesNamed.end())
	{
															// Create set of all client interfaces for the named object
		for(items = 0; it != objectClientInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
															// If this is the specified client interface, erase it
			if(it->second == clientInterface)
			{
															// Erase this entry
				objectClientInterfacesNamed.erase(it);
															// Return OK
				return Status();
			}
															// Get next
			it++;
		}
	}

																// Not found, so return false
	return Status(Status::Status_Error_Failed_To_Find_Return_Client_Interface);
}

unsigned int ObjectManager::removeObjectClientInterfaces(const PTRMI::Name &objectName)
{
	NameClientInterfaceMap::iterator	it;
	unsigned int						items = 0;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return 0;
	}
															// Find the first client interface for the named object
	if((it = objectClientInterfacesNamed.find(objectName)) != objectClientInterfacesNamed.end())
	{
															// Create set of all client interfaces for the named object
		for(items = 0; it != objectClientInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
			objectClientInterfacesNamed.erase(it);
		}
	}

																// Return number removed
	return items;
}


Status ObjectManager::addObjectServerInterface(const PTRMI::GUID *clientManager, ServerInterfaceBase *serverInterface)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	if(clientManager == NULL || serverInterface == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

															// Create new object lock for server interface with immediate lock
	if((status = newServerInterfaceLock(serverInterface, true)).isFailed())
	{
		releaseServerInterface(serverInterface);
		deleteObjectServerInterface(serverInterface);
		return status;
	}
															// Add interface to name and GUID indexing.
	objectServerInterfacesNamed.insert(NameServerInterfaceMap::value_type(serverInterface->getObjectName(), serverInterface));
	objectServerInterfacesGUID.insert(GUIDServerInterfaceMap::value_type(serverInterface->getInterfaceGUID(), serverInterface));

															// Record mapping between the remote client manager that owns the object and the object on the server side
	if((status = addClientManagerServerInterface(clientManager, serverInterface)).isFailed())
	{
		releaseServerInterface(serverInterface);
		deleteObjectServerInterface(serverInterface);
		return status;
	}
															// Release the server interface lock obtained by newServerInterfaceLock()
	releaseServerInterface(serverInterface);
															// Return OK
	return Status();
}

// Pip Option
/*
unsigned int ObjectManager::getObjectServerInterfaces(const PTRMI::Name &objectName, ServerInterfaceSet &result)
{
	NameServerInterfaceMap::iterator	it;
	unsigned int						items = 0;

	MutexScope mutexScope(objectManagerMutex);
															// Find the first client interface for the named object
	if((it = objectServerInterfacesNamed.find(objectName)) != objectServerInterfacesNamed.end())
	{
		result.clear();
															// Create set of all client interfaces for the named object
		for(items = 0; it != objectServerInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
			result.push_back(it->second);

			it++;
		}
	}

	
															// Return number of items
	return items;
}
*/

Status ObjectManager::removeObjectServerInterface(ServerInterfaceBase *serverInterface)
{
	NameServerInterfaceMap::iterator		it;
	unsigned int							items = 0;
	//ObjectInfo							*	objectInfo;
	Status									status;

	if(serverInterface == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}
															// NOTE: server interface must already be locked / released by caller

															// Get object's name
	const Name &objectName = serverInterface->getObjectName();

															// Remove from server interface from client manager indexing
	if((status = removeClientManagerServerInterface(serverInterface->getClientManagerGUID(), serverInterface->getInterfaceGUID())).isFailed())
	{
		return status;
	}


	GUIDServerInterfaceMap::iterator i;
															// If indexed by GUID, remove GUID entry
	if((i = objectServerInterfacesGUID.find(serverInterface->getInterfaceGUID())) != objectServerInterfacesGUID.end())
	{
		objectServerInterfacesGUID.erase(i);
	}

															// Find the first in the sequence of server interface for the named object
	if((it = objectServerInterfacesNamed.find(objectName)) != objectServerInterfacesNamed.end())
	{
															// For each interface associated with given object name
		for(items = 0; it != objectServerInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
															// If this is the specified client interface, erase it from indexing
			if(it->second == serverInterface)
			{
															// Erase this entry
				objectServerInterfacesNamed.erase(it);
															// Return OK
				return Status();
			}
															// Get next
			it++;
		}

	}

															// Not found, so return error
	return Status(Status::Status_Error_Failed_To_Find_Server_Interface);
}


unsigned int ObjectManager::removeObjectServerInterfaces(const PTRMI::Name &objectName)
{
	NameServerInterfaceMap::iterator	it;
	unsigned int						items = 0;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return 0;
	}
															// Find the first client interface for the named object
	if((it = objectServerInterfacesNamed.find(objectName)) != objectServerInterfacesNamed.end())
	{
															// Create set of all client interfaces for the named object
		for(items = 0; it != objectServerInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
			objectServerInterfacesNamed.erase(it);
		}
	}
															// Return number removed
	return items;
}


MetaInterfaceBase * ObjectManager::getMetaInterface(const PTRMI::Name &objectClassName)
{
	NameMetaInterfaceMap::iterator it;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Find meta interface for class name
	it = objectMetaInterfaces.find(objectClassName);
															// If found
	if(it != objectMetaInterfaces.end())
	{
		MetaInterfaceBase *result = it->second;
															// Return the MetaInterface
		
		return result;
	}
															// Not found, so return NULL
	return NULL;
}


RemotableObject * ObjectManager::newObject(const PTRMI::Name &objectClassName, const PTRMI::Name &newObjectName, const PTRMI::GUID &clientManager)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Get class's meta interface
	MetaInterfaceBase *metaInterface = getMetaInterface(objectClassName);
															// If meta interface obtained
	if(metaInterface)
	{
		RemotableObject *object;
															// Get meta interface to instantiate a new object
		if((object = metaInterface->newObject()) == NULL)
		{
			
			return NULL;
		}
															// Store information about the object, including that we allocated it (managed)
		addObjectInfo(newObjectName, objectClassName, clientManager, object, true);
	}
															// Meta interface not found, so return NULL
	return NULL;
}


ServerInterfaceBase	* ObjectManager::newObjectServerInterface(const PTRMI::Name &objectClassName, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager)
{
	MetaInterfaceBase	*	metaInterface;
	RemotableObject		*	object;
	ObjectInfo			*	objectInfo;
	Name					hostName;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Get object class Meta Interface for named Object Class											
	if((metaInterface = getMetaInterface(objectClassName)) == NULL)
	{
		return NULL;
	}
															// Get existing object
	if((object = getObject(objectName)) == NULL)
	{
		return NULL;
	}
															// Get ObjectInfo for object
	if((objectInfo = getObjectInfo(objectName)) == NULL)
	{
		return NULL;
	}

															// Create an instance of the class's server interface
	ServerInterfaceBase *serverInterfaceBase = metaInterface->newServerInterface();

															// If interface created OK
	if(serverInterfaceBase)
	{
															// Set server interface's object
		serverInterfaceBase->setObject(object);
															// Set the name of the object
		serverInterfaceBase->setObjectName(objectName);
															// Extract protocol and host address to form host name
		objectName.getProtocolHostAddress(hostName);
															// Add the remote manager GUID to the host name
		hostName = clientManager;
															// Set the host name
		serverInterfaceBase->setHostName(hostName);
															// Set the remote client manager GUID
//		serverInterfaceBase->setClientManagerGUID(clientManager);
															// Increment reference count to object
		objectInfo->incrementReferenceCounter();
															// Add to set of server interfaces
		addObjectServerInterface(&clientManager, serverInterfaceBase);
															// Return the new server interface
		
		return serverInterfaceBase;
	}

															// Class not found, so return NULL
	return NULL;
}

ServerInterfaceBase	* ObjectManager::newObjectServerInterface(const PTRMI::Name &objectName, const PTRMI::GUID &clientManager)
{
	ObjectInfo	*objectInfo;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Get Object Info for named object
	if((objectInfo = getObjectInfo(objectName)) == NULL)
	{
		
		return NULL;
	}
															// Get new server interface based on the class name
	return newObjectServerInterface(objectInfo->getClassName(), objectName, clientManager);
}


ServerInterfaceBase * ObjectManager::newObjectAndServerInterface(const PTRMI::Name &objectClassName, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager)
{
	MetaInterfaceBase	*	metaInterface		= NULL;
	ServerInterfaceBase *	serverInterface		= NULL;
	ObjectInfo			*	objectInfo			= NULL;
	Status					status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}

	try
	{
															// Get MetaInterface for named class
		if((metaInterface = getMetaInterface(objectClassName)) == NULL)
		{
															// MetaInterface for class not found, so return NULL
			throw Status(Status::Status_Error_Failed_To_Find_Object_Meta_Interface);
		}
															// Create an instance of the class's object and server interface
		if((serverInterface = metaInterface->newObjectAndServerInterface()) == NULL)
		{
			throw Status(Status::Status_Error_Failed_To_Create_Server_Interface);
		}
															// Note: No need to lock server interface as it's not added to the object manager yet
															// Set the object name in the server interface
		serverInterface->setObjectName(objectName);

		Name hostName;
															// Extract protocol and host address from object URL
		objectName.getProtocolHostAddress(hostName);
															// Add GUID to host name
		hostName = clientManager;
															// Set the host name
		serverInterface->setHostName(hostName);
															// Set the GUID of the client's Manager object
//		serverInterface->setClientManagerGUID(clientManager);

															// Add object to ObjectManager
		if((objectInfo = addObject(objectClassName, objectName, clientManager, serverInterface->getObject(), true)) == NULL)
		{

			throw Status(Status::Status_Error_Failed_To_Create_Server_Interface);
		}
															// Add server interface to ObjectManager
		if((status = addObjectServerInterface(&clientManager, serverInterface)).isFailed())
		{
			throw status;
		}
															// Increment reference count to object
		objectInfo->incrementReferenceCounter();
	}
	catch(Status /*errorStatus*/)
	{
															// Failed, so delete server interface (and object)
		if(serverInterface)
		{
			deleteObjectServerInterface(serverInterface);
		}
															// Make sure object is deleted in case server interface ref counting did not delete it
		deleteObject(objectName);
															// Return NULL
		return NULL;
	}
	
															// Return server interface
	return serverInterface;
}


ObjectInfo *ObjectManager::addObjectInfo(const PTRMI::Name &objectName, const PTRMI::Name &className, const PTRMI::GUID &clientManager, RemotableObject *object, bool managed)
{
	NameObjectInfoMap::iterator it; 

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Make sure object info doesn't already exist
	if(getObjectInfo(objectName) != NULL)
		return NULL;
															// Create example object info
	ObjectInfo	objectInfo(objectName, className, clientManager, object, managed);

															// Add ObjectInfo to set of objects
	objects[objectName] = objectInfo;
															// Return the new object info just added
	return getObjectInfo(objectName);
}


PTRMI::ObjectInfo *ObjectManager::getObjectInfo(const PTRMI::Name &objectName)
{
	NameObjectInfoMap::iterator it;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
	
	if((it = objects.find(objectName)) != objects.end())
	{
		ObjectInfo *info = &(it->second);
		
		return info;
	}

	return NULL;
}


void ObjectManager::deleteObjectInfo(const PTRMI::Name &objectName)
{
	MutexScope mutexScope(objectManagerMutex);
	
	objects.erase(objectName);
}


RemotableObject	* ObjectManager::getObject(const PTRMI::Name &objectName)
{
	ObjectInfo *objectInfo;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Get information about object	
	if((objectInfo = getObjectInfo(objectName)) != NULL)
	{
		RemotableObject *result = objectInfo->getObject();
															// Return pointer to object
		return result;
	}
															// Not found, so return NULL
	return NULL;
}


bool ObjectManager::deleteObject(const PTRMI::Name &objectName)
{
	ObjectInfo			*	objectInfo;
	MetaInterfaceBase	*	metaInterfaceBase;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return false;
	}
															// Get the named object
												
	if((objectInfo = getObjectInfo(objectName)) == NULL)
	{
		
		return false;
	}
															// Get object's class name
	const Name &objectClass = objectInfo->getClassName();

															// If this object is managed (instantiated by the API)
	if(objectInfo->getManaged())
	{
															// Get object class's Meta Interface
		if((metaInterfaceBase = getMetaInterface(objectClass)) == NULL)
		{
															// Return false as Meta Interface not found
			
			return false;
		}
															// Delete the object (through meta interface because it knows the type)
		metaInterfaceBase->deleteObject(objectInfo->getObject());
	}
															// Delete information about object
	deleteObjectInfo(objectName);

	
															// Return deleted OK
	return true;
}


ClientInterfaceBase * ObjectManager::getObjectClientInterface(const PTRMI::GUID &interfaceGUID)
{
															// Find unique entry based on GUID
	GUIDClientInterfaceMap::iterator it;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
	
	if((it = objectClientInterfacesGUID.find(interfaceGUID)) != objectClientInterfacesGUID.end())
	{
		ClientInterfaceBase *result = it->second;
		
		return result;
	}

	

	return NULL;
}

ServerInterfaceBase * ObjectManager::getObjectServerInterface(const PTRMI::GUID &interfaceGUID)
{
															// Find unique entry based on GUID
	GUIDServerInterfaceMap::iterator it;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}

	if((it = objectServerInterfacesGUID.find(interfaceGUID)) != objectServerInterfacesGUID.end())
	{
		ServerInterfaceBase *result = it->second;
		
		return result;
	}

															// Return not found	
	return NULL;
}


ObjectInfo *ObjectManager::addObject(const PTRMI::Name &className, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager, RemotableObject *object, bool managed)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		Status status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		return NULL;
	}
															// Add whether object is managed (alloated by RMI) or non managed (externally allocated)
	return addObjectInfo(objectName, className, clientManager, object, managed);
}


PTRMI::Status ObjectManager::deleteObjectServerInterface(const PTRMI::GUID &serverInterfaceGUID)
{
	Status					status;
	ServerInterfaceBase *	serverInterface;

	{
		MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
		if(mutexScope.isLocked() == false)
		{
			return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		}
															// Get the server interface
		if((serverInterface = getObjectServerInterface(serverInterfaceGUID)) == NULL)
		{
			return Status(Status::Status_Error_Failed_To_Find_Server_Interface);
		}
	}
															// Delete the server interface
	return deleteObjectServerInterface(serverInterface);
}


PTRMI::Status ObjectManager::deleteObjectServerInterface(ServerInterfaceBase *serverInterface)
{
	Status	status;

	if(serverInterface == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Delete the server interface lock first
															// This means access to the server interface was exclusive and nothing else can lock it
															// Note: Because deleteServerInterfaceLock() is locking, it can not be called within the main object manager mutex
	if((status = deleteServerInterfaceLock(serverInterface)).isFailed())
	{
		return status;
	}
															// Mutex the remainder of this function
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}
															// Remove object server interface indexing
	if((status = removeObjectServerInterface(serverInterface)).isFailed())
	{
		return status;
	}
															// Discard and potentially delete the object
	discardObject(serverInterface->getObjectName());
															// Delete the server interface object
	delete serverInterface;
															// Return OK
	return Status();
}


Status ObjectManager::releaseServerManagerClientInterfaces(const PTRMI::GUID & serverManager)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return serverManagerClientInterfacesGUID.applyGroupItems<ObjectManager>(serverManager, this, &ObjectManager::releaseClientInterface);
}


Status ObjectManager::deleteClientManagerServerInterfaces(const PTRMI::GUID &clientManager)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return clientManagerServerInterfacesGUID.applyGroupItems<ObjectManager>(clientManager, this, static_cast<Status (ObjectManager::*)(ServerInterfaceBase *)>(&ObjectManager::deleteObjectServerInterface));
}


PTRMI::Status ObjectManager::discardObject(const Name &objectName)
{
	ObjectInfo			*	objectInfo;
	MetaInterfaceBase	*	metaInterface;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}
															// Get object info for discarded object
	if((objectInfo = getObjectInfo(objectName)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Object_Info);
	}
															// If reference count is zero, delete object
	if(objectInfo->decrementReferenceCounter() == 0)
	{
															// If allocation managed internally, delete object
		if(objectInfo->getManaged())
		{
															// Get Meta Interface for object
			if((metaInterface = getMetaInterface(objectInfo->getClassName())) == NULL)
			{
				return Status(Status::Status_Error_Failed_To_Find_Object_Meta_Interface);
			}
															// Delete object
			metaInterface->deleteObject(objectInfo->getObject());
															// Delete the object info
			deleteObjectInfo(objectName);
		}
	}
															// Return OK
	return Status();
}


Status ObjectManager::addServerManagerClientInterface(const PTRMI::GUID *serverManager, ClientInterfaceBase *clientInterface)
{
	if(serverManager == NULL || clientInterface == NULL || serverManager->isValidGUID() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return serverManagerClientInterfacesGUID.addItem(*serverManager, clientInterface->getInterfaceGUID(), clientInterface);
}


PTRMI::Status ObjectManager::addClientManagerServerInterface(const PTRMI::GUID *clientManager, ServerInterfaceBase *serverInterface)
{
															// Note: server interface must already be locked by caller
	if(clientManager == NULL || serverInterface == NULL || clientManager->isValidGUID() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return clientManagerServerInterfacesGUID.addItem(*clientManager, serverInterface->getInterfaceGUID(), serverInterface);
}


PTRMI::Status ObjectManager::removeServerManagerClientInterface(const PTRMI::GUID &serverManager, const PTRMI::GUID &clientInterfaceGUID)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return serverManagerClientInterfacesGUID.deleteItem(serverManager, clientInterfaceGUID);
}


PTRMI::Status ObjectManager::removeClientManagerServerInterface(const PTRMI::GUID &clientManager, const PTRMI::GUID &serverInterfaceGUID)
{

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return clientManagerServerInterfacesGUID.deleteItem(clientManager, serverInterfaceGUID);
}


PTRMI::Status ObjectManager::deleteObjectClientInterfaces(void)
{
	GUIDClientInterfaceMap::iterator	i;
	Status								status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}


	for(i = objectClientInterfacesGUID.begin(); i != objectClientInterfacesGUID.end(); i = objectClientInterfacesGUID.begin())
	{
		if(i->second)
		{
			if((status = deleteObjectClientInterface(i->second)).isFailed())
			{
				return status;
			}
		}
	}

	return status;
}


PTRMI::Status ObjectManager::deleteObjectServerInterfaces(void)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	return clientManagerServerInterfacesGUID.applyGroupKeys<ObjectManager>(this, &ObjectManager::deleteClientManagerServerInterfaces);
}


PTRMI::Status ObjectManager::deleteAll(void)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

															// Delete all client interfaces
	deleteObjectClientInterfaces();
															// Delete all server interfaces (and therefore objects with managed lifetime)
	deleteObjectServerInterfaces();
															// Error handler server interface has now been deleted, so set ptr to NULL
	setErrorHandlerServerInterface(NULL);
															// Return OK
	return status;
}


Status ObjectManager::newServerInterfaceLock(ServerInterfaceBase *serverInterface, bool immediateLock)
{
	return serverInterfaceLockManager.newLock(serverInterface, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT, &objectManagerMutex, immediateLock);
}


Status ObjectManager::deleteServerInterfaceLock(ServerInterfaceBase *serverInterface)
{
	return serverInterfaceLockManager.deleteLock(serverInterface, false, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT, &objectManagerMutex, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_SPIN_RETRIES, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_SPIN_TIMEOUT);
}


Status ObjectManager::lockServerInterface(ServerInterfaceBase *serverInterface)
{
	return serverInterfaceLockManager.lock(serverInterface, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT, &objectManagerMutex);
}


ServerInterfaceBase *ObjectManager::lockServerInterface(PTRMI::GUID &serverInterfaceGUID)
{
	ServerInterfaceBase *serverInterface = getObjectServerInterface(serverInterfaceGUID);

	if(serverInterfaceLockManager.lock(serverInterface, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT, &objectManagerMutex).isOK())
	{
		return serverInterface;
	}

	return NULL;
}


Status ObjectManager::releaseServerInterface(ServerInterfaceBase *serverInterface)
{
	return serverInterfaceLockManager.release(serverInterface, OBJECT_MANAGER_SERVER_INTERFACE_OBJECT_LOCK_TIMEOUT, &objectManagerMutex);
}


Status ObjectManager::releaseClientInterface(ClientInterfaceBase *clientInterface)
{
	if(clientInterface == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	return clientInterface->releaseStream();
}


Status ObjectManager::deleteHost(const PTRMI::GUID &hostGUID)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}
															// Release all ClientInterfaces associated with the specified host as a Server
															// This must be called before deleting ServerInterfaces because their deletion causes Pipe shutdown
															// and therefore shutdown of this thread when used with PipeTCP.
	releaseServerManagerClientInterfaces(hostGUID);
															// Delete all ServerInterfaces associated with the specified host as a Client
	deleteClientManagerServerInterfaces(hostGUID);

	return status;
}


Status ObjectManager::deleteAllHosts(void)
{
	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}
															// Delete resources associated with all connected client hosts
	clientManagerServerInterfacesGUID.applyGroupKeys(this, &ObjectManager::deleteHost, false);

															// Delete resources associated with all remote servers
	serverManagerClientInterfacesGUID.applyGroupKeys(this, &ObjectManager::deleteHost, false);

	return Status();
}


Status ObjectManager::pingInactiveHosts(void)
{
	std::vector<PTRMI::Host *>					lockedInactiveHosts;
	std::vector<PTRMI::Host *>::iterator		it;
	Status										status;
	PTRMI::Host								*	host;

															// Lock Object Manager
	{
		MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
		if(mutexScope.isLocked() == false)
		{
			return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
		}

		serverManagerClientInterfacesGUID.applyGroupKeys<ObjectManager, std::vector<PTRMI::Host *> &>(this, &ObjectManager::spinLockInactiveHost, lockedInactiveHosts, true);
	}


	for(it = lockedInactiveHosts.begin(); it != lockedInactiveHosts.end(); it++)
	{
		if(host = *it)
		{
			pingInactiveHost(host->getManagerGUID());
		}

		getManager().releaseHost(host);
	}

	return Status();
}


Status ObjectManager::spinLockInactiveHost(const PTRMI::GUID &hostGUID, std::vector<PTRMI::Host *> &resultHosts)
{
	PTRMI::Host *	host;

	if(host = getManager().spinLockHost(hostGUID))
	{
		if(host->getTimeSinceLastUsed() > 10.0)
		{
			resultHosts.push_back(host);
			return Status();
		}

		getManager().releaseHost(host);
	}

	return Status();
}


Status ObjectManager::pingInactiveHost(const PTRMI::GUID &hostGUID)
{
	ServerManagerClientInterfaceMap::Group				*	group;
	ServerManagerClientInterfaceMap::Group::iterator		it;
	ClientInterfaceBase									*	clientInterface;
	ClientInterfaceExtData								*	extData = NULL;
	Status													status;


	group = serverManagerClientInterfacesGUID.getGroup(hostGUID);

	if(group == NULL)
	{
		return Status(Status::Status_Error_Failed);
	}

	for(it = group->begin(); extData == NULL && it != group->end(); it++)
	{
		if(clientInterface = it->second)
		{
			if(extData = clientInterface->getExtData())
			{
				RemotePtr<Manager> remoteManager(getFirstServerManagerClientInterface<ManagerClientInterface>(hostGUID));

				if(remoteManager.isValid())
				{
					status = getManager().swapClientServerManagerInfo(remoteManager, clientInterface->getExtData());
				}
				else
				{
					getManager().releaseHost(hostGUID);
					Status::log(L"ObjectManager::pingInactiveHost() failed to get remote manager", L"");
					return Status(Status::Status_Error_Object_Ping_Failed);
				}
			}
		}
	}

	return status;
}


Status ObjectManager::getInfo(ManagerInfo &managerInfo)
{
	ObjectManagerInfoParam	param;
	Status					status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	param.setManagerInfo(managerInfo);

	getInfoObjects(managerInfo);
															// Write ClientInterfaces associated with each host
	serverManagerClientInterfacesGUID.applyGroupKeys<ObjectManager, ObjectManagerInfoParam &>(this, &ObjectManager::getInfoHostClientInterfaces, param, false);
															// Write ServerInterfaces associated with each host
	clientManagerServerInterfacesGUID.applyGroupKeys<ObjectManager, ObjectManagerInfoParam &>(this, &ObjectManager::getInfoHostServerInterfaces, param, false);

	return status;
}


Status ObjectManager::getInfoObjects(ManagerInfo &managerInfo)
{
	NameObjectInfoMap::iterator		it;
	std::wstring					objectURL;
	Status							status;
	ObjectInfo					*	objectInfo;
	std::wstring					objectClass;
	bool							objectManaged;
	unsigned int					objectReferences;
	ManagerInfo::KeyPath			objectsPath;
	ManagerInfo::KeyPath			path;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	managerInfo.getManagerStatusObjectsPath(objectsPath);

	for(it = objects.begin(); it != objects.end(); it++)
	{

		path = objectsPath;
		path.push_back(it->first.getURL().getString());

		if(objectInfo = getObjectInfo(it->first))
		{
			objectClass			= objectInfo->getClassName().getURL().getString();
			objectManaged		= objectInfo->getManaged();
			objectReferences	= objectInfo->getReferenceCounter();

			managerInfo.add(path, std::wstring(L"Class"), ManagerInfoVariantGraph::VGVariantWString(objectClass));
			managerInfo.add(path, std::wstring(L"Managed"), ManagerInfoVariantGraph::VGVariantBool(objectManaged));
			managerInfo.add(path, std::wstring(L"References"), ManagerInfoVariantGraph::VGVariantUnsignedInt(objectReferences));
		}
	}

	return status;
}


Status ObjectManager::getInfoHostClientInterfaces(const PTRMI::GUID &hostGUID, ObjectManagerInfoParam &param)
{
	ManagerInfo::KeyPath	path;
	std::wstring			hostGUIDString;
	Status					status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	if(param.getManagerInfo())
	{
		param.getManagerInfo()->getManagerStatusClientInterfacesPath(path);
		
		hostGUID.getHexString(hostGUIDString);
		path.push_back(hostGUIDString);

		param.setBaseKeyPath(path);

		serverManagerClientInterfacesGUID.applyGroupItems<ObjectManager, ObjectManagerInfoParam &>(hostGUID, this, &ObjectManager::getInfoHostClientInterface, param);
	}

	return status;
}


Status ObjectManager::getInfoHostClientInterface(ClientInterfaceBase * &objectInterface, ObjectManagerInfoParam &param)
{
	std::wstring			objectName;
	ManagerInfo::KeyPath	path;
	Status					status;
	PTRMI::GUID				interfaceGUID;
	std::wstring			interfaceGUIDString;
	std::wstring			objectURL;
	std::wstring			objectClass;
	double					objectIdle;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	path = param.getBaseKeyPath();

	if(objectInterface)
	{

		objectInterface->getInterfaceGUID().getHexString(interfaceGUIDString);
		path.push_back(interfaceGUIDString);

		objectClass	= objectInterface->getObjectClass().getURL().getString();
		objectURL	= objectInterface->getObjectName().getURL().getString();

		objectIdle	= objectInterface->getIdleTimeSeconds();

		param.getManagerInfo()->add(path, std::wstring(L"Class"), ManagerInfoVariantGraph::VGVariantWString(objectClass));
		param.getManagerInfo()->add(path, std::wstring(L"URL"), ManagerInfoVariantGraph::VGVariantWString(objectURL));
		param.getManagerInfo()->add(path, std::wstring(L"Idle"), ManagerInfoVariantGraph::VGVariantDouble(objectIdle));
	}

	return status;
}


Status ObjectManager::getInfoHostServerInterfaces(const PTRMI::GUID &hostGUID, ObjectManagerInfoParam &param)
{
	ManagerInfo::KeyPath	path;
	std::wstring			hostGUIDString;
	Status					status;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	if(param.getManagerInfo())
	{
		param.getManagerInfo()->getManagerStatusServerInterfacesPath(path);

		hostGUID.getHexString(hostGUIDString);
		path.push_back(hostGUIDString);

		param.setBaseKeyPath(path);

		clientManagerServerInterfacesGUID.applyGroupItems<ObjectManager, ObjectManagerInfoParam &>(hostGUID, this, &ObjectManager::getInfoHostServerInterface, param);
	}

	return status;
}


Status ObjectManager::getInfoHostServerInterface(ServerInterfaceBase * &objectInterface, ObjectManagerInfoParam &param)
{
	std::wstring			objectName;
	ManagerInfo::KeyPath	path;
	Status					status;
	PTRMI::GUID				interfaceGUID;
	std::wstring			interfaceGUIDString;
	std::wstring			objectURL;
	std::wstring			objectClass;
	double					objectIdle;

	MutexScope mutexScope(objectManagerMutex, OBJECT_MANAGER_LOCK_TIMEOUT);
	if(mutexScope.isLocked() == false)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Object_Manager);
	}

	path = param.getBaseKeyPath();

	if(objectInterface)
	{
		objectInterface->getInterfaceGUID().getHexString(interfaceGUIDString);
		path.push_back(interfaceGUIDString);

		objectClass	= objectInterface->getObjectClass().getURL().getString();
		objectURL	= objectInterface->getObjectName().getURL().getString();
		objectIdle	= objectInterface->getIdleTimeSeconds();

		param.getManagerInfo()->add(path, std::wstring(L"Class"), ManagerInfoVariantGraph::VGVariantWString(objectClass));
		param.getManagerInfo()->add(path, std::wstring(L"URL"), ManagerInfoVariantGraph::VGVariantWString(objectURL));
		param.getManagerInfo()->add(path, std::wstring(L"Idle"), ManagerInfoVariantGraph::VGVariantDouble(objectIdle));
	}

	return status;
}


} // End PTRMI namespace
