#pragma once

#include <PTRMI/PipeManager.h>
#include <PTRMI/Thread.h>


namespace PTRMI
{
	class PipeTCP;

	const unsigned int	PIPE_MANAGER_TCP_DEFAULT_LISTENER_PORT = 8090;

	class PipeManagerTCP : public PipeManager
	{

	public:

		typedef HostAddressIP4::Port	Port;

	protected:

		SOCKET			socketListener;
		Port			listenerPort;
		Thread			listenerThread;

	protected:

		Status			initialize					(Port initListenerPort);
		Status			shutdown					(void);

		Status			initializeWinsock			(void);

		Status			setupListener				(void);
		Status			shutdownListener			(void);

static	unsigned long	STDCALL_ATTRIBUTE listenerCall		(void *params);
		Status			listener					(void);

		bool			getValidProtocolHostAddress	(const URL &objectName, URL &protocol, HostAddressIP4 &hostAddress) const;

		void			setListenerPort				(Port initListenerPort);


		Status			resolveHostAddress			(const URL &url, HostAddressIP4 &hostAddressIP);

		Status			connect						(const URL &pipeHostAddress, SOCKET &socket);
		PipeTCP		*	newDispatchedPipe			(const URL &pipeHostAddress, SOCKET &socket, Stream *stream);

// Old Here moved to protected
		Status			addPipeHostGUID				(PipeTCP *pipe);
		Pipe		*	newPipe						(const URL &pipeHostAddress, Stream &stream);

		Pipe		*	newPipe						(Stream &stream, const URL &objectName);
		Pipe		*	newPipe						(const URL &pipeHostAddress);

		Status			discardPipe					(Pipe *pipe, bool &deleted);

	public:

						PipeManagerTCP				(const PipeManagerName &name);
			    	   ~PipeManagerTCP				(void);

		Pipe		*	newPipe						(const Name &hostName);

		Status			deletePipe					(Pipe *pipe);

//		bool			deletePipe					(const URL &pipeHostAddress);
// 		bool			deletePipe					(const PTRMI::GUID &hostGUID);
// 		bool			deletePipe					(Pipe *pipe);

		Port			getListenerPort				(void);

	};

} // End PTRMI namespace


