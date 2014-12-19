//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCSysCfg.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Returns the architecture type
//-----------------------------------------------------------------------------
inline HFCSysCfg::SYSTEM_CONFIG HFCSysCfg::GetArchitectureTypeCode()
    {
    return m_processorArchitecture;
    }

//-----------------------------------------------------------------------------
// Returns the processor type
//-----------------------------------------------------------------------------
inline HFCSysCfg::SYSTEM_CONFIG HFCSysCfg::GetProcessorTypeCode()
    {
    return m_processorType;
    }

//-----------------------------------------------------------------------------
// Returns the operating system type
//-----------------------------------------------------------------------------
inline HFCSysCfg::SYSTEM_CONFIG HFCSysCfg::GetOperatingSystemTypeCode()
    {
    return m_osId;
    }

//-----------------------------------------------------------------------------
// Returns the number of processors
//-----------------------------------------------------------------------------
inline uint32_t HFCSysCfg::GetNumberOfProcessor()
    {
    return m_numberOfProcessor;
    }

//-----------------------------------------------------------------------------
// Returns the processor level
//-----------------------------------------------------------------------------
inline HFCSysCfg::SYSTEM_CONFIG HFCSysCfg::GetProcessorLevelCode()
    {
    return m_processorLevel;
    }

//-----------------------------------------------------------------------------
// Returns the operating system major version number
//-----------------------------------------------------------------------------
inline uint32_t HFCSysCfg::GetMajorVersion()
    {
    return m_osVersion.dwMajorVersion;
    }

//-----------------------------------------------------------------------------
// Returns the operating system minor version number
//-----------------------------------------------------------------------------
inline uint32_t HFCSysCfg::GetMinorVersion()
    {
    return m_osVersion.dwMinorVersion;
    }

//-----------------------------------------------------------------------------
// Returns the operating system string description
//-----------------------------------------------------------------------------
inline const WChar* HFCSysCfg::GetOperatingSystemStr()
    {
    return m_name[m_osId];
    }

//-----------------------------------------------------------------------------
// Returns the architecture string description
//-----------------------------------------------------------------------------
inline const WChar* HFCSysCfg::GetArchitectureTypeStr()
    {
    return m_name[m_processorArchitecture];
    }

//-----------------------------------------------------------------------------
// Returns the processor string description
//-----------------------------------------------------------------------------
inline const WChar* HFCSysCfg::GetProcessorTypeStr()
    {
    return m_name[m_processorType];
    }

//-----------------------------------------------------------------------------
// Returns the processor level string description
//-----------------------------------------------------------------------------
inline const WChar* HFCSysCfg::GetProcessorLevelStr()
    {
    return m_name[m_processorLevel];
    }

//-----------------------------------------------------------------------------
// Returns the operating system build number
//-----------------------------------------------------------------------------
inline uint32_t HFCSysCfg::GetBuildNumber()
    {
    return m_dwBuildNumber;
    }

//-----------------------------------------------------------------------------
// Returns the operating system additional information string description
//-----------------------------------------------------------------------------
inline const WChar* HFCSysCfg::GetAdditionalInfoStr()
    {
    return m_osVersion.szCSDVersion;
    }

//-----------------------------------------------------------------------------
// Returns the computer disk drive information list
//-----------------------------------------------------------------------------
inline HFCSysCfg::DRIVEINFOLIST HFCSysCfg::GetDriveInfoList()
    {
    return m_DriveInfoList;
    }

