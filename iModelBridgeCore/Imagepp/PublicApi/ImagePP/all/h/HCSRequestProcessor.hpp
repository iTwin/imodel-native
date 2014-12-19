//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestProcessor.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestProcessor
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"
#include "HFCInternetConnection.h"
#include "HCSConnectionPool.h"
#include "HFCBinStream.h"


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
inline HCSRequestProcessor::HCSRequestProcessor(HCSConnectionPool& pi_rPool)
    {
    SetConnectionPool(pi_rPool);
    }


//-----------------------------------------------------------------------------
// Public
// Destroyer
//-----------------------------------------------------------------------------
inline HCSRequestProcessor::~HCSRequestProcessor()
    {
    }


//-----------------------------------------------------------------------------
// Public
// returns the connection pool associated with the processor
//-----------------------------------------------------------------------------
inline HCSConnectionPool& HCSRequestProcessor::GetConnectionPool() const
    {
    HFCMonitor PoolMonitor(m_PoolKey);

    return (*m_pPool);
    }


//-----------------------------------------------------------------------------
// Public
// changes the connection pool associated withe the process
//-----------------------------------------------------------------------------
inline void HCSRequestProcessor::SetConnectionPool(HCSConnectionPool& pi_rPool)
    {
    HFCMonitor PoolMonitor(m_PoolKey);

    // we do not destroy the pool, because this object does not own it
    m_pPool = &pi_rPool;
    }


//-----------------------------------------------------------------------------
// Protected
// Gets the next active connection from the pool
//-----------------------------------------------------------------------------
inline HFCInternetConnection* HCSRequestProcessor::GetActiveConnectionFromPool(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(m_pConnection == 0); // the caller should release the
    HFCInternetConnection* pConnection;

    // Secure the pool & get a connection
        {
        HFCMonitor PoolMonitor(m_PoolKey);
        pConnection = m_pPool->GetActiveConnection(pi_TimeOut);
        }

    // change the internal connection pointer
    m_ConnectionKey.ClaimKey();
    m_pConnection = pConnection;

    return (m_pConnection);
    }


//-----------------------------------------------------------------------------
// Protected
// Gets the next connection from the pool, active or not
//-----------------------------------------------------------------------------
inline HFCInternetConnection* HCSRequestProcessor::GetConnectionFromPool(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);
    HPRECONDITION(m_pConnection == 0); // the caller should release the
    HFCInternetConnection* pConnection;

    // Secure the pool & get a connection
        {
        HFCMonitor PoolMonitor(m_PoolKey);
        pConnection = m_pPool->GetConnection(pi_TimeOut);
        }

    // change the internal connection pointer
    m_ConnectionKey.ClaimKey();
    m_pConnection = pConnection;

    return (m_pConnection);
    }


//-----------------------------------------------------------------------------
// Protected
// Destroys the current active connection.  Thread-safe.
//-----------------------------------------------------------------------------
inline void HCSRequestProcessor::DestroyConnection(bool pi_OnlyDisconnect)
    {
    // disconnect the connection
    if (m_pConnection != 0)
        m_pConnection->Disconnect();

    // delete the connection
    if (!pi_OnlyDisconnect)
        {
        // release the connection and destroy after the release
        HFCInternetConnection* pConnection = m_pConnection.release();
        m_ConnectionKey.ReleaseKey();

        delete pConnection;
        }
    else
        {
        m_ConnectionKey.ReleaseKey();
        }
    }


//-----------------------------------------------------------------------------
// Protected
// Releases the current active connection.  Thread-safe.
//-----------------------------------------------------------------------------
inline void HCSRequestProcessor::ReleaseConnection()
    {
    // place the connection in a temp pointer and signal that the connection
    // is free
    HFCInternetConnection* pConnection = m_pConnection.release();
    m_ConnectionKey.ReleaseKey();

    // replace the connection in the pool
        {
        HFCMonitor PoolMonitor(m_PoolKey);
        if (pConnection->IsConnected())
            m_pPool->AddConnection(pConnection);
        }
    }


