#pragma once

#include <string>
#include <map>

#include <PTRMI/Pipe.h>
#include <PTRMI/ObjectLockManager.h>

#include <list>


namespace PTRMI
{

class Stream;
															// Main mutex timeout is 30 seconds
const unsigned long PIPE_MANAGER_OBJECT_LOCK_TIMEOUT		= 1000 * 30;
															// Spin retries 60 times every one second
const unsigned int	PIPE_MANAGER_OBJECT_LOCK_SPIN_RETRIES	= 60;
const unsigned int	PIPE_MANAGER_OBJECT_LOCK_SPIN_TIMEOUT	= 1000;


class PipeManager
{
public:

	typedef std::wstring						PipeManagerName;

	typedef std::list<Pipe *>					PipeSet;

//	typedef std::map<URL, Pipe *>				PipeHostAddressPipeMap;
//	typedef std::map<PTRMI::GUID, Pipe *>		GUIDPipeMap;

protected:

	Mutex								pipeManagerMutex;

	PipeManagerName						pipeManagerName;

	PipeSet								pipes;

//	PipeHostAddressPipeMap				pipesHostAddress;
//	GUIDPipeMap							pipesGUID;


protected:

//	bool								addPipe					(Pipe *pipe);
//	bool								removePipe				(Pipe *pipe);

	Status								addPipe					(Pipe *pipe);
	Status								removePipe				(Pipe *pipe);

	Pipe							*	getPipe					(Stream &stream);
	Pipe							*	getPipe					(const Name &hostName);

public:

										PipeManager				(const PipeManagerName &managerName);
	virtual							   ~PipeManager				(void);

	void								setName					(const PipeManagerName &managerName);
	const PipeManagerName			&	getName					(void);

//	template<typename T> T			*	getPipe					(const URL &pipeHostAddress);
//	template<typename T> T			*	getPipe					(const PTRMI::GUID &guid);

//	Pipe *								getPipe					(const Name &hostName);
//	Pipe *								getPipe					(const URL &hostAddress);
//	Pipe *								getPipe					(const PTRMI::GUID &guid);

	Pipe							*	newPipe					(Stream &stream);
	virtual Pipe					*	newPipe					(const Name &hostName) = 0;

	Status								lockPipeSendReceive		(Pipe *pipe);
	Status								releasePipeSendReceive	(Pipe *pipe);

	Status								lockPipeSend			(Pipe *pipe);
	Status								releasePipeSend			(Pipe *pipe);

	Status								lockPipeReceive			(Pipe *pipe);
	Status								releasePipeReceive		(Pipe *pipe);

//	virtual bool						deletePipe				(const URL &pipeHostAddress) = 0;
//	virtual bool						deletePipe				(const PTRMI::GUID &hostGUID) = 0;
//	virtual bool						deletePipe				(Pipe *pipe) = 0;


//	virtual Status						discardPipe				(Pipe *pipe, bool &deleted);

	Status								initializeStream		(Stream &stream);

	virtual Status						deletePipe				(Pipe * pipe);

	Status								beginSend				(Stream &stream);
	Status								endSend					(Stream &stream);
	Status								beginReceive			(Stream &stream);
	Status								endReceive				(Stream &stream);

	Status								sendMessage				(Stream &stream, Message::MessageType messageType);

	Status								waitForResult			(Stream &stream, PTRMI::Event &event);
	Status								signalEndMethod			(Stream &stream);
	Status								waitForEndMethod		(Stream &stream);
};


/*
template<typename T>
inline T *PipeManager::getPipe(const URL &hostAddress)
{
	MutexScope mutexScope(pipeManagerMutex);

	PipeHostAddressPipeMap::iterator it;

	if((it = pipesHostAddress.find(hostAddress)) != pipesHostAddress.end())
	{
		return dynamic_cast<T *>(it->second);
	}

	return NULL;
}
*/

/*
template<typename T>
inline T*PipeManager::getPipe(const PTRMI::GUID &guid)
{
	MutexScope mutexScope(pipeManagerMutex);

	GUIDPipeMap::iterator it;

	if((it = pipesGUID.find(guid)) != pipesGUID.end())
	{
		return dynamic_cast<T *>(it->second);
	}

	return NULL;
}
*/


}