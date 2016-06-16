/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/FSTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "FSTest.h"
#include <Bentley/BeTest.h>
#include <BeSQLite/BeSQLite.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

BeFileName FSTest::GetAssetsDir()
    {
    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    return path;
    }

BeFileName FSTest::GetTempDir()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    return path;
    }

BeFileName FSTest::StubFilePath(Utf8StringCR customFileName)
    {
    BeFileName fileName;
    if (customFileName.empty())
        {
        BeSQLite::BeSQLiteLib::Initialize(GetTempDir());
        fileName = BeFileName(BeSQLite::BeGuid().ToString() + ".txt");
        }
    else
        {
        fileName = BeFileName(customFileName);
        }

    BeFileName filePath = GetTempDir().AppendToPath(fileName);
    return filePath;
    }

BeFileName FSTest::StubFile(Utf8StringCR content, Utf8StringCR customFileName)
    {
    BeFileName filePath = StubFilePath(customFileName);

    BeFile file;
    file.Create(filePath);
    file.Write(nullptr, content.c_str(), static_cast<uint32_t>(content.length()));
    file.Close();

    return filePath;
    }

Utf8String FSTest::ReadFile(BeFileNameCR filePath)
    {
    bvector<Byte> fileContents;

    BeFile file;
    BeFileStatus status;

    status = file.Open(filePath, BeFileAccess::Read);
    BeAssert(status == BeFileStatus::Success);

    status = file.ReadEntireFile(fileContents);
    BeAssert(status == BeFileStatus::Success);

    status = file.Close();
    BeAssert(status == BeFileStatus::Success);

    Utf8String stringContents;
    stringContents.append(fileContents.begin(), fileContents.end());
    return stringContents;
    }

void FSTest::WriteToFile(Utf8StringCR content, BeFileNameCR filePath)
    {
    uint32_t written = 0;

    BeFile file;
    BeFileStatus status;

    status = file.Create(filePath, true);
    BeAssert(status == BeFileStatus::Success);

    status = file.Write(&written, content.c_str(), (uint32_t) content.size());
    BeAssert(status == BeFileStatus::Success);

    status = file.Close();
    BeAssert(status == BeFileStatus::Success);
    }