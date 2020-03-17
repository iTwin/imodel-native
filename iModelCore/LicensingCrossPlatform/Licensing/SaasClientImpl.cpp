/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SaasClientImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "LicensingDb.h"
#include "Providers/UlasProvider.h"

#include <Licensing/Utils/LogFileHelper.h>

#include <Bentley/BeSystemInfo.h>
#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

SaasClientImpl::SaasClientImpl
(
    int productId,
    Utf8StringCR featureString,
    IUlasProviderPtr ulasProvider,
    IEntitlementProviderPtr entitlementProvider
)
    {
    m_deviceId = BeSystemInfo::GetDeviceId();
    if (m_deviceId.Equals(""))
        m_deviceId = "DefaultDevice";

    m_productId = productId;


    m_featureString = featureString;
    m_ulasProvider = ulasProvider;
    m_entitlementProvider = entitlementProvider;
    }

bool LicenseStatusToRunStatus(LicenseStatus status)
    {
    switch (status)
        {
            case LicenseStatus::Ok: // intentional fallthrough
            case LicenseStatus::Offline: // intentional fallthrough
            case LicenseStatus::Trial:
                return true;

            case LicenseStatus::Expired: // intentional fallthrough
            case LicenseStatus::Error: // intentional fallthrough
            case LicenseStatus::AccessDenied: // intentional fallthrough
            case LicenseStatus::DisabledByLogSend: // intentional fallthrough
            case LicenseStatus::DisabledByPolicy: // intentional fallthrough
            case LicenseStatus::NotEntitled: // intentional fallthrough
            default:
                return false;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Matt Yale          9/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<TrackUsageStatus> SaasClientImpl::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId, AuthType authType, std::vector<int> productIds, Utf8StringCR deviceId, Utf8StringCR correlationId)
    {
    LOG.debug("UlasProvider::RealtimeTrackUsage");
    if (deviceId == "") //required for web v4 call TODO there is m_deviceId as well
        {
        return TrackUsageStatus::BadParam;
        }

    // override system device id if it is specified here
    if (!deviceId.empty())
        {
        m_deviceId = deviceId;
        }

    return m_entitlementProvider->FetchWebEntitlementV4(productIds, version, deviceId, projectId, accessToken, authType).then(
        [=] (WebEntitlementResult result)
        {
        auto allowed = LicenseStatusToRunStatus(result.Status);
        if (allowed)
            {
            return m_ulasProvider->RealtimeTrackUsage(accessToken, result.ProductId, m_featureString, m_deviceId, version, projectId, result.Status, correlationId, authType, result.PrincipalId)
                .then([=] ()
                    {
                    return TrackUsageStatus::Success;
                    })
                .onError([](std::exception const& e)
                    {
                    return TrackUsageStatus::EntitledButErrorUsageTracking;
                    });
            }
        else
            {
            return folly::makeFuture(TrackUsageStatus::NotEntitled);
            }
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Evan.Preslar                    03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<folly::Unit> SaasClientImpl::PostUserUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId, AuthType authType, int productId, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, Utf8StringCR principalId)
    {
    LOG.debug("SaasClientImpl::PostUserUsage");

    // override system product id if it is specified here
    if (productId != -1)
        {
        m_productId = productId;
        }

    // override system device id if it is specified here
    if (!deviceId.empty())
        {
        m_deviceId = deviceId;
        }

    return m_ulasProvider->RealtimeTrackUsage(accessToken, m_productId, m_featureString, m_deviceId, version, projectId, usageType, correlationId, authType, principalId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Evan.Preslar                    03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<folly::Unit> SaasClientImpl::PostFeatureUsage(Utf8StringCR accessToken, FeatureEvent featureEvent, AuthType authType, int productId, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, Utf8StringCR principalId)
    {
    LOG.debug("SaasClientImpl::PostFeatureUsage");

    // override system product id if it is specified here
    if (productId != -1)
        {
        m_productId = productId;
        }

    // override system device id if it is specified here
    if (!deviceId.empty())
        {
        m_deviceId = deviceId;
        }

    return m_ulasProvider->RealtimeMarkFeature(accessToken, featureEvent, m_productId, m_featureString, m_deviceId, usageType, correlationId, authType, principalId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Luke.Lindsey            11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<EntitlementResult> SaasClientImpl::CheckEntitlement
(
    Utf8StringCR accessToken,
    BeVersionCR version,
    Utf8StringCR projectId,
    AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Utf8StringCR correlationId
)
    {
    auto product = productId == -1 ? m_productId : productId;
    std::vector<int> productIds {product};
    return m_entitlementProvider->FetchWebEntitlementV4(productIds, version, deviceId, projectId, accessToken, authType).then(
        [=] (WebEntitlementResult e)
        {
        auto usageType = LicenseStatusToUsageType(e.Status);
        EntitlementResult entitlement {LicenseStatusToRunStatus(e.Status), e.PrincipalId, usageType};
        return entitlement;
        });
    }