//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
inline const Utf8String& HFCURLSql::GetQuery() const
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