//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCInternetConnection.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCInternetConnection
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// Changes the time out value.
//-----------------------------------------------------------------------------
inline void HFCInternetConnection::SetTimeOut(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut > 0);

    m_TimeOut = pi_TimeOut;
    }

//-----------------------------------------------------------------------------
// public
// Gets the current time out value.
//-----------------------------------------------------------------------------
inline uint32_t HFCInternetConnection::GetTimeOut() const
    {
    return (m_TimeOut);
    }

//-----------------------------------------------------------------------------
// public
// Sets the user name.
//-----------------------------------------------------------------------------
inline void HFCInternetConnection::SetUserName(const WString& pi_rUserName)
    {
    m_UserName = pi_rUserName;
    }

//-----------------------------------------------------------------------------
// public
// Gets the user name.
//-----------------------------------------------------------------------------
inline const WString& HFCInternetConnection::GetUserName() const
    {
    return m_UserName;
    }

//-----------------------------------------------------------------------------
// public
// Sets the password.
//-----------------------------------------------------------------------------
inline void HFCInternetConnection::SetPassword(const WString& pi_rPassword)
    {
    m_Password = pi_rPassword;
    }


//-----------------------------------------------------------------------------
// public
// Gets the password.
//-----------------------------------------------------------------------------
inline const WString& HFCInternetConnection::GetPassword() const
    {
    return m_Password;
    }

//-----------------------------------------------------------------------------
// public
// Sets the proxy user name.
//-----------------------------------------------------------------------------
inline void HFCInternetConnection::SetProxyUserName(const WString& pi_rUserName)
    {
    m_ProxyUserName = pi_rUserName;
    }

//-----------------------------------------------------------------------------
// public
// Gets the proxy user name.
//-----------------------------------------------------------------------------
inline const WString& HFCInternetConnection::GetProxyUserName() const
    {
    return m_ProxyUserName;
    }

//-----------------------------------------------------------------------------
// public
// Sets the proxy password.
//-----------------------------------------------------------------------------
inline void HFCInternetConnection::SetProxyPassword(const WString& pi_rPassword)
    {
    m_ProxyPassword = pi_rPassword;
    }


//-----------------------------------------------------------------------------
// public
// Gets the proxy password.
//-----------------------------------------------------------------------------
inline const WString& HFCInternetConnection::GetProxyPassword() const
    {
    return m_ProxyPassword;
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HFCInternetConnectionException
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
END_IMAGEPP_NAMESPACE
