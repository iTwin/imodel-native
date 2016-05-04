#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Manager.h>
#include <PTRMI/InterfaceBase.h>
#include <PTRMI/Stream.h>



namespace PTRMI
{

unsigned int				Stream::sendMessageNumRetries					= 5;
Stream::TimeMilliseconds	Stream::sendMessageDelayMilliseconds			= 1000;
Stream::TimeMilliseconds	Stream::sendMessageDelayIncrementMilliseconds	= 1000;


Stream::Stream(void)
{
	clear();
}


Stream::Stream(InterfaceBase *initInterfaceLocal)
{
	clear();

	setInterfaceLocal(initInterfaceLocal);
}


void Stream::clear(void)
{
															// Initially no owning interface
	setInterfaceLocal(NULL);
															// Initially no receive buffer
	setReceiveBuffer(NULL);
}


bool Stream::isValid(void)
{
	return (getInterfaceLocal() && getSendBuffer() && getReceiveBuffer() && getHostName().isPartiallyValid());
}


Status Stream::lockCaller(void)
{
	if(callerMutex.wait(STREAM_CALLER_MUTEX_TIMEOUT))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Lock_Stream_Caller);
}


Status Stream::releaseCaller(void)
{
	if(callerMutex.release())
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Release_Stream_Caller);
}


PTRMI::Status Stream::beginClientMethod(const Name &methodName)
{
	Host					*	host;
	Message::MessageVersion		messageVersion;
	Status						status;

															// Make sure only one thread is using the stream for calling
	if((status = lockCaller()).isFailed())
	{
		return status;
	}
															// If Host's pipe has failed, return an error
	if(getStatus().is(Status::Status_Error_Pipe_Failed))
	{
		releaseCaller();
		return getStatus();
	}
															// Lock the host, creating a new host if not found
															// NOTE: This lock is released when the method is completed
	if((host = getManager().lockOrNewLockedHost(getHostName())) == NULL)
	{
		releaseCaller();
		return Status(Status::Status_Error_Failed_To_Lock_Host);
	}
															// Get the message header version used with this host
	messageVersion = host->getMessageVersionUsed();

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	if((status = getManager().getPipeProtocolManager().beginSend(*this)).isFailed())
	{
		releaseCaller();

		getManager().releaseHost(host);

		return status;
	}
#endif
															// Reset read/write positions
	resetSendBuffer();

	const Name *receiver;

	if(getInterfaceLocal()->getRemoteInterface().isValidGUID())
	{
		receiver = &getInterfaceLocal()->getRemoteInterface();
	}
	else
	{
		receiver = &getInterfaceLocal()->getObjectName();
	}
															// Create the message header
	message.sendCallHeader(messageVersion, *getSendBuffer(), Status(), getManager().getManagerGUID(), getInterfaceLocal()->getInterfaceGUID(), *receiver, methodName);


	return Status(Status::Status_OK);
}


bool Stream::isSendRetryStatus(Status &status)
{
															// Don't retry sending messages if these statuses have been returned
	if(status.is(Status::Status_OK) ||
	   status.is(Status::Status_Error_Failed_To_Find_Server_Interface) ||
	   status.is(Status::Status_Server_Class_Method_Not_Found) ||
	   status.is(Status::Status_Error_Pipe_Failed))
	{
		return false;
	}
															// Otherwise retry
	return true;
}


Status Stream::sendMessage(void)
{
	unsigned int		sendCount;
	bool				trySend = true;
	TimeMilliseconds	retryDelay = getSendMessageDelay();

	if(getHostName().isPartiallyValid() == false)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Do 1+retries attempts at making RMI call
	for(sendCount = 0; trySend && sendCount <= getSendMessageNumRetries(); sendCount++)
	{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// Try message send
		status = getManager().getPipeProtocolManager().sendMessage(*this, Message::MessageType_Call);
#endif
															// If RMI failed and status warrants retrying the send
		if(trySend = isSendRetryStatus(status))
		{
															// Back off for a period of time
			Sleep(retryDelay);
															// Increment delay between retries
			retryDelay += getSendMessageDelayIncrement();
		}

	}
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// End Send. Notify pipe sending has ended (Note: For Ext pipe, receive has also completed)
	getManager().getPipeProtocolManager().endSend(*this);
#endif
															// Return RMI call status
	return status;
}


Status Stream::invokeClientMethod(ClientInterfaceBase &clientInterface, bool methodResult, bool attemptRecovery)
{
	Status		status;

	if(isValid() == false)
	{
		return Status(Status::Status_Error_Stream_Invalid);
	}

															// Update the header's record of total message size including buffer
	message.updateMessageSizeTotal(*getSendBuffer());
															// Send message
	status = sendMessage();
															// If RMI call failed or not waiting for a reply, return now
	if(status.isFailed() || methodResult == false)
	{
		return status;
	}
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// If this pipe architecture requires waiting for a message, block and wait
	if((status = getManager().getPipeProtocolManager().waitForResult(*this, receiveMessageEvent)).isFailed())
	{
		return status;
	}
#endif
															// Return should now be in the stream's receive buffer

															// Return OK
	return Status(Status::Status_OK);
}


