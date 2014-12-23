#include <PTRMI/Pipe.h>
#include <PTRMI/Manager.h>
#include <PTRMI/PipeProtocolManager.h>

namespace PTRMI
{


Pipe::Pipe(void)
{
	setPipeManager(NULL);
															// Set default messaging version to use between client and server
	messageVersion = Message::getDefaultMessageVersion();
}


Pipe::Pipe(const Name &hostName, PipeManager *pipeManager)
{
	setHostName(hostName);

	setPipeManager(pipeManager);
															// Set default messaging version to use between client and server
	messageVersion = Message::getDefaultMessageVersion();
}


Pipe::~Pipe(void)
{

}

void Pipe::setPipeManager(PipeManager *initPipeManager)
{
	pipeManager = initPipeManager;
}


PipeManager * Pipe::getPipeManager(void)
{
	return pipeManager;
}


DataBuffer * Pipe::getReceiveBuffer(void)
{
	return &receiveBuffer;
}


Status Pipe::setMessageVersion(Message::MessageVersion version)
{
	Status	status;

	if((status = lockPipeSendReceive()).isFailed())
	{
		return status;
	}

	messageVersion = version;

	return releasePipeSendReceive();
	
}


Message::MessageVersion Pipe::getMessageVersion(void)
{
	return messageVersion;
}

void Pipe::setStatus(Status initStatus)
{
	status = initStatus;
}


Status Pipe::getStatus(void)
{
	return status;
}


void Pipe::setHostName(const Name &initHostName)
{
	hostName = initHostName;
}


const Name &Pipe::getHostName(void)
{
	return hostName;
}


bool Pipe::updateHostName(const Name &initHostName)
{
															// Update to fill in missing URL or GUID in name
	return hostName.update(initHostName);
}

} // End PTRMI namespace