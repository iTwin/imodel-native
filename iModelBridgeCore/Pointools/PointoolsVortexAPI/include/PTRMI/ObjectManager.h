#pragma once

#include <PTRMI/ClientInterface.h>
#include <PTRMI/ServerInterface.h>
#include <PTRMI/Name.h>

#include <PTRMI/MetaInterface.h>
#include <PTRMI/ObjectInfo.h>

#include <PTRMI/NullObject.h>

#include <PTRMI/ObjectLockManager.h>

#include <PTRMI/KeyKeyMap.h>

#include <PTRMI/ManagerInfo.h>


#define OBJECT_MANAGER_LOCK_TIMEOUT	30 * 1000


namespace PTRMI
{

class Host;



class ObjectManagerInfoParam
{
protected:

	ManagerInfo				*	managerInfo;
	ManagerInfo::KeyPath		baseKeyPath;

public:

	void						setManagerInfo		(ManagerInfo &initManagerInfo)	{managerInfo = &initManagerInfo;}
	ManagerInfo				*	getManagerInfo		(void)							{return managerInfo;}

	void						setBaseKeyPath		(ManagerInfo::KeyPath &path)	{baseKeyPath = path;}
	ManagerInfo::KeyPath	&	getBaseKeyPath		(void)							{return baseKeyPath;}


};


class ObjectManager
{

protected:

	typedef std::map<const PTRMI::Name, MetaInterfaceBase *>						NameMetaInterfaceMap;

	typedef std::map<const PTRMI::Name, ObjectInfo>									NameObjectInfoMap;

	typedef std::multimap<const PTRMI::Name, ClientInterfaceBase *>					NameClientInterfaceMap;
	typedef std::multimap<const PTRMI::Name, ServerInterfaceBase *>					NameServerInterfaceMap;

//	typedef std::map<const PTRMI::GUID, ClientInterfaceBase *>						GUIDClientInterfaceMap;

//	typedef std::vector<const ClientInterfaceBase *>								ClientInterfaceSet;
//	typedef std::vector<const ServerInterfaceBase *>								ServerInterfaceSet;
	
	typedef KeyKeyMap<const PTRMI::GUID, const PTRMI::GUID, ServerInterfaceBase *>	ClientManagerServerInterfaceMap;
	typedef ClientManagerServerInterfaceMap::Group									GUIDServerInterfaceMap;

	typedef KeyKeyMap<const PTRMI::GUID, const PTRMI::GUID, ClientInterfaceBase *>	ServerManagerClientInterfaceMap;
	typedef ServerManagerClientInterfaceMap::Group									GUIDClientInterfaceMap;


	typedef ObjectLockManager<ServerInterfaceBase>									ServerInterfaceLockManager;

protected:
															// Main object manager Mutex
	Mutex								objectManagerMutex;
															// Meta interfaces for object classes
	NameMetaInterfaceMap				objectMetaInterfaces;
															// Map object names to locally created objects and info
	NameObjectInfoMap					objects;

															// Locally Instantiated ClientInterfaces indexed by object name
	NameClientInterfaceMap				objectClientInterfacesNamed;
															// Locally Instantiated ServerInterfaces indexed by object name
	NameServerInterfaceMap				objectServerInterfacesNamed;

															// Locally Instantiated Client interfaces indexed by Client Interface GUID
	GUIDClientInterfaceMap				objectClientInterfacesGUID;
															// Locally Instantiated Server interfaces indexed by Server Interface GUID
	GUIDServerInterfaceMap				objectServerInterfacesGUID;

															// Map ClientInterfaces associated with a remote server Host Manager (remote Manager GUID -> ClientInterface GUID -> ClientInterface Ptr)
	ServerManagerClientInterfaceMap		serverManagerClientInterfacesGUID;
															// Map ServerInterfaces associated with a remote client Host Manager (remote Manager GUID -> ClientInterface GUID -> ServerInterface Ptr)
	ClientManagerServerInterfaceMap		clientManagerServerInterfacesGUID;

															// Error handler object used to respond PTRMI protocol level errors
	PTRMI::NullObject					errorHandlerObject;
	ServerInterfaceBase				*	errorHandlerServerInterface;
	PTRMI::GUID							errorHandlerClientManagerGUID;

	ServerInterfaceLockManager			serverInterfaceLockManager;

protected:

	ObjectInfo						*	addObjectInfo							(const PTRMI::Name &objectName, const PTRMI::Name &className, const PTRMI::GUID &clientManager, RemotableObject *object, bool managed);

