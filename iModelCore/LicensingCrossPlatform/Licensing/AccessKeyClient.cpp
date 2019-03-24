/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/AccessKeyClient.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/AccessKeyClient.h>

#include "Providers/BuddiProvider.h"
#include "Providers/PolicyProvider.h"
#include "Providers/UlasProvider.h"
#include "AccessKeyClientImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClient::AccessKeyClient
(
    std::shared_ptr<struct AccessKeyClientImpl> implementation
)
    {
    m_impl = implementation;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClientPtr AccessKeyClient::Create
(
    Utf8StringCR accessKey,
    ClientInfoPtr clientInfo,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    IHttpHandlerPtr httpHandler
)
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, clientInfo, httpHandler, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<AccessKeyClient>(new AccessKeyClient(std::make_shared<AccessKeyClientImpl>(accessKey, clientInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus AccessKeyClient::StartApplication()
    {
    return m_impl->StartApplication();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AccessKeyClient::StopApplication()
    {
    return m_impl->StopApplication();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AccessKeyClient::MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData)
    {
    return m_impl->MarkFeature(featureId, featureUserData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus AccessKeyClient::GetLicenseStatus()
    {
    return m_impl->GetLicenseStatus();
    }
