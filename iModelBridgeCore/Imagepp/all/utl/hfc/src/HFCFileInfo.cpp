//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCFileInfo.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCFileInfo
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCFileInfo.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HFCFileInfo::HFCFileInfo(const WString& pi_Name, uint32_t pi_Flag)
    {
    m_FileName = pi_Name;
    m_Flag = pi_Flag;
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HFCFileInfo::~HFCFileInfo()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator==
//-----------------------------------------------------------------------------
bool HFCFileInfo::operator==(const HFCFileInfo& pi_Src)
    {
    if(pi_Src.m_Flag == m_Flag && pi_Src.m_FileName == m_FileName)
        return true;
    return false;
    }

//-----------------------------------------------------------------------------
// public
// AsChar.
//-----------------------------------------------------------------------------
WString HFCFileInfo::AsChar()
    {
    WChar  flags[34];

#if defined (ANDROID) || defined (__APPLE__)
    swprintf (flags, 33, L"%u", m_Flag);
#elif defined (_WIN32)
    _itow_s(m_Flag, flags, 33, 10);
#endif
    return WString(m_FileName + L"," + flags);
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HFCFileInfo::HFCFileInfo(const HFCFileInfo& pi_Src)
    {
    CommonCopy(pi_Src);
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HFCFileInfo& HFCFileInfo::operator=(const HFCFileInfo& pi_Src)
    {
    // Avoid copy of self
    if( &pi_Src != this )
        CommonCopy(pi_Src);
    return *this;
    }

//-----------------------------------------------------------------------------
// private
//
//-----------------------------------------------------------------------------
void HFCFileInfo::CommonCopy(const HFCFileInfo& pi_Src)
    {
    m_FileName = pi_Src.m_FileName;
    m_Flag     = pi_Src.m_Flag;
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCFileInfo::SetReadOnly(bool pi_State)
    {
    if( pi_State )
        m_Flag |= HFC_FILEINFO_RDONLY;
    else
        m_Flag &= ~HFC_FILEINFO_RDONLY;
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCFileInfo::SetDirectory(bool pi_State)
    {
    if( pi_State )
        m_Flag |= HFC_FILEINFO_SUBDIR;
    else
        m_Flag &= ~HFC_FILEINFO_SUBDIR;
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCFileInfo::SetHidden(bool pi_State)
    {
    if( pi_State )
        m_Flag |= HFC_FILEINFO_HIDDEN;
    else
        m_Flag &= ~HFC_FILEINFO_HIDDEN;
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCFileInfo::SetName(const WString& pi_Name)
    {
    m_FileName = pi_Name;
    }
