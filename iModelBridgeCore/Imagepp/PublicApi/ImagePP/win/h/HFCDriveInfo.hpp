//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCDriveInfo.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
inline bool HFCDriveInfo::operator==(const HFCDriveInfo& pi_Cmp) const
    {
    return (m_Drive == pi_Cmp.m_Drive);
    }

inline bool HFCDriveInfo::operator<(const HFCDriveInfo& pi_Cmp) const
    {
    return (m_Drive < pi_Cmp.m_Drive);
    }

inline bool HFCDriveInfo::operator>(const HFCDriveInfo& pi_Cmp) const
    {
    return (m_Drive > pi_Cmp.m_Drive);
    }

inline bool HFCDriveInfo::operator!=(const HFCDriveInfo& pi_Cmp) const
    {
    return (m_Drive != pi_Cmp.m_Drive);
    }

inline bool HFCDriveInfo::IsCdRom()
    {
    return (m_DriveType == DRIVE_CDROM);
    }

inline bool HFCDriveInfo::IsRemovable()
    {
    return (m_DriveType == DRIVE_REMOVABLE);
    }

inline bool HFCDriveInfo::IsFixed()
    {
    return (m_DriveType == DRIVE_FIXED);
    }

inline bool HFCDriveInfo::IsRemote()
    {
    return (m_DriveType == DRIVE_REMOTE);
    }

inline bool HFCDriveInfo::IsRamDisk()
    {
    return (m_DriveType == DRIVE_RAMDISK);
    }

inline WChar HFCDriveInfo::GetDriveLetter()
    {
    return m_Drive;
    }

inline uint32_t HFCDriveInfo::GetDriveType()
    {
    return m_DriveType;
    }

inline unsigned __int64 HFCDriveInfo::GetDriveTotalSpace()
    {
    return m_TotalSpace;
    }

inline unsigned __int64 HFCDriveInfo::GetDriveFreeSpace()
    {
    return m_FreeSpace;
    }

inline void HFCDriveInfo::SetDriveLetter(WChar pi_DriveLetter)
    {
    m_Drive = pi_DriveLetter;
    }

inline void HFCDriveInfo::SetDriveType(uint32_t pi_DriveType)
    {
    m_DriveType = pi_DriveType;
    }

inline void HFCDriveInfo::SetDriveTotalSpace(unsigned __int64 pi_TotalSpace)
    {
    m_TotalSpace = pi_TotalSpace;
    }

inline void HFCDriveInfo::SetDriveFreeSpace(unsigned __int64 pi_FreeSpace)
    {
    m_FreeSpace = pi_FreeSpace;
    }

