
#include <PTRMI/Manager.h>
#include <PTRMI/PipeManager.h>


namespace PTRMI
{

PipeManager::PipeManager(const PipeManagerName &initPipeManagerName)
{
	setName(initPipeManagerName);
}


PipeManager::~PipeManager(void)
{

}


void PipeManager::setName(const PipeManagerName &initName)
{
	pipeManagerName = initName;
}


const PTRMI::PipeManager::PipeManagerName & PipeManager::getName(void)
{
	return pipeManagerName;
}


Pipe *PipeManager::newPipe(Stream &stream)
{
	const Name	&hostName = stream.getHostName();

	return newPipe(hostName);
}


Status PipeManager::addPipe(Pipe *pipe)
{
	Host	*	host;

	if(pipe == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Add_Pipe_Manager_Pipe);
	}
															// Mutex this manager
	MutexScope mutexScope(pipeManagerMutex);

	if(mutexScope.isLocked())
	{
															// Lock Host for access
		if(host = getManager().lockHost(pipe->getHostName()))
		{
															// Set Pipe in Host
			host->setPipe(pipe);
															// Add pipe to Manager's set
			pipes.push_back(pipe);
															// Release host
			getManager().releaseHost(host);
															// Return OK
			return Status();
		}
	}

	return Status(Status::Status_Error_Failed_To_Add_Pipe_Manager_Pipe);
}


Status PipeManager::removePipe(Pipe *pipe)
{
	if(pipe == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Deletion);
	}
															// Mutex this manager
	MutexScope mutexScope(pipeManagerMutex);

	if(mutexScope.isLocked())
	{
		PipeSet::iterator it = std::find(pipes.begin(), pipes.end(), pipe);
		if(it != pipes.end())
		{
			pipes.erase(it);

			return Status();
		}
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Deletion);
}

/*
bool PipeManager::addPipe(Pipe *pipe)
{
	bool	added = false;

	MutexScope mutexScope(pipeManagerMutex);

	if(pipe)
	{
		if(pipe->hasHostAddress())
		{
			pipesHostAddress[pipe->getHostAddress()] = pipe;
			added = true;
		}

		if(pipe->hasHostGUID())
		{
			pipesGUID[pipe->getHostGUID()] = pipe;
			added = true;
		}
	}

	return added;
}


bool PipeManager::removePipe(Pipe *pipe)
{
															// Note: Pipe lock must have been deleted by caller
	bool removed = false;
															// Make sure pipe is specified
	if(pipe == NULL)
	{
		return false;
	}
															// Validate pipe
	if(pipe->hasHostAddress() == false && pipe->hasHostGUID() == false)
	{
		return false;
	}
															// Mutex the remainder of this function
	MutexScope mutexScope(pipeManagerMutex);
															// Note: Pipe must already be locked by caller
	if(pipe->hasHostAddress())
	{
		PipeHostAddressPipeMap::iterator it;

		if((it = pipesHostAddress.find(pipe->getHostAddress())) != pipesHostAddress.end())
		{
			pipesHostAddress.erase(it);
			removed = true;
		}
	}

	if(pipe->hasHostGUID())
	{
		GUIDPipeMap::iterator it;

		if((it = pipesGUID.find(pipe->getHostGUID())) != pipesGUID.end())
		{
			pipesGUID.erase(it);
			removed = true;
		}
	}
															// Remove pipe from Host (Potentially deleting Host if no pipes remain)
//	getManager().getHostManager().removeHostPipe(pipe);
															// Return whether pipe items were fully removed
	return removed;
}
*/

/*
Pipe * PipeManager::getPipe(const URL &hostAddress)
{
	return getPipe<Pipe>(hostAddress);
}
*/

/*
Pipe * PipeManager::getPipe(const PTRMI::GUID &guid)
{
	return getPipe<Pipe>(guid);
}
*/

Pipe *PipeManager::getPipe(const Name &hostName)
{
	Host	*	host;
	Pipe	*	pipe = NULL;

	if(host = getManager().lockHost(hostName))
	{
		pipe = host->getPipe();

		getManager().releaseHost(host);
	}

	return pipe;
}


Pipe *PipeManager::getPipe(Stream &stream)
{
	return getPipe(stream.getHostName());
}


Status PipeManager::beginSend(Stream &stream)
{
	Pipe *	pipe;
															// Get pipe to communicate with host	
	if(pipe = getPipe(stream))
	{
															// If pipe has an error status, return it
		if(pipe->getStatus().isFailed())
		{
			return pipe->getStatus();
		}
															// Bind stream to pipe
		pipe->initializeStream(&stream);

		return pipe->beginSend();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::endSend(Stream &stream)
{
	Pipe *	pipe;
	
	if(pipe = getPipe(stream))
	{
		return pipe->endSend();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::beginReceive(Stream &stream)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->beginReceive();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);

}


Status PipeManager::endReceive(Stream &stream)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->endReceive();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::sendMessage(Stream &stream, Message::MessageType messageType)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->sendMessage(stream, messageType);
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::waitForResult(Stream &stream, PTRMI::Event &event)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->waitForResult(event);
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::signalEndMethod(Stream &stream)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->signalEndMethod();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::waitForEndMethod(Stream &stream)
{
	Pipe *	pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->waitForEndMethod();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
}


Status PipeManager::initializeStream(Stream &stream)
{
	Pipe *pipe;

	if(pipe = getPipe(stream))
	{
		return pipe->initializeStream(&stream);
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Initialization);
}


PTRMI::Status PipeManager::deletePipe(Pipe *pipe)
{
	if(pipe)
	{
															// Remove pipe indexing
		return removePipe(pipe);
	}

	return Status(Status::Status_Error_Failed_To_Delete_Pipe);
}


/*
bool PipeManager::deletePipe(Pipe *pipe)
{
	if(pipe == NULL)
	{
		return false;
	}
															// Remove pipe indexing
	if(removePipe(pipe) == false)
	{
		return false;
	}
															// Delete the pipe
	delete pipe;
															// Return OK
	return true;
}
*/

/*
bool PipeManager::deletePipe(const URL &name)
{
	Pipe *	pipe;
															// Get the pipe based on host address	
	if((pipe = getPipe(name)) == NULL)
	{
		return false;
	}
															// Delete the pipe
	return deletePipe(pipe);
}
*/

/*
bool PipeManager::deletePipe(const PTRMI::GUID &hostGUID)
{
	Pipe *	pipe;
															// Get the pipe based on host address	
	if((pipe = getPipe(hostGUID)) == NULL)
	{
		return false;
	}
															// Delete the pipe
	return deletePipe(pipe);
}
*/

/*
Status PipeManager::discardPipe(Pipe *pipe, bool &deleted)
{
	Status	status;

	if(pipe == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// If pipe is specified
	if((status = pipe->discard(deleted)).isFailed())
	{
		return status;
	}
															// If pipe was deleted
	if(deleted)
	{
															// Remove pipe indexing
		removePipe(pipe);
	}
															// Return OK
	return Status();
}
*/



} // End PTRMI namespace


