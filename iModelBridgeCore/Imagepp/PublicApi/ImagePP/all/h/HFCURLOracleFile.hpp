//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLOracleFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
inline const WString& HFCURLOracleFile::GetHost() const
    {
    return m_Host;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the Path part of the
// URL string.
//-----------------------------------------------------------------------------
inline const WString& HFCURLOracleFile::GetPath() const
    {
    return m_Path;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the DirTableName part of the
// URL string.
//-----------------------------------------------------------------------------
inline const WString& HFCURLOracleFile::GetDirTableName() const
    {
    return m_DirTableName;
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the Schema part of the
// URL string.
//-----------------------------------------------------------------------------
inline const WString& HFCURLOracleFile::GetSchema() const
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