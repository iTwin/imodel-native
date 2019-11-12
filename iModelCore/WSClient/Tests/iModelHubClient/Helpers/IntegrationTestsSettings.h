/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Client/ClientInfo.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
struct IntegrationTestsSettings
    {
    private:
        Http::Credentials m_nonAdminCredentials;
        Http::Credentials m_adminCredentials;
        Http::Credentials m_serviceAccountCredentials;

        Utf8String m_projectId;
        Utf8String m_assetId;
        Utf8String m_url;
        WebServices::UrlProvider::Environment m_environment;
        static BeFileName ResolveSettingsPath();
        static Json::Value ReadSettingsJson(BeFileNameCR settingsFile);
        static WebServices::UrlProvider::Environment ResolveEnvironment(Json::Value settings);
        void ReadSettings(BeFileNameCR settingsFile);
        IntegrationTestsSettings();
    public:
        static IntegrationTestsSettings& Instance();
        Http::CredentialsCR GetValidAdminCredentials() const;
        Http::CredentialsCR GetValidServiceAccountCredentials() const;
        Http::CredentialsCR GetValidNonAdminCredentials() const;
        Http::Credentials GetWrongUsername() const;
        Http::Credentials GetWrongPassword() const;
        Utf8String GetProjectId() const;
        Utf8String GetAssetId() const;
        Utf8String GetServerUrl() const;
        WebServices::UrlProvider::Environment GetEnvironment() const;
        static WebServices::UrlProvider::Environment ReadEnvironment();
        WebServices::ClientInfoPtr GetClientInfo() const;
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
