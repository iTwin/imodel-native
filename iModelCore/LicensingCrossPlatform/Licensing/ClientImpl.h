/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/Licensing/ClientImpl.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include "UsageDb.h"
#include "PolicyToken.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/DelayedExecutor.h>

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientImpl> ClientImplPtr;
struct ClientImpl
{
private:
    BeFileName m_dbPath;
    std::shared_ptr<IConnectAuthenticationProvider> m_authProvider;
    ClientInfoPtr m_clientInfo;
    ConnectSignInManager::UserInfo m_userInfo;
    IHttpHandlerPtr m_httpHandler;
    ITimeRetrieverPtr m_timeRetriever;
    IDelayedExecutorPtr m_delayedExecutor;

    int64_t m_heartbeatInterval;
    int64_t m_lastRunningheartbeatStartTime = 0;

    std::unique_ptr<UsageDb> m_usageDb;

private:
    folly::Future<Utf8String> PerformGetPolicyRequest();

    void HeartbeatUsage(int64_t currentTime);

public:
    LICENSING_EXPORT ClientImpl
        (
        BeFileNameCR dbPath,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        ClientInfoPtr clientInfo,
        const ConnectSignInManager::UserInfo& userInfo,
        IHttpHandlerPtr httpHandler,
        uint64_t heartbeatInterval,
        ITimeRetrieverPtr timeRetriever = TimeRetriever::Get(),
        IDelayedExecutorPtr delayedExecutor = DelayedExecutor::Get()
        );

    LICENSING_EXPORT BentleyStatus StartApplication(); 
    LICENSING_EXPORT BentleyStatus StopApplication();

    LICENSING_EXPORT folly::Future<Utf8String> GetCertificate();
    LICENSING_EXPORT folly::Future<std::shared_ptr<PolicyToken>> GetPolicy();

    // usageSCV usage file to send
    // The company ID in SAP. // TODO: figure out where to get this one from.
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsage(BeFileNameCR usageSCV, Utf8StringCR ultId);

    // Used in tests
    LICENSING_EXPORT UsageDb& GetUsageDb();
};

END_BENTLEY_LICENSING_NAMESPACE
