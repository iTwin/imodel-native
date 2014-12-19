//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCDriveInfo.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Definition for class HFCDriveInfo
//-----------------------------------------------------------------------------
#pragma once

//####################################################
// INCLUDE FILES
//####################################################

class HFCDriveInfo
    {
public:

    // Construction - destruction
    HFCDriveInfo();
    HFCDriveInfo(WChar pi_DriveLetter,
                 uint32_t pi_DriveType,
                 unsigned __int64 pi_TotalSpace,
                 unsigned __int64 pi_FreeSpace);
    HFCDriveInfo(const HFCDriveInfo& pi_Src);
    virtual ~HFCDriveInfo();

    // Assignment operator
    HFCDriveInfo& operator=(const HFCDriveInfo& pi_Src);

    // Comparison operator
    bool operator==(const HFCDriveInfo& pi_Cmp) const;
    bool operator<(const HFCDriveInfo& pi_Cmp) const;
    bool operator>(const HFCDriveInfo& pi_Cmp) const;
    bool operator!=(const HFCDriveInfo& pi_Cmp) const;

    // Operations
    bool IsCdRom();
    bool IsRemovable();
    bool IsFixed();
    bool IsRemote();
    bool IsRamDisk();

    // Get - set
    WChar  GetDriveLetter();
    uint32_t GetDriveType();
    unsigned __int64 GetDriveTotalSpace();
    unsigned __int64 GetDriveFreeSpace();

    void  SetDriveLetter(WChar pi_DriveLetter);
    void  SetDriveType(uint32_t pi_DriveType);
    void  SetDriveTotalSpace(unsigned __int64 pi_TotalSpace);
    void  SetDriveFreeSpace(unsigned __int64 pi_FreeSpace);

protected:

private:

    void CommonCopy(const HFCDriveInfo& pi_Src);

    WChar  m_Drive;
    uint32_t m_DriveType;
    unsigned __int64 m_TotalSpace;
    unsigned __int64 m_FreeSpace;
    };

#include "HFCDriveInfo.hpp"
