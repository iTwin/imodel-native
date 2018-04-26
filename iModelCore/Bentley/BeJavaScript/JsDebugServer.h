/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/JsDebugServer.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <fstream>
#include <iostream>
#include <locale>
#include <stdlib.h>

#if defined (__unix__)
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netdb.h>
#else
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_MEMORY_
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#undef min
#undef max

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4100)
    #pragma warning(disable:4127)
    #pragma warning(disable:4701)
    #pragma warning(disable:4996)
    #pragma warning(disable:6385)
#endif

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeAssert.h>
#include<Bentley/BeThread.h>
#include<Bentley/bset.h>
#include<Bentley/bmap.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                              Marius.Balcaitis    08/2017
//=======================================================================================
struct JsDebugServer : RefCounted<NonCopyableClass>
    {
    #if defined (__unix__)
        typedef int SOCKET_TYPE;
        #define INVALID_SOCKET -1
        #define CLOSE_SOCKET ::close
    #else
        typedef SOCKET SOCKET_TYPE;
        #define CLOSE_SOCKET closesocket
    #endif

    //=======================================================================================
    // @bsiclass                                              Marius.Balcaitis    08/2017
    //=======================================================================================
    struct IConnection : RefCounted<NonCopyableClass>
        {
        virtual void WaitForConnection() = 0;
        virtual bool IsConnected() = 0;
        virtual bool IsValid() = 0;

        virtual int Send(void const *payload, size_t len) = 0;
        virtual int Recv() = 0;
        virtual void Close() = 0;
        };
    typedef RefCountedPtr<IConnection> IConnectionPtr;

    typedef websocketpp::server<websocketpp::config::core> websocketpp_server;

    typedef void(*MainThreadDispatcher)(void(*callback)(void*arg),void*arg);

private:

    //=======================================================================================
    // @bsiclass                                              Marius.Balcaitis    08/2017
    //=======================================================================================
    struct Connection : IConnection
        {
        friend struct JsDebugServer;

    private:
        //=======================================================================================
        // @bsiclass                                              Marius.Balcaitis    08/2017
        //=======================================================================================
        struct Core : RefCounted<NonCopyableClass>
            {
            friend struct JsDebugServer;

        private:
            //=======================================================================================
            // @bsiclass                                              Marius.Balcaitis    08/2017
            //=======================================================================================
            struct Context
                {
                SOCKET_TYPE m_listeningSocket;
                SOCKET_TYPE m_connectedSocket;
                const char* m_listeningPort;

                std::function<void(const std::string&)> m_messageHandler;

                BeMutex m_syncMutex;
                BeConditionVariable m_syncCV;

                bool m_isInitialized;

                Context(const std::function<void(const std::string&)>& messageHandler, const char* listeningPort);
                ~Context();

                bool Initialize();
                void Destroy();
                };

            //=======================================================================================
            // @bsiclass                                                  Steve.Wilson    08/2017
            //=======================================================================================
            struct SocketRelayStreamBuf : public std::streambuf
                {
                private:
                    Core& m_core;

                protected:
                    virtual int_type overflow(int_type c) override;
                    virtual std::streamsize xsputn(const char* s, std::streamsize n) override;

                public:
                    SocketRelayStreamBuf(Core& core) : m_core(core) {};
                };

            //=======================================================================================
            // @bsiclass                                              Marius.Balcaitis    08/2017
            //=======================================================================================
            struct ConnectionCloseHandler
                {
                private:
                    Core& m_core;

                public:
                    ConnectionCloseHandler(Core& core) : m_core(core) {};
                    void operator() (websocketpp::connection_hdl hdl);
                };

            //=======================================================================================
            // @bsiclass                                              Marius.Balcaitis    08/2017
            //=======================================================================================
            struct MessageReceiveHandler
                {
                private:
                    Core& m_core;

                public:
                    MessageReceiveHandler(Core& core) : m_core(core) {};;
                    void operator() (websocketpp::connection_hdl hdl, websocketpp_server::message_ptr msg);
                };


            ConnectionCloseHandler m_closeHandler;
            MessageReceiveHandler  m_msgReceiveHandler;

            SocketRelayStreamBuf m_websocketppOutputStreamBuffer;
            std::ostream m_websocketppOutputStream;
            websocketpp_server::connection_ptr m_websocketppServerConnection;

            bool m_isTerminated;
            bool m_isProcessingMessages;
            int  m_totalBytesSent;

            Context m_context;
            RefCountedPtr<JsDebugServer> m_server;

        
            static void ProcessIncomingMessages(void*);

            int SendInternal(const char* payload, int len);

            void OnSocketConnected();
            void OnSocketClosed();

        public:
            Core(JsDebugServer& server, const std::function<void(const std::string&)>& messageHandler, const char* listeningPort);
            ~Core();

            void Serve();
            void Terminate();

            int Send(void const *payload, size_t len);
            int Recv(bool shouldBlock);

            void WaitForConnection();

            bool IsIncomingDataAvailable();
            bool IsConnected();
            bool IsValid();
            };
        typedef RefCountedPtr<Core> CorePtr;

        CorePtr m_core;

        Connection(JsDebugServer& server, const std::function<void(const std::string&)>& messageHandler, const char* listeningPort);
        ~Connection();

        void CreateThreadForListening();

    public:
        int Send(void const *payload, size_t len) override;
        int Recv() override;

        bool IsConnected() override;
        bool IsValid() override;

        void WaitForConnection() override;
        void Close() override;
        };

    static RefCountedPtr<JsDebugServer> s_instance;
    static MainThreadDispatcher s_mainThreadDispatcher;

    bset<Connection*> m_existingConnections;
    websocketpp_server m_websocketppServer;
    
    void OnConnectionClosed(Connection& connection);

public:
    static void _StartListeningLoop(void* arg);
    static void SetMainThreadDispatcher(MainThreadDispatcher dispatcher);

    static JsDebugServer& GetInstance();
    IConnectionPtr Listen(const std::function<void(const std::string&)>& messageHandler, const char* listeningPort);
    };

END_BENTLEY_NAMESPACE
