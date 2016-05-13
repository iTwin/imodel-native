// ***************************************************************************************************************************
// (c)2011 Pointools Ltd.
//
// Author	: Lee Bull
//
// Notes	: DO NOT CHANGE THE ORDER of any functions in this file.
//			  Alternative orders will compile but behave differently, due to implied template resolution.
//
// ***************************************************************************************************************************

#pragma once

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/PTRMI.h>
#include <PTRMI/Mutex.h>
#include <PTRMI/DataBuffer.h>
#include <PTRMI/InterfaceParameter.h>
#include <PTRMI/Message.h>
#include <PTRMI/Event.h>

namespace PTRMI
{
	class InterfaceBase;
	class ClientInterfaceBase;

	const unsigned int DEFAULT_STREAM_BUFFER_SIZE					= 1024 * 1024 * 5;

	const unsigned int STREAM_MAX_SEND_MESSAGE_NUM_RETRIES			= 32;
	const unsigned int STREAM_MAX_SEND_MESSAGE_MAX_DELAY			= 1000 * 32;
	const unsigned int STREAM_MAX_SEND_MESSAGE_MAX_DELAY_INCREMENT	= 1000 * 32;

	const unsigned long STREAM_DEFAULT_RECEIVE_MESSAGE_TIMEOUT		= 1000 * 5;

	const unsigned long	STREAM_CALLER_MUTEX_TIMEOUT					= 1000 * 30;


	class Stream
	{

	public:

		typedef unsigned long	TimeMilliseconds;

	protected:

		InterfaceBase		*	interfaceLocal;

		Status					status;

		Mutex					callerMutex;

		DataBuffer				sendBuffer;
		DataBuffer			*	receiveBuffer;

		Message					message;

		Event					receiveMessageEvent;

static	unsigned int			sendMessageNumRetries;
static	TimeMilliseconds		sendMessageDelayMilliseconds;
static	TimeMilliseconds		sendMessageDelayIncrementMilliseconds;

	protected:

		Status					sendMessageHeader			(void);
		Status					receiveMessageHeader		(void);

		Status					sendMessage					(void);

//		Status					manageClientPipe			(void);
//		Status					getOrCreateClientPipe		(void);

		bool					isSendRetryStatus				(Status &status);

	protected:

		Status					lockCaller					(void);
		Status					releaseCaller				(void);


	public:

								Stream						(void);
								Stream						(InterfaceBase *initInterfaecLocal);
							   ~Stream						(void);

		void					clear						(void);

		void					setHostName					(const Name &initHostName);
		const Name				getHostName					(void);

		bool					updateHostName				(const Name &initHostName);

		void					setBufferMode				(DataBuffer::Mode bufferMode);

		bool					isValid						(void);

		void					setInterfaceLocal			(InterfaceBase *initInterfaceLocal);
		InterfaceBase		*	getInterfaceLocal			(void);

//		Status					setAndReferencePipe			(Pipe *initPipe);

		PTRMI::GUID				getHostGUID					(void);

		Status					beginClientMethod			(const Name &method);
		Status					invokeClientMethod			(ClientInterfaceBase &clientInterface, bool methodResult = true, bool attemptRecovery = true);
		Status					endClientMethod				(bool methodCancelled = false);

		Status					initializeServerInvoke		(void);

		Status					receiveMessage				(void);

		void					setReceiveBuffer			(DataBuffer *initReceiveBuffer);

		DataBuffer *			getSendBuffer				(void);
		DataBuffer *			getReceiveBuffer			(void);

		bool					setSendBufferSize			(DataBuffer::DataSize size, bool = false);
		Status					setSendBufferSizeMin		(DataBuffer::DataSize size);
		DataBuffer::DataSize	getSendBufferSize			(void);

		bool					setReceiveBufferSize		(DataBuffer::DataSize size);
		Status					setReceiveBufferSizeMin		(DataBuffer::DataSize size);
		DataBuffer::DataSize	getReceiveBufferSize		(void);

		bool 					setSendReceiveBufferSize	(DataBuffer::DataSize size);

		void					resetSendReceiveBuffers		(void);
		void					resetSendBuffer				(void);
		void					resetReceiveBuffer			(void);
		void					resetSendBufferRead			(void);
		void					resetReceiveBufferRead		(void);

		void 					setStatus					(Status initStatus);
		Status					getStatus					(void);

static	bool					setSendMessageNumRetries	(unsigned int numRetries);
static	unsigned int			getSendMessageNumRetries	(void);

static	bool					setSendMessageDelay			(TimeMilliseconds delayMilliseconds);
static	TimeMilliseconds		getSendMessageDelay			(void);

static	bool					setSendMessageDelayIncrement(TimeMilliseconds delayIncrementMilliseconds);
static	TimeMilliseconds		getSendMessageDelayIncrement(void);

static	Stream::TimeMilliseconds getReceiveMessageTimeout	(void);

															// Generic template method for send
		template<typename T> Stream & operator <<(T &v)
		{
			assert(getSendBuffer());

			(*getSendBuffer()) << v;

			return *this;
		}

