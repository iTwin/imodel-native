/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/JsDebugServer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "JsDebugServer.h"

BEGIN_BENTLEY_NAMESPACE

#if defined (BENTLEYCONFIG_OS_WINDOWS)
static void cleanupWSA()
    {
    WSACleanup();
    }
#endif

RefCountedPtr<JsDebugServer> JsDebugServer::s_instance;
JsDebugServer::MainThreadDispatcher JsDebugServer::s_mainThreadDispatcher = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer& JsDebugServer::GetInstance()
    {
    BeAssert(s_mainThreadDispatcher != nullptr);

    if (s_instance.IsNull())
        {
        #if defined (BENTLEYCONFIG_OS_WINDOWS)
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            atexit(cleanupWSA);
        #endif

        s_instance = new JsDebugServer();
        }

    return *s_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::IConnectionPtr JsDebugServer::Listen(const std::function<void(const std::string&)>& messageHandler, const char* listeningPort)
    {
    for (auto it = m_existingConnections.begin(); it != m_existingConnections.end(); it++)
        {
        if (0 == strcmp(((*it)->m_core->m_context.m_listeningPort), listeningPort))
            {
            NativeLogging::LoggingManager::GetLogger("JsDebugServer")->error("Failed to create connection because listening port is already used by other connection");
            return nullptr;
            }
        }

    Connection* connection = new Connection(*this, messageHandler, listeningPort);
    m_existingConnections.insert(connection);

    return connection;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::_StartListeningLoop(void* arg)
    {
    Connection::Core* core = reinterpret_cast<Connection::Core*>(arg);
    core->Serve();
    core->m_context.m_syncCV.notify_one();
    core->Release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::SetMainThreadDispatcher(MainThreadDispatcher dispatcher)
    {
    if (s_mainThreadDispatcher == nullptr)
        {
        BeAssert(dispatcher != nullptr);
        s_mainThreadDispatcher = dispatcher;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::OnConnectionClosed(Connection& connection)
    {
    m_existingConnections.erase(&connection);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::Core::Context::Context(const std::function<void(const std::string&)>& messageHandler, const char* listeningPort) :
    m_messageHandler(messageHandler),
    m_listeningPort(listeningPort),
    m_isInitialized(false),
    m_listeningSocket(INVALID_SOCKET),
    m_connectedSocket(INVALID_SOCKET)
    {
    BeAssert(listeningPort != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::Core::Context::~Context()
    {
    Destroy();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::Context::Destroy()
    {
    if (m_listeningSocket != INVALID_SOCKET)
        {
        CLOSE_SOCKET(m_listeningSocket);
        m_listeningSocket = INVALID_SOCKET;
        }
    if (m_connectedSocket != INVALID_SOCKET)
        {
        CLOSE_SOCKET(m_connectedSocket);
        m_connectedSocket = INVALID_SOCKET;
        }
    m_messageHandler = nullptr;
    m_isInitialized = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::Core::Context::Initialize()
    {
    if (m_isInitialized)
        return true;

    BeAssert(m_listeningPort != nullptr);

    struct addrinfo addrInfoHints;
    std::memset(&addrInfoHints, 0, sizeof(addrInfoHints));

    addrInfoHints.ai_family = AF_INET;
    addrInfoHints.ai_socktype = SOCK_STREAM;
    addrInfoHints.ai_protocol = IPPROTO_TCP;
    addrInfoHints.ai_flags = AI_PASSIVE;

    struct addrinfo *addrInfoResult = NULL;
    getaddrinfo(NULL, m_listeningPort, &addrInfoHints, &addrInfoResult);

    m_listeningSocket = socket(addrInfoResult->ai_family, addrInfoResult->ai_socktype, addrInfoResult->ai_protocol);
    if (m_listeningSocket == INVALID_SOCKET)
        {
        NativeLogging::LoggingManager::GetLogger("JsDebugServer")->error("Failed to create listening socket");
        freeaddrinfo(addrInfoResult);
        return false;
        }

    int reuse = 1;
    if (setsockopt(m_listeningSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        {
        NativeLogging::LoggingManager::GetLogger("JsDebugServer")->error("Setsockopt(SO_REUSEADDR) failed");
        freeaddrinfo(addrInfoResult);
        CLOSE_SOCKET(m_listeningSocket);
        m_listeningSocket = INVALID_SOCKET;
        return false;
        }

    if (0 != bind(m_listeningSocket, addrInfoResult->ai_addr, (int)addrInfoResult->ai_addrlen))
        {
        NativeLogging::LoggingManager::GetLogger("JsDebugServer")->error("Failed to bind listening socket");
        freeaddrinfo(addrInfoResult);
        CLOSE_SOCKET(m_listeningSocket);
        m_listeningSocket = INVALID_SOCKET;
        return false;
        }

    freeaddrinfo(addrInfoResult);

    if (0 != ::listen(m_listeningSocket, SOMAXCONN))
        {
        NativeLogging::LoggingManager::GetLogger("JsDebugServer")->error("Failed to listen on listening socket");
        CLOSE_SOCKET(m_listeningSocket);
        m_listeningSocket = INVALID_SOCKET;
        return false;
        }
    
    m_isInitialized = true;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Steve.Wilson    08/2017
//--------------+------------------------------------------------------------------------
std::streambuf::int_type JsDebugServer::Connection::Core::SocketRelayStreamBuf::overflow(int_type c)
    {
    if (c != EOF)
        {
        char ch = static_cast <char> (c);
        if (m_core.SendInternal(&ch, 1) != 1)
            return EOF;
        }

    return c;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Steve.Wilson    08/2017
//--------------+------------------------------------------------------------------------
std::streamsize JsDebugServer::Connection::Core::SocketRelayStreamBuf::xsputn(const char* s, std::streamsize n)
    {
    return m_core.SendInternal(s, static_cast <int> (n));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::Connection(JsDebugServer& server, const std::function<void(const std::string&)>& messageHandler, const char* listeningPort)
    {
    m_core = new Core(server, messageHandler, listeningPort);

    m_core->AddRef();
    CreateThreadForListening();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::~Connection()
    {
    Close();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
#if defined (__unix__)
static void* ListeningThreadHandler(void* arg)
#else
static unsigned __stdcall ListeningThreadHandler(void* arg)
#endif
    {
    BeThreadUtilities::SetCurrentThreadName("Bentley_V8JsDebug");
    JsDebugServer::_StartListeningLoop(arg);
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::CreateThreadForListening()
    {
    m_core->AddRef();
    BeThreadUtilities::StartNewThread(ListeningThreadHandler, m_core.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
int JsDebugServer::Connection::Send(void const *payload, size_t len)
    {
    if (m_core.IsNull())
        return -1;

    return m_core->Send(payload, len);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
int JsDebugServer::Connection::Recv()
    {
    if (m_core.IsNull())
        return -1;

    return m_core->Recv(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::IsConnected()
    {
    return (!m_core.IsNull() && m_core->IsConnected());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::IsValid()
    {
    return (!m_core.IsNull() && m_core->IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::WaitForConnection()
    {
    if (m_core.IsNull())
        return;

    m_core->WaitForConnection();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Close()
    {
    if (m_core.IsNull())
        return;

    m_core->Terminate();
    m_core->m_server->OnConnectionClosed(*this);
    m_core = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::MessageReceiveHandler::operator()(websocketpp::connection_hdl hdl, websocketpp_server::message_ptr msg)
    {
    if (msg->get_opcode() == websocketpp::frame::opcode::text)
        {
        m_core.m_context.m_messageHandler(msg->get_payload());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::ConnectionCloseHandler::operator()(websocketpp::connection_hdl hdl)
    {
    if (m_core.m_context.m_connectedSocket != INVALID_SOCKET)
        {
        CLOSE_SOCKET(m_core.m_context.m_connectedSocket);
        m_core.m_context.m_connectedSocket = INVALID_SOCKET;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::Core::Core(JsDebugServer& server, const std::function<void(const std::string&)>& messageHandler, const char* listeningPort) :
    m_context(messageHandler, listeningPort),
    m_websocketppOutputStreamBuffer(*this),
    m_websocketppOutputStream(&m_websocketppOutputStreamBuffer),
    m_server(&server),
    m_isTerminated(false),
    m_isProcessingMessages(false),
    m_closeHandler(*this),
    m_msgReceiveHandler(*this)
    {
    m_context.Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
JsDebugServer::Connection::Core::~Core()
    {
    Terminate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::Terminate()
    {
    if (m_isTerminated)
        return;

    BeMutexHolder stateSync(m_context.m_syncMutex);
    m_isTerminated = true;
    OnSocketClosed();
    m_context.Destroy();
    m_context.m_syncCV.notify_one();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::ProcessIncomingMessages(void* context)
    {
    Core* core = reinterpret_cast<Core*>(context);
    BeMutexHolder syncProcessing(core->m_context.m_syncMutex);

    if (core->IsValid() && core->IsConnected())
        {
        core->Recv(false);
        }

    core->m_isProcessingMessages = false;
    core->m_context.m_syncCV.notify_one();

    core->Release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::OnSocketConnected()
    {
    m_websocketppServerConnection = m_server->m_websocketppServer.get_connection();

    m_websocketppServerConnection->set_close_handler(m_closeHandler);
    m_websocketppServerConnection->set_fail_handler(m_closeHandler);
    m_websocketppServerConnection->set_message_handler(m_msgReceiveHandler);
    m_websocketppServerConnection->register_ostream(&m_websocketppOutputStream);
    m_websocketppServerConnection->start();

    m_totalBytesSent = 0;

    m_context.m_syncCV.notify_one();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::OnSocketClosed()
    {
    if (m_websocketppServerConnection.get() == nullptr)
        return;

    websocketpp::session::state::value state = m_websocketppServerConnection->get_state();
    if (state != websocketpp::session::state::closing && state != websocketpp::session::state::closed)
        {
        std::error_code ec;
        m_websocketppServerConnection->terminate(ec);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::Serve()
    {
    BeMutexHolder syncState(m_context.m_syncMutex);

    if (JsDebugServer::s_mainThreadDispatcher == nullptr)
        return;

    while (IsValid())
        {
        syncState.unlock();
        m_context.m_connectedSocket = accept(m_context.m_listeningSocket, NULL, NULL);
        syncState.lock();

        if (m_context.m_connectedSocket == INVALID_SOCKET)
            {
            continue;
            }
        else if(!IsValid())
            {
            CLOSE_SOCKET(m_context.m_connectedSocket);
            m_context.m_connectedSocket = INVALID_SOCKET;
            continue;
            }

        OnSocketConnected();

        fd_set readfds;
        while (IsValid() && IsConnected())
            {
            FD_ZERO(&readfds);
            FD_SET(m_context.m_connectedSocket, &readfds);

            syncState.unlock();
            int status = select((int)(m_context.m_connectedSocket + 1), &readfds, NULL, NULL, NULL);
            syncState.lock();

            if (status < 1)
                {
                OnSocketClosed();
                break;
                }
            else
                {
                if (!IsIncomingDataAvailable())
                    continue;

                if (!m_isProcessingMessages)
                    {
                    this->AddRef();
                    JsDebugServer::s_mainThreadDispatcher(&ProcessIncomingMessages, this);
                    m_isProcessingMessages = true;
                    }

                while (m_isProcessingMessages && IsConnected())
                    {
                    m_context.m_syncCV.InfiniteWait(syncState);
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::Core::IsIncomingDataAvailable()
    {
    if (!IsConnected())
        return false;
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_context.m_connectedSocket, &readfds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    int result = select((int)(m_context.m_connectedSocket + 1), &readfds, NULL, NULL, &timeout);
    if (result > 0)
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
int JsDebugServer::Connection::Core::Send(void const *payload, size_t len)
    {
    BeMutexHolder(m_context.m_syncMutex);

    if (!IsValid() || !IsConnected())
        return -1;

    m_totalBytesSent = 0;
    m_websocketppServerConnection->send(payload, len, websocketpp::frame::opcode::text);
    if (m_totalBytesSent < 0)
        {
        OnSocketClosed();
        }

    return m_totalBytesSent;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
int JsDebugServer::Connection::Core::Recv(bool shouldBlock)
    {
    BeMutexHolder(m_context.m_syncMutex);

    if (!IsValid() || !IsConnected())
        return -1;

    char clientSocketRecvBuffer[1024];
    int clientSocketRecvResponse;
    size_t clientSocketReceivedBytes;
    size_t websocketppReadStart;

    int recvNum = 0;

    while (IsIncomingDataAvailable() || shouldBlock)
        {
        shouldBlock = false;
        clientSocketRecvResponse = ::recv(m_context.m_connectedSocket, clientSocketRecvBuffer, _countof(clientSocketRecvBuffer), 0);

        if (clientSocketRecvResponse > 0)
            {
            clientSocketReceivedBytes = clientSocketRecvResponse;
            websocketppReadStart = 0;
            while (websocketppReadStart < clientSocketReceivedBytes)
                {
                websocketppReadStart += m_websocketppServerConnection->readsome(
                    clientSocketRecvBuffer + websocketppReadStart,
                    clientSocketReceivedBytes - websocketppReadStart);
                }
            }
        else
            {
            OnSocketClosed();
            return clientSocketRecvResponse;
            }

        recvNum += clientSocketRecvResponse;
        }

    return recvNum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::Core::IsConnected()
    {
    BeMutexHolder syncState(m_context.m_syncMutex);
    return (m_context.m_connectedSocket != INVALID_SOCKET);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool JsDebugServer::Connection::Core::IsValid()
    {
    BeMutexHolder syncState(m_context.m_syncMutex);
    return (!m_isTerminated && m_context.m_isInitialized);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void JsDebugServer::Connection::Core::WaitForConnection()
    {
    BeMutexHolder waitForConnection(m_context.m_syncMutex);
    while (!IsConnected() && IsValid())
        {
        m_context.m_syncCV.notify_one();
        m_context.m_syncCV.InfiniteWait(waitForConnection);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
int JsDebugServer::Connection::Core::SendInternal(const char* payload, int len)
    {
    // We should return negative number in case of failure, but we are returning length in all cases because
    // websocketpp continues writing buffers to stream event after it detects that the stream is bad and
    // terminates the connection which causes illegal memory access exceptions (buffers are iterated too far)

    if (m_totalBytesSent < 0)
        return len;

    int nBytesSent = ::send(m_context.m_connectedSocket, payload, len, 0);
    if (nBytesSent > 0)
        m_totalBytesSent += nBytesSent;
    else
        m_totalBytesSent = nBytesSent;

    return len;
    }

END_BENTLEY_NAMESPACE
