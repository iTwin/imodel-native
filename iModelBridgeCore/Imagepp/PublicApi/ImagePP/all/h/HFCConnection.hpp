//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCConnection.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCConnection
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Indicates if the connection is still established
//-----------------------------------------------------------------------------
inline bool HFCConnection::IsConnected() const
    {
    return (m_ConnectionState);
    }


//-----------------------------------------------------------------------------
// protected
// Changes the state of the connection.  Provided for derived class.
//-----------------------------------------------------------------------------
inline void HFCConnection::SetConnected(bool pi_IsConnected)
    {
    m_ConnectionState = pi_IsConnected;
    }



//-----------------------------------------------------------------------------
// public
// Returns the string to the connection HOST
//-----------------------------------------------------------------------------
inline const WString& HFCConnection::GetServer() const
    {
    return m_Server;
    }

