/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Licensing/Licensing.h>
#include <Licensing/AuthType.h>
#include <Licensing/ApplicationInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct ClientConfig> ClientConfigPtr;
typedef const struct  ClientConfig& ClientConfigCR;
typedef struct ClientConfig& ClientConfigR;

struct ClientConfig
    {

    private:
        WebServices::ConnectSignInManager::UserInfo m_userInfo;
        ApplicationInfoPtr m_appInfo;
        std::shared_ptr<WebServices::IConnectAuthenticationProvider> m_authProvider;
        BeFileName m_dbPath;
        bool m_offlineMode = false;
        Utf8String m_projectId;
        Utf8String m_featureString;
        Http::IHttpHandlerPtr m_customHttpHandler;
        AuthType m_authType = AuthType::OIDC;

    public:

        ClientConfig
        (
            WebServices::ConnectSignInManager::UserInfo userInfo,
            ApplicationInfoPtr appInfo,
            std::shared_ptr<WebServices::IConnectAuthenticationProvider> authProvider,
            BeFileName dbPath
        ) : m_userInfo(userInfo), m_appInfo(appInfo), m_authProvider(authProvider), m_dbPath(dbPath) {}
           

        virtual ~ClientConfig() {}

        void SetUserInfo(WebServices::ConnectSignInManager::UserInfo userInfo) { m_userInfo = userInfo; }

        WebServices::ConnectSignInManager::UserInfo GetUserInfo() { return m_userInfo; }

        void SetAppInfo(ApplicationInfoPtr appInfo)
            {
            m_appInfo = appInfo;
            }

        ApplicationInfoPtr GetAppInfo() { return m_appInfo; }

        void SetAuthProvider(std::shared_ptr<WebServices::IConnectAuthenticationProvider> authProvider) { m_authProvider = authProvider; }

        std::shared_ptr<WebServices::IConnectAuthenticationProvider> GetAuthProvider() { return m_authProvider; }

        void SetDBPath(BeFileNameCR dbPath) { m_dbPath = dbPath; }

        BeFileName GetDBPath() { return m_dbPath; }

        void SetOfflineMode(bool isOffline) { m_offlineMode = isOffline; }

        bool GetOfflineMode() { return m_offlineMode; }

        void SetProjectId(Utf8StringCR projectId) { m_projectId = projectId; }

        Utf8String GetProjectId() { return m_projectId; }

        void SetFeatureString(Utf8StringCR featureString) { m_featureString = featureString; }

        Utf8String GetFeatureString() { return m_featureString; }

        void SetCustomHttpHandler(Http::IHttpHandlerPtr custom) { m_customHttpHandler = custom; }

        Http::IHttpHandlerPtr GetCustomHttpHandler() { return m_customHttpHandler; }

        void SetAuthType(AuthType type) { m_authType = type; }

        AuthType GetAuthType() { return m_authType; }
    };
END_BENTLEY_LICENSING_NAMESPACE


