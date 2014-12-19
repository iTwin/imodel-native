//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSNamedPipeServerConnection.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeServerConnection
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSNamedPipeServerConnectionConfig::HCSNamedPipeServerConnectionConfig(const WString& pi_rName)
    {
    SetName(pi_rName);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSNamedPipeServerConnectionConfig::~HCSNamedPipeServerConnectionConfig()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnectionConfig* HCSNamedPipeServerConnectionConfig::Clone() const
    {
    return new HCSNamedPipeServerConnectionConfig(GetName());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnection* HCSNamedPipeServerConnectionConfig::Create()
    {
    return new HCSNamedPipeServerConnection(GetName());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const WString& HCSNamedPipeServerConnectionConfig::GetName() const
    {
    return m_Name;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HCSNamedPipeServerConnectionConfig::SetName(const WString& pi_rName)
    {
    HPRECONDITION(!pi_rName.empty());

    m_Name = pi_rName;
    }
