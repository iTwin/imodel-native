//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLSql.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLSql
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the host part of the
// URL string.
//-----------------------------------------------------------------------------
inline const WString& HFCURLSql::GetQuery() const
    {
    return m_Query;
    }


//-----------------------------------------------------------------------------
// HasPathTo
//-----------------------------------------------------------------------------
inline bool HFCURLSql::HasPathTo(HFCURL* pi_pURL)
    {
    return false;
    }
END_IMAGEPP_NAMESPACE