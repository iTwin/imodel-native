//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSServerConnection.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSServerConnection
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
inline HCSServerConnection::HCSServerConnection(const WString& pi_rServer)
    : HFCConnection(pi_rServer)
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
inline HCSServerConnection::~HCSServerConnection()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Disabled connections method
//-----------------------------------------------------------------------------
inline bool HCSServerConnection::Connect(const WString& pi_rUserName,
                                          const WString& pi_rPassword,
                                          time_t         pi_TimeOut)
    {
    HASSERT(false);
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// Disabled connections method
//-----------------------------------------------------------------------------
inline bool HCSServerConnection::ValidateConnect(uint32_t pi_TimeOut)
    {
    HASSERT(false);
    return false;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnectionConfig::HCSServerConnectionConfig()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HCSServerConnectionConfig::~HCSServerConnectionConfig()
    {
    }