															// Generic template method for receive
		template<typename T> Stream & operator >>(T &v)
		{
			(*getReceiveBuffer()) >> v;

			return *this;
		}
															// Generic template method for send (pointer)
		template<typename T> Stream & operator << (T* v)
		{
			(*getSendBuffer()) << (*v);

			return *this;
		}
															// Generic template method for receive (pointer)
		template<typename T> Stream & operator >> (T *v)
		{
			(*getReceiveBuffer()) >> (*v);

			return *this;
		}
															// General send
		template<typename T> Stream & send(T &v)
		{
			(*this) << v.get();

			return *this;
		}
															// General receive
		template<typename T> Stream &receive(T &v)
		{
			(*this) >> v.get();

			return *this;
		}

		template<typename T> Stream &sendPartial(T *v)
		{
			v->c(*getSendBuffer());

			return *this;
		}

		template<typename T> Stream &sendPartial(T &v)
		{
			v.writePartial(*getSendBuffer());

			return *this;
		}

		template<typename T> Stream &receivePartial(T &v)
		{
			v.readPartial(*getReceiveBuffer());

			return *this;
		}

		template<typename T> Stream &receivePartial(T *v)
		{
			v->readPartial(*getReceiveBuffer(), *getSendBuffer());

			return *this;
		}


															// Send Parameter (Client) rule for Inputs
		template<typename T> Stream & operator <<(PC<In<T>> &v)
		{
			return send(v);									// Send input to server
		}
															// Receive Parameter (Client) rule for Input													// Receive Parameter (Client) rule base for Inputs
		template<typename T> Stream & operator >>(PC<In<T>> &v)
		{
			return *this;									// Input, so receive nothing
		}
															// Send Parameter (Client) rule for void parameters
		template<> Stream & operator <<(PCVoid &v)
		{
			return *this;									// Do nothing as item is an output
		}
															// Receive Parameter (Client) rule for void parameters
		template<> Stream & operator >>(PCVoid &v)
		{
			return *this;									// Do nothing as item is an output
		}
															// Send Parameter (Client) rule for Inputs
		template<typename T> Stream & operator <<(PC<Out<T>> &v)
		{
			return *this;									// Do nothing as item is an output
		}
															// Receive Parameter (Client) rule for Inputs
		template<typename T> Stream & operator >>(PC<Out<T>> &v)
		{
			return receive(v);								// Receive output sent by server
		}

															// Send Parameter (Client) rule for Inputs
		template<typename T> Stream & operator <<(PC<InOut<T>> &v)
		{
			return send(v);									// Send input to server
		}
															// Receive Parameter (Client) rule base for Inputs
		template<typename T> Stream & operator >>(PC<InOut<T>> &v)
		{
			return receive(v);								// Receive output from server
		}

															// ******************************************

															// Send Parameter (Server) rule for Input
		template<typename T> Stream & operator <<(PS<In<T>> &v)
		{
															// Do nothing because input doesn't need to be sent back
			return *this;
		}
															// Receive Parameter (Server) rule for Input
		template<typename T> Stream & operator >>(PS<In<T>> &v)
		{
			return receive(v);								// Receive input sent by client
		}

															// Send Parameter (Server) rule for void
		template<typename T> Stream & operator <<(PSVoid &v)
		{
			return *this;									// Do nothing as parameter is void
		}

															// Receive Parameter (Server) rule for void
		template<typename T> Stream & operator >>(PSVoid &v)
		{
			return *this;									// Do nothing as parameter is void
		}
															// Send Parameter (Server) rule for Output
		template<typename T> Stream & operator <<(PS<Out<T>> &v)
		{
			return send(v);									// Send output back to client
		}
															// Receive Parameter (Server) rule for Output
		template<typename T> Stream & operator >>(PS<Out<T>> &v)
		{
			return *this;									// Do nothing as output is not sent by client
		}

															// Send Parameter (Server) rule for Input Output
		template<typename T> Stream & operator <<(PS<InOut<T>> &v)
		{
			return send(v);									// Send output back to client
		}
															// Receive Parameter (Server) rule for Input Output
		template<typename T> Stream & operator >>(PS<InOut<T>> &v)
		{
			return receive(v);								// Receive input sent by client
		}


															// *******************************************

		template<typename T> Stream &operator<<(PC<Out<const PTRMI::ArrayDirect<T>>> &value)
		{
			sendPartial(*(value.getContainer()));

			return *this;
		}

		template<typename T> Stream &operator<<(PC<Out<PTRMI::ArrayDirect<T>>> &value)
		{
			sendPartial(*(value.getContainer()));

			return *this;
		}

		template<typename T> Stream &operator>>(PC<Out<ArrayDirect<T>>> &value)
		{
			receive(value);

			return *this;
		}


		template<typename T> Stream &operator<<(PS<Out<const PTRMI::ArrayDirect<T>>> &value)
		{
															// Do nothing because it's already in the buffer
			return *this;
		}


		template<typename T> Stream &operator<<(PS<Out<PTRMI::ArrayDirect<T>>> &value)
		{
															// Do nothing because it's already in the buffer
			return *this;
		}


		template<typename T> Stream &operator>>(PS<Out<ArrayDirect<T>>> &value)
		{
			receivePartial(value.getContainer());

			return *this;
		}
	};
}
#else
namespace PTRMI
    {
    class Stream;
    };
#endif