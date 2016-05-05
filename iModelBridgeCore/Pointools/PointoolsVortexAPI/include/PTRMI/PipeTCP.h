#pragma once

#include <PTRMI/Pipe.h>
#include <PTRMI/Thread.h>
#include <PTRMI/DataBuffer.h>
#include <PTRMI/Event.h>

namespace PTRMI
{

const unsigned int	PIPE_TCP_DEFAULT_BUFFER_SIZE				= 1024 * 1024 * 5;

const unsigned long	PIPE_TCP_DEFAULT_LOCK_TIMEOUT				= 1000 * 30;

const unsigned long	PIPE_TCP_DEFAULT_WAIT_FOR_RESULT_TIMEOUT	= 1000 * 10;


class PipeTCP : public Pipe
{

protected:

		HostAddressIP4			hostAddressIP4;
		SOCKET					socket;

		Thread					dispatcherThread;

		bool					connected;

		Event					endMethodEvent;

		Mutex					sendMutex;

protected:

static	unsigned long STDCALL_ATTRIBUTE	dispatcherCall			(void *params);
		Status					dispatcher				(void);

		Status					lockSend				(void);
		Status					releaseSend				(void);

		unsigned long			getWaitForResultTimeout	(void);

public:
								PipeTCP					(void);
								PipeTCP					(const URL &pipeName, PipeManager *initPipeManager);
								PipeTCP					(const PTRMI::GUID hostGUID, PipeManager *initPipeManager);
							   ~PipeTCP					(void);

		Status					beginSend				(void);
		Status					endSend					(void);
		Status					beginReceive			(void);
		Status					endReceive				(void);

		Status					lockPipeSendReceive				(void);
		Status					releasePipeSendReceive				(void);

		Status					initialize				(void);
		Status					shutdown				(void);

		void					setConnected			(bool initConnected);
		bool					getConnected			(void);

		Status					runDispatcher			(SOCKET initSocket);
		Status					stopDispatcher			(void);
		Status					requestStopDispatcher	(void);

		void					setSocket				(SOCKET initSocket);
		SOCKET					getSocket				(void);

		Status					initializeStream		(Stream *stream);

		Status					sendMessage				(Stream &stream, Message::MessageType type);
		Status					receiveMessage			(void);

		DataBuffer::DataSize	receiveData				(DataBuffer &dataBuffer, DataBuffer::DataSize numBytesMin);

		Status					waitForResult			(PTRMI::Event &event);

		Status					signalEndMethod			(void);
		Status					waitForEndMethod		(void);
};

} // End PTRMI namespace
