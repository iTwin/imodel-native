#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Dispatcher.h>

#include <PTRMI/Manager.h>
#include <PTRMI/ObjectManager.h>

namespace PTRMI
{



PTRMI::Status Dispatcher::send(Message &message)
{
	Status status;

	return status;
}


PTRMI::Status Dispatcher::dispatch(Message &header, const Name &hostName, Message::MessageType &messageType)
{
	Status	status;

	messageType = header.getMessageType();

	if(messageType == Message::MessageType_Call)
	{

#ifdef PTRMI_LOGGING
		Status::log(L"Dispatcher dispatching Call", header.getMethodName().getString().c_str());
#endif

		status = receiveAndDispatchCall(header, hostName);
	}
	else
	if(header.getMessageType() == Message::MessageType_Return)
	{

#ifdef PTRMI_LOGGING
		Status::log(L"Dispatcher dispatching Return", L"");
#endif

		status = receiveAndDispatchReturn(header, hostName);
	}
	else
	{
		status = Status(Status::Status_Error_Invalid_Dispatch_Type);
	}

	return status;
}


ServerInterfaceBase *Dispatcher::getReceiverServerInterface(const PTRMI::GUID &clientManager, const Name &receiver)
{
	ServerInterfaceBase *serverInterface;
															// Get receiving ServerInterface's GUID
	if(receiver.isValidGUID())
	{
															// Get ServerInterface if GUID is defined
		if((serverInterface = getManager().getObjectManager().getObjectServerInterface(receiver)) == NULL)
		{
			return NULL;
		}
	}
	else
	{
		URL receiverObjectName;
															// Get Receiver object name from URL
		if(receiver.getObject(receiverObjectName))
		{
															// Create new ServerInterface for object
			if((serverInterface = getManager().getObjectManager().newObjectServerInterface(receiverObjectName, clientManager)) == NULL)
			{
				return NULL;
			}
		}
	}

	return serverInterface;
}


PTRMI::Status Dispatcher::updateHostName(Stream &stream, const Name &hostName, const PTRMI::GUID &guid)
{
	Status		status;
	bool		updated;
															// Copy given name
	Name		newName(hostName);
															// Add a host GUID in case it's missing
	newName.setGUID(guid);
															// Update the host name based on new GUID part
	if((status = getManager().getHostManager().updateHostName(newName, updated)).isFailed())
	{
		return status;
	}

	stream.updateHostName(newName);

	return status;
}


PTRMI::Status Dispatcher::receiveAndDispatchCall(Message &callHeader, const Name &hostName)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER

	Status						status;
	Status						sendStatus;
	ServerInterfaceBase 	*	serverInterface;
	Stream					*	stream;
	Message						returnHeader;
	Host					*	host;
	Message::MessageVersion		messageVersion;


	try
	{
															// Get or create server interface for object
		if((serverInterface = getReceiverServerInterface(callHeader.getSenderManager(), callHeader.getReceiver())) == NULL)
		{
															// If object doesn't exist, use the error handler object to send an error response
			serverInterface = getManager().getObjectManager().getErrorHandlerServerInterface();
															// Bind it with the Host name
			serverInterface->setHostName(hostName);
															// Set error that will be returned as PTRMI protocol error
			status.set(Status::Status_Error_Failed_To_Find_Server_Interface);
		}

															// Lock the server interface for use
		if((sendStatus = getManager().getObjectManager().lockServerInterface(serverInterface)).isFailed())
		{
			return sendStatus;
		}
															// Enforce up to date GUID binding with client interface
		serverInterface->setRemoteInterface(callHeader.getSender());

															// Get server interface's stream
		if((stream = serverInterface->getStream()) == NULL)
		{
			throw Status(Status::Status_Error_Failed_To_Get_Server_Interface_Stream);
		}

															// Make sure Host has the GUID of the client's Manager
		updateHostName(*stream, hostName, callHeader.getSenderManager());

															// Initialize a pipe for use with this stream
		if((sendStatus = getManager().getPipeProtocolManager().initializeStream(*stream)).isFailed())
		{
			throw Status(Status::Status_Error_Failed_To_Initialize_Pipe);
		}

															// Make sure host is locked 
		if(host = getManager().lockHost(hostName))
		{
															// Get message header version used with this host
			messageVersion = host->getMessageVersionUsed();
															// Release host
			getManager().releaseHost(host);
		}
		else
		{
			return Status(Status::Status_Error_Failed_To_Lock_Host);
		}

															// Reset send buffer ready for return message
		stream->resetSendBuffer();

															// Make sure stream is now valid
		if(stream->isValid() == false)
		{
			throw Status(Status::Status_Error_Stream_Invalid);
		}
															// Prepare return header. Sender is this server interface, receiver is the invoking client interface
		returnHeader.sendReturnHeader(messageVersion, *stream->getSendBuffer(), Status(), getManager().getManagerGUID(), serverInterface->getInterfaceGUID(), callHeader.getSender());

															// Invoke server interface method if no errors have occurred so far
		if(status.isOK())
		{
			status = serverInterface->invokeServerMethod(callHeader.getMethodName(), *stream);
		}
		else
		{
															// Receiving ServerInterface not found, so invoke a Null in the error handler to keep consistency in the dispatch mechanism
			serverInterface->invokeServerMethod(Name(NULL_OBJECT_NULL_METHOD_NAME), *stream);
		}

															// Set RMI protocol level status
		returnHeader.setUpdatedStatus(*stream->getSendBuffer(), status);
															// Update total size of message
		returnHeader.updateMessageSizeTotal(*stream->getSendBuffer());
															// Send method invocation return values
		sendStatus = getManager().getPipeProtocolManager().sendMessage(*stream, Message::MessageType_Return);
	}
	catch(Status s)
	{
		sendStatus = s;
	}

															// Release the server interface
	getManager().getObjectManager().releaseServerInterface(serverInterface);

															// Return lower level send status
	return sendStatus;

#else
    Status status;
    return status;
#endif
}


PTRMI::Status Dispatcher::receiveAndDispatchReturn(Message &header, const Name &hostName)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER

	Status					status;
	ClientInterfaceBase *	receiver;
	Stream				*	stream;

															// Get receiver. If receiver not found, error occurred
	if((receiver = getManager().getObjectManager().getObjectClientInterface(header.getReceiver())) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Return_Client_Interface);
	}
															// Get the client interface stream
	if((stream = receiver->getStream()) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Get_Client_Interface_Stream);
	}
															// Make sure HostName is up to date based on GUIDs
	updateHostName(*stream, hostName, header.getSenderManager());

															// Initialize a pipe for use with this stream
	if((status = getManager().getPipeProtocolManager().initializeStream(*stream)).isFailed())
	{
		throw Status(Status::Status_Error_Failed_To_Initialize_Pipe);
	}

															// Enforce up to date GUID binding with remote server interface
	receiver->setRemoteInterface(header.getSender());
															// Set PTRMI level status in receiving ServerInterface
	receiver->setStatus(header.getStatus());

															// Inform waiting thread that message has been received
	if((status = stream->receiveMessage()).isFailed())
	{
		return status;
	}
															// Wait for method to consume results (if pipe is blocking)
	getManager().getPipeProtocolManager().waitForEndMethod(*stream);
															// Return the end status
	return status;

#else
    Status status;
    return status;
#endif
}


};