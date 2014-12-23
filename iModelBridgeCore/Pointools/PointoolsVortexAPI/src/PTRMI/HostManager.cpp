
#include <PTRMI/HostManager.h>
#include <PTRMI/Pipe.h>
#include <PTRMI/Status.h>

#include <PTRMI/DoubleKeyMap.h>

namespace PTRMI
{


Status HostManager::newHost(const Name &hostName, const ManagerInfo *managerInfo, Host **lockedHost)
{
	Host	*	host;
	Status		status;

	if(hostName.isValidURL())
	{
		if((status = hostMap.newItem(hostName.getURL(), &host)).isOK())
		{
			if(hostName.isValidGUID())
			{
				status = hostMap.addIndex(hostName.getURL(), hostName.getGUID());
			}
		}
	}
	else
	{
		if(hostName.isValidGUID())
		{
			status = hostMap.newItem(hostName.getGUID(), &host);
		}
	}

	if(host && managerInfo)
	{
		host->setManagerInfo(*managerInfo);
	}

	if(lockedHost == NULL)
	{
		hostMap.releaseItem(host);
	}
	else
	{
		*lockedHost = host;
	}


	return status;
}


Status HostManager::updateHostName(const Name &hostName, bool &updated)
{
	Host	*	host;

	updated = false;
															// Lock Host for access
	if(host = lockHost(hostName))
	{
															// Update Host's internal host name.
		if(host->updateName(hostName) == false)
		{
															// If no update occured, just exit
			releaseHost(host);

			return Status();
		}
															// Release host
		releaseHost(host);
															// Return updated
		updated = true;
															// Update HostManager indexing
		return hostMap.updateIndex(host->getName().getURL(), host->getName().getGUID());
	}

															// Return failed to lock host
	return Status(Status::Status_Error_Failed_To_Lock_Host);
}


Status HostManager::deleteHost(const Name &hostName, bool preLocked)
{
															// If GUID is valid
	if(hostName.isValidGUID())
	{
															// Delete GUID and URL based entry
		return hostMap.deleteItem(hostName.getGUID(), preLocked);
	}
															// If URL is valid	
	if(hostName.isValidURL())
	{
															// Delete GUID and URL based entry
		return hostMap.deleteItem(hostName.getURL(), preLocked);
	}

	return Status(Status::Status_Warning_Failed_To_Find_Host);
}


Host *HostManager::spinLockHost(const Name &hostName, bool *hostFound)
{
	return lockHost(hostName, hostFound, 0);
}


Host *HostManager::lockHost(const Name &hostName, bool *hostFound, unsigned long timeOut)
{
	Host *	host	= NULL;
	bool	found	= false;

															// Lock based on GUID as a preference
	if(hostName.isValidGUID())
	{
															// Attempt to find and lock by GUID
		host = hostMap.lockItem(hostName.getGUID(), &found, timeOut);
	}

	if(found == false && host == NULL)
	{
															// Alternatively lock based on URL if GUID is not registered yet
		host = hostMap.lockItem(hostName.getURL(), &found, timeOut);
	}
															// If return value requested, return it
	if(hostFound)
	{
		*hostFound = found;
	}
															// If host locked, return host
	if(host)
	{
		return host;
	}
															// If found, lock failed. If not found, host wasn't found.
	if(found)
	{
		if(timeOut > 0)
		{
			Status status(Status::Status_Error_Failed_To_Lock_Host);
			Status::log(L"Warning: HostManager::lockHost() failed to lock host", L"");
		}
	}
	else
	{
		Status status(Status::Status_Warning_Failed_To_Find_Host);
		Status::log(L"Warning: HostManager::lockHost() failed to find host for lock", L"");
	}
															// Return failed
	return NULL;
}


Host *HostManager::lockOrNewLockedHost(const Name &hostName, const ManagerInfo *managerInfo)
{
	Host	*	host;
	Status		status;

	if(hostName.isValidGUID())
	{
		if(host = hostMap.lockOrNewLockedItem(hostName.getGUID()))
		{
			if(hostName.isValidURL())
			{
				status = hostMap.addIndex(hostName.getGUID(), hostName.getURL());
			}
		}
	}
	else
	{
		if(hostName.isValidURL())
		{
			host = hostMap.lockOrNewLockedItem(hostName.getURL());
		}
	}

	if(status.isOK() && host && managerInfo)
	{
		host->setManagerInfo(*managerInfo);
	}
															// If host found, return it
	if(host)
	{
		return host;
	}
															// Log error status
	Status statusError(Status::Status_Error_Failed_To_Lock_Host);
	Status::log(L"Warning: HostManager::lockOrNewLockedHost() failed to create or lock host", L"");

															// Return failed
	return NULL;
}


Status HostManager::releaseHost(const Name &hostName)
{
	bool	found;
	Status	status;
															// If given hostName has GUID
	if(hostName.isValidGUID())
	{
															// Try to release the host based on the GUID
		if((status = hostMap.releaseItem(hostName.getGUID(), &found)).isOK())
		{
															// Return OK
			return status;
		}
															// If found but failed, return error
		if(found)
		{
			return Status(Status::Status_Error_Failed_To_Release_Host);
		}
	}
															// Given name has no valid GUID
															// Try to release based on URL name
	if((status = hostMap.releaseItem(hostName.getURL())).isOK())
	{
		return status;
	}
															// Return failed
	return Status(Status::Status_Error_Failed_To_Release_Host);
}


Status HostManager::releaseHost(Host *host)
{
	return hostMap.releaseItem(host);
}


Status HostManager::deleteAll(void)
{
	return hostMap.deleteAll();
}


unsigned int HostManager::getNumHosts(void)
{
	return hostMap.getNumItems();
}




} // End PTRMI namespace
