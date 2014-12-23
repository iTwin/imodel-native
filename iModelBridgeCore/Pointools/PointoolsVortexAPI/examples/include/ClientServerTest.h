/******************************************************************************

Pointools Vortex API Examples

ClientServer.h

Demonstrates basic client server operation

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CLIENT_SERVER_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_CLIENT_SERVER_TOOL_H_

#include "VortexExampleApp.h"

class ClientServerTest : public Tool
{
public:

	ClientServerTest();
	
private:
	int		initializeClient();
	int		initializeServer();

	static 
	PTbool	releaseBufferCallBack(PTvoid *buffer);

	static
	PTuint	serverCallBack(	PTvoid *clientToServer, PTuint clientToServerSize, 
							PTvoid *extDataSend, PTuint extDataSendSize, 
							PTvoid **serverToClient, PTuint *serverToClientSize);

};

#endif

