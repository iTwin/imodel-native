#pragma once 


//#include <PTRMI/GUID.h>
//#include <PTRMI/Mutex.h>
//#include <PTRMI/URL.h>
#include <PTRMI/Message.h>
#include <PTRMI/DataBuffer.h>
//#include <PTRMI/Event.h>

namespace PTRMI
{

class PipeManager;
class Stream;
class Event;

class Pipe
{

protected:

	Name							hostName;

	PipeManager					*	pipeManager;

	DataBuffer						receiveBuffer;

	Message::MessageVersion			messageVersion;

	Status							status;

protected:


public:

									Pipe					(void);
									Pipe					(const Name &hostName, PipeManager *initPipeManager);
	virtual						   ~Pipe					(void);

	void							setStatus				(Status initStatus);
	Status							getStatus				(void);

	void							setHostName				(const Name &initHostName);
	const Name					&	getHostName				(void);

	bool							updateHostName			(const Name &initHostName);

	virtual Status					beginSend				(void) = 0;
	virtual Status					endSend					(void) = 0;
	virtual Status					beginReceive			(void) = 0;
	virtual Status					endReceive				(void) = 0;

	virtual Status					lockPipeSendReceive		(void) = 0;
	virtual Status					releasePipeSendReceive	(void) = 0;


	void							setPipeManager			(PipeManager * initPipeManager);
	PipeManager					 *	getPipeManager			(void);

	virtual Status					initializeStream		(Stream * stream) = 0;

	Status							setMessageVersion		(Message::MessageVersion version);
	Message::MessageVersion			getMessageVersion		(void);

	virtual Status					sendMessage				(Stream &stream, Message::MessageType type) = 0;
	virtual Status					receiveMessage			(void) = 0;

	virtual DataBuffer::DataSize	receiveData				(DataBuffer &dataBuffer, DataBuffer::DataSize numBytesMin) = 0;

	virtual Status					waitForResult			(PTRMI::Event &event) = 0;

	virtual Status					signalEndMethod			(void) = 0;
	virtual Status					waitForEndMethod		(void) = 0;

	virtual DataBuffer			*	getReceiveBuffer		(void);
};
	
} // End PTRMI namespace