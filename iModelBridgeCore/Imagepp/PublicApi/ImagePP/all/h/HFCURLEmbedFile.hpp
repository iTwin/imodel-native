//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLEmbedFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLEmbedFile
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the Path part of the
// URL string.
//-----------------------------------------------------------------------------
inline const WString& HFCURLEmbedFile::GetPath() const
    {
    return m_Path;
    }

//-----------------------------------------------------------------------------
// public
// HasPathTo
//-----------------------------------------------------------------------------
inline bool HFCURLEmbedFile::HasPathTo(HFCURL* pi_pURL)
    {
    return false;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overridden.
-----------------------------------------------------------------------------*/
inline void HFCURLEmbedFile::SetCreationTime(time_t   pi_NewTime)
    {
    m_creationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overridden.
-----------------------------------------------------------------------------*/
inline time_t HFCURLEmbedFile::GetCreationTime() const
    {
    return m_creationTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overridden.
-----------------------------------------------------------------------------*/
inline void HFCURLEmbedFile::SetModificationTime(time_t   pi_NewTime)
    {
    m_modificationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overridden.
-----------------------------------------------------------------------------*/
inline time_t HFCURLEmbedFile::GetModificationTime() const
    {
    return m_modificationTime;
    }

END_IMAGEPP_NAMESPACE