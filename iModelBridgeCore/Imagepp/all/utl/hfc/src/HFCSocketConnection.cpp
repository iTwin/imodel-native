//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCSocketConnection.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCSocketConnection
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCSocketConnection.h>
#include <Imagepp/all/h/HFCThread.h>

#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
#include <Winsock2.h>
#include <Winerror.h>
#include <wininet.h>
#endif

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const WString s_Device(L"socket connection");
static const int32_t s_SleepTime = 100;
static const size_t s_GrowSize = 1024;
static const size_t s_MaxSize  = 128 * 1024;
static const size_t s_ReadSize = 16 * 1024;


//-----------------------------------------------------------------------------
// SocketThread Declaration
//-----------------------------------------------------------------------------
class ImagePP::SocketThread : public HFCThread
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    SocketThread(HFCSocketConnection& pi_rConnection);
    ~SocketThread();


    //--------------------------------------
    // HFCThread Overrides
    //--------------------------------------

    virtual void    Go();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Reference to the connection that owns the thread
    HFCSocketConnection&
    m_rConnection;
    };

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFCSocketConnection::HFCSocketConnection(const WString& pi_rServer,
                                         const WString& pi_rUserName,
                                         const WString& pi_rPassword)
    : HFCInternetConnection(pi_rServer, pi_rUserName, pi_rPassword),
      m_Buffer             (s_GrowSize, s_MaxSize),
      m_SocketEvent        (true, false)
    {
    // Normally I would verify that the URL is indeed an common internet, but
    // there is no more class ID.
    // HPRECONDITION(GetURL()->GetClassID() == HFCURLCommonInternet::CLASS_ID);

    m_pURL = (HFCURLCommonInternet*)HFCURL::Instanciate(pi_rServer);

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    m_hSocket = 0;
#endif
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCSocketConnection::~HFCSocketConnection()
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
// public
//
//-----------------------------------------------------------------------------
void HFCSocketConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // Since the send command can send atomically parts of the buffer,
    // loop until all the data has been sent
    const char* pData = (const char*)pi_pData;
    while (pi_DataSize > 0)
        {
        // Check the data available without waiting, just to detect
        // disconnections.
        WaitDataAvailable(0);

        int32_t SendSize;
        int32_t ToSend = (int32_t)pi_DataSize;
        if (pi_DataSize > INT_MAX)
            ToSend = INT_MAX;

        // If the send code is SOCKET_ERROR, throw
        if ((SendSize = send(m_hSocket, pData, ToSend, 0)) == (int)SOCKET_ERROR)
            {
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
void HFCSocketConnection::Receive(Byte* po_pData, size_t pi_DataSize)
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
void HFCSocketConnection::Receive(Byte* po_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(po_pData       != 0);
    HPRECONDITION(pio_pDataSize  != 0);
    HPRECONDITION(*pio_pDataSize >  0);

    size_t DataAvailable;

    // Wait for data.  Will block until something happens, good or bad.
    if ((DataAvailable = WaitDataAvailable()) > 0)
        {
        // Compute the size of the data to copy
        *pio_pDataSize = MIN(DataAvailable, *pio_pDataSize);

        // Copy the data into the output buffer
        HFCMonitor BufferMonitor(m_BufferKey);
        memcpy(po_pData, m_Buffer.GetData(), *pio_pDataSize);

        // Update the buffer.  If it becomes empty, reset the socket event
        m_Buffer.MarkReadData(*pio_pDataSize);
        if (m_Buffer.GetDataSize() == 0)
            m_SocketEvent.Reset();
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HFCSocketConnection::Connect(const WString& pi_rUserName,
                                   const WString& pi_rPassword,
                                   time_t         pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(!IsConnected());      // must not be already connected
    HPRECONDITION(m_hSocket == 0);      // see above
    //HPRECONDITION(m_pThread == 0);
    bool Result = true;

    try
        {
        m_pThread = 0;

#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
        // create the socket handle
        m_hSocket = socket(PF_INET, SOCK_STREAM, 0);
        if (m_hSocket == 0)
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CREATE);

        // Bind the socket to a port
        struct sockaddr_in SockAddr;
        SockAddr.sin_family      = AF_INET;
        SockAddr.sin_addr.s_addr = INADDR_ANY;
        SockAddr.sin_port        = htons(0);
        if (::bind(m_hSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_BIND);

        // Set up the sock address to connect to
        struct sockaddr_in PeerAddr;
        memset(&PeerAddr, 0, sizeof(PeerAddr));
        PeerAddr.sin_family       = AF_INET;
        PeerAddr.sin_port         = htons((USHORT)BeStringUtilities::Wtoi(m_pURL->GetPort().c_str()));

        size_t  destinationBuffSize = m_pURL->GetHost().GetMaxLocaleCharBytes();
        char*  HostMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(HostMBS, m_pURL->GetHost().c_str(),destinationBuffSize);
        PeerAddr.sin_addr.s_addr  = inet_addr(HostMBS);

        // if no address was given by host, try to identify through DNS
        struct hostent* pHost;
        if ((PeerAddr.sin_addr.s_addr == INADDR_NONE) && (pHost = gethostbyname(HostMBS)) )
            {
            PeerAddr.sin_addr.s_addr = ((struct in_addr*)pHost->h_addr)->s_addr;
            }

        // Cannot resolve to a dotted address
        if (PeerAddr.sin_addr.s_addr == INADDR_NONE)
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);

        // Set the socket in nonblocking mode
        u_long NonBlocking = 1;
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
                throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);
            }

        // Now we're connected
        SetConnected(true);
        m_SocketEvent.Reset();

        // Reset the socket in blocking mode
        NonBlocking = 0;
        ioctlsocket(m_hSocket, FIONBIO, &NonBlocking);

        // setup the socket input buffer size's to a single 256x256x32 bits tile size.
        uint32_t Size =  262144;
        setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&Size, sizeof(Size));
#endif

        // Start the socket receive thread
        m_pThread = new SocketThread(*this);
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
bool HFCSocketConnection::ValidateConnect(uint32_t pi_TimeOut)
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
void HFCSocketConnection::Disconnect()
    {
    // Set the connection state to false
    SetConnected(false);

    // Close the socket
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // make sure that the socket closes gracefully without waiting here for it
    bool Option = 1;
    setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&Option, sizeof(Option));

    // Close the socket
    closesocket(m_hSocket);
    m_hSocket = 0;
#endif

    // Destroy the thread
    m_pThread = 0;

    // Signal something for other thread to unblock
    m_SocketEvent.Signal();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HFCSocketConnection::WaitDataAvailable()
    {
    // Wait until the socket event is signaled
    m_SocketEvent.WaitUntilSignaled();

    // If the connection is lost, throw up
    if ((m_Buffer.GetDataSize() == 0) && !IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // Read the data size of the buffer
    HFCMonitor BufferMonitor(m_BufferKey);
    return m_Buffer.GetDataSize();
    }



//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HFCSocketConnection::WaitDataAvailable(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);

    // Wait until the socket event is signaled
    m_SocketEvent.WaitUntilSignaled(pi_TimeOut);

    HFCMonitor BufferMonitor(m_BufferKey);

    // If there is no data in the buffer and the connection is lost, throw.
    // This will permit to empty the buffer even if the connection is lost.
    if ((m_Buffer.GetDataSize() == 0) && !IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // Read the data size of the buffer
    return m_Buffer.GetDataSize();
    }




//-----------------------------------------------------------------------------
// public
// ntohl
//-----------------------------------------------------------------------------
uint32_t HFCSocketConnection::ntohl2 (uint32_t pi_NetLong)
{
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    return ::ntohl(pi_NetLong);
#endif
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// SocketThread Implementation
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
SocketThread::SocketThread(HFCSocketConnection& pi_rConnection)
    : HFCThread(false),
      m_rConnection(pi_rConnection)
    {
    HPRECONDITION(pi_rConnection.m_hSocket != 0);

    // Start the thread
    StartThread();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
SocketThread::~SocketThread()
    {
    // make sure that the socket closes gracefully without waiting here for it
    bool Option = 1;
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    setsockopt(m_rConnection.m_hSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&Option, sizeof(Option));

    // close the socket to force any blocking to end
    closesocket(m_rConnection.m_hSocket);
#endif
    m_rConnection.m_hSocket = 0;

    // Signal that the thread can stop
    StopThread();

    // And wait until it does
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void SocketThread::Go()
    {
    HArrayAutoPtr<Byte> pBuffer(new Byte[s_ReadSize]);
    fd_set               SocketSet;
    int32_t             TimeOut;
    struct timeval       TimeVal;
    int32_t              ReadSize;

    try
        {
        // loop
        while (CanRun())
            {
#if defined (ANDROID) || defined (__APPLE__)
                //DM-Android
#elif defined (_WIN32)
            // Setup select params
            FD_ZERO(&SocketSet);
            FD_SET(m_rConnection.m_hSocket, &SocketSet);
            TimeOut = m_rConnection.GetTimeOut();
            TimeVal.tv_sec  = TimeOut / 1000;           // number of seconds
            TimeVal.tv_usec = (TimeOut%1000) * 1000;    // number of micro-seconds

            // Verify if there is something waiting on the socket
            if (select(0, &SocketSet, NULL, NULL, (TimeOut != LONG_MAX ? &TimeVal : NULL)) == 1)
                {
                // receive.  will block until something happens
                if ((ReadSize = recv(m_rConnection.m_hSocket,
                                     (char*)pBuffer.get(),
                                     s_ReadSize,
                                     0)) == SOCKET_ERROR)
                    {
                    //HDEBUGCODE(DWORD LastError = WSAGetLastError();)
                    throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_LISTEN);
                    }

                // Data was read from the channel
                else if (ReadSize > 0)
                    {
                    // add the new data in the buffer
                    HFCMonitor BufferMonitor(m_rConnection.m_BufferKey);
                    m_rConnection.m_Buffer.AddData(pBuffer, ReadSize);

                    // Signal that there is something to read in the buffer
                    m_rConnection.m_SocketEvent.Signal();
                    }

                else
                    throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_CLOSED_BY_CLIENT);
                }

            // the select timed-out, so disconnect
            else
                throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SELECT);
#endif
            }
        }
    catch(...)
        {
        }

    // signal in case anything is waiting
    m_rConnection.SetConnected(false);

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // make sure that the socket closes gracefully without waiting here for it
    bool Option = 1;
    setsockopt(m_rConnection.m_hSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&Option, sizeof(Option));

    // close the socket
    closesocket(m_rConnection.m_hSocket);
    m_rConnection.m_hSocket = 0;
#endif

    m_rConnection.m_SocketEvent.Signal();
    }

