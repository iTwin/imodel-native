#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/PipeManager.h>
#include <PTRMI/PipeTCP.h>
#include <PTRMI/Stream.h>
#include <PTRMI/Manager.h>
#include <PTRMI/PipeManagerTCP.h>


namespace PTRMI
{

PipeTCP::PipeTCP(void)
{
	initialize();
}

PipeTCP::PipeTCP(const URL &pipeHostAddress, PipeManager *initPipeManager) : Pipe(pipeHostAddress, initPipeManager)
{
	initialize();

	setPipeManager(initPipeManager);
}

PipeTCP::PipeTCP(const PTRMI::GUID hostGUID, PipeManager *initPipeManager) : Pipe(hostGUID, initPipeManager)
{
	initialize();

	setPipeManager(initPipeManager);
}


PipeTCP::~PipeTCP(void)
{
															// Remove Pipe indexing before calling shutDown()
//	removePipe();
															// Shut down dispatcher thread
	shutdown();
}


Status PipeTCP::shutdown(void)
{
	Status	status;
															// Shut down TCP/IP socket
	closesocket(socket);
															// Stop dispatcher thread running
//	stopDispatcher();

	return status;
}


Status PipeTCP::initialize(void)
{

	Status	status;
															// Initialize receive buffer
	if(getReceiveBuffer())
	{
		getReceiveBuffer()->setPipe(this);
		getReceiveBuffer()->resizeInternalBufferMin(PIPE_TCP_DEFAULT_BUFFER_SIZE);
		getReceiveBuffer()->setMode(DataBuffer::Mode_Internal);
	}

															// Not initially connected
	setConnected(false);



	// NOTE: Change this to be more configurable and aligned with listener port in manager
	hostAddressIP4.setPort(8090);

	return status;
}


void PipeTCP::setSocket(SOCKET initSocket)
{
	socket = initSocket;
}


SOCKET PipeTCP::getSocket(void)
{
	return socket;
}


PTRMI::Status PipeTCP::runDispatcher(SOCKET initSocket)
{
	Status status;

	setSocket(initSocket);
															// Run dispatcher thread
	if((status = dispatcherThread.start(&PipeTCP::dispatcherCall, reinterpret_cast<void *>(this))).isFailed())
	{
		return status;
	}

	return status;
}


Status PipeTCP::requestStopDispatcher(void)
{
	return dispatcherThread.stop();
}


PTRMI::Status PipeTCP::stopDispatcher(void)
{
	Status	status;

	dispatcherThread.stop();

	dispatcherThread.join();

	return status;
}

unsigned long __stdcall PipeTCP::dispatcherCall(void *params)
{
	PipeTCP	*	pipeTCP;
	Status		status;
															// Run dispatcher
	if((pipeTCP = reinterpret_cast<PipeTCP *>(params)) != NULL)
	{
		status = pipeTCP->dispatcher();	
	}
															// Delete this TCP Pipe
	delete pipeTCP;

	return 1;
}


Status PipeTCP::dispatcher(void)
{
	Status			status;
															// Make sure the pipe manager is defined
	if(getPipeManager() == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_Pipe_Manager);
	}
															// While receiving is OK and the dispatch thread should run
	while(status.isOK() && dispatcherThread.getStop() == false)
	{
		status = receiveMessage();
	}
															// Return status
	return status;
}


PTRMI::Status PipeTCP::sendMessage(Stream &stream, Message::MessageType type)
{
	Status			status;
	DataBuffer *	dataBuffer;

															// Get stream's send buffer
	if((dataBuffer = stream.getSendBuffer()) == NULL)
	{
		endSend();
		return Status(Status::Status_Error_Stream_Invalid);
	}
	
															// Send on TCP/IP socket
	if(::send(socket, reinterpret_cast<const char *>(dataBuffer->getBuffer()), dataBuffer->getDataSize(), 0) == SOCKET_ERROR)
	{
		setConnected(false);
		status = Status(Status::Status_Error_Socket_Sending);
		int WSAError = WSAGetLastError();
	}

															// Return final send status
	return status;
}

// Pip Option
#include <ptapi/PointoolsVortexAPI.h>

PTRMI::Status PipeTCP::receiveMessage(void)
{
	Message						headerMessage;
	Status						status;
	Message::MessageType		messageType;

															// Make sure the pipe manager is specified
	if(getPipeManager() == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_Pipe_Manager);
	}

	try
	{
															// Receive the header
		if((status = headerMessage.receiveHeader(receiveBuffer)).isFailed())
		{
			throw status;
		}
															// Receive and dispatch message
		getManager().getDispatcher().dispatch(headerMessage, headerMessage.getSenderManager(), messageType);

															// If message dispatched was a call
		if(messageType == Message::MessageType_Call)
		{
															// Sending reply is now done
			endSend();
		}
	}
	catch(Status /*error*/)
	{
    // RB_VORTEX_TODO: do something with error???
	}
															// Return status
	return status;
}