	bool								addObjectClientInterface				(ClientInterfaceBase *clientInterface);
	Status								removeObjectClientInterface				(ClientInterfaceBase *clientInterface);
	unsigned int						removeObjectClientInterfaces			(const PTRMI::Name &objectName);
															// Indexing from remote server manager GUIDs to the set of client interfaces associated with them
	Status								removeServerManagerClientInterface		(const PTRMI::GUID &serverManager, const PTRMI::GUID &clientInterfaceGUID);
															// Indexing from remote client manager GUIDs to the set of server interfaces associated with them
	Status								addClientManagerServerInterface			(const PTRMI::GUID *clientManager, ServerInterfaceBase * serverInterface);
	Status								removeClientManagerServerInterface		(const PTRMI::GUID &clientManager, const PTRMI::GUID &serverInterfaceGUID);

	Status								addObjectServerInterface				(const PTRMI::GUID *clientManager, ServerInterfaceBase *serverInterface);
	Status								removeObjectServerInterface				(ServerInterfaceBase *serverInterface);
	unsigned int						removeObjectServerInterfaces			(const PTRMI::Name &objectName);

	MetaInterfaceBase	*				getMetaInterface						(const PTRMI::Name &objectClassName);

	Status								createErrorHandlerServerInterface		(const PTRMI::GUID &managerGUID);
	void								setErrorHandlerServerInterface			(ServerInterfaceBase *serverInterface);

	Status								newServerInterfaceLock					(ServerInterfaceBase *serverInterface, bool immediateLock = false);
	Status								deleteServerInterfaceLock				(ServerInterfaceBase *serverInterface);

	Status								releaseClientInterface					(ClientInterfaceBase *clientInterface);

	Status								pingInactiveHost						(const PTRMI::GUID &hostGUID);

	Status								getInfoHostClientInterfaces				(const PTRMI::GUID &hostGUID, ObjectManagerInfoParam &param);
	Status								getInfoHostClientInterface				(ClientInterfaceBase * &objectInterface, ObjectManagerInfoParam &param);
	Status								getInfoHostServerInterfaces				(const PTRMI::GUID &hostGUID, ObjectManagerInfoParam &param);
	Status								getInfoHostServerInterface				(ServerInterfaceBase * &objectInterface, ObjectManagerInfoParam &param);
	Status								getInfoObjects							(ManagerInfo &managerInfo);

public:

										ObjectManager							(void);
									   ~ObjectManager							(void);

	Status								initialize								(const PTRMI::GUID &managerGUID);


	template<typename Meta> void		newMetaInterface						(const PTRMI::Name &objectClassName);

	ClientInterfaceBase				*	getObjectClientInterface				(const PTRMI::GUID &interfaceGUID);
	ServerInterfaceBase				*	getObjectServerInterface				(const PTRMI::GUID &interfaceGUID);

	RemotableObject					*	newObject								(const PTRMI::Name &objectClassName, const PTRMI::Name &newObjectName, const PTRMI::GUID &clientManager);
	template<typename Obj> Obj		*	newObject								(const PTRMI::Name &objectClassName, const PTRMI::Name &newObjectName, const PTRMI::GUID &clientManager);

	ObjectInfo						*	addObject								(const PTRMI::Name &objectClassName, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager, RemotableObject *object, bool managed = true);
	RemotableObject					*	getObject								(const PTRMI::Name &objectName);
	bool								deleteObject							(const PTRMI::Name &objectName);

	ObjectInfo						*	getObjectInfo							(const PTRMI::Name &objectName);
	void								deleteObjectInfo						(const PTRMI::Name &objectName);

	Status								addServerManagerClientInterface			(const PTRMI::GUID *serverManager, ClientInterfaceBase *clientInterface);

	Status								pingInactiveHosts						(void);

	Status								spinLockInactiveHost					(const PTRMI::GUID &hostGUID, std::vector<PTRMI::Host *> &resultHosts);

	// ************************************************************************

	template<typename Obj>
	typename ObjClientInfo<Obj>::I *	newObjectClientInterface				(const PTRMI::Name &objectClass, const PTRMI::Name &objectName, const PTRMI::Name *remoteServerInterfaceName = NULL);

	Status								deleteObjectClientInterface				(ClientInterfaceBase *clientInterface);


	Status								deleteObjectServerInterface				(const PTRMI::GUID &serverInterfaceGUID);
	Status								deleteObjectServerInterface				(ServerInterfaceBase *serverInterface);

