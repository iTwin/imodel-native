
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    WebServices::ClientInfoPtr clientInfo,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler
    )
    {
    ApplicationInfoPtr applicationInfo = std::make_shared<ApplicationInfo>(clientInfo->GetApplicationVersion(), clientInfo->GetDeviceId(), clientInfo->GetApplicationProductId());
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, httpHandler, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<AccessKeyClient>(new AccessKeyClient(std::make_shared<AccessKeyClientImpl>(accessKey, applicationInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr, "")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClientPtr AccessKeyClient::Create
    (
    Utf8StringCR accessKey,
    ApplicationInfoPtr applicationInfo,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, httpHandler, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<AccessKeyClient>(new AccessKeyClient(std::make_shared<AccessKeyClientImpl>(accessKey, applicationInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr, "")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    7/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClientPtr AccessKeyClient::AgnosticCreateWithUltimate
    (
    Utf8StringCR accessKey,
    ApplicationInfoPtr applicationInfo,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR ultimateId,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, httpHandler, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<AccessKeyClient>(new AccessKeyClient(std::make_shared<AccessKeyClientImpl>(accessKey, applicationInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr, ultimateId)));
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t AccessKeyClient::GetTrialDaysRemaining()
    {
    return m_impl->GetTrialDaysRemaining();
    }

int64_t AccessKeyClient::ImportCheckout(BeFileNameCR filepath)
{
	return m_impl->ImportCheckout(filepath);
}

BentleyStatus AccessKeyClient::DeleteLocalCheckout(Utf8StringCR productId)
    {
    return m_impl->DeleteLocalCheckout(productId);
    }
