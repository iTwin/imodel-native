//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHTTPConnection.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCHTTPConnection
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const WString& HFCHTTPConnection::GetExtention() const
    {
    return (m_Extention);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HFCHTTPConnection::SetExtention(const WString& pi_rBase)
    {
    m_Extention = pi_rBase;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const string& HFCHTTPConnection::GetSearchBase() const
    {
    return (m_SearchBase);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HFCHTTPConnection::SetSearchBase(const string& pi_rBase)
    {
    m_SearchBase = pi_rBase;
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const HFCPtr<HFCURL>& HFCHTTPConnection::GetURL()
    {
    return (HFCPtr<HFCURL>&)m_pURL;
    }
END_IMAGEPP_NAMESPACE