	Status								releaseServerManagerClientInterfaces	(const PTRMI::GUID & serverManager);
	Status								deleteClientManagerServerInterfaces		(const PTRMI::GUID &clientManager);

	Status								discardObject							(const Name &objectName);

	ServerInterfaceBase				*	newObjectServerInterface				(const PTRMI::Name &objectClassName, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager);
	ServerInterfaceBase				*	newObjectServerInterface				(const PTRMI::Name &objectName, const PTRMI::GUID &clientManager);
	ServerInterfaceBase				*	newObjectAndServerInterface				(const PTRMI::Name &objectClassName, const PTRMI::Name &objectName, const PTRMI::GUID &clientManager);

	ServerInterfaceBase				*	getErrorHandlerServerInterface			(void);

	Status								deleteObjectClientInterfaces			(void);
	Status								deleteObjectServerInterfaces			(void);

	Status								lockServerInterface						(ServerInterfaceBase *serverInterface);
	ServerInterfaceBase				*	lockServerInterface						(PTRMI::GUID &serverInterfaceGUID);
	Status								releaseServerInterface					(ServerInterfaceBase *serverInterface);

	Status								deleteHost								(const PTRMI::GUID &hostGUID);
	Status								deleteAllHosts							(void);

	Status								deleteAll								(void);

//	unsigned int						getObjectClientInterfaces				(const PTRMI::Name &objectName, ClientInterfaceSet &result);
//	template<typename Obj> unsigned int	getObjectClientInterfaces				(const PTRMI::Name &objectName, typename typename ObjClientInfo<Obj>::Set &result);
//	unsigned int						getObjectServerInterfaces				(const PTRMI::Name &objectName, ServerInterfaceSet &result);
//	template<typename Obj> unsigned int	getObjectServerInterfaces				(const PTRMI::Name &objectName, typename typename ObjServerInfo<Obj>::Set &result);

	template<typename Obj>
	typename ObjClientInfo<Obj>::I *	getFirstObjectClientInterface			(const PTRMI::Name &objectName);
	//	template<typename Obj>
//	typename ObjServerInfo<Obj>::I *	getFirstObjectServerInterface			(const PTRMI::Name &objectName);

	template<typename I> unsigned int	getServerManagerClientInterfaces		(const PTRMI::GUID &hostGUID, std::vector<I *> &result);
	template<typename I> I *			getFirstServerManagerClientInterface	(const PTRMI::GUID &hostGUID);