PTRMI::Status Stream::endClientMethod(bool methodCancelled)
{
	Status					status;
															// Let Pipe know that results have been consumed

															// If method was completed
//	if(methodCancelled == false)
	{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// Signal end of method
		getManager().getPipeProtocolManager().signalEndMethod(*this);
#endif
	}
															// Invocation complete, so release caller mutex on stream
	releaseCaller();

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// End receiving reply
	status = getManager().getPipeProtocolManager().endReceive(*this);
#endif
															// If successful
	if(status.isOK())
	{
		PTRMI::Host *host;
															// Get ptr to PTRMI Host
		if((host = getManager().lockHost(getHostName())) == NULL)
		{
			return Status(Status::Status_Error_Failed_To_Lock_Host);
		}
															// Time stamp host as being successfully used now
		host->setTimeLastUsed();		
															// Release host once
		getManager().releaseHost(host);
	}

															// Release host
	getManager().releaseHost(getHostName());

															// Return status
	return status;
}


DataBuffer * Stream::getSendBuffer(void)
{
	return &sendBuffer;
}


DataBuffer * Stream::getReceiveBuffer(void)
{
	return receiveBuffer;
}


bool Stream::setSendBufferSize(DataBuffer::DataSize size, bool minimumSize)
{
	if(getSendBuffer())
	{
		getSendBuffer()->resizeInternalBuffer(size, minimumSize);

		return true;
	}

	return false;
}

DataBuffer::DataSize Stream::getSendBufferSize(void)
{
	if(getSendBuffer())
		return getSendBuffer()->getBufferSize();

	return 0;
}

bool Stream::setReceiveBufferSize(DataBuffer::DataSize size)
{
	if(getReceiveBuffer())
	{
		getReceiveBuffer()->resizeInternalBuffer(size);

		return true;
	}

	return false;
}

DataBuffer::DataSize Stream::getReceiveBufferSize(void)
{
	if(getReceiveBuffer())
		return getReceiveBuffer()->getBufferSize();

	return 0;
}


bool Stream::setSendReceiveBufferSize(DataBuffer::DataSize size)
{
	if(setSendBufferSize(size) == false)
		return false;

	if(setReceiveBufferSize(size) == false)
		return false;

	return true;
}

void Stream::setStatus(Status initStatus)
{
	status = initStatus;
}

PTRMI::Status Stream::getStatus(void)
{
	return status;
}

void Stream::setInterfaceLocal(InterfaceBase *initInterfaceLocal)
{
	interfaceLocal = initInterfaceLocal;
}

InterfaceBase* Stream::getInterfaceLocal(void)
{
	return interfaceLocal;
}

/*
Status Stream::setAndReferencePipe(Pipe *initPipe)
{
	Status		status;
	Pipe	*	previousPipe;
															// Note: First pipe must be set to NULL by setPipe()

															// Get the existing pipe as the 'previous' pipe
	previousPipe = getPipe();

															// If pipe is set or changed
	if(initPipe != previousPipe)
	{
		PipeProtocolManager *pipeProtocolManager;

		if((pipeProtocolManager = getManager().getPipeProtocolManager()) == NULL)
		{
			return Status(Status::Status_Error_Failed_To_Find_Pipe_Protocol_Manager);
		}
															// If a pipe is currently defined
		if(previousPipe != NULL)
		{
															// Delete Pipe if reference count is zero
			pipeProtocolManager->discardPipe(previousPipe);
		}

															// Set the new pipe
		setPipe(initPipe);

															// If the new pipe is defined
		if(initPipe)
		{
															// Register this stream's new reference to the pipe
			if((status = initPipe->incrementReferenceCounter()).isOK())
			{
															// Initialize this stream for use with the pipe
				status = initPipe->initializeStream(this);
			}
		}

	}
															// Return OK
	return status;

}
*/


bool Stream::updateHostName(const Name &initHostName)
{
	if(getInterfaceLocal())
	{
		return getInterfaceLocal()->updateHostName(initHostName);
	}

	return false;
}


const PTRMI::Name Stream::getHostName(void)
{
	if(getInterfaceLocal())
	{
		return getInterfaceLocal()->getHostName();
	}

	return Name();
}


Stream::TimeMilliseconds Stream::getReceiveMessageTimeout(void)
{
															// Wait for a period of time that is at least the default minimum
															// and at most, the full timeout specified for communication retries
	return std::max(STREAM_DEFAULT_RECEIVE_MESSAGE_TIMEOUT, static_cast<TimeMilliseconds>(getSendMessageDelay() * getSendMessageNumRetries()));

}


PTRMI::Status Stream::receiveMessage()
{
															// Signal to stream that a message has been received
															// and is ready in the receive buffer
	if(receiveMessageEvent.signal())
	{
		return Status(Status::Status_OK);
	}

	return Status(Status::Status_Error_Receive_Message_Signal);
}

/*
PTRMI::Status Stream::manageClientPipe(void)
{
	Status					status;
															// Get the pipe if it is bound, an existing pipe to the server host, or create a new pipe if one doesn't exist
	if((status = getOrCreateClientPipe()).isFailed())
	{
		return status;
	}

	return status;
}
*/

