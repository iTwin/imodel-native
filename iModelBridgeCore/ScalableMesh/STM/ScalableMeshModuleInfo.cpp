/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshModuleInfo.cpp,v $
|   $Revision: 1.0 $
|       $Date: 2019/06/28 $
|     $Author: Richard.Bois $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ScalableMeshModuleInfo.h"

#include <ScalableMesh/ScalableMeshLib.h>  
#include "Stores/SMStoreUtils.h"

#include <Bentley/BeFileName.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Richard.Bois                 05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt InitializeModuleHandle(ScalableMeshModuleInfo* moduleInfo)
    {
#if _WIN32
    if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                         GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCSTR)InitializeModuleHandle, &moduleInfo->m_handle) == 0)
        {
        int ret = GetLastError();
        (void)ret;
        return ERROR;
        }
#endif
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Richard.Bois                 05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshModuleInfo::ScalableMeshModuleInfo()
    {
    if(SUCCESS != InitializeModuleHandle(this))
        {
        BeAssert(!"Couldn't initialize module info");
        return;
        }

#if _WIN32
    char module_path[MAX_PATH];
    if(GetModuleFileName(m_handle, module_path, sizeof(module_path)) == 0)
        {
        int ret = GetLastError();
        (void)ret;
        BeAssert(!"Couldn't extract module file path");
        return;
        }

    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize = GetFileVersionInfoSize(module_path, &verHandle);

    auto productInfo = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProductInfo();
    m_productName = productInfo.m_productName;
    m_productVersion = productInfo.m_productVersion;
    m_publisherName = BEFILENAME(GetFileNameAndExtension, BeFileName(module_path));

    if(verSize != NULL)
        {
        LPSTR verData = new char[verSize];

        if(GetFileVersionInfo(module_path, verHandle, verSize, verData))
            {
            if(VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
                {
                if(size)
                    {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if(verInfo->dwSignature == 0xfeef04bd)
                        {
                        m_publisherVersion = WPrintfString(L"%d.%d.%d.%d", (verInfo->dwFileVersionMS >> 16) & 0xffff,
                            (verInfo->dwFileVersionMS >> 0) & 0xffff,
                                                           (verInfo->dwFileVersionLS >> 16) & 0xffff,
                                                           (verInfo->dwFileVersionLS >> 0) & 0xffff);
                        //BeFile dll_file;
                        //
                        //if(BeFileStatus::Success == OPEN_FILE(dll_file, WString(module_path).c_str(), BeFileAccess::Read))
                        //    {
                        //    FILETIME FileTime;
                        //    SYSTEMTIME stUTC;
                        //    if(GetFileTime(dll_file.GetHandle(), &FileTime, NULL, NULL))
                        //        {
                        //        FileTimeToSystemTime(&FileTime, &stUTC);
                        //        m_dllCreationTime = WPrintfString(L"%d-%02d-%02d %02d:%02d:%02d, Coordinated Universal Time (UTC)", stUTC.wYear, stUTC.wMonth, stUTC.wDay, stUTC.wHour, stUTC.wMinute, stUTC.wSecond);
                        //        }
                        //    }
                        }
                    }
                }
            }
        }
#else
    auto productInfo = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProductInfo();
    m_productName = productInfo.m_productName;
    m_productVersion = productInfo.m_productVersion;
    m_publisherName = productInfo.m_productName;
    m_publisherVersion = m_productVersion;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Richard.Bois                 05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshModuleInfo::ToJson(Json::Value& output) const
    {
    if(!m_publisherName.empty()) output["Publisher"] = Utf8String(m_publisherName.c_str());
    if(!m_publisherVersion.empty()) output["Publisher Version"] = Utf8String(m_publisherVersion.c_str());
    //if (!m_dllCreationTime.empty()) output["Module Creation Date"] = Utf8String(m_dllCreationTime.c_str());
    if(!m_productName.empty()) output["Product"] = Utf8String(m_productName.c_str());
    if(!m_productVersion.empty()) output["Product Version"] = Utf8String(m_productVersion.c_str());

    return !output.empty();
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
