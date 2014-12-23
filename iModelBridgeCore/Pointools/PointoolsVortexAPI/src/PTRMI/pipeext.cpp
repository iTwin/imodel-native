
#include <PTRMI/PipeExt.h>
#include <PTRMI/Stream.h>
#include <PTRMI/PipeManagerExt.h>

namespace PTRMI
{


PipeExt::PipeExt(void)
{
	clear();
}

/*
PipeExt::PipeExt(const URL &pipeHostAddress, PipeManager *pipeManager) : Pipe(pipeHostAddress, pipeManager)
{
	clear();
}
*/

/*
PipeExt::PipeExt(const PTRMI::GUID &hostGUID, PipeManager *pipeManager) : Pipe(hostGUID, pipeManager)
{
	clear();
}
*/

PipeExt::PipeExt(const Name &hostName, PipeManager *pipeManager) : Pipe(hostName, pipeManager)
{
	clear();
}


// Note: Need to have receiver buffer passed too for external call ?

PTRMI::Status PipeExt::sendMessage(Stream &stream, Message::MessageType messageType)
{
	Status	status;
															// If send is a call (invocation)
	if(messageType == Message::MessageType_Call)
	{
		PipeManagerExt *pipeManagerExt = dynamic_cast<PipeManagerExt *>(getPipeManager());

		if(pipeManagerExt)
		{
															// Send using external messaging transport
			status = pipeManagerExt->sendExternalCall(stream);
		}
		else
		{
			status = Status(Status::Status_Error_Failed_To_Find_Pipe_Pipe_Manager);
		}
	}
	else
	if(messageType == Message::MessageType_Return)
	{
		DataBuffer *sendBuffer;
															// Get stream's send buffer		
		if((sendBuffer = stream.getSendBuffer()) == NULL)
		{
			return Status(Status::Status_Error_Stream_Invalid);
		}
															// Record send buffer information to return to external caller
		setInternalSendBuffer(sendBuffer->getBuffer());

		setInternalSendBufferSize(sendBuffer->getDataSize());
	}
															// Return status
	return status;
}


PTRMI::Status PipeExt::receiveMessage(void)
{
															// Data already in buffer
	return Status();
}


DataBuffer::DataSize PipeExt::receiveData(DataBuffer &buffer, DataBuffer::DataSize numBytesMin)
{
															// Return 0, data fetching not supported in Ext
	return 0;
}


PTRMI::Status PipeExt::initializeStream(Stream *stream)
{
															// If stream has changed pipes
	if(stream)
	{
															// Update the Pipe's host name if necessary to fill in missing GUID
		updateHostName(stream->getHostName());
															// Set stream's receive buffer to this pipe's receive buffer
		stream->setReceiveBuffer(getReceiveBuffer());
															// Set up internal send buffer
															// Resize to at least the default size
		stream->setSendBufferSize(PIPE_EXT_DEFAULT_BUFFER_SIZE, true);
		stream->getSendBuffer()->setMode(DataBuffer::Mode_Internal);
															// Set up external receive buffer
//		stream->getReceiveBuffer()->setExternalBuffer(getExternalReceiveBuffer(), getExternalReceiveBufferSize());
//		stream->getReceiveBuffer()->setMode(DataBuffer::Mode_External);

		return Status();
	}

	return Status(Status::Status_Error_Bad_Parameter);
}


void PipeExt::setExternalReceiveBuffer(DataBuffer::Data *buffer)
{
	externalReceiveBuffer = buffer;
}


DataBuffer::Data *PipeExt::getExternalReceiveBuffer(void)
{
	return externalReceiveBuffer;
}


void PipeExt::setExternalReceiveBufferSize(unsigned long size)
{
	externalReceiveBufferSize = size;
}


unsigned long PipeExt::getExternalReceiveBufferSize(void)
{
	return externalReceiveBufferSize;
}


void PipeExt::clear(void)
{
															// Initially no external receive buffer
	setExternalReceiveBuffer(NULL);
	setExternalReceiveBufferSize(0);
}


void PipeExt::setInternalSendBufferSize(unsigned long size)
{
	internalSendBufferSize = size;
}

unsigned long PipeExt::getInternalSendBufferSize(void)
{
	return internalSendBufferSize;
}

void PipeExt::setInternalSendBuffer(DataBuffer::Data *dataBuffer)
{
	internalSendBuffer = dataBuffer;
}

DataBuffer::Data * PipeExt::getInternalSendBuffer(void)
{
	return internalSendBuffer;
}

Status PipeExt::waitForResult(PTRMI::Event &event)
{
															// THe Ext architecture has a non blocking return, so return OK because data is ready
	return Status();
}


PTRMI::Status PipeExt::signalEndMethod(void)
{
	return releaseExternalBuffer();
	// Pipe is non blocking
	return Status();
}


PTRMI::Status PipeExt::waitForEndMethod(void)
{
															// Pipe is non blocking
	return Status();
}


PTRMI::Status PipeExt::releaseExternalReceiveBuffer(void)
{
															// If external buffer is defined
	if(getReceiveBuffer())
	{
															// Clear the buffer
		getReceiveBuffer()->clearExternalBuffer();

		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Receive_Buffer);
}


PTRMI::Status PipeExt::releaseExternalBuffer(void)
{
	PipeManagerExt *pipeManagerExt;
															// Release this pipe's External buffer to the App
	if((pipeManagerExt = dynamic_cast<PipeManagerExt *>(this->getPipeManager())) != NULL)
	{
		pipeManagerExt->releaseExternalBuffer(*this);
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Find_Pipe_Pipe_Manager);
}


Status PipeExt::lockPipeSendReceive(void)
{
	if(sendReceiveMutex.wait(PIPE_EXT_LOCK_TIMEOUT))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Lock_Pipe);
}


Status PipeExt::releasePipeSendReceive(void)
{
	if(sendReceiveMutex.release())
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed_To_Release_Pipe);
}


Status PipeExt::beginSend(void)
{
															// Ext pipe locks completely on send or receive
	return lockPipeSendReceive();
}


Status PipeExt::endSend(void)
{
															// Ext pipe locks completely on send or receive
	return releasePipeSendReceive();
}


Status PipeExt::beginReceive(void)
{
															// Ext pipe locks completely on send or receive
	return lockPipeSendReceive();
}


Status PipeExt::endReceive(void)
{
															// Ext pipe locks completely on send or receive
	return releasePipeSendReceive();
}




} // End PTRMI namespace