	Status								getInfo									(ManagerInfo &managerInfo);
};



template<typename Obj>
inline typename ObjClientInfo<Obj>::I * ObjectManager::newObjectClientInterface(const PTRMI::Name &objectClass, const PTRMI::Name &objectName, const PTRMI::Name *remoteServerInterfaceName)
{
	ObjClientInfo<Obj>::I *	objectClientInterface;
	Name					hostName;

	MutexScope mutexScope(objectManagerMutex);
															// Create new Client Interface for the remote object
	if((objectClientInterface = new ObjClientInfo<Obj>::I) == NULL)
	{
		
		Status status(Status::Status_Error_Memory_Allocation);
	}
															// Set class name of remote object
	objectClientInterface->setObjectClass(objectClass);
															// Set name of remote object
	objectClientInterface->setObjectName(objectName);
															// Extract protocol and host address as URL host name
	objectName.getProtocolHostAddress(hostName);
															// Set host name
	objectClientInterface->setHostName(hostName);

															// If remote server interface known at this time
	if(remoteServerInterfaceName && remoteServerInterfaceName->isValid())
	{
															// Bind to remote server interface
		objectClientInterface->setRemoteInterface(*remoteServerInterfaceName);
	}

															// Add the new Client Interface to keep a record
	addObjectClientInterface(objectClientInterface);

	
															// Return the client interface
	return objectClientInterface;
}


inline Status ObjectManager::deleteObjectClientInterface(ClientInterfaceBase *clientInterface)
{
	Status	status;

	MutexScope mutexScope(objectManagerMutex);

	if(clientInterface)
	{
		if((status = removeObjectClientInterface(clientInterface)).isFailed())
			return status;

		delete clientInterface;

		return Status();
	}

	return Status(Status::Status_Error_Bad_Parameter);
}


template<typename Obj>
inline typename ObjClientInfo<Obj>::I * ObjectManager::getFirstObjectClientInterface(const PTRMI::Name &name)
{
	NameClientInterfaceMap::iterator	i;
	ObjClientInfo<Obj>::I *				result = NULL;

	MutexScope mutexScope(objectManagerMutex);
															// Find named object's server interface
	i = objectClientInterfacesNamed.find(name);
															// If found
	if(i != objectClientInterfacesNamed.end())
	{
															// Down cast to valid derived type (or NULL if invalid)
		result = dynamic_cast<ObjClientInfo<Obj>::I *>(i->second);
	}
															// Not found, so return NULL
	return result;
}


template<typename I>
inline unsigned int ObjectManager::getServerManagerClientInterfaces(const PTRMI::GUID &hostGUID, std::vector<I *> &result)
{
	//ServerManagerClientInterfaceMap::Group	*	group;
	//ClientInterfaceBase						*	clientInterface;

	MutexScope mutexScope(objectManagerMutex);

	return serverManagerClientInterfacesGUID.getGroupItemsOfType(hostGUID, result);
}


template<typename I> I *ObjectManager::getFirstServerManagerClientInterface(const PTRMI::GUID &hostGUID)
{
	std::vector<I *>	clientInterfaces;

	if(getServerManagerClientInterfaces(hostGUID, clientInterfaces) > 0)
	{
		return clientInterfaces[0];
	}

	return NULL;
}


/*
template<typename Obj>
inline typename ObjServerInfo<Obj>::I * ObjectManager::getFirstObjectServerInterface(const PTRMI::Name &name)
{
	NameServerInterfaceMap::iterator	i;
	ObjServerInfo<Obj>::I *				result = NULL;

	MutexScope mutexScope(objectManagerMutex);
															// Find named object's server interface
	i = objectServerInterfacesNamed.find(name);
															// If found
	if(i != objectServerInterfacesNamed.end())
	{
															// Down cast to valid derived type (or NULL if invalid)
		result = dynamic_cast<ObjServerInfo<Obj>::I *>(i->second);
	}

	
															// Not found, so return NULL
	return NULL;
}
*/


template<typename Obj>
inline void ObjectManager::newMetaInterface(const PTRMI::Name &objectClassName)
{
	MetaInterface<Obj> *metaInterface;

	MutexScope mutexScope(objectManagerMutex);
															// If MetaInterface does not exist for object class
	if(getMetaInterface(objectClassName) == NULL)
	{
															// Create new meta class for specified object class

		if((metaInterface = new MetaInterface<Obj>(objectClassName)) == NULL)
		{
															// Memory allocation error
			Status status(Status::Status_Error_Memory_Allocation);
		}
															// Add the meta interface to thhe object manager
		objectMetaInterfaces.insert(NameMetaInterfaceMap::value_type(objectClassName, metaInterface));
	}

	
}


template<typename Obj>
inline Obj *ObjectManager::newObject(const PTRMI::Name &newObjectClassName, const PTRMI::Name &newObjectName, const PTRMI::GUID &clientManager)
{
	Obj *object = NULL;

	MutexScope mutexScope(objectManagerMutex);
															// Get meta interface for given object class name
	MetaInterface<Obj> *metaInterface = dynamic_cast<MetaInterface<Obj> *>(getMetaInterface(newObjectClassName));
															// If obtained
	if(metaInterface)
	{
		// If object created
		if(object = metaInterface->newObjectTyped())
		{
															// Store info about the named object including that it is allocated by this API (managed)
			addObjectInfo(newObjectName, newObjectClassName, clientManager, object, true);
		}
	}

															// Meta interface not found for class, so return NULL
	return object;
}

/*
template<typename Obj>
unsigned int ObjectManager::getObjectClientInterfaces(const PTRMI::Name &objectName, typename ObjClientInfo<Obj>::Set &result)
{
	NameClientInterfaceMap::iterator it;
	unsigned int									items = 0;

	MutexScope mutexScope(objectManagerMutex);
															// Find the first client interface for the named object
	if((it = objectClientInterfacesNamed.find(objectName)) != objectClientInterfacesNamed.end())
	{
		result.clear();
															// Create set of all client interfaces for the named object
		for(items = 0; it != objectClientInterfacesNamed.end() && (it->first == objectName) ; items++)
		{
			result.push_back(dynamic_cast<ObjClientInfo<Obj>::I *>(it->second));

			it++;
		}
	}

	
															// Return number of items
	return items;
}
*/

/*
template<typename Obj>
unsigned int ObjectManager::getObjectServerInterfaces(const PTRMI::Name &objectName, typename ObjServerInfo<Obj>::Set &result)
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
			result.push_back(dynamic_cast<ObjServerInfo<Obj>::I *>(it->second));

			it++;
		}
	}

	
															// Return number of items
	return items;
}
*/

} // End PTRMI namespace
