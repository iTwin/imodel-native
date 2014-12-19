//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/SocketTest.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSServerListener.h>
#include <Imagepp/all/h/HFCThread.h>
#include <Imagepp/all/h/HFCMutex.h>
#include <Imagepp/all/h/KeyboardThread.h>

#include <Imagepp/all/h/HCSBufferedConnectionPool.h>
#include <Imagepp/all/h/HCSSocketServerConnection.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HCSSocketConnectionPool.h>
#include <Imagepp/all/h/HCSNamedPipeServerConnection.h>
#include <Imagepp/all/h/HCSNamedPipeConnection.h>
#include <Imagepp/all/h/HCSNamedPipeConnectionPool.h>

static const string s_Message("hello, world!\r\n");
static const string s_Response("hello, world!");
static const string s_Marker("\r\n");
static const string s_Server("server");
static const string s_Client("client");
typedef HFCExclusiveKey OutputKeyType;
//typedef HFCMutex OutputKeyType;
static OutputKeyType s_OutputKey;


#include <Imagepp/all/h/ServerThread.h>
//#include "ServerThread2.h"
#include <Imagepp/all/h/ClientThread.h>
//#include "ClientThread2.h"
#include <Imagepp/all/h/PUBRequestProcessor.h>
#include <Imagepp/all/h/PUBRequestDispatcher.h>
#include <Imagepp/all/h/PUBCache.h>
#include <Imagepp/all/h/PUBEngineThread.h>


typedef enum
    {
    SERVER,
    CLIENT
    } ProgramMode;


void main(int pi_Argc, char** pi_Argv)
    {
    WSAData Data;
    WSAStartup(MAKEWORD(1, 1), &Data);
    HFCEvent StopEvent(true, false);

    // take the arguments has the port
    bool Valid = false;
    unsigned short Port;
    uint32_t NumThreads = 1;
    uint32_t NumListeners = 1;
    string  Host("127.0.0.1");
    ProgramMode Mode;
    if (pi_Argc >= 4)
        {
        ctype<char> Converter;
        string Argv1(pi_Argv[1]);
        Converter.tolower(Argv1.begin(), Argv1.end());
        string Argv2(pi_Argv[2]);
        Converter.tolower(Argv2.begin(), Argv2.end());
        string Argv3(pi_Argv[3]);
        Converter.tolower(Argv2.begin(), Argv2.end());

        // verify the first argument
        if ((Argv1 == s_Server) || (Argv1 == s_Client))
            {
            Mode = (Argv1 == s_Server ? SERVER : CLIENT);

            if (Converter.scan_not(ctype<char>::digit, Argv2.begin(), Argv2.end()) == Argv2.end())
                {
                wistringstream Stream(Argv2);
                Stream >> Port;
                Valid = true;
                }

            if (Converter.scan_not(ctype<char>::digit, Argv3.begin(), Argv3.end()) == Argv3.end())
                {
                wistringstream Stream(Argv3);
                Stream >> NumThreads;
                Valid = true;
                }

            if (Valid && (Mode == SERVER) && (pi_Argc >= 5))
                {
                string Argv4(pi_Argv[4]);
                Converter.tolower(Argv4.begin(), Argv4.end());

                if (Converter.scan_not(ctype<char>::digit, Argv4.begin(), Argv4.end()) == Argv4.end())
                    {
                    wistringstream Stream(Argv4);
                    Stream >> NumListeners;
                    Valid = true;
                    }
                else
                    Valid = false;
                }

            if (Valid && (Mode == CLIENT) && (pi_Argc >= 5))
                {
                Host = string(pi_Argv[4]);
                Valid = true;
                }
            }
        }

    if (Valid)
        {
        KeyboardThread KeyboardThread('s', 'd');
        if (Mode == SERVER)
            {
            // Create the request dispatcher
            PUBRequestDispatcher Dispatcher;

            // now, create the engine thread that will empty the dispatcher
            list<PUBEngineThread*> EngineThreads;
            for (uint32_t Engine = 0; Engine < NumThreads; ++Engine)
                EngineThreads.push_back(new PUBEngineThread(Engine + 1, Dispatcher));

            // Create the pool shared with the server and the processor
            HCSSocketConnectionPool Pool(30, 30);
            //HCSBufferedConnectionPool Pool(ThePool, 30, 30);
            //HCSNamedPipeConnectionPool Pool(30, 30);
            //HCSConnectionPool Pool(30, 30);

            // Create the request processor & its cache
            PUBCache            Cache(20);
            list<PUBRequestProcessor*> ProcessorThreads;
            for (uint32_t Processor = 0; Processor < NumThreads; ++Processor)
                ProcessorThreads.push_back(new PUBRequestProcessor(Pool, Cache, Dispatcher));
#if 0
            // now, create the engine thread that will empty the dispatcher
            list<ServerThread*> EngineThreads;
            for (uint32_t Engine = 0; Engine < NumThreads; ++Engine)
                EngineThreads.push_back(new ServerThread(Engine + 1, Pool, StopEvent));
#endif

            // now, the only thing that is missing, is the server listener to
            // fill the pool with connections, which are handled by the processor,
            // which fills the dispatcher with requests that are handled by the
            // engine threads.


            // start the listening thread, which adds connections to the pool
            HCSSocketServerConnectionConfig Config(Port);
            //HCSNamedPipeServerConnectionConfig Config("\\\\.\\pipe\\TEST");
            list<HCSServerListener*> ListenerThreads;
            for (uint32_t Listener = 0; Listener < NumListeners; ++Listener)
                ListenerThreads.push_back(new HCSServerListener(Config, Pool));

            // wait for the user to press the stop key
            while (!KeyboardThread.WaitUntilSignaled(1000))
                {
                cout << endl << "There are " << Pool.GetConnectionCount() << " in the connection pool"<< endl;
                }

            StopEvent.Signal();

            // Stop the listening threads
            while (ListenerThreads.size() > 0)
                {
                delete *ListenerThreads.begin();
                ListenerThreads.pop_front();
                }

            // Stop the processor threads
            while (ProcessorThreads.size() > 0)
                {
                delete *ProcessorThreads.begin();
                ProcessorThreads.pop_front();
                }

            // Stop the engine threads
            while (EngineThreads.size() > 0)
                {
                delete *EngineThreads.begin();
                EngineThreads.pop_front();
                }
            }
        else
            {
            list<ClientThread*> Threads;
            for (uint32_t i = 0; i < NumThreads; ++i)
                Threads.push_back(new ClientThread(i + 1, Host, Port, StopEvent));

            // wait for the user to press the stop key
            KeyboardThread.WaitUntilSignaled();
            StopEvent.Signal();

            while (Threads.size() > 0)
                {
                delete *Threads.begin();
                Threads.pop_front();
                }
            }
        }
    else
        {
        cout << "Usage:" << endl << endl;
        cout << "SocketTest.exe SERVER Port NumThreads" << endl;
        cout << "SocketTest.exe CLIENT Port NumThreads [Host]" << endl;
        }

    WSACleanup();
    }
