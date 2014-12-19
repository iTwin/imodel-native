//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSBufferedConnectionPool.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSBufferedConnectionPool
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSBufferedConnectionPool.h>
#include <Imagepp/all/h/HCSBufferedConnection.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HCSBufferedConnection::HCSBufferedConnection(HFCInternetConnection* pi_pConnection,
                                                    size_t                 pi_BufferGrowSize,
                                                    bool                  pi_OwnsConnection,
                                                    size_t                 pi_ReadSize)
    : HFCInternetConnection(pi_pConnection->GetServer(),
                            pi_pConnection->GetUserName(),
                            pi_pConnection->GetPassword()),
    m_Buffer(pi_BufferGrowSize)
    {
    HPRECONDITION(pi_ReadSize > 0);
    HPRECONDITION(pi_pConnection != 0);

    m_pConnection = pi_pConnection;
    SetConnected(m_pConnection->IsConnected());
    m_ReadSize = pi_ReadSize;
    m_HasConnectionOwnership = pi_OwnsConnection;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HCSBufferedConnection::~HCSBufferedConnection()
    {
    if (m_HasConnectionOwnership)
        delete m_pConnection;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSBufferedConnectionPool::HCSBufferedConnectionPool(HCSConnectionPool& pi_rPool)
    : m_rPool(pi_rPool)
    {
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSBufferedConnectionPool::HCSBufferedConnectionPool(HCSConnectionPool& pi_rPool,
                                                     time_t             pi_ExpirationTime,
                                                     uint32_t           pi_ExpirationInterval)
    : HCSConnectionPool(pi_ExpirationTime, pi_ExpirationInterval),
      m_rPool(pi_rPool)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destroyer
//-----------------------------------------------------------------------------
HCSBufferedConnectionPool::~HCSBufferedConnectionPool()
    {
    // destroy the buffered connections in the list
    for (BufferedConnectionList::iterator Itr = m_BufferedConnections.begin();
         Itr != m_BufferedConnections.end();
         ++Itr)
        delete *Itr;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnectionPool::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    HCSBufferedConnection* pConnection = static_cast<HCSBufferedConnection*>(pi_pConnection);
    HFCMonitor Monitor(m_Key);

    if (pi_pConnection->IsCompatibleWith(HCSBufferedConnection::CLASS_ID))
        {
        // if there is still data in the buffer, add the connection to the list.
        // Otherwise, return the internal connection to the buffered pool
        if (pConnection->m_Buffer.GetDataSize() > 0)
            m_BufferedConnections.push_back(pConnection);

        else
            {
            // The sub-pool has its own synchronization
            Monitor.ReleaseKey();

            m_rPool.AddConnection(pConnection->m_pConnection);
            pConnection->SetConnectionOwnership(false);
            delete pConnection;
            }
        }

    else
        {
        m_rPool.AddConnection(pConnection);
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnectionPool::RemoveConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HCSBufferedConnection::CLASS_ID));
    HCSBufferedConnection* pConnection = static_cast<HCSBufferedConnection*>(pi_pConnection);
    HFCMonitor Monitor(m_Key);

    // remove the buffered connection from this pool
    m_BufferedConnections.remove(pConnection);

    Monitor.ReleaseKey();

    // remove the internal connection from the buffered pool
    m_rPool.RemoveConnection(pConnection->m_pConnection);

    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HFCInternetConnection*
HCSBufferedConnectionPool::GetConnection(uint32_t pi_TimeOut)
    {
    HFCMonitor Monitor(m_Key);
    HAutoPtr<HFCInternetConnection> pResult;

    // if there are connection in the internal list, use it
    if (m_BufferedConnections.size() > 0)
        {
        pResult = *m_BufferedConnections.begin();
        m_BufferedConnections.pop_front();
        }

    // if not, go find one in the buffered pool
    else
        {
        // The sub-pool has its own synchronization
        Monitor.ReleaseKey();

        pResult = m_rPool.GetConnection(pi_TimeOut);
        if (pResult != 0)
            pResult = new HCSBufferedConnection(pResult.release(), 1024, true);
        }

    return pResult.release();
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HFCInternetConnection*
HCSBufferedConnectionPool::GetActiveConnection(uint32_t pi_TimeOut)
    {
    HFCMonitor Monitor(m_Key);
    HAutoPtr<HFCInternetConnection> pResult;

    // if there are connection in the internal list, use it
    if (!m_BufferedConnections.empty())
        {
        pResult = *m_BufferedConnections.begin();
        m_BufferedConnections.pop_front();
        }

    // if not, go find one in the buffered pool
    else
        {
        // The sub-pool has its own synchronization
        Monitor.ReleaseKey();

        pResult = m_rPool.GetActiveConnection(pi_TimeOut);
        if (pResult != 0)
            {
            pResult = new HCSBufferedConnection(pResult.release(), 1024, true);
            }
        }

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
bool HCSBufferedConnection::Connect(const WString& pi_rUserName,
                                            const WString& pi_rPassword,
                                            time_t         pi_TimeOut)
    {
    try
        {
        m_pConnection->Connect(pi_rUserName, pi_rPassword, pi_TimeOut);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }

    SetConnected(m_pConnection->IsConnected());
    return IsConnected();
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
bool HCSBufferedConnection::ValidateConnect(uint32_t pi_TimeOut)
    {
    try
        {
        m_pConnection->ValidateConnect(pi_TimeOut);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }

    SetConnected(m_pConnection->IsConnected());
    return IsConnected();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnection::Disconnect()
    {
    try
        {
        m_pConnection->Disconnect();
        SetConnected(false);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    try
        {
        m_pConnection->Send(pi_pData, pi_DataSize);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }


//-----------------------------------------------------------------------------
// public
// Partial send.
//-----------------------------------------------------------------------------
void HCSBufferedConnection::SendImmediate(const Byte* pi_pData, size_t* pio_pDataSize)
    {
    try
        {
        m_pConnection->SendImmediate(pi_pData, pio_pDataSize);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnection::Receive(Byte* po_pData, size_t pi_DataSize)
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
// Public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnection::Receive(Byte* po_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(pio_pDataSize != 0);
    size_t ReadSize;

    // if there is data in the buffer, return it up to the specified size
    if (m_Buffer.GetDataSize() > 0)
        {
        ReadSize = min(*pio_pDataSize, (int32_t)m_Buffer.GetDataSize());
        memcpy(po_pData, m_Buffer.GetData(), ReadSize);
        m_Buffer.MarkReadData(ReadSize);

        *pio_pDataSize = ReadSize;
        }

    else
        {
        try
            {
            // read data from the connection
            ReadSize = m_ReadSize;
            Byte* pBuffer = m_Buffer.PrepareForNewData(ReadSize);
            m_pConnection->Receive(pBuffer, &ReadSize);
            m_Buffer.SetNewDataSize(ReadSize);

            // use what is in the buffer to fill the request
            if (m_Buffer.GetDataSize() > 0)
                {
                ReadSize = min(*pio_pDataSize, m_Buffer.GetDataSize());
                memcpy(po_pData, m_Buffer.GetData(), ReadSize);
                m_Buffer.MarkReadData(ReadSize);

                *pio_pDataSize = ReadSize;
                }
            else
                {
                *pio_pDataSize = 0;
                }
            }
        catch(...)
            {
            SetConnected(m_pConnection->IsConnected());
            *pio_pDataSize = 0;
            throw;
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
size_t HCSBufferedConnection::WaitDataAvailable()
    {
    try
        {
        if (m_Buffer.GetDataSize() > 0)
            return m_Buffer.GetDataSize();
        else
            return m_pConnection->WaitDataAvailable();
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
size_t HCSBufferedConnection::WaitDataAvailable(uint32_t pi_TimeOut)
    {
    try
        {
        if (m_Buffer.GetDataSize() > 0)
            return m_Buffer.GetDataSize();
        else
            return m_pConnection->WaitDataAvailable(pi_TimeOut);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HCSBufferedConnection::WaitDataWriteable()
    {
    try
        {
        return m_pConnection->WaitDataWriteable();
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HCSBufferedConnection::WaitDataWriteable(uint32_t pi_TimeOut)
    {
    try
        {
        return m_pConnection->WaitDataWriteable(pi_TimeOut);
        }
    catch(...)
        {
        SetConnected(m_pConnection->IsConnected());
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSBufferedConnection::GetInternalConnection() const
    {
    return m_pConnection;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HCSBufferedConnection::HasConnectionOwnership() const
    {
    return m_HasConnectionOwnership;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSBufferedConnection::SetConnectionOwnership(bool pi_Owner)
    {
    m_HasConnectionOwnership = pi_Owner;
    }