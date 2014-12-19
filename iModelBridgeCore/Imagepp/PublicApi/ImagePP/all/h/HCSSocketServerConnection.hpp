//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSSocketServerConnection.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketServerConnection
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint32_t HCSSocketServerConnection::GetBacklog() const
    {
    return m_Backlog;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HCSSocketServerConnection::SetBacklog(uint32_t pi_Backlog)
    {
    HPRECONDITION(pi_Backlog > 0);

    m_Backlog = pi_Backlog;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HCSSocketServerConnection::IsConnectionShared() const
    {
    return m_Shared;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HCSSocketServerConnection::ShareConnection(bool pi_Shared)
    {
    m_Shared = pi_Shared;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const string& HCSSocketServerConnection::GetAddress() const
    {
    HPRECONDITION(IsConnected());

    return m_Address;
    }


//-----------------------------------------------------------------------------
// Public
// Returns the obtained port
//-----------------------------------------------------------------------------
inline unsigned short HCSSocketServerConnection::GetPort() const
    {
    HPRECONDITION(IsConnected());

    return m_Port;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSSocketServerConnectionConfig::HCSSocketServerConnectionConfig()
    {
    m_Backlog = UINT_MAX;
    m_Shared  = false;
    m_Port = 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSSocketServerConnectionConfig::HCSSocketServerConnectionConfig(unsigned short pi_Port)
    {
    m_Backlog = UINT_MAX;
    m_Shared  = false;
    m_Port = pi_Port;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSSocketServerConnectionConfig::HCSSocketServerConnectionConfig(const string& pi_rAddress)
    {
    HPRECONDITION(!pi_rAddress.empty());

    m_Backlog = UINT_MAX;
    m_Shared  = false;
    m_Address = pi_rAddress;
    m_Port    = 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSSocketServerConnectionConfig::HCSSocketServerConnectionConfig(const string& pi_rAddress,
                                                                        unsigned short pi_Port)
    {
    HPRECONDITION(!pi_rAddress.empty());
    HPRECONDITION(pi_Port > 0);

    m_Backlog = UINT_MAX;
    m_Shared  = false;
    m_Address = pi_rAddress;
    m_Port    = pi_Port;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSSocketServerConnectionConfig::~HCSSocketServerConnectionConfig()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnectionConfig* HCSSocketServerConnectionConfig::Clone() const
    {
    HAutoPtr<HCSSocketServerConnectionConfig> pResult;

    if (GetAddress().empty())
        {
        if (GetPort() == 0)
            pResult = new HCSSocketServerConnectionConfig();
        else
            pResult = new HCSSocketServerConnectionConfig(GetPort());

        }
    else
        {
        if (GetPort() == 0)
            pResult = new HCSSocketServerConnectionConfig(GetAddress());
        else
            pResult = new HCSSocketServerConnectionConfig(GetAddress(), GetPort());
        }

    // set up the backlog and the share
    pResult->SetBacklog(m_Backlog);
    pResult->ShareConnection(m_Shared);

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnection* HCSSocketServerConnectionConfig::Create()
    {
    HAutoPtr<HCSSocketServerConnection> pResult;

    if (GetAddress().empty())
        {
        if (GetPort() == 0)
            pResult = new HCSSocketServerConnection;
        else
            pResult = new HCSSocketServerConnection(GetPort());

        }
    else
        {
        if (GetPort() == 0)
            pResult = new HCSSocketServerConnection(GetAddress());
        else
            pResult = new HCSSocketServerConnection(GetAddress(), GetPort());
        }

    // set up the backlog and the share
    pResult->SetBacklog(m_Backlog);
    pResult->ShareConnection(m_Shared);

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint32_t HCSSocketServerConnectionConfig::GetBacklog() const
    {
    return m_Backlog;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HCSSocketServerConnectionConfig::SetBacklog(uint32_t pi_Backlog)
    {
    HPRECONDITION(pi_Backlog > 0);

    m_Backlog = pi_Backlog;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HCSSocketServerConnectionConfig::IsConnectionShared() const
    {
    return m_Shared;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HCSSocketServerConnectionConfig::ShareConnection(bool pi_Shared)
    {
    m_Shared = pi_Shared;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const string& HCSSocketServerConnectionConfig::GetAddress() const
    {
    return m_Address;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline unsigned short HCSSocketServerConnectionConfig::GetPort() const
    {
    return m_Port;
    }

