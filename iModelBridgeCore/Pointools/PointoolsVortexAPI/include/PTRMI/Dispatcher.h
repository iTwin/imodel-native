#pragma once

#include <PTRMI/Status.h>
#include <PTRMI/Message.h>
#include <PTRMI/DataBuffer.h>
#include <PTRMI/Stream.h>

namespace PTRMI
{
	class ServerInterfaceBase;

class Dispatcher
{

protected:

	ServerInterfaceBase *	getReceiverServerInterface	(const PTRMI::GUID &clientManager, const Name &receiver);

protected:

	Status					send						(Message &message);

	Status					receiveAndDispatchCall		(Message &header, const Name &hostName);

	Status					receiveAndDispatchReturn	(Message &header, const Name &hostName);

	Status					updateHostName				(Stream &stream, const Name &hostName, const PTRMI::GUID &guid);


public:

	Status					dispatch					(Message &header, const Name &hostName, Message::MessageType &messageType);
};


};