/*
PTRMI::Status Stream::getOrCreateClientPipe(void)
{
	Pipe *pipe;
															// If pipe exists, return OK
	if(pipe = getPipe())
	{
		return Status();
	}

	InterfaceBase *clientInterface;
															// Make sure this stream has an interface
	if((clientInterface = getInterfaceLocal()) == NULL)
	{
		return Status(Status::Status_Error_Stream_Has_No_Client_Interface);
	}


	URL remoteHostURL;
	
	clientInterface->getObjectName().getProtocolHostAddress(remoteHostURL);
															// Get a pipe if one exists already
	if(pipe = getManager().getPipeProtocolManager().getPipe(remoteHostURL))
	{
		setAndReferencePipe(pipe);
		return Status();
	}
															// Create a new pipe
	if((pipe = getManager().getPipeProtocolManager().newPipe(*this, remoteHostURL)))
	{
		setAndReferencePipe(pipe);

		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Create_Stream_Pipe);
}
*/

void Stream::setBufferMode(DataBuffer::Mode bufferMode)
{
	if(getSendBuffer())
	{
		getSendBuffer()->setMode(bufferMode);
	}

	if(getReceiveBuffer())
	{
		getReceiveBuffer()->setMode(bufferMode);
	}
}

void Stream::resetSendReceiveBuffers(void)
{
	resetSendBuffer();
	resetReceiveBuffer();
}

void Stream::resetSendBuffer(void)
{
	if(getSendBuffer())
	{
															// Reset pointers
		getSendBuffer()->reset();
	}
}

void Stream::resetReceiveBuffer(void)
{
	if(getReceiveBuffer())
	{
															// Reset pointers
		getReceiveBuffer()->reset();
	}
}


void Stream::resetSendBufferRead(void)
{
															// Reset send buffer read pointer only
	if(getSendBuffer())
		getSendBuffer()->resetRead();
}

void Stream::resetReceiveBufferRead(void)
{
	if(getReceiveBuffer())
		getReceiveBuffer()->resetRead();
}

void Stream::setReceiveBuffer(DataBuffer *initReceiveBuffer)
{
	receiveBuffer = initReceiveBuffer;
}


PTRMI::Status Stream::setSendBufferSizeMin(DataBuffer::DataSize size)
{
	if(getSendBuffer())
	{
		return getSendBuffer()->resizeInternalBufferMin(size);
	}

	return Status(Status::Status_Error_Stream_Has_No_Send_Buffer);
}


PTRMI::Status Stream::setReceiveBufferSizeMin(DataBuffer::DataSize size)
{
	if(getReceiveBuffer())
	{
		return getReceiveBuffer()->resizeInternalBufferMin(size);
	}

	return Status(Status::Status_Error_Stream_Has_No_Receive_Buffer);
}


bool Stream::setSendMessageNumRetries(unsigned int numRetries)
{
	if(numRetries <= STREAM_MAX_SEND_MESSAGE_NUM_RETRIES)
	{
		sendMessageNumRetries = numRetries;
		return true;
	}

	return false;
}


unsigned int Stream::getSendMessageNumRetries(void)
{
	return sendMessageNumRetries;
}


bool Stream::setSendMessageDelay(TimeMilliseconds delayMilliseconds)
{
	if(delayMilliseconds <= STREAM_MAX_SEND_MESSAGE_MAX_DELAY)
	{
		sendMessageDelayMilliseconds = delayMilliseconds;
		return true;
	}

	return false;
}


Stream::TimeMilliseconds Stream::getSendMessageDelay(void)
{
	return sendMessageDelayMilliseconds;
}


bool Stream::setSendMessageDelayIncrement(TimeMilliseconds delayIncrementMilliseconds)
{
	if(delayIncrementMilliseconds <= STREAM_MAX_SEND_MESSAGE_MAX_DELAY_INCREMENT)
	{
		sendMessageDelayIncrementMilliseconds = delayIncrementMilliseconds;
		return true;
	}

	return false;
}


Stream::TimeMilliseconds Stream::getSendMessageDelayIncrement(void)
{
	return sendMessageDelayIncrementMilliseconds;
}


Stream::~Stream(void)
{

}


Status Stream::initializeServerInvoke(void)
{
	Status		status;

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// Begin sending method reply
															// (This is done before endReceive() to overlap locking for pipes that need to maintain recursive mutex)
	if((status = getManager().getPipeProtocolManager().beginSend(*this)).isFailed())
	{
		return status;
	}
															// End receiving method invocation.
															// (This is done after beginSend() to overlap locking for pipes that need to maintain recursive mutex)
	if((status = getManager().getPipeProtocolManager().endReceive(*this)).isFailed())
	{
		return status;
	}
#endif
															// End receiving method invocation.
															// Return OK
	return status;	
}


PTRMI::GUID Stream::getHostGUID(void)
{
	Host		*	host;
	PTRMI::GUID		hostGUID;

	if(host = getManager().lockHost(getHostName()))
	{
		hostGUID = host->getManagerGUID();

		getManager().releaseHost(host);
	}

	return hostGUID;
}


} // End PTRMI namespace