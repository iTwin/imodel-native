/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/IntegrationTestsSettings.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
        Utf8String m_url;
        WebServices::UrlProvider::Environment m_environment;
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
        Utf8String GetServerUrl() const;
        WebServices::UrlProvider::Environment GetEnvironment() const;
        WebServices::ClientInfoPtr GetClientInfo() const;
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
