#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Manager.h>
#include <PTRMI/PipeManagerExt.h>
#include <PTRMI/DataBuffer.h>
#include <PTRMI/Message.h>
#include <PTRMI/ClientInterfaceExtData.h>

bool STDCALL_ATTRIBUTE ptServerClientLost(uint64_t clientIDFirst64, uint64_t clientIDSecond64);

namespace PTRMI
{

PipeManagerExt::PipeManagerExt(const PipeManagerName &name) : PipeManager(name)
{
															// Initially no external call defined
	setSendExternalCallFunction(NULL);
															// Initially no external release buffer call defined
	setReleaseExternalBufferFunction(NULL);
}


PipeManagerExt::~PipeManagerExt(void)
{

}


Pipe *PipeManagerExt::newPipe(const Name &hostName)
{
	PipeExt		*	pipe;
	Host		*	host;
	URL				protocol;
	Status			status;

	if(hostName.isValidURL())
	{
		if(hostName.getURL().getProtocol(protocol) == false || (protocol == URL::PT_PTRE) == false)
			return NULL;
	}
	else
	{
		if(hostName.isValidGUID() == false)
		{
			return NULL;
		}
	}

	if((host = getManager().lockHost(hostName)) == NULL)
	{
		return NULL;
	}

	if((pipe = new PipeExt(hostName, this)) == NULL)
	{
		Status status(Status::Status_Error_Failed_To_Create_Stream_Pipe);
	}

	if((status = addPipe(pipe)).isFailed())
	{
		delete pipe;
	}

	getManager().releaseHost(host);

	return pipe;
}


Status PipeManagerExt::deletePipe(Pipe *pipe)
{
	Status	status;
															// Delete in PipeManager
	if((status = PipeManager::deletePipe(pipe)).isOK())
	{
															// If a PipeExt is given
		if(dynamic_cast<PipeExt *>(pipe))
		{
															// Delete pipe
			delete pipe;
															// Return OK
			return status;
		}
	}
															// Return error
	return Status(Status::Status_Error_Failed_To_Delete_Pipe);
}

/*
Pipe * PipeManagerExt::newPipe(Stream &stream, const URL &pipeHostAddress)
{
	PipeExt *	pipe;
	URL			protocol;

	if(pipeHostAddress.getProtocol(protocol) == false || (protocol == URL::PT_PTRE) == false)
		return NULL;
	
	if((pipe = new PipeExt(pipeHostAddress, this)) == NULL)
	{
		return NULL;
	}

	if(addPipe(pipe) == false)
	{
		delete pipe;
		return NULL;
	}

	return pipe;
}


Pipe * PipeManagerExt::newPipe(Stream &stream, const PTRMI::GUID &hostGUID)
{
	PipeExt *	pipe;

	if((pipe = new PipeExt(hostGUID, this)) == NULL)
	{
		return NULL;
	}

	if(addPipe(pipe) == false)
	{
		delete pipe;
		return NULL;
	}

	return pipe;
}
*/

/*
PipeExt * PipeManagerExt::newPipe(const PTRMI::GUID &pipeGUID)
{
	PipeExt *	pipeExt = NULL;

	Host	*	host;
	Name		hostName(pipeGUID);


	try
	{
		if((host = getManager().lockHost(hostName)) == NULL)
		{
															// Return without unlocking host
			return NULL;
		}
															// Create new PipeExt
		if((pipeExt = new PipeExt(pipeGUID, this)) == NULL)
		{
			throw NULL;
		}
															// Add pipe to the Host

//		if(host->addPipe(*pipeExt).isFailed())
//		{
//			throw NULL;
//		}

															// Add the new pipe
		if(addPipe(pipeExt) == false)
		{
//			host->removePipe(*pipeExt);
			throw NULL;		
		}
	}
	catch(...)
	{
		if(pipeExt)
		{
			delete pipeExt;
		}
	}

	getManager().releaseHost(hostName);

	return pipeExt;
}
*/

/*
bool PipeManagerExt::deletePipe(const URL &pipeHostAddress)
{
	return PipeManager::deletePipe(pipeHostAddress);
}
*/

/*
bool PipeManagerExt::deletePipe(const PTRMI::GUID &hostGUID)
{
	return PipeManager::deletePipe(hostGUID);
}
*/

/*
bool PipeManagerExt::deletePipe(Pipe *pipe)
{
	return PipeManager::deletePipe(pipe);
}
*/

PTRMI::Status PipeManagerExt::sendExternalCall(Stream &stream)
{
	void					*	receiveBufferExt;
	unsigned int				receiveBufferSize;
	Status						status;
	DataBuffer				*	sendBuffer;
	DataBuffer				*	receiveBuffer;
	ClientInterfaceBase		*	clientInterface;
	ClientInterfaceExtData	*	clientExtData;
	DataBuffer::Data		*	clientExtDataRaw = NULL;
	DataBuffer::DataSize		clientExtDataSize = 0;
	Pipe					*	pipe;
	Message::MessageType		messageType;


	if(externalCallFunction == NULL)
	{
		return Status(Status::Status_Error_External_Call_Function_Not_Defined);
	}

															// Get stream's pipe
	if((pipe = getPipe(stream)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Find_Pipe_For_Invoke);
	}
															// Notify Pipe it will receive a result
	if((status = pipe->beginReceive()).isFailed())
	{
		return status;
	}

															// Get Ext data if present
	if((sendBuffer = stream.getSendBuffer()) == NULL || sendBuffer->getBuffer() == NULL)
		return Status(Status::Status_Error_Stream_Has_No_Send_Buffer);

	if((receiveBuffer = stream.getReceiveBuffer()) == NULL)
		return Status(Status::Status_Error_Stream_Has_No_Receive_Buffer);

	if((clientInterface = dynamic_cast<ClientInterfaceBase *>(stream.getInterfaceLocal())) == NULL)
		return Status(Status::Status_Error_Stream_Has_No_Client_Interface);

	if((clientExtData = clientInterface->getExtData()) != NULL)
	{
		clientExtDataRaw = clientExtData->getExtData(&clientExtDataSize);
	}

															// Send call message using external function
	if(getSendExternalCallFunction())
	{

#ifdef _DEBUG
#ifdef PTRMI_LOGGING
		Status::log(L"sendExternalCall Calling Server Callback", L"");
#endif
#endif

		ExternalErrorCode	errorCode;
															// Invoke call using external communications mechanism
		errorCode = (getSendExternalCallFunction())(sendBuffer->getBuffer(), sendBuffer->getDataSize(), clientExtDataRaw, clientExtDataSize, &receiveBufferExt, &receiveBufferSize);
															// Map pre-defined external error codes to Status error codes
		if((status = getExternalErrorCodeStatus(errorCode)).isFailed())
		{
															// If external Pipe failed, set status in Pipe
			if(status.is(Status::Status_Error_Pipe_Failed))
			{
															// Set status in Pipe
				pipe->setStatus(status);
			}
															// Return if an error has occurred
			return status;
		}
	}
	else
	{
		return Status(Status::Status_Error_External_Call_Function_Not_Defined);
	}

															// Set receive buffer to the external buffer
	receiveBuffer->setExternalBuffer(reinterpret_cast<DataBuffer::Data *>(receiveBufferExt), static_cast<DataBuffer::DataSize>(receiveBufferSize));
															// Set receive buffer to external mode
	receiveBuffer->setMode(DataBuffer::Mode_External);


	Message message;
															// Consume the header from the receive buffer
	message.receiveHeader(*receiveBuffer);
															// Do some simple receive management (Result will not be formally dispatched at this point because Pipe type is non blocking)
	if((status = getManager().getDispatcher().dispatch(message, stream.getHostName(), messageType)).isFailed())
	{
		return status;
	}
															// Get low level stream status
	status = message.getStatus();
															// Return status
	return status;
}


PTRMI::Status PipeManagerExt::receiveExternalCall(void *receiveBufferExt, unsigned int receiveSizeExt, void **sendBufferExt, unsigned int *sendSizeExt, PTRMI::GUID *clientManager, ExternalProcessRequestCallback externalProcessRequestCallback)
{
	Status						status;
	DataBuffer					headerBuffer;
	Message						header;
	PipeExt					*	pipeExt;
	Message::MessageType		messageType;
	Host					*	remoteHost;

															// Make sure pointers to buffer info is given
	if(sendBufferExt == NULL || sendBufferExt == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Default is no send buffer result in case of error
	*sendBufferExt	= NULL;
	*sendSizeExt	= 0;
															// Bind receive buffer to temporary buffer
	headerBuffer.setExternalBuffer(reinterpret_cast<DataBuffer::Data *>(receiveBufferExt), receiveSizeExt);
	headerBuffer.setMode(DataBuffer::Mode_External);

															// Receive message header (Message and buffer are on stack so thread safe)
	if((status = header.receiveHeader(headerBuffer)).isFailed())
	{
		return status;
	}
															// Get GUID of remote host
	const PTRMI::GUID &remoteHostGUID = header.getSenderManager();

															// [Note: Use this to manually induce a call to ptServerClientLost() when debugging client object recovery
#ifdef _DEBUG
static bool testFail = false;
if(testFail)
{
	ptServerClientLost(remoteHostGUID.getRawFirst64(), remoteHostGUID.getRawSecond64());
	testFail = false;
}
#endif

															// If requested by passing a ptr to a GUID
	if(clientManager)
	{
															// Return GUID of remote client manager
		*clientManager = remoteHostGUID;
	}
															// Return error if not valid
	if(remoteHostGUID.isValidGUID() == false)
	{
		return Status(Status::Status_Error_Host_GUID_Invalid);
	}

															// Create a host name from GUID
	Name remoteHostName(remoteHostGUID);
															// Get and lock host or add new locked Host if doesn't already exist
	if((remoteHost = getManager().lockOrNewLockedHost(remoteHostName)) == NULL)
	{
		return Status(Status::Status_Error_Failed_To_Lock_Host);
	}
															// Get pipe and down cast to check type
	if((pipeExt = dynamic_cast<PipeExt *>(getPipe(remoteHostName))) == NULL)
	{
															// Create new pipe based on GUID as identifier
		if((pipeExt = dynamic_cast<PipeExt *>(newPipe(remoteHostName))) == NULL)
		{
			getManager().releaseHost(remoteHost);

			return Status(Status::Status_Error_Failed_To_Create_Stream_Pipe);
		}
	}
															// Notify Pipe that receiving will begin
	if((status = pipeExt->beginReceive()).isFailed())
	{
		getManager().releaseHost(remoteHost);

		return status;
	}

	try
	{
															// Copy header buffer's state
		if(pipeExt->getReceiveBuffer())
		{
			if((status = pipeExt->getReceiveBuffer()->copyExternalBufferState(headerBuffer)).isFailed())
			{
				throw status;
			}
		}
		else
		{
			throw Status(Status::Status_Error_Failed_To_Find_Pipe_Receive_Buffer);
		}

															// Dispatch received message, invoke and get return
		if((status = getManager().getDispatcher().dispatch(header, remoteHostName, messageType)).isFailed())
		{
			throw status;
		}

															// Return send buffer
		*sendBufferExt = pipeExt->getInternalSendBuffer();
															// Return size of sent data
		*sendSizeExt = pipeExt->getInternalSendBufferSize();
	}
    catch (Status /*error*/)
        {
        // RB_VORTEX_TODO: do something with error???
        }

															// Release caller supplied receive buffer
	releaseExternalBuffer(*pipeExt);
															// End sending reply on pipe
	status = pipeExt->endSend();

															// If external receive callback specified, call it with the buffer to be sent to the Client
	if(externalProcessRequestCallback)
	{
		unsigned int callbackResult;

		if((callbackResult = (*externalProcessRequestCallback)(*sendBufferExt, *sendSizeExt, remoteHostGUID.getRawFirst64(), remoteHostGUID.getRawSecond64())) != EXT_PIPE_ERROR_OK)
		{
			status = Status(Status::Status_Error_External_Call_Function_Failed);
		}
	}

															// Release the host
	getManager().releaseHost(remoteHost);
															// Return status
	return status;
}


PTRMI::Status PipeManagerExt::receiveExternalCallCB(void *receive, unsigned int receiveSize, PTRMI::GUID *clientManager, ExternalProcessRequestCallback externalProcessRequestCallback)
{
	if(externalProcessRequestCallback)
	{
		void		*	sendBufferExt;
		unsigned int	sendSizeExt;
															// Call receiveExternalCall using supplied callback to process the resulting buffer to be sent to the client
															// sendBufferExt and sendSizeExt are dummies here
		return receiveExternalCall(receive, receiveSize, &sendBufferExt, &sendSizeExt, clientManager, externalProcessRequestCallback);
	}

	return Status(Status::Status_Error_External_Call_Function_Not_Defined);
}


void PipeManagerExt::setSendExternalCallFunction(ExternalCallFunction function)
{
	externalCallFunction = function;
}

PipeManagerExt::ExternalCallFunction PipeManagerExt::getSendExternalCallFunction(void)
{
	return externalCallFunction;
}


void PipeManagerExt::setReleaseExternalBufferFunction(ExternalReleaseBufferFunction function)
{
	externalReleaseBufferFunction = function;
}


PipeManagerExt::ExternalReleaseBufferFunction PipeManagerExt::getReleaseExternalBufferFunction(void)
{
	return externalReleaseBufferFunction;
}


void PipeManagerExt::releaseExternalBuffer(PipeExt &pipeExt)
{
															// If release function defined
	if(getReleaseExternalBufferFunction())
	{
															// Allow external Application to deallocate the buffer
		(getReleaseExternalBufferFunction())(pipeExt.getReceiveBuffer()->getExternalBuffer());
															// Release the external buffer (without deallocating)
		pipeExt.releaseExternalReceiveBuffer();
	}
}


Status PipeManagerExt::getExternalErrorCodeStatus(ExternalErrorCode errorCode)
{
	switch(errorCode)
	{
	case 0:
		return Status(Status::Status_OK);					// Temporary fix until Bentley Quebec change OK from 0 to 1

	case EXT_PIPE_ERROR_OK:
		return Status(Status::Status_OK);

	case EXT_PIPE_ERROR_EXT_DATA_EMPTY:
		return Status(Status::Status_Error_Ext_Data_Bad);

	case EXT_PIPE_ERROR_COMMUNICATION_FAILURE:
		return Status(Status::Status_Error_Pipe_Failed);

	case EXT_PIPE_ERROR_BAD_DATA:
		return Status(Status::Status_Error_Pipe_Failed);

	case EXT_PIPE_ERROR_DATA_OUT_OF_DATE:
		return Status(Status::Status_Error_Ext_Data_Out_Of_Date);
	}
															// Unknown error code, so return pipe failure as a default
	return Status(Status::Status_Error_Pipe_Failed);
}




} // End PTRMI namespace

