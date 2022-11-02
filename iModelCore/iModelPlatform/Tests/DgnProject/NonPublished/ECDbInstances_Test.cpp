/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
#include <Bentley/Logging.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

static Byte s_Utf8BOM[] = {0xef, 0xbb, 0xbf};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadStringFromUtf8File(Utf8String& strValue, WCharCP path)
    {
    strValue = "";

    BeFile file;
    BeFileStatus fileStatus = file.Open(path, BeFileAccess::Read);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus))
        return false;

    uint64_t rawSize;
    fileStatus = file.GetSize(rawSize);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close();
        return false;
        }
    uint32_t sizeToRead = (uint32_t) rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    fileStatus = file.Read(buffer, &sizeRead, sizeToRead);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus && sizeRead == sizeToRead))
        {
        file.Close();
        return false;
        }

    // Validate it's a UTF8 file
    if (!EXPECTED_CONDITION(buffer[0] == s_Utf8BOM[0] && buffer[1] == s_Utf8BOM[1] && buffer[2] == s_Utf8BOM[2]))
        {
        file.Close();
        return false;
        }

    for (uint32_t ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        strValue.append(1, buffer[ii]);
        }

    file.Close();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadJsonFromFile(Json::Value& jsonValue, WCharCP path)
    {
    Utf8String strValue;
    if (!ReadStringFromUtf8File(strValue, path))
        return false;

    return Json::Reader::Parse(strValue, jsonValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteStringToUtf8File(WCharCP path, Utf8StringCR strValue)
    {
#if defined (_WIN32)
    FILE* file = _wfsopen(path, L"w" CSS_UTF8, _SH_DENYWR);
#else
    FILE* file = fopen(Utf8String(path).c_str(), "w");
#endif
    if (file == NULL)
        {
        BeAssert(false);
        return false;
        }
    fwprintf(file, L"%ls", WString(strValue.c_str(), BentleyCharEncoding::Utf8).c_str());
    fclose(file);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteJsonToFile(WCharCP path, const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write(jsonValue);
    return WriteStringToUtf8File(path, strValue);
    }

