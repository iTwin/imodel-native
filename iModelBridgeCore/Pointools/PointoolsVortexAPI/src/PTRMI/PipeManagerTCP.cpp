#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Stream.h>
#include <PTRMI/PipeManagerTCP.h>
#include <PTRMI/PipeTCP.h>

namespace PTRMI
{

PipeManagerTCP::PipeManagerTCP(const PipeManagerName &name) : PipeManager(name)
{
// Pip Option
#ifndef _WIN64
	initialize(PIPE_MANAGER_TCP_DEFAULT_LISTENER_PORT);
#else
	initialize(PIPE_MANAGER_TCP_DEFAULT_LISTENER_PORT + 1);
#endif

}

PipeManagerTCP::~PipeManagerTCP(void)
{

}


bool PipeManagerTCP::getValidProtocolHostAddress(const URL &pipeHostAddress, URL &protocol, HostAddressIP4 &hostAddressIP4) const
{
	return (pipeHostAddress.getProtocol(protocol) && pipeHostAddress.getHostAddress(hostAddressIP4));
}


PTRMI::Status PipeManagerTCP::resolveHostAddress(const URL &url, HostAddressIP4 &hostAddressIP)
{
	std::string			stdURL;
	std::wstring		wURL;
	hostent			*	hostEnt;
	unsigned char	*	addressArray;
	URL					hostURL;
															// Try to parse URL to dotted host address
	url.getHostAddress(hostAddressIP);
															// If not parsed, potentially alpha
	if(hostAddressIP.getIPDefined() == false)
	{
		hostAddressIP.getURL(wURL, false);

		hostURL = wURL;
															// Convert wide URL to std string
		hostURL.getString(stdURL);
															// Resolve host name
		if((hostEnt = gethostbyname(stdURL.c_str())) == NULL)
		{
															// Return not found
			return Status(Status::Status_Failed_To_Resolve_Host_URL);
		}
															// Get dotted decimal (binary format)
		if((addressArray = reinterpret_cast<HostAddressIP4::SubValue *>(hostEnt->h_addr_list[0])) == NULL)
		{
															// Return not found
			return Status(Status::Status_Failed_To_Resolve_Host_URL);
		}

		hostAddressIP.setSubValues(addressArray);
	}

	return Status();
}


PTRMI::Status PipeManagerTCP::connect(const URL &pipeHostAddress, SOCKET &socket)
{
	Status			status;
	sockaddr_in		remoteHostAddress;
	HostAddressIP4	hostAddressIP4;

															// Set default port number to connect to (can be over-ridden by URL)
	hostAddressIP4.setPort(getListenerPort());
															// Resolve URL or IP to the IP
	if((status = resolveHostAddress(pipeHostAddress, hostAddressIP4)).isFailed())
		return status;
															// Get address as Winsock address
	hostAddressIP4.getAddress(remoteHostAddress);
															// Set address family
	remoteHostAddress.sin_family = AF_INET;
															// Set port number
	remoteHostAddress.sin_port	 = htons(hostAddressIP4.getPort());
															// Create socket
	if((socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		return Status(Status::Status_Failed_To_Create_Client_Socket);
	}
															// Connect to remote host
	if(::connect(socket, (SOCKADDR*) &remoteHostAddress, sizeof(remoteHostAddress) ) == SOCKET_ERROR)
	{
		closesocket(socket);
		socket = NULL;
		int error = WSAGetLastError();

		return Status(Status::Status_Failed_To_Connect_To_Host);
	}
															// Return end status
	return status;
}


// Internal function
// This is called after a PipeManagerTCP::connect() version is called to get the connect the socket
// This is called after an accept() has got the socket

PipeTCP *PipeManagerTCP::newDispatchedPipe(const URL &pipeHostAddress, SOCKET &socket, Stream *stream)
{
	PipeTCP	*	pipeTCP;
	Status		status;
															// Create a new pipe
	if((pipeTCP = new PipeTCP(pipeHostAddress, this)) == NULL)
		return NULL;
															// Set the host address URL
//	pipeTCP->setHostAddress(pipeHostAddress);				// Needed?
	pipeTCP->setHostName(pipeHostAddress);

															// Set pipe's socket
	pipeTCP->setSocket(socket);

															// Add pipe to manager
	if(addPipe(pipeTCP).isOK() == false)
	{
		delete pipeTCP;
		return NULL;
	}

	if(stream)
	{
															// Bind pipe to stream
//		stream->setAndReferencePipe(pipeTCP);
															// Set initial buffer sizes
//		stream->setSendReceiveBufferSize(PIPE_MANAGER_TCP_DEFAULT_BUFFER_SIZE);
															// Set DataBuffer mode to internal
		stream->setBufferMode(DataBuffer::Mode_Internal);
	}
																// Run dispatcher on this new pipe
	if((status = pipeTCP->runDispatcher(pipeTCP->getSocket())).isFailed())
		return NULL;

	return pipeTCP;
}


Pipe * PipeManagerTCP::newPipe(Stream &stream, const URL &pipeHostAddress)
{
	Status		status;
	SOCKET		socket;
	Pipe	*	pipe;

	if((status = connect(pipeHostAddress, socket)).isFailed())
	{
		return NULL;
	}

	pipe = newDispatchedPipe(pipeHostAddress, socket, &stream);

	return pipe;
}


Pipe * PipeManagerTCP::newPipe(const URL &pipeHostAddress)
{
	Pipe *	pipe;
															// Create a new pipe
	if((pipe = new PipeTCP(pipeHostAddress, this)) == NULL)
		return NULL;
															// State that we need internally managed buffers in the stream
															// Set the host address URL
	pipe->setHostName(pipeHostAddress);
															// Add pipe to manager
	if(addPipe(pipe).isOK() == false)
	{
		delete pipe;
		return NULL;
	}
															// Return new pipe
	return pipe;
}



/*
bool PipeManagerTCP::deletePipe(const URL &pipeHostAddress)
{
	return deletePipe(getPipe(pipeHostAddress));
}
*/

/*
bool PipeManagerTCP::deletePipe(const PTRMI::GUID &hostGUID)
{
	return deletePipe(getPipe(hostGUID));
}
*/

/*
bool PipeManagerTCP::deletePipe(Pipe *pipe)
{
	PipeTCP *pipeTCP;

	if((pipeTCP = dynamic_cast<PipeTCP *>(pipe)) == NULL)
	{
		return false;
	}
															// Delete the pipe
	PipeManager::deletePipe(pipe);

															// Shut down threads running on this pipe (TCP pipe specific)
	if(pipeTCP->shutdown().isFailed())
	{
		return false;
	}

	return true;
}
*/

PTRMI::Status PipeManagerTCP::initialize(Port initListenerPort)
{
	Status	status;
															// Set listener port
	setListenerPort(initListenerPort);
															// Initialize Winsock API
	if((status = initializeWinsock()).isFailed())
		return status;
															// Set up port listener on separate thread
	if((status = setupListener()).isFailed())
		return status;
															// Return OK
	return status;
}

PTRMI::Status PipeManagerTCP::shutdown(void)
{
	Status	status;
															// Shut down listener thread
	if((status = shutdownListener()).isFailed())
		return status;

	WSACleanup();
															// Return OK
	return status;
}

PTRMI::Status PipeManagerTCP::setupListener(void)
{
	Status	status;
															// Run listener on separate thread
	status = listenerThread.start(&PipeManagerTCP::listenerCall, reinterpret_cast<void *>(this));

	return status;
}

void PipeManagerTCP::setListenerPort(Port initListenerPort)
{
	listenerPort = initListenerPort;
}

PipeManagerTCP::Port PipeManagerTCP::getListenerPort(void)
{
	return listenerPort;
}

Status PipeManagerTCP::shutdownListener(void)
{
	Status	status;

	return status;
}

PTRMI::Status PipeManagerTCP::initializeWinsock(void)
{
	WSADATA			wsaData;
	int				wsaret;

	if((wsaret = WSAStartup(0x101, &wsaData)) != NULL)
	{
		return Status(Status::Status_Error_Failed_To_Initialize_Winsock);
	}

	return Status();

}

unsigned long STDCALL_ATTRIBUTE PipeManagerTCP::listenerCall(void *params)
{
															// If parameters not passed, exit
	if(params == NULL)
		return 0;

	Status status = reinterpret_cast<PipeManagerTCP *>(params)->listener();

	if(status.isOK())
		return 0;

	return 1;
}


Status PipeManagerTCP::listener(void)
{
	sockaddr_in		local;
	sockaddr_in		from;
	SOCKET			socketToClient;


	local.sin_family		= AF_INET;						// Address family
	local.sin_addr.s_addr	= INADDR_ANY;					
															// port to listen on
	local.sin_port			= htons(getListenerPort());

															// Create listener socket
	if((socketListener = socket(AF_INET,SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return Status(Status::Status_Error_Failed_To_Create_Listener_Socket);
	}
															// Bind socket with sockaddr
	if(bind(socketListener, reinterpret_cast<sockaddr *>(&local), sizeof(local)) != 0)
	{
		return Status(Status::Status_Error_Failed_To_Bind_Listener_Socket);
	}
															// Listen for incoming TCP/IP connections
	listen(socketListener, 10);

	int fromlen = sizeof(from);

	while(true)
	{
		socketToClient = accept(socketListener, reinterpret_cast<struct sockaddr *>(&from), &fromlen);

		HostAddressIP4::SubValue *p = reinterpret_cast<HostAddressIP4::SubValue *>(&from.sin_addr.s_net);

		HostAddressIP4 hostAddress(p);
		URL hostAddressURL = hostAddress.getURL();

		newDispatchedPipe(hostAddressURL, socketToClient, NULL);
	}

															// Shutting down, so release the socket
	closesocket(socketListener);

}

/*
PTRMI::Status PipeManagerTCP::discardPipe(Pipe *pipe, bool &deleted)
{
	Status		status;
	PipeTCP	*	pipeTCP = dynamic_cast<PipeTCP *>(pipe);

	if(pipeTCP == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// If pipe is specified
	if((status = pipeTCP->discard(deleted, Pipe::PipeReferenceCounter::ActionSignal)).isFailed())
	{
		return status;
	}
															// If pipe was deleted
	if(deleted)
	{
															// Remove pipe indexing
		removePipe(pipe);
															// Signal that this dispatcher thread should terminate when safe
		pipeTCP->requestStopDispatcher();
	}
															// Return OK
	return Status();
}
*/


Pipe *PipeManagerTCP::newPipe(const Name &hostName)
{
	return NULL;
}

Status PipeManagerTCP::deletePipe(Pipe *pipe)
{
	return Status();
}


} // End PTRMI namespace
