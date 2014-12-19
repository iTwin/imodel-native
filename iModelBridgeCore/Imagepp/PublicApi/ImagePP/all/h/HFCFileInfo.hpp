//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCFileInfo.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
inline bool HFCFileInfo::IsReadOnly()
    {
    return ((m_Flag & HFC_FILEINFO_RDONLY) != 0);
    }

inline bool HFCFileInfo::IsDirectory()
    {
    return ((m_Flag & HFC_FILEINFO_SUBDIR) != 0);
    }

inline bool HFCFileInfo::IsHidden()
    {
    return ((m_Flag & HFC_FILEINFO_HIDDEN) != 0);
    }

inline bool HFCFileInfo::FlagSet(uint32_t pi_Flag)
    {
    return ((m_Flag & pi_Flag) != 0);
    }

inline const WString& HFCFileInfo::GetName()
    {
    return m_FileName;
    }

inline uint32_t HFCFileInfo::GetFlag()
    {
    return m_Flag;
    }
