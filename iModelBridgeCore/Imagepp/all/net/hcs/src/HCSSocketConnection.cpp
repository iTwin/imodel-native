//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSSocketConnection.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketConnection
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HFCThread.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const WString s_Device(L"socket connection");


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
#ifdef _WIN32
HCSSocketConnection::HCSSocketConnection(SOCKET pi_Socket)
    : HFCInternetConnection(L"", L"", L"")
    {
    HPRECONDITION(pi_Socket != NULL);

    // Copy the socket
    m_hSocket = pi_Socket;

    // That's what we assume!
    m_Blocking = true;

    // Now we're connected
    SetConnected(true);

    // setup the socket input buffer size's to a single 256x256x32 bits tile size.
    uint32_t Size =  262144;
    setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&Size, sizeof(Size));

    // we instanciate the m_pURL for now only with the IP. The port should be implemented
    SOCKADDR_IN PeerAddr;
    int32_t addrLen = sizeof(SOCKADDR_IN);
    memset(&PeerAddr, 0, addrLen);

    if (getpeername (m_hSocket, (SOCKADDR*)&PeerAddr, &addrLen))
        {
        int LastError = WSAGetLastError();
        throw HFCInternetConnectionException(s_Device,
                                             HFCInternetConnectionException::CANNOT_GET_PEER_NAME);
        }

    WString tempoStr;
    BeStringUtilities::CurrentLocaleCharToWChar( tempoStr,inet_ntoa(PeerAddr.sin_addr));

    m_pURL = (HFCURLCommonInternet*)HFCURL::Instanciate(L"http://" + tempoStr);
    }
#endif


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSSocketConnection::HCSSocketConnection(const WString& pi_rServer,
                                         const WString& pi_rUserName,
                                         const WString& pi_rPassword)
    : HFCInternetConnection(pi_rServer, pi_rUserName, pi_rPassword)
    {
    // Normally I would verify that the URL is indeed an common internet, but
    // there is no more class ID.
    // HPRECONDITION(GetURL()->GetClassID() == HFCURLCommonInternet::CLASS_ID);
    m_pURL = (HFCURLCommonInternet*)HFCURL::Instanciate(pi_rServer);

    m_Blocking = true;

#ifdef _WIN32
    m_hSocket = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSSocketConnection::~HCSSocketConnection()
    {
    try
        {
        Disconnect();

#ifdef _WIN32
        closesocket(m_hSocket);
        m_hSocket = NULL;
#endif
        }
    catch(...)
        {
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSSocketConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0 && pi_DataSize < INT_MAX);


    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device,
                                             HFCInternetConnectionException::CONNECTION_LOST);

#ifdef _WIN32
    // Since the send command can send atomically parts of the buffer,
    // loop until all the data has been sent
    const char* pData = (const char*)pi_pData;
    while (pi_DataSize > 0)
        {
        // If the send code is SOCKET_ERROR, throw
        size_t SendSize;
        if ((SendSize = send(m_hSocket, pData, (int)pi_DataSize, 0)) == (int32_t)SOCKET_ERROR)
            {
            int LastError = WSAGetLastError();
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEND);
            }

        // Update
        pi_DataSize -= SendSize;
        pData       += SendSize;
        }
#endif
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSSocketConnection::SendImmediate(const Byte* pi_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(*pio_pDataSize > 0 && *pio_pDataSize < INT_MAX);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

#ifdef _WIN32
    const char* pData = (const char*)pi_pData;
    size_t SendSize;
    if ((SendSize = send(m_hSocket, pData, (int)*pio_pDataSize, 0)) == (int32_t)SOCKET_ERROR)
        {
        int LastError = WSAGetLastError();
        if (LastError != WSAEWOULDBLOCK)
            {
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEND);
            }
        SendSize = 0;
        }

    // Update
    *pio_pDataSize = SendSize;
