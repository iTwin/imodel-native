//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPageFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRFPageFile
//-----------------------------------------------------------------------------




BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// GetURL
// File information
//-----------------------------------------------------------------------------
inline const HFCPtr<HFCURL>& HRFPageFile::GetURL() const
    {
    return m_pURL;
    }

//-----------------------------------------------------------------------------
// Public
// CountPages
// File information
//-----------------------------------------------------------------------------
inline uint32_t HRFPageFile::CountPages() const
    {
    return (uint32_t)m_ListOfPageDescriptor.size();
    }

//-----------------------------------------------------------------------------
// Public
// GetPageDescriptor
// File information
//-----------------------------------------------------------------------------
inline HFCPtr<HRFPageDescriptor> HRFPageFile::GetPageDescriptor(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return m_ListOfPageDescriptor[pi_Page];
    }

//-----------------------------------------------------------------------------
// Public
// GetAccessMode
// Access mode management
//-----------------------------------------------------------------------------
inline HFCAccessMode HRFPageFile::GetAccessMode() const
    {
    return m_FileAccessMode;
    }
END_IMAGEPP_NAMESPACE
