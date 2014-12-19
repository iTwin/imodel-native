//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCSysCfg.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Definition for class HFCSysCfg
//-----------------------------------------------------------------------------

#pragma once


//####################################################
// INCLUDE FILES
//####################################################

#include "HFCDriveInfo.h"


class HFCSysCfg
    {
public:

    typedef list< HFCDriveInfo, allocator<HFCDriveInfo> > DRIVEINFOLIST;

    enum SYSTEM_CONFIG
        {
        ARCHITECTURE_INTEL = 0,
        ARCHITECTURE_MIPS,
        ARCHITECTURE_ALPHA,
        ARCHITECTURE_PPC,
        INTEL_80386,
        INTEL_80486,
        INTEL_PENTIUM,
        INTEL_PENTIUM_PRO_II,
        MIPS_R4000,
        ALPHA_21064,
        ALPHA_21066,
        ALPHA_21164,
        PPC_601,
        PPC_603,
        PPC_604,
        PPC_603P,
        PPC_604P,
        PPC_620,
        WINDOWS_NT,
        WINDOWS_95,
        WIN32S,
        UNKNOWN
        };

    // Construction - destruction
    _HDLLw HFCSysCfg();
    _HDLLw ~HFCSysCfg();

    // Get-set methods
    SYSTEM_CONFIG GetArchitectureTypeCode();
    SYSTEM_CONFIG GetProcessorTypeCode();
    SYSTEM_CONFIG GetOperatingSystemTypeCode();
    uint32_t GetNumberOfProcessor();
    SYSTEM_CONFIG GetProcessorLevelCode();
    uint32_t GetMajorVersion();
    uint32_t GetMinorVersion();
    uint32_t GetBuildNumber();
    size_t GetTotalPhysicalMemory();
    size_t GetAvailPhysicalMemory();
    size_t GetTotalVirtualMemory();
    size_t GetAvailVirtualMemory();
    HFCSysCfg::DRIVEINFOLIST GetDriveInfoList();
    const WChar*  GetOperatingSystemStr();
    const WChar*  GetArchitectureTypeStr();
    const WChar*  GetProcessorTypeStr();
    const WChar*  GetProcessorLevelStr();
    const WChar*  GetAdditionalInfoStr();

private:

    // Not implemented
    HFCSysCfg(const HFCSysCfg& pi_rObject);
    HFCSysCfg& operator=(const HFCSysCfg& pi_rObject);

    // Fill methods
    void FillIntel();
    void FillMips();
    void FillAlpha();
    void FillPPC();
    void FillDriveInfoList();
    void InitString();

    // Attributes
    WChar* m_name[23];
    uint32_t m_numberOfProcessor;
    SYSTEM_CONFIG m_processorArchitecture, m_processorLevel, m_processorType;
    SYSTEM_CONFIG m_osId;
    SYSTEM_INFO   m_info;
    DWORD          m_dwBuildNumber;
    OSVERSIONINFO m_osVersion;
    DRIVEINFOLIST m_DriveInfoList;
    };

#include "HFCSysCfg.hpp"