#endif
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSSocketConnection::Receive(Byte* po_pData, size_t pi_DataSize)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    size_t ReadSize;

    // Read if there is still data to be read
    while ((ReadSize = pi_DataSize) > 0)
        {
        // Read available data
        Receive(po_pData, &ReadSize);

        // update pointer and data size with the previous read data
        po_pData    += ReadSize;
        pi_DataSize -= ReadSize;
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSSocketConnection::Receive(Byte* po_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(po_pData       != 0);
    HPRECONDITION(pio_pDataSize  != 0);
    HPRECONDITION(*pio_pDataSize >  0 && *pio_pDataSize < INT_MAX);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    if (!m_Blocking)
        WaitDataAvailable();

    // read some data from the socket
    size_t ReadSize = recv(m_hSocket, (char*)po_pData, (int)*pio_pDataSize, 0);

    // verify an error
    if ((ReadSize == 0) || (ReadSize == SOCKET_ERROR))
        {
        int LastError = WSAGetLastError();
        Disconnect();
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);
        }

    // at this point, we have read something from the connection
    *pio_pDataSize = ReadSize;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HCSSocketConnection::Connect(const WString& pi_rUserName,
                                   const WString& pi_rPassword,
                                   time_t         pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(!IsConnected());      // must not be already connected
    bool Result = true;

    try
        {
#ifdef _WIN32
        // create the socket handle
        m_hSocket = socket(PF_INET, SOCK_STREAM, 0);
        if (m_hSocket == NULL)
            {
            int LastError = WSAGetLastError();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CREATE);
            }

        // Bind the socket to a port
        struct sockaddr_in SockAddr;
        SockAddr.sin_family      = AF_INET;
        SockAddr.sin_addr.s_addr = INADDR_ANY;
        SockAddr.sin_port        = htons(0);
        if (::bind(m_hSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
            {
            int LastError = WSAGetLastError();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_BIND);
            }

        // Set up the sock address to connect to
        struct sockaddr_in PeerAddr;
        memset(&PeerAddr, 0, sizeof(PeerAddr));
        PeerAddr.sin_family       = AF_INET;
        PeerAddr.sin_port         = htons((u_short)BeStringUtilities::Wtoi(m_pURL->GetPort().c_str()));

        size_t  destinationBuffSize = m_pURL->GetHost().GetMaxLocaleCharBytes();
        char*  URLMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(URLMBS, m_pURL->GetHost().c_str(),destinationBuffSize);

        PeerAddr.sin_addr.s_addr  = inet_addr(URLMBS);

        // if no address was given by host, try to identify through DNS
        struct hostent* pHost;
        if ((PeerAddr.sin_addr.s_addr == INADDR_NONE) &&
            (pHost = gethostbyname(URLMBS) ))
            {
            PeerAddr.sin_addr.s_addr = ((struct in_addr*)pHost->h_addr)->s_addr;
            }

        // Cannot resolve to a dotted address
        if (PeerAddr.sin_addr.s_addr == INADDR_NONE)
            {
            int LastError = WSAGetLastError();
            throw HFCInternetConnectionException(s_Device,
                                                 HFCInternetConnectionException::INVALID_DOMAIN_NAME);
            }

        // Set the socket in nonblocking mode
        uint32_t NonBlocking = m_Blocking ? 0 : 1;
        ioctlsocket(m_hSocket, FIONBIO, &NonBlocking);

        // Connect, if unable, throw
        if (connect(m_hSocket, (SOCKADDR*)&PeerAddr, sizeof(PeerAddr)) == SOCKET_ERROR)
            {
            // prepare parameters for the select
            fd_set         SocketSet = {1, {m_hSocket}};
            struct timeval TimeOut   = {(long)pi_TimeOut / 1000, (pi_TimeOut%1000) * 1000};

            // If the error is not WOULDBLOCK or the select fails, then throw an exception
            if ((WSAGetLastError() != WSAEWOULDBLOCK) ||
                (select(0, NULL, &SocketSet, NULL, &TimeOut) != 1))
                {
                int LastError = WSAGetLastError();
                throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SELECT);
                }
            }

        // Now we're connected
        SetConnected(true);

        // Reset the socket in blocking mode
        NonBlocking = 0;
        ioctlsocket(m_hSocket, FIONBIO, &NonBlocking);

        // setup the socket input buffer size's to a single 256x256x32 bits tile size.
        uint32_t Size =  262144;
        setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&Size, sizeof(Size));
#endif
        Result = true;
        }
    catch(...)
        {
        Disconnect();
        Result = false;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HCSSocketConnection::ValidateConnect(uint32_t pi_TimeOut)
    {
    bool Result = IsConnected();

    if (!Result)
        Result = Connect(m_UserName,
                         m_Password,
                         pi_TimeOut);

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSSocketConnection::Disconnect()
    {
    // Set the connection state to false
    SetConnected(false);

    // Close the socket
#ifdef _WIN32
    // only shutdown the socket.  we will close the handle only when the
    // object is being destroyed.  Shutdown in send only so that we can call
    // recv succesfully while there is accumulated data in the buffer.
    shutdown(m_hSocket, 1);
#endif
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HCSSocketConnection::WaitDataAvailable()
    {
    uint32_t Result = 0;

    // Verify the readability on the socket in order to detect if data has arrived
    // or if the connection was closed
    fd_set SocketSet = {1, {m_hSocket}};
    if (select(0, &SocketSet, NULL, NULL, NULL) == 1)
        {
        // peek on byte on the socket in order to detect if the socket received data
        // or if the connection was closed.
        //
        // recv results:
        //
        //  >0              Data on the socket
        //  0               Closed Gracefully
        //  SOCKET_ERROR    Connection error
        char Buffer;
        if ((recv(m_hSocket, &Buffer, 1, MSG_PEEK) != 1) ||
            (ioctlsocket(m_hSocket, FIONREAD, &Result) == SOCKET_ERROR))
            {
            int LastError = WSAGetLastError();
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);
            }
        }

    return Result;
    }



//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HCSSocketConnection::WaitDataAvailable(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    u_long Result = 0;

    // Verify the readability on the socket in order to detect if data has arrived
    // or if the connection was closed
    fd_set SocketSet = {1, {m_hSocket}};
    struct timeval TimeOut = {pi_TimeOut/1000, (pi_TimeOut%1000) * 1000};
    if (select(0, &SocketSet, NULL, NULL, &TimeOut) == 1)
        {
        // peek on byte on the socket in order to detect if the socket received data
        // or if the connection was closed.
        //
        // recv results:
        //
        //  >0              Data on the socket
        //  0               Closed Gracefully
        //  SOCKET_ERROR    Connection error
        char Buffer;
        if ((recv(m_hSocket, &Buffer, 1, MSG_PEEK) != 1) ||
            (ioctlsocket(m_hSocket, FIONREAD, &Result) == SOCKET_ERROR))
            {
            int LastError = WSAGetLastError();
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);
            }
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HCSSocketConnection::WaitDataWriteable()
    {
    bool Result = false;

    fd_set SocketSet = {1, {m_hSocket}};
    if (select(0, NULL, &SocketSet, NULL, NULL) == SOCKET_ERROR)
        {
        int LastError = WSAGetLastError();
        Disconnect();
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SELECT);
        }
    else
        {
        Result = true;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HCSSocketConnection::WaitDataWriteable(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);

    fd_set SocketSet = {1, {m_hSocket}};
    struct timeval TimeOut = {pi_TimeOut/1000, (pi_TimeOut%1000) * 1000};

    int SelectResult = select(0, NULL, &SocketSet, NULL, &TimeOut);

    if (SelectResult == SOCKET_ERROR)
        {
        int LastError = WSAGetLastError();
        Disconnect();
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SELECT);
        }
    else
        return (SelectResult > 0);
    }

//-----------------------------------------------------------------------------
// public
// Return the URL of the client
//-----------------------------------------------------------------------------
HFCPtr<HFCURLCommonInternet> HCSSocketConnection::GetURL()
    {
    return m_pURL;
    }


//-----------------------------------------------------------------------------
// public
// Set the blocking mode of the socket
//-----------------------------------------------------------------------------
bool HCSSocketConnection::SetBlockingMode(bool pi_Blocking)
    {
    bool Result = false;

    if (pi_Blocking != m_Blocking)
        {
#ifdef _WIN32
        if (m_hSocket != NULL)  // We at least need a socket
            {
            // Set the socket in nonblocking mode
            uint32_t NonBlocking = pi_Blocking ? 0 : 1;
            if (ioctlsocket(m_hSocket, FIONBIO, &NonBlocking) != SOCKET_ERROR)
                {
                m_Blocking = pi_Blocking;
                Result = true;
                }
            }
        else
            {
            // In this case, we will set the mode in Connect()
            Result = true;
            }
#endif
        }

    return Result;
    }
