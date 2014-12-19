//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSSocketServerConnection.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketServerConnection
//-----------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSSocketServerConnection.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HCSException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const WString s_Device(L"socket connection");


//-----------------------------------------------------------------------------
// Public
// Default contructor, which create a server socket on a winsock assigned port
//-----------------------------------------------------------------------------
HCSSocketServerConnection::HCSSocketServerConnection()
    : HCSServerConnection(L"")
    {
    // Initialize the backlog and the share
    m_Backlog = UINT_MAX;
    m_Shared  = false;

    // Use 0, winsock will assign an empty port when the socket will be created
    m_Port = 0;

#ifdef _WIN32
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketServerConnection::HCSSocketServerConnection(unsigned short pi_Port)
    : HCSServerConnection(L"")
    {
    HPRECONDITION(pi_Port > 0);

    // Initialize the backlog and the share
    m_Backlog = UINT_MAX;
    m_Shared  = false;

    m_Port    = pi_Port;

#ifdef _WIN32
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketServerConnection::HCSSocketServerConnection(const string& pi_rAddress)
    : HCSServerConnection(L"")
    {
    HPRECONDITION(!pi_rAddress.empty());

    // Initialize the backlog and the share
    m_Backlog = UINT_MAX;
    m_Shared  = false;

    m_Address = pi_rAddress;
    m_Port    = 0;

#ifdef _WIN32
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketServerConnection::HCSSocketServerConnection(const string& pi_rAddress,
                                                     unsigned short pi_Port)
    : HCSServerConnection(L"")
    {
    HPRECONDITION(!pi_rAddress.empty());
    HPRECONDITION(pi_Port > 0);

    // Initialize the backlog and the share
    m_Backlog = UINT_MAX;
    m_Shared  = false;

    m_Address = pi_rAddress;
    m_Port    = pi_Port;

#ifdef _WIN32
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketServerConnection::~HCSSocketServerConnection()
    {
    try
        {
        Disconnect();
        }
    catch(...)
        {
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HCSSocketServerConnection::Connect(time_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(!IsConnected());      // must not be already connected
    HPRECONDITION(m_hSocket == NULL);   // see above

    try
        {
#ifdef _WIN32
        // create the socket handle
        m_hSocket = socket(PF_INET, SOCK_STREAM, 0);
        if (m_hSocket == NULL)
            throw HCSException(HCS_CANNOT_CREATE_EXCEPTION, s_Device);

        // Share the socket (to be reused by another server socket), if needed
        if (m_Shared)
            {
            bool Reuse =  1;
            setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&Reuse, sizeof(Reuse));
            }

        // Bind the socket to an address and port
        struct sockaddr_in SockAddr;
        SockAddr.sin_family = AF_INET;

        // Set the address.
        //
        // 1. If the address is empty, bind to any address.
        // 2. If not empty, convert the address string to an integer representation
        // 3. If not empty and cannot be converted, use the DNS
        SockAddr.sin_addr.s_addr = INADDR_ANY;
        if (!m_Address.empty())
            {
            // try to convert the address from a string to an integer
            SockAddr.sin_addr.s_addr  = inet_addr(m_Address.c_str());

            // if it was successful, the address will be anything but INADDR_NONE
            if (SockAddr.sin_addr.s_addr == INADDR_NONE)
                {
                // could not map the address to an integer representation, DNS it!
                struct hostent* pHost = gethostbyname(m_Address.c_str());
                if (pHost == 0)
                    throw HCSException(HCS_INVALID_DOMAIN_NAME_EXCEPTION, s_Device);
                SockAddr.sin_addr.s_addr = ((struct in_addr*)pHost->h_addr)->s_addr;
                }
            }

        // Set the port
        SockAddr.sin_port = htons(m_Port);

        // Bind to the address and port
        if (::bind(m_hSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
            throw HCSException(HCS_CANNOT_BIND_EXCEPTION, s_Device);

        // place the socket in a listen mode
        int32_t Backlog = SOMAXCONN;
        if (m_Backlog < INT_MAX)
            Backlog = m_Backlog;
        if (listen(m_hSocket, Backlog) == SOCKET_ERROR)
            throw HCSException(HCS_CANNOT_LISTEN_EXCEPTION, s_Device);

        // Now get the address and port to which we are binded
        SOCKADDR_IN sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        int nSockAddrLen = sizeof(sockAddr);
        if (getsockname(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen) == SOCKET_ERROR)
            throw HCSException(HCS_CANNOT_GET_NAME_EXCEPTION, s_Device);
        m_Address = inet_ntoa(sockAddr.sin_addr);
        m_Port    = ntohs(sockAddr.sin_port);

        if (m_Address.compare("0.0.0.0") == 0)
            {
            char HostName[256];
            if (gethostname(HostName, 256) != SOCKET_ERROR)
                {
                LPHOSTENT lphe = gethostbyname(HostName);
                m_Address = inet_ntoa(*(struct in_addr far*)(lphe->h_addr));
                }
            }
#endif

        // if we get through here, we are connected
        SetConnected(true);
        }
    catch(...)
        {
        Disconnect();
        }

    return IsConnected();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSSocketServerConnection::Disconnect()
    {
    // Set the connection state to false
    SetConnected(false);

    // Close the socket
#ifdef _WIN32
    closesocket(m_hSocket);
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSSocketServerConnection::Accept()
    {
    HAutoPtr<HCSSocketConnection> pResult;

    // Normally, there should be a precondition to verify that the connection is
    // connected before trying to accept.  Instead, because this class can be controlled
    // by another thread, which closes the connection, we will throw if the conenction
    // is not connected.
    if (!IsConnected())
        throw HCSException(HCS_NOT_CONNECTED_EXCEPTION, s_Device);

#ifdef _WIN32
    // Initialize the peer's socket address structure
    SOCKADDR_IN sockAddr;
    int32_t addrLen = sizeof(SOCKADDR_IN);
    memset(&sockAddr, 0, addrLen);

    // Accept a connection
    SOCKET hSocket = accept(m_hSocket, (SOCKADDR*)&sockAddr, &addrLen);
    if( hSocket != INVALID_SOCKET )
        pResult = new HCSSocketConnection(hSocket);
    else
        {
        int LastError = WSAGetLastError();
        cout << endl << "accept error: " << LastError << endl;
        throw HCSException(HCS_CANNOT_ACCEPT_CONNECTION_EXCEPTION, s_Device);
        }

#endif

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSSocketServerConnection::Accept(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HAutoPtr<HCSSocketConnection> pResult;

    // Normally, there should be a precondition to verify that the connection is
    // connected before trying to accept.  Instead, because this class can be controlled
    // by another thread, which closes the connection, we will throw if the conenction
    // is not connected.
    if (!IsConnected())
        throw HCSException(HCS_NOT_CONNECTED_EXCEPTION, s_Device);

#ifdef _WIN32
    // prepare parameters for the select
    fd_set         SocketSet = {1, {m_hSocket}};
    struct timeval TimeOut   = {pi_TimeOut / 1000, (pi_TimeOut%1000) * 1000};
    if (select(0, NULL, &SocketSet, NULL, &TimeOut) == 1)
        pResult = (HCSSocketConnection*)Accept();
#endif

    return pResult.release();
    }

