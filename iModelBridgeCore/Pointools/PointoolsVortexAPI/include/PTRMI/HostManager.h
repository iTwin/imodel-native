#pragma once

#include <PTRMI/ObjectLockManager.h>
#include <PTRMI/Host.h>
#include <PTRMI/Mutex.h>
#include <PTRMI/DoubleKeyMap.h>

#include <map>

#define PTRMI_HOST_MANAGER_LOCK_TIMEOUT		1000 * 10
#define PTRMI_HOST_MANAGER_LOCK_RETRIES		30
#define PTRMI_HOST_MANAGER_LOCK_SLEEP		1000

namespace PTRMI
{




class HostManager
{
protected:

	typedef	DoubleKeyMap<PTRMI::URL, PTRMI::GUID, Host>	URLGUIDHostMap;

protected:

	URLGUIDHostMap			hostMap;

public:

	Status					newHost				(const Name &hostName, const ManagerInfo *managerInfo, Host **lockedHost = false);

	Status					updateHostName		(const Name &hostName, bool &updated);

	Status					deleteHost			(const Name &hostName, bool preLocked = false);
	Status					deleteAll			(void);

	unsigned int			getNumHosts			(void);

	Host				*	lockHost			(const Name &hostName, bool *hostFound = NULL, unsigned long timeOut = PTRMI_HOST_MANAGER_LOCK_TIMEOUT);
	Host				*	lockOrNewLockedHost	(const Name &hostName, const ManagerInfo *managerInfo = NULL);

	Host				*	spinLockHost		(const Name &hostName, bool *hostFound = NULL);

	Status					releaseHost			(const Name &hostName);
	Status					releaseHost			(Host *host);
};


} // End PTRMI namespace