void PipeTCP::setConnected(bool initConnected)
{
	connected = initConnected;
}

bool PipeTCP::getConnected(void)
{
	return connected;
}



PTRMI::Status PipeTCP::initializeStream(Stream *stream)
{
	if(stream)
	{
															// Set stream's receive buffer to the one managed by this pipe
		stream->setReceiveBuffer(getReceiveBuffer());
		getReceiveBuffer()->setMode(DataBuffer::Mode_Internal);
															// Ensure minimum buffer sizes for use with TCP
		stream->setSendBufferSizeMin(PIPE_TCP_DEFAULT_BUFFER_SIZE);
		stream->setReceiveBufferSizeMin(PIPE_TCP_DEFAULT_BUFFER_SIZE);
															// Set DataBuffer mode to internal
		stream->setBufferMode(DataBuffer::Mode_Internal);
															// Return OK
		return Status();
	}

	return Status(Status::Status_Error_Bad_Parameter);
}


unsigned long PipeTCP::getWaitForResultTimeout(void)
{

#ifndef _DEBUG
															// Return default timeout
	return PIPE_TCP_DEFAULT_WAIT_FOR_RESULT_TIMEOUT;
#else
															// If Debug, wait indefinitely. This allows server side debugging without client timeout.
															// However, use carefully as this can change the over all behaviour
	return INFINITE;
#endif

}

PTRMI::Status PipeTCP::waitForResult(PTRMI::Event &event)
{

															// This architecture has blocking wait for result
															// because receiving dispatcher thread posts result to waiting thread
	if(event.wait(getWaitForResultTimeout()))
	{
		return Status();
	}
															// Return Wait timed out
	return Status(Status::Status_Error_Message_Wait_Timeout);
}


DataBuffer::DataSize PipeTCP::receiveData(DataBuffer &buffer, DataBuffer::DataSize numBytesMin)
{
	Status						status;
	DataBuffer::DataSize		receivedSize = 0;
	DataBuffer::DataSize		receivedSizeTotal = 0;
															// While data is still needed
	while(receivedSizeTotal < numBytesMin)
	{
		receivedSize = recv(socket, reinterpret_cast<char *>(buffer.getWritePtrAddress()), buffer.getBufferSizeRemaining(), 0);

		if(receivedSize == SOCKET_ERROR)
		{
															// Connection closed with error
			Status status(Status::Status_Error_Socket_Receiving);

/*
															// Delete all resources for this client. Thread will exit here if any server interfaces are created for remote host
			PTRMI::getManager().deleteHost(getHostGUID());
															// Delete if no server interface deletion caused pipe to be deleted
			delete this;
*/
															// Return zero (Note: This thread should have shut down at this point)
			return 0;
		}
		else
		if(receivedSize == 0)
		{
/*
															// Delete all resources for this client
			PTRMI::getManager().deleteHost(getHostGUID());
															// Delete if no server interface deletion caused pipe to be deleted
			delete this;
*/
															// Return zero (Note: This thread should have shut down at this point)
			return 0;
		}
															// Advance the write pointer
		buffer.advanceWritePtr(receivedSize);
															// Increase total received count
		receivedSizeTotal += receivedSize;
	}
															// Return OK
	return receivedSizeTotal;
}


Status PipeTCP::signalEndMethod(void)
{
	endMethodEvent.signal();

	return Status();
}


PTRMI::Status PipeTCP::waitForEndMethod(void)
{
	endMethodEvent.wait();

	return Status();
}


Status PipeTCP::beginSend(void)
{
															// Sending is mutexed, so lock mutex
	return lockSend();
}


Status PipeTCP::endSend(void)
{
															// Sending is mutexed, so release mutex
	return releaseSend();
}


Status PipeTCP::beginReceive(void)
{
															// Receive is exclusively owned by dispatch thread, so no mutexing is done
	return Status();
}

Status PipeTCP::endReceive(void)
{
															// Receive is exclusively owned by dispatch thread, so no mutexing is done
	return Status();
}

Status PipeTCP::lockSend(void)
{
	if(sendMutex.wait(PIPE_TCP_DEFAULT_LOCK_TIMEOUT))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Lock_Pipe);
}

Status PipeTCP::releaseSend(void)
{
	if(sendMutex.release())
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Release_Pipe);
}


Status PipeTCP::lockPipeSendReceive(void)
{
	Status	status;

	if((status = lockSend()).isFailed())
	{
		return status;
	}

	return status;
}


Status PipeTCP::releasePipeSendReceive(void)
{
	Status	status;

	if((status = releaseSend()).isFailed())
	{
		return status;
	}

	return status;
}


} // End PTRMI namespace
