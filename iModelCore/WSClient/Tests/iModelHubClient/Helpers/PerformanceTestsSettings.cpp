/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestsSettings.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/TxnManager.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnMaterial.h>

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
PerformanceTestSettings& PerformanceTestSettings::Instance()
    {
    static PerformanceTestSettings* s_instance = nullptr;
    if (nullptr == s_instance)
        {
        s_instance = new PerformanceTestSettings();
        BeFileName fileName = Dgn::DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        fileName = fileName.AppendToPath(L"PerformanceTests.json");
        s_instance->ReadSettings(fileName);
        }

    return *s_instance;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
void PerformanceTestSettings::ReadSettings(BeFileNameCR settingsFile)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(settingsFile.c_str(), BeFileAccess::Read))
        return;

    ByteStream byteStream;
    if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
        return;

    Utf8String contents((Utf8CP) byteStream.GetData(), byteStream.GetSize());
    file.Close();

    Json::Reader reader;
    Json::Value settings;
    if (!reader.Parse(contents, settings))
        return;

    m_codePostSize = settings["CodePostSize"].asInt64();
    m_codeGetSize = settings["CodeGetSize"].asInt64();
    m_codeGetByIdSize = settings["CodeGetByIDSize"].asInt64();
    m_lockPostSize = settings["LockPostSize"].asInt64();
    m_lockGetSize = settings["LockGetSize"].asInt64();
    m_lockGetByIdSize = settings["LockGetByIDSize"].asInt64();
    m_codeGetAttemptsCount = settings["CodeGetAttemptsCount"].asInt64();
    m_lockGetAttemptsCount = settings["LockGetAttemptsCount"].asInt64();
    m_codeGetSplitCount = settings["CodeGetSplitCount"].asInt64();
    m_lockGetSplitCount = settings["LockGetSplitCount"].asInt64();

    m_codeGetSizeSecondCall = m_codeGetSize;
    if (settings.isMember("SecondCallCodeGetSize"))
        m_codeGetSizeSecondCall = settings["SecondCallCodeGetSize"].asInt64();
    m_codeGetByIdSizeSecondCall = m_codeGetByIdSize;
    if (settings.isMember("SecondCallCodeGetByIDSize"))
        m_codeGetByIdSizeSecondCall = settings["SecondCallCodeGetByIDSize"].asInt64();
    m_lockGetSizeSecondCall = m_lockGetSize;
    if (settings.isMember("SecondCallLockGetSize"))
        m_lockGetSizeSecondCall = settings["SecondCallLockGetSize"].asInt64();
    m_lockGetByIdSizeSecondCall = m_lockGetByIdSize;
    if (settings.isMember("SecondCallLockGetByIDSize"))
        m_lockGetByIdSizeSecondCall = settings["SecondCallLockGetByIDSize"].asInt64();
    }
