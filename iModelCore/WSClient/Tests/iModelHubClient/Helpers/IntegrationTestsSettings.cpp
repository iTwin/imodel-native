/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IntegrationTestsSettings.h"
#include <Bentley/BeTest.h>
#include <Bentley/BeSystemInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

BeFileName IntegrationTestsSettings::ResolveSettingsPath()
    {
    BeFileName fileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(fileName);
    fileName = fileName.AppendToPath(L"IntegrationTests.json");
    return fileName;
    }

IntegrationTestsSettings::IntegrationTestsSettings()
    {
    BeFileName fileName = ResolveSettingsPath();
    ReadSettings(fileName);
    }

IntegrationTestsSettings& IntegrationTestsSettings::Instance()
    {
    static IntegrationTestsSettings s_instance;
    return s_instance;
    }

Json::Value IntegrationTestsSettings::ReadSettingsJson(BeFileNameCR settingsFile)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(settingsFile.c_str(), BeFileAccess::Read))
        return nullptr;

    ByteStream byteStream;
    if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
        return nullptr;

    Utf8String contents((Utf8CP) byteStream.GetData(), byteStream.GetSize());
    file.Close();

    Json::Reader reader;
    Json::Value settings;
    if (!reader.Parse(contents, settings))
        return nullptr;

    return settings;
    }

UrlProvider::Environment IntegrationTestsSettings::ResolveEnvironment(Json::Value settings)
    {
    Utf8String environment = settings["Environment"].asString();
    if ("DEV" == environment)
        return UrlProvider::Environment::Dev;
    else if ("QA" == environment)
        return UrlProvider::Environment::Qa;
    else if ("PROD" == environment)
        return UrlProvider::Environment::Release;

    return UrlProvider::Environment::Dev;
    }

void IntegrationTestsSettings::ReadSettings(BeFileNameCR settingsFile)
    {
    Json::Value settings = ReadSettingsJson(settingsFile);

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

    m_url = settings["ServerUrl"].asString();
    if (Utf8String::IsNullOrEmpty(m_url.c_str()))
        m_url = UrlProvider::Urls::iModelHubApi.Get();
    m_projectId = settings["ProjectId"].asString();
    m_assetId = settings["AssetId"].asString();
    m_isiModelBank = settings.isMember("iModelBank") && settings["iModelBank"].asBool();
    m_storageClientType = settings["StorageClientType"].asString();

    m_environment = ResolveEnvironment(settings);
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

Utf8String IntegrationTestsSettings::GetAssetId() const
    {
    return m_assetId;
    }

Utf8String IntegrationTestsSettings::GetServerUrl() const
    {
    return m_url;
    }

Utf8String IntegrationTestsSettings::GetStorageClientType() const
    {
    return m_storageClientType;
    }

bool IntegrationTestsSettings::IsiModelBank() const
    {
    return m_isiModelBank;
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

UrlProvider::Environment IntegrationTestsSettings::ReadEnvironment()
    {
    BeFileName fileName = ResolveSettingsPath();
    Json::Value settings = ReadSettingsJson(fileName);

    return ResolveEnvironment(settings);
    }
