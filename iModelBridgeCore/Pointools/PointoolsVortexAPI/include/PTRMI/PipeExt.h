#pragma once

#include <PTRMI/Pipe.h>
#include <PTRMI/DataBuffer.h>


namespace PTRMI
{

	const unsigned int	PIPE_EXT_DEFAULT_BUFFER_SIZE	= 1024 * 1024 * 5;

	const unsigned long	PIPE_EXT_LOCK_TIMEOUT			= 1000 * 30;

	class PipeExt : public Pipe
	{

	protected:

		DataBuffer::Data 	*	externalReceiveBuffer;
		unsigned long			externalReceiveBufferSize;
		unsigned long			internalSendBufferSize;
		DataBuffer::Data 	*	internalSendBuffer;

		Mutex					sendReceiveMutex;

	protected:

		Status					releaseExternalBuffer		(void);

	public:
								PipeExt						(void);
								PipeExt						(const Name &hostName, PipeManager *initPipeManager);
//								PipeExt						(const URL &name, PipeManager *initPipeManager);
//								PipeExt						(const URL &name, PipeManager *initPipeManager);
//								PipeExt						(const PTRMI::GUID &hostGUID, PipeManager *pipeManager);

		void					clear						(void);

		Status					beginSend					(void);
		Status					endSend						(void);
		Status					beginReceive				(void);
		Status					endReceive					(void);

		Status					lockPipeSendReceive					(void);
		Status					releasePipeSendReceive					(void);

		Status					initializeStream			(Stream * stream);

		void					setExternalReceiveBuffer	(DataBuffer::Data *buffer);
		DataBuffer::Data	 *	getExternalReceiveBuffer	(void);

		void					setExternalReceiveBufferSize(unsigned long size);
		unsigned long			getExternalReceiveBufferSize(void);

		void					setInternalSendBuffer		(DataBuffer::Data *dataBuffer);
		DataBuffer::Data	 *	getInternalSendBuffer		(void);

		void					setInternalSendBufferSize	(unsigned long size);
		unsigned long			getInternalSendBufferSize	(void);

		Status					releaseExternalReceiveBuffer(void);

		Status					sendMessage					(Stream &stream, Message::MessageType type);
		Status					receiveMessage				(void);

		DataBuffer::DataSize	receiveData					(DataBuffer &buffer, DataBuffer::DataSize numBytesMin);

		Status					waitForResult				(PTRMI::Event &event);

		Status					signalEndMethod				(void);

		Status					waitForEndMethod			(void);
	};

} // End PTRMI namespace
