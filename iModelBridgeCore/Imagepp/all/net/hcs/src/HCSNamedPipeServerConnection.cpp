//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSNamedPipeServerConnection.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeServerConnection
//-----------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSNamedPipeServerConnection.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HCSNamedPipeConnection.h>
#include <Imagepp/all/h/HCSException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const WString s_Device(L"named pipe server connection");


//-----------------------------------------------------------------------------
// Override HFCEvent in order to make GetHandle public
//-----------------------------------------------------------------------------
class NamedPipeEvent : public HFCEvent
    {
public:
    NamedPipeEvent(bool         pi_ManualReset = true,
                   bool         pi_Signaled    = true)
        : HFCEvent(pi_ManualReset, pi_Signaled)
        {
        };

    virtual HFCHandle GetHandle() const
        {
        return HFCEvent::GetHandle();
        }
    };


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSNamedPipeServerConnection::HCSNamedPipeServerConnection(const WString& pi_rName)
    : HCSServerConnection(pi_rName),
      m_PipeEvent(true, false)
    {
#ifdef _WIN32
    m_hPipe = NULL;
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSNamedPipeServerConnection::~HCSNamedPipeServerConnection()
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
bool HCSNamedPipeServerConnection::Connect(time_t pi_TimeOut)
    {
    // User default buffer sizes
    return ConnectExt((DWORD)pi_TimeOut, 1024, 1024);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HCSNamedPipeServerConnection::ConnectExt(DWORD pi_TimeOut,
                                               DWORD pi_OutBufferSize,
                                               DWORD pi_InBufferSize)
    {
    HPRECONDITION(pi_TimeOut > 0);
#ifdef _WIN32
    HPRECONDITION(m_hPipe == NULL);      // must not be already connected
#else
    HPRECONDITION(!IsConnected());      // must not be already connected
#endif

    try
        {
#ifdef _WIN32
        // add a NULL security descriptor. Setting NULL as security attributes
        // in CreateNamedPipe simply inherits the security attributes.  We
        // have to create a NULL sa and give it to CreateNamedPipe.
        //
        // See http://support.microsoft.com/support/kb/articles/Q102/7/98.asp
        //
        // This permits a user logged on as SebastienT to control a service
        // logged as Local System or another network user.
        SECURITY_ATTRIBUTES  sa;
        PSECURITY_DESCRIPTOR pSD;
        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(pSD, true, (PACL) NULL, false);
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = true;

        // create the named pipe.
        m_hPipe = ::CreateNamedPipeW(GetServer().c_str(),
                                    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                    PIPE_TYPE_BYTE | PIPE_WAIT,
                                    PIPE_UNLIMITED_INSTANCES,
                                    pi_OutBufferSize,
                                    pi_InBufferSize,
                                    pi_TimeOut,
                                    &sa);

        if (pSD != NULL)
            LocalFree(pSD);

        if (m_hPipe == INVALID_HANDLE_VALUE)
            throw HCSException(HCS_CANNOT_CREATE_EXCEPTION, s_Device);
#endif

        // if we get through here, we are connected
        SetConnected(true);
        m_PipeEvent.Reset();
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
void HCSNamedPipeServerConnection::Disconnect()
    {
    // Set the connection state to false
    SetConnected(false);

    // Close the socket
#ifdef _WIN32

    // Pipe life is normally transferred to a connection object. Close it
    // if a connection was never "Accepted" with the pipe.
    if (m_hPipe != NULL && m_hPipe != INVALID_HANDLE_VALUE)
        {
        ::CloseHandle(m_hPipe);
        m_hPipe = NULL;
        }

    m_PipeEvent.Signal();
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSNamedPipeServerConnection::Accept()
    {
    HAutoPtr<HCSNamedPipeConnection> pResult;

    // Normally, there should be a precondition to verify that the connection is
    // connected before trying to accept.  Instead, because this class can be controlled
    // by another thread, which closes the connection, we will throw if the conenction
    // is not connected.
    if (!IsConnected() && m_PipeEvent.WaitUntilSignaled(0))
        throw HCSException(HCS_NOT_CONNECTED_EXCEPTION, s_Device);

#ifdef _WIN32
    HPRECONDITION(m_hPipe != NULL);

    // Create the event that will be signaled when the connection is made
    NamedPipeEvent Event(true, false);
    OVERLAPPED Overlapped;
    memset(&Overlapped, 0, sizeof(OVERLAPPED));
    Overlapped.hEvent = Event.GetHandle();

    // Wait for the client connection.  If the connect is already made,
    // signal the damn event and get on with our life!
    HFCSynchroContainer Synchros;
    Synchros.AddSynchro(&Event);
    Synchros.AddSynchro(&m_PipeEvent);
    if (!ConnectNamedPipe(m_hPipe, &Overlapped))
        {
        DWORD LastError = GetLastError();
        if (LastError == ERROR_PIPE_CONNECTED)
            Event.Signal();
        }

    uint32_t WaitResult = HFCSynchro::WaitForMultipleObjects(Synchros, false);
    if (WaitResult == 0)    // first one is signaled
        {
        // create the new connection from the current handle
        pResult = new HCSNamedPipeConnection(m_hPipe);
        m_hPipe = NULL;

        // create a new handle to accept a new connection
        Connect(10000);
        }
    else
        {
        // Make sure there won't be asynchronous stuff behind our back
        CancelIo(m_hPipe);

        throw HCSException(s_Device);
        }
#endif

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSNamedPipeServerConnection::Accept(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HAutoPtr<HCSNamedPipeConnection> pResult;

    // Normally, there should be a precondition to verify that the connection is
    // connected before trying to accept.  Instead, because this class can be controlled
    // by another thread, which closes the connection, we will throw if the conenction
    // is not connected.
    if (!IsConnected() && m_PipeEvent.WaitUntilSignaled(0))
        throw HCSException(HCS_NOT_CONNECTED_EXCEPTION, s_Device);

#ifdef _WIN32
    HPRECONDITION(m_hPipe != NULL);

    // Create the event that will be signaled when the connection is made
    NamedPipeEvent Event(true, false);
    OVERLAPPED Overlapped;
    memset(&Overlapped, 0, sizeof(OVERLAPPED));
    Overlapped.hEvent = Event.GetHandle();

    // Wait for the client connection.  If the connect is already made,
    // signal the damn event and get on with our life!
    HFCSynchroContainer Synchros;
    Synchros.AddSynchro(&Event);
    Synchros.AddSynchro(&m_PipeEvent);
    if (!ConnectNamedPipe(m_hPipe, &Overlapped))
        {
        DWORD LastError = GetLastError();
        if (LastError == ERROR_PIPE_CONNECTED ||
            LastError == ERROR_NO_DATA)
            Event.Signal();
        }
    uint32_t WaitResult = HFCSynchro::WaitForMultipleObjects(Synchros, pi_TimeOut, false);
    if (WaitResult == 0)   // first one is signaled
        {
        // create the new connection from the current handle
        pResult = new HCSNamedPipeConnection(m_hPipe);
        m_hPipe = NULL;

        // create a new handle to accept a new connection
        Connect(10000);
        }
    else
        {
        // Make sure there won't be asynchronous stuff behind our back
        CancelIo(m_hPipe);

        if (WaitResult != HFC_SYNCHRO_TIMEOUT)
            throw HCSException(s_Device);
        }

#endif

    return pResult.release();
    }

