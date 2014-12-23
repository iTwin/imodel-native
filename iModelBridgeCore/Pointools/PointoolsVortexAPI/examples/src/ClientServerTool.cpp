/******************************************************************************

Pointools Vortex API Examples

ClientServerTest.cpp

Demonstrates basic client server operation

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/

#include "ClientServerTest.h"

ClientServerTest::ClientServerTest() : Tool(-1, -1) 
{
	initializeServer();
	initializeClient();
}
//-----------------------------------------------------------------------------
int ClientServerTest::initializeClient()
//-----------------------------------------------------------------------------
{

#ifndef WIN64
	ptSetClientServerLogFile(L"C:\\ClientServerLog32.txt");
#else
	ptSetClientServerLogFile(L"C:\\ClientServerLog64.txt");
#endif
															// Set up client server caching system

															// Enable local caching of client server POD files															
	ptEnableClientServerCaching(false);
//	ptEnableClientServerCaching(true);

//	ptSetClientServerCacheDataSize(512);
	ptSetClientServerCacheDataSize(2048);

															// Set the folder to use for caches (NOTE: FOLDER MUST ALREADY EXIST)
	ptSetClientCacheFolder(L"C:\\PODCache");
															// Use server file paths to name cache files

	ptSetClientServerCacheNameMode(PT_CLIENT_SERVER_CACHE_NAME_MODE_GUID);
															// Set streaming preferences (min/start speed, max speed, refresh speed, speed multiplier)
//	ptSetClientStreaming(1024*128, 1024*1024*5, 1024*1024, 1.2);
//	ptSetClientStreaming(1024*128, 1024*512, 1024*512, 1.2);
	ptSetClientStreaming(1024*64, 1024*128, 1024*128, 1.2);

															// Set a default data size to use in new caches (Try to make it 2^n series)
															// Set up cache completion threshold. POD cache completes if deficit of less than this threshold occurrs

	ptSetClientCacheCompletionThreshold(0);					// Disable auto cache completion
															// Retry 5 times, initially with 1 second delay, incrementing by 1 second on each retry
//	ptSetClientServerSendRetries(5, 1000, 1000);

// Pip Test
ptSetClientServerSendRetries(5, 1000 * 60 * 60, 1000 * 60 * 60);
															// Create a fake pod to Open when browsing
															// NOTE: REPLACE THESE FILE PATHS !!
															// NOTE: This will cause a new cache to be created each time this example is run if GUID name mode is used for cache files.

															// Comment out to create a fake file for testing
/*
unsigned char data[256];
for(unsigned int t = 0; t < 256; t++)
	data[t] = t;

	bool ret = ptCreateFakePOD(L"C:\\a.pod", data, 256, L"C:\\n_fake.pod");
*/

	return 1;
}
//-----------------------------------------------------------------------------
int ClientServerTest::initializeServer()
//-----------------------------------------------------------------------------
{
															// Set up cross over test callback
	ptSetServerCallBack(serverCallBack);
															// Set up callback to release buffers allocated by this example when API no longer needs them
	ptSetReleaseClientServerBufferCallBack(releaseBufferCallBack);

	return 1;
}
//-----------------------------------------------------------------------------
PTbool ClientServerTest::releaseBufferCallBack(PTvoid *buffer)
//-----------------------------------------------------------------------------
{
															// If buffer is provided, delete it
	if(buffer)
	{
		delete []buffer;
															// Return OK
		return true;
	}
															// Failed, so return false
	return false;
}
//-----------------------------------------------------------------------------
PTuint  ClientServerTest::serverCallBack(	PTvoid *clientToServer, 
						PTuint clientToServerSize, PTvoid *extDataSend, 
						PTuint extDataSendSize, PTvoid **serverToClient, 
						PTuint *serverToClientSize)
//-----------------------------------------------------------------------------
{
	PTuint64		clientID[2];

															// Check external payload data has been included
	if(extDataSend == NULL || extDataSendSize == 0)
	{
		wchar_t error[] = L"ERROR ! Data not sent";
	}
															// Verify the contents of the data
/*
	unsigned int	t;
	if(extDataSend && extDataSendSize > 0)
	{
		for(t = 0; t < 256; t++)
		{
			if(((unsigned char *) extDataSend)[t] != (unsigned char) t)
			{
				wchar_t error[] = L"ERROR ! Data does not match";
			}
		}
	}
*/
															// Create a copy of data sent from client to server
															// This simulates the situation on the server after it has received data

															// Create a buffer (on server side) containing a copy of sent data
															// This buffer will later by deallocated by calling releaseBufferCallBack()
	PTvoid *clientToServerCopy = new unsigned char[clientToServerSize];
	memcpy(clientToServerCopy, clientToServer, clientToServerSize);

															// Run request on server	
															// Cross Over Send & Receive
	PTvoid *serverToClientTemp;
	PTuint	serverToClientSizeTemp;
	PTuint	errorCode;
															// Process server request and also return session ID of client (128 bit)
	errorCode = ptProcessServerRequestClientID(clientToServerCopy, clientToServerSize, &serverToClientTemp, &serverToClientSizeTemp, &(clientID[0]), &(clientID[1]));

															// Create a copy of data sent from server to client
															// This simulates the situation on the client after the return data has been received

															// Create a buffer (on client side) containing response from server 
															// This buffer will later by deallocated by calling releaseBufferCallBack()
	PTvoid *serverToClientCopy = new unsigned char[serverToClientSizeTemp];
	memcpy(serverToClientCopy, serverToClientTemp, serverToClientSizeTemp);

															// Pass return back to Vortex
	*serverToClient		= serverToClientCopy;
	*serverToClientSize	= serverToClientSizeTemp;


															// Induce failure for testing
/*
static unsigned int failureCount = 0;
if(++failureCount == 25)
{
															// Claim that connection to server has been lost
	ptServerClientLost(clientID[0], clientID[1]);
}
*/

															// Return OK
	return errorCode;	
}