//-----------------------------------------------------------------------------
// Private
// Check for the separator string at the end of the buffer. Used in
// GetRequestFromConnection, because we're sure that the separator will be at
// the complete end of the buffer: we read one byte at a time.
//-----------------------------------------------------------------------------
inline bool HCSRequestProcessor::BufferEndsWithSeparator(const HFCBuffer& pi_rBuffer,
                                                          const string&    pi_rSeparator) const
    {
    bool Result = false;

    if (pi_rBuffer.GetDataSize() >= pi_rSeparator.size())
        {
        Result = (memcmp(&pi_rBuffer.GetData()[pi_rBuffer.GetDataSize() - pi_rSeparator.size()],
                         pi_rSeparator.c_str(),
                         pi_rSeparator.size()) == 0);
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Protected
// Extracts a request from the current connection.  The time out is in milli-seconds.
//-----------------------------------------------------------------------------
inline string HCSRequestProcessor::GetRequestFromConnection(const string& pi_rSeparator,
                                                            uint32_t      pi_TimeOut)
    {
    HPRECONDITION(m_pConnection != 0);
    HPRECONDITION(!pi_rSeparator.empty());
    HFCGenericMonitor<ConnectionKeyType> ConnectionMonitor(m_ConnectionKey);
    string Result;

    bool Valid;
    HFCBuffer Buffer(128);  // Grow 128 bytes at a time
    size_t DataAvailable = 1;

    // wait until a separator arrives.  Loop until a time out occurs in the wait for
    // data to arrive.  If a disconnection occurs, the function will throw.
    while (!BufferEndsWithSeparator(Buffer, pi_rSeparator) &&
           m_pConnection->IsConnected() &&
           (Valid = (m_pConnection->WaitDataAvailable(pi_TimeOut) > 0)) )
        {
        // Data should be there, since the connection pool gave
        // us an active connection.

        // read it
        m_pConnection->Receive(Buffer.PrepareForNewData(DataAvailable), &DataAvailable);
        Buffer.SetNewDataSize(DataAvailable);
        }

    if (Buffer.GetDataSize() > 0)
        Result.insert(0, (string::value_type*)Buffer.GetData(), Buffer.GetDataSize());

    return (Result);
    }


//-----------------------------------------------------------------------------
// Protected
// Extracts bytes from the current connection.  The time out is in milli-seconds.
//-----------------------------------------------------------------------------
inline uint32_t HCSRequestProcessor::ReadBytesFromConnection(uint32_t      pi_BytesToRead,
                                                           uint32_t      pi_TimeOut,
                                                           HFCBinStream* po_pOutputStream)
    {
    HPRECONDITION(pi_BytesToRead > 0);
    HPRECONDITION(po_pOutputStream != 0);
    HFCGenericMonitor<ConnectionKeyType> ConnectionMonitor(m_ConnectionKey);

    uint32_t BytesRemaining = pi_BytesToRead;
    size_t BytesArrived;
    size_t BytesToRead;
    HFCBuffer Buffer(1024); // Grow by 1024 bytes, never downsize

    // Read the specified bytes. Loop until a time out occurs in the wait for
    // data to arrive.  If a disconnection occurs, the function will throw.
    while (BytesRemaining &&
           m_pConnection->IsConnected() &&
           (BytesArrived = m_pConnection->WaitDataAvailable(pi_TimeOut)) > 0 )
        {
        BytesToRead = min(BytesArrived, BytesRemaining);

        // read it
        m_pConnection->Receive(Buffer.PrepareForNewData(BytesToRead), &BytesToRead);
        // Update the HFCBuffer DataSize
        Buffer.SetNewDataSize(BytesToRead);

        po_pOutputStream->Write(Buffer.GetData(), BytesToRead);

        Buffer.Clear();

        BytesRemaining -= BytesToRead;
        }

    // Bytes read
    return (pi_BytesToRead - BytesRemaining);
    }
