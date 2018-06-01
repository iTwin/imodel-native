/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/IntegrationTestsSettings.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsSettings.h"
#include <Bentley/BeTest.h>
#include <Bentley/BeSystemInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

IntegrationTestsSettings::IntegrationTestsSettings()
    {
    BeFileName fileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(fileName);
    fileName = fileName.AppendToPath(L"IntegrationTests.json");
    ReadSettings(fileName);
    }

IntegrationTestsSettings& IntegrationTestsSettings::Instance()
    {
    static IntegrationTestsSettings s_instance;
    return s_instance;
    }

void IntegrationTestsSettings::ReadSettings(BeFileNameCR settingsFile)
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

    auto users = settings["Users"];
    for (auto user : users)
        {
        if (user["IsAdmin"].asBool())
            {
            m_adminCredentials = Credentials(user["Username"].asString(), user["Password"].asString());
            continue;
            }
        if (user["IsServiceAccount"].asBool())
            {
            m_serviceAccountCredentials = Credentials(user["Username"].asString(), user["Password"].asString());
            continue;
            }

        m_nonAdminCredentials = Credentials(user["Username"].asString(), user["Password"].asString());
        }

    m_projectId = settings["ProjectId"].asString();
    Utf8String environment = settings["Environment"].asString();
    if ("DEV" == environment)
        m_environment = UrlProvider::Environment::Dev;
    else if ("QA" == environment)
        m_environment = UrlProvider::Environment::Qa;
    else if ("PROD" == environment)
        m_environment = UrlProvider::Environment::Release;
    }

CredentialsCR IntegrationTestsSettings::GetValidAdminCredentials() const
    {
    return m_adminCredentials;
    }

CredentialsCR IntegrationTestsSettings::GetValidServiceAccountCredentials() const
    {
    return m_serviceAccountCredentials;
    }

CredentialsCR IntegrationTestsSettings::GetValidNonAdminCredentials() const
    {
    return m_nonAdminCredentials;
    }

Credentials IntegrationTestsSettings::GetWrongUsername() const
    {
    return Credentials(m_adminCredentials.GetUsername() + "wrong", m_adminCredentials.GetPassword());
    }

Credentials IntegrationTestsSettings::GetWrongPassword() const
    {
    return Credentials(m_adminCredentials.GetUsername(), m_adminCredentials.GetPassword() + "wrong");
    }

Utf8String IntegrationTestsSettings::GetProjectId() const
    {
    return m_projectId;
    }

ClientInfoPtr IntegrationTestsSettings::GetClientInfo() const
    {
    Utf8String deviceId = BeSystemInfo::GetDeviceId();

    Utf8String model = BeSystemInfo::GetMachineName();
    if (!model.empty())
        {
        model += "; ";
        }

    Utf8PrintfString systemDescription("%s %s %s",
        model.c_str(),
        BeSystemInfo::GetOSName().c_str(),
        BeSystemInfo::GetOSVersion().c_str());
    
    return std::shared_ptr<WebServices::ClientInfo>(new WebServices::ClientInfo("iModelHub.ClientAPITests", BeVersion(1, 0), "05f0c41d-413f-4431-8e66-22999dfe16fa", deviceId, systemDescription, "2485"));
    }

UrlProvider::Environment IntegrationTestsSettings::GetEnvironment() const
    {
    return m_environment;
    }
