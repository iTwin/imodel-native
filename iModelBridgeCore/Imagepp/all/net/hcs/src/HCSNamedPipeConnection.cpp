//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSNamedPipeConnection.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeConnection
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSNamedPipeConnection.h>
#include <Imagepp/all/h/HFCThread.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const WString s_Device(L"named pipe connection");

static char s_DummyBuffer;

// Give 1 minute for the overlapped write to succeed at least partially
#define OVERLAPPED_WRITE_TIMEOUT    60000


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
#ifdef _WIN32
HCSNamedPipeConnection::HCSNamedPipeConnection(HFCHandle pi_Pipe)
    : HFCInternetConnection(L"", L"", L""),
      m_ReadEvent(true, false),
      m_WriteEvent(true, false)
    {
    HPRECONDITION(pi_Pipe != NULL);

    // Copy the pipe handle
    m_hPipe = pi_Pipe;

    // setup the overlapped structures
    memset(&m_ReadOverlapped, 0, sizeof(m_ReadOverlapped));
    m_ReadOverlapped.hEvent = m_ReadEvent.GetHandle();
    memset(&m_WriteOverlapped, 0, sizeof(m_WriteOverlapped));
    m_WriteOverlapped.hEvent = m_WriteEvent.GetHandle();

    // Setup the overlapped IO for read operations
    SetupReadOverlapped();

    // Now we're connected
    SetConnected(true);
    }
#endif


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSNamedPipeConnection::HCSNamedPipeConnection(const WString& pi_rServer)
    : HFCInternetConnection(pi_rServer, L"", L"")
    {
#ifdef _WIN32
    m_hPipe = NULL;

    // setup the overlapped structures
    memset(&m_ReadOverlapped, 0, sizeof(m_ReadOverlapped));
    m_ReadOverlapped.hEvent = m_ReadEvent.GetHandle();
    memset(&m_WriteOverlapped, 0, sizeof(m_WriteOverlapped));
    m_WriteOverlapped.hEvent = m_WriteEvent.GetHandle();
#endif
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSNamedPipeConnection::~HCSNamedPipeConnection()
    {
    try
        {
        Disconnect();

#ifdef _WIN32
        if (m_hPipe != NULL)
            {
            ::CloseHandle(m_hPipe);
            m_hPipe = NULL;
            }
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
void HCSNamedPipeConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0 && pi_DataSize <= UINT32_MAX);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

#ifdef _WIN32
    // Since the send command can send atomically parts of the buffer,
    // loop until all the data has been sent
    const char* pData = (const char*)pi_pData;
    while (pi_DataSize > 0)
        {
        // set-up the overlapped IO
        m_WriteEvent.Reset();

        DWORD SendSize;
        if (!WriteFile(m_hPipe, pData, (DWORD)pi_DataSize, &SendSize, &m_WriteOverlapped) &&
            (GetLastError() != ERROR_IO_PENDING))
            {
            //int LastError = GetLastError();
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEND);
            }

        // wait for the operation to complete
        if (!m_WriteEvent.WaitUntilSignaled(OVERLAPPED_WRITE_TIMEOUT))
            {
            // At least part of the data should have been sent by now.
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEND);
            }

        SendSize = (DWORD)m_WriteOverlapped.InternalHigh;   // the value cannot be greater than SendSize

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
void HCSNamedPipeConnection::Receive(Byte* po_pData, size_t pi_DataSize)
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
void HCSNamedPipeConnection::Receive(Byte* po_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(po_pData       != 0);
    HPRECONDITION(pio_pDataSize  != 0);
    HPRECONDITION(*pio_pDataSize >  0 && *pio_pDataSize <= UINT32_MAX);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // read some data from the socket
    DWORD ReadSize;
    if (!::ReadFile(m_hPipe, po_pData, (DWORD)*pio_pDataSize, &ReadSize, NULL))
        {
        //int LastError = GetLastError();
        Disconnect();
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_READ);
        }

    // at this point, we have read something from the connection
    *pio_pDataSize = ReadSize;

    // Setup the overlapped IO for read operations
    SetupReadOverlapped();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HCSNamedPipeConnection::Connect(const WString& pi_rUserName,
                                      const WString& pi_rPassword,
                                      time_t         pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(!IsConnected());      // must not be already connected

    return Connect(pi_TimeOut);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HCSNamedPipeConnection::Connect(time_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(!IsConnected());      // must not be already connected

    try
        {
#ifdef _WIN32
        // Wait for the named pipe to be created
        if (!::WaitNamedPipeW(GetServer().c_str(), (DWORD)pi_TimeOut))
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);

        // at this point, the named pipe exists, so connect to it
        m_hPipe = ::CreateFileW(GetServer().c_str(),
                               GENERIC_WRITE | GENERIC_READ,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                               NULL);
        if (m_hPipe == INVALID_HANDLE_VALUE)
            {
            m_hPipe = NULL;
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CREATE);
            }

        // Now we're connected
        SetConnected(true);

        // Setup the overlapped IO for read operations
        SetupReadOverlapped();
#endif
        }
    catch(...)
        {
        Disconnect();
        }

    return IsConnected();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HCSNamedPipeConnection::ValidateConnect(uint32_t pi_TimeOut)
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
void HCSNamedPipeConnection::Disconnect()
    {
    // Set the connection state to false
    SetConnected(false);

    // Close the socket
#ifdef _WIN32
    if (m_hPipe != NULL)
        {
        ::FlushFileBuffers(m_hPipe);
        ::DisconnectNamedPipe(m_hPipe);
        }
#endif
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HCSNamedPipeConnection::WaitDataAvailable()
    {
    DWORD Result = 0;

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // wait for the event to be signaled
    m_ReadEvent.WaitUntilSignaled();

    if (!::PeekNamedPipe(m_hPipe, NULL, 0, NULL, &Result, NULL))
        {
        //int LastError = GetLastError();
        Disconnect();
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_PEEK);
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HCSNamedPipeConnection::WaitDataAvailable(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    DWORD Result = 0;

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // wait for the event to become signaled
    if (m_ReadEvent.WaitUntilSignaled(pi_TimeOut))
        {
        if (!::PeekNamedPipe(m_hPipe, NULL, 0, NULL, &Result, NULL))
            {
            //int LastError = GetLastError();
            Disconnect();
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_PEEK);
            }
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Private
// Sets-up the overlapped for reading
//-----------------------------------------------------------------------------
void HCSNamedPipeConnection::SetupReadOverlapped()
    {
    // Setup the overlapped IO for read operations
    m_ReadEvent.Reset();
    if (!ReadFile(m_hPipe, &s_DummyBuffer, 0, NULL, &m_ReadOverlapped))
        {
        DWORD LastError = GetLastError();

        // if the peer has closed the connection, just disconnect
        if (LastError == ERROR_BROKEN_PIPE)
            {
            m_ReadEvent.Signal();
            Disconnect();
            }

        else if (LastError != ERROR_IO_PENDING)
            {
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_READ);
            }
        }
    }
