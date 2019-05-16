/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Windows.h>

USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
EXPORT_ATTRIBUTE BentleyStatus ExtractFileVersion(Utf8StringR versionString, BeFileNameCR filePath)
    {
    BentleyStatus   status = BSIERROR;
#ifdef BENTLEY_WIN32
    // query the size for DLL version info 
    ::DWORD dllHandle = 0;
    ::DWORD versionInfoSize = ::GetFileVersionInfoSizeW(filePath.c_str(), &dllHandle);
    if (versionInfoSize == 0)
        {
        status = static_cast<BentleyStatus>(::GetLastError());
        return  status;
        }

    ::LPBYTE buffer = nullptr;
    ::LPSTR versionInfoData = static_cast<::LPSTR>(new char[versionInfoSize]);
    if (versionInfoData == nullptr)
        return  status;

    // extract version info into a buffer
    if (::GetFileVersionInfoW(filePath.c_str(), dllHandle, versionInfoSize, versionInfoData))
        {
        // read version info from the extracted buffer
        ::UINT readSize = 0;
        if (::VerQueryValueW(versionInfoData, L"\\", reinterpret_cast<VOID FAR* FAR*>(&buffer), &readSize) && readSize > 0)
            {
            // parse version into to output string
            ::VS_FIXEDFILEINFO* versonInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(buffer);
            if (versonInfo != nullptr && versonInfo->dwSignature == 0xfeef04bd)
                {
                uint32_t majorV = (versonInfo->dwFileVersionMS >> 16) & 0xffff;
                uint32_t minorV = (versonInfo->dwFileVersionMS >>  0) & 0xffff;
                uint32_t subV1  = (versonInfo->dwFileVersionLS >> 16) & 0xffff;
                uint32_t subV2  = (versonInfo->dwFileVersionLS >>  0) & 0xffff;
                versionString.Sprintf ("%d.%d.%d.%d", majorV, minorV, subV1, subV2);
                }
            }
        }
    delete[] versionInfoData;
    status = static_cast<BentleyStatus>(::GetLastError());
#endif  // BENTLEY_WIN32
    return  status;
    }
