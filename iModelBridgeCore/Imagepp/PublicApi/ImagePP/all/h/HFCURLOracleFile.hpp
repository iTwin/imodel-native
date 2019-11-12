//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLOracleFile
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the host part of the
// URL string.
//-----------------------------------------------------------------------------
inline const Utf8String& HFCURLOracleFile::GetHost() const
    {
    return m_Host;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the Path part of the
// URL string.
//-----------------------------------------------------------------------------
inline const Utf8String& HFCURLOracleFile::GetPath() const
    {
    return m_Path;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the DirTableName part of the
// URL string.
//-----------------------------------------------------------------------------
inline const Utf8String& HFCURLOracleFile::GetDirTableName() const
    {
    return m_DirTableName;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the Schema part of the
// URL string.
//-----------------------------------------------------------------------------
inline const Utf8String& HFCURLOracleFile::GetSchema() const
    {
    return m_Schema;
    }

//-----------------------------------------------------------------------------
// public
// HasPathTo
//-----------------------------------------------------------------------------
inline bool HFCURLOracleFile::HasPathTo(HFCURL* pi_pURL)
    {
    return false;
    }
END_IMAGEPP_NAMESPACE