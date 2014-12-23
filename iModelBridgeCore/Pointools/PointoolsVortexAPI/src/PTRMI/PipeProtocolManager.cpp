
#include <PTRMI/PipeProtocolManager.h>
#include <PTRMI/PipeManagerExt.h>
#include <PTRMI/PipeManagerTCP.h>
#include <PTRMI/Stream.h>
#include <PTRMI/Host.h>
#include <PTRMI/Manager.h>


namespace PTRMI
{

PipeProtocolManager::PipeProtocolManager(void)
{
	initializePipeManagers();
}


void PipeProtocolManager::addPipeManager(PipeManager *pipeManager)
{
	MutexScope	mutexScope(mutex);

	if(mutexScope.isLocked() == false)
	{
		return;
	}

	if(pipeManager)
	{
		pipeManagers[pipeManager->getName()] = pipeManager;
	}
}


void PipeProtocolManager::newPipeManager(const PipeManagerName &name)
{
	PipeManager *pipeManager = NULL;
															// Note: Change this to poll defined manager types

															// If Ext, create an Ext (External)

#pragma message("***********************************")
#pragma message("PTRMI Pipe Protocols included:")

#ifdef POINTOOLS_PTRMI_PIPE_MANAGER_EXT

#pragma message("Pipe Included : Ext")

	if(name == L"Ext")
	{
		pipeManager = new PipeManagerExt(name);
	}

#endif
															// Only include TCP/IP specified for removal in the build
#ifdef POINTOOLS_PTRMI_PIPE_MANAGER_TCP

#pragma message("Pipe Included : TCP/IP")
															// If TCP, create a TCP
	if(name == L"TCP")
	{
		pipeManager = new PipeManagerTCP(name);
	}

#endif

#pragma message("***********************************")

															// If PipeManager created, add it to this protocol manager
	if(pipeManager)
	{
		addPipeManager(pipeManager);
	}
}


PipeManager * PipeProtocolManager::getPipeManager(const PipeManagerName &name)
{
	MutexScope	mutexScope(mutex);

	if(mutexScope.isLocked() == false)
	{
		return NULL;		
	}

	NamePipeManagerMap::iterator it;
															// If pipe manager found
	if((it = pipeManagers.find(name)) != pipeManagers.end())
	{
															// Return it
		return it->second;
	}
															// Not found, so return NULL
	return NULL;
}


void PipeProtocolManager::deletePipeManager(const PipeManagerName &name)
{
	NamePipeManagerMap::iterator it;

	MutexScope mutexScope(mutex);

	if(mutexScope.isLocked() == false)
		return;
															// If pipe manager found
	if((it = pipeManagers.find(name)) != pipeManagers.end())
	{
		if(it->second)
		{
			delete it->second;
		}
			
		pipeManagers.erase(it);
	}
}

unsigned int PipeProtocolManager::initializePipeManagers(void)
{
	unsigned int count = 0;

	MutexScope mutexScope(mutex);

	if(mutexScope.isLocked() == false)
		return 0;
															// Note: This should use polling fore extensibility in future
															// Create Ext (External) pipe manager
	newPipeManager(PipeManagerName(L"Ext"));
	count++;
															// Create TCP (TCP/IP) pipe manager
#ifndef NO_PTRMI_PIPE_MANAGER_TCP
 	newPipeManager(PipeManagerName(L"TCP"));
 	count++;
#endif
															// Return initialized OK
	return count;
}

unsigned int PipeProtocolManager::shutdownPipeManagers(void)
{
	NamePipeManagerMap::iterator	it;
	unsigned int					count = 0;

	MutexScope	mutexScope(mutex);

	if(mutexScope.isLocked() == false)
	{
		return 0;
	}

	for(it = pipeManagers.begin(); it != pipeManagers.end(); it++)
	{
		if(it->second)
		{
			delete it->second;

			count++;
		}

		pipeManagers.erase(it);
	}

	return count;
}

/*
Pipe *PipeProtocolManager::getPipe(const URL &pipeName, PipeManager **manager)
{
	NamePipeManagerMap::iterator		it;
	PipeManager						*	pipeManager;
	Pipe							*	pipe;
															// For each pipe manager
	for(it = pipeManagers.begin(); it != pipeManagers.end(); it++)
	{
		pipeManager = it->second;

		if(pipeManager)
		{
															// Look for pipe in manager
			if((pipe = pipeManager->getPipe(pipeName)) != NULL)
			{
				if(manager)
				{
					*manager = pipeManager;
				}

				return pipe;
			}
		}
	}

	if(manager)
	{
		*manager = NULL;
	}
															// Pipe not found in any pipe manager
	return NULL;
}
*/

/*
Pipe *PipeProtocolManager::getPipe(const PTRMI::GUID &clientManager, PipeManager **manager)
{
	NamePipeManagerMap::iterator		it;
	Pipe							*	pipe;
															// For all defined pipe protocols
	for(it = pipeManagers.begin(); it != pipeManagers.end(); it++)
	{
		PipeManager *pipeManager = it->second;

		if(pipeManager)
		{
															// If pipe with given host is present in the pipe manager, return it
			if(pipe = pipeManager->getPipe(clientManager))
			{
															// If returning the pipe manager owning pipe, set result
				if(manager)
				{
					*manager = pipeManager;
				}
															// Return the pipe
				return pipe;
			}
		}
	}
															// If manager result specified, set to NULL
	if(manager)
	{
		*manager = NULL;
	}
															// Return pipe not found in any protocol
	return NULL;
}
*/

/*
Status PipeProtocolManager::removePipe(Pipe *pipe)
{
	if(pipe == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	PipeManager						*	pipeManager;
	PipePipeManagerMap::iterator		it;

	MutexScope mutexScope(pipesMutex);

															// Remove pipe from pipe manager
	if((pipeManager = getPipeManager(pipe)) != NULL)
	{
		pipeManager->removePipe(pipe);
	}
															// Find pipe in the protocol manager
	if((it = pipes.find(pipe)) != pipes.end())
	{
		pipes.erase(it);
															// Return OK
		return Status();
	}
															// Pipe not found
	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Removal);
}
*/


Pipe *PipeProtocolManager::newPipe(Stream &stream)
{
	NamePipeManagerMap::iterator		it;
	Status								status;
	Pipe							*	pipe;
	Name								hostName;

	MutexScope mutexScope(mutex);

	if(mutexScope.isLocked() == false)
	{
		return NULL;
	}
															// For each PipeManager type
	for(it = pipeManagers.begin(); it != pipeManagers.end(); it++)
	{
		PipeManager *pipeManager = it->second;

		if(pipeManager)
		{	
															// Try to create pipe
															// NOTE: It is the PipeManager's responsibility to call addPipe() in the PipeProtocolManager
			if(pipe = pipeManager->newPipe(stream))
			{
															// Initialize Stream for use with this Pipe
				pipe->initializeStream(&stream);
															// Return pipe if successful
				return pipe;
			}
		}
	}
															// Unable to create Pipe
	return NULL;
}



/*
Status PipeProtocolManager::deletePipe(const PTRMI::GUID &clientManager)
{
	Pipe		*	pipe;
	Status			status;
	PipeManager	*	pipeManager;
															// Get pipe associated with the specified host
	if((pipe = getPipe(clientManager, &pipeManager)) == NULL || pipeManager == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Deletion);
	}
															// Attempt to delete the pipe
	if(pipeManager->deletePipe(pipe) == false)
	{
		return Status(Status::Status_Error_Failed_To_Delete_Pipe);
	}
															// Remove pipe indexing in pipe protocol manager
	removePipe(pipe);
															// Return OK
	return status;
}
*/

/*
Status PipeProtocolManager::discardPipe(Pipe *pipe)
{
	PipeManager *		pipeManager;
	Status				status;
	bool				deleted;
															// Get the pipe's pipe manager
	if((pipeManager = getPipeManager(pipe)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Discard);
	}
															// Discard the pipe. Note: Removal from protocol manager is done through Pipe destructor
	status = pipeManager->discardPipe(pipe, deleted);
															// Return status
	return status;
}
*/

// ********************** New Code ************************************



PipeManager *PipeProtocolManager::getPipeManager(Stream &stream, bool &pipeExist)
{
	Host		*	host;
	Pipe		*	pipe;
	PipeManager *	pipeManager = NULL;

	pipeExist = false;

	if((host = getManager().getHostManager().lockHost(stream.getHostName())) == NULL)
	{
		return NULL;
	}

	if(pipe = host->getPipe())
	{
		pipeManager = pipe->getPipeManager();

		pipeExist = true;
	}

	getManager().releaseHost(host);

	return pipeManager;
}


Status PipeProtocolManager::sendMessage(Stream &stream, Message::MessageType messageType)
{
	PipeManager *	pipeManager;
	bool			pipeExist;
															// Get pipe's PipeManager (Pipe obtained from Host)
	if(pipeManager = getPipeManager(stream, pipeExist))
	{
															// Call sendMesage() on Pipe's PipeManager
		return pipeManager->sendMessage(stream, messageType);
	}
	else
	{
															// Pipe not found or doesn't exist
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::beginSend(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->beginSend(stream);
	}
															// If Pipe doesn't exist
	if(pipeExist == false)
	{
															// Create a new pipe to host
		if(newPipe(stream))
		{
															// Get pipe's pipe manager
			if(pipeManager = getPipeManager(stream, pipeExist))
			{
															// Attempt to beginSend
				return pipeManager->beginSend(stream);
			}
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::endSend(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->endSend(stream);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::beginReceive(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->beginReceive(stream);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::endReceive(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->endReceive(stream);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::waitForResult(Stream &stream, PTRMI::Event &event)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->waitForResult(stream, event);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::signalEndMethod(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->signalEndMethod(stream);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::waitForEndMethod(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

	if(pipeManager = getPipeManager(stream, pipeExist))
	{
		return pipeManager->waitForEndMethod(stream);
	}
	else
	{
		if(pipeExist == false)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


PTRMI::Status PipeProtocolManager::initializeStream(Stream &stream)
{
	PipeManager *	pipeManager;
	bool			pipeExist;

															// Get pipe manager associated with host's pipe
	if(pipeManager = getPipeManager(stream, pipeExist))
	{
															// Initialize stream for use with existing pipe
		return pipeManager->initializeStream(stream);
	}
	else
	{
															// If pipe doesn't exist
		if(pipeExist == false)
		{
															// Create a new pipe initialized with this stream
			if(newPipe(stream))
			{
				return Status();
			}
		}
	}
															// Return error
	return Status(Status::Status_Error_Failed_To_Find_Pipe_Manager);
}


Status PipeProtocolManager::deletePipe(Pipe *pipe)
{
	PipeManager	*	pipeManager;

	if(pipe)
	{
		if(pipeManager = pipe->getPipeManager())
		{
			return pipeManager->deletePipe(pipe);
		}
	}

	return Status(Status::Status_Error_Failed_To_Delete_Pipe);
}

/*
Status PipeProtocolManager::deletePipe(const Name &hostName)
{
	Host		*	host;
	PipeManager	*	pipeManager;
	Pipe		*	pipe;

	if(hostName.isValidGUID() == false)
	{
		return Status(Status::Status_Error_Failed_To_Delete_Pipe);
	}
															// Lock the host for access on this thread
	if(host = getManager().lockHost(hostName))
	{
															// Get the host's Pipe
		if(pipe = host->getPipe())
		{
															// Get the Pipe's PipeManager
			if(pipeManager = pipe->getPipeManager())
			{
															// Request that the pipe manager deletes the pipe
				return pipeManager->deletePipe(pipe);
			}
		}
															// Host now has no Pipe
		host->setPipe(NULL);
															// Release the host
		getManager().releaseHost(host);
	}

	return Status(Status::Status_Error_Failed_To_Delete_Pipe);
}
*/

} // End PTRMI namespace