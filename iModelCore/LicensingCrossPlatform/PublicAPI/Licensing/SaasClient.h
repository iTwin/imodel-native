/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/EntitlementResult.h>

#include "LicenseStatus.h"

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/UsageType.h>
#include <Licensing/AuthType.h>
#include <Licensing/TrackUsageStatus.h>


BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SaasClient> SaasClientPtr;

struct SaasClient
    {
private:
    std::shared_ptr<struct ISaasClient> m_impl;

    SaasClient
        (
        std::shared_ptr<struct ISaasClient> implementation
        );

public:
    //! Creates a SaaS Licensing Client
    //! @param[in] productId ProductId, must be provided if want to track against a particular product, alternative is tracking linked back to OIDC client id
    //! @param[in] featureString Feature string of the application to track usage of
    //! @param[in] customHttpHandler CustomHttpHandler, defaults to a nullptr
    LICENSING_EXPORT static SaasClientPtr Create
    (
        int productId = 0,
        Utf8StringCR featureString = "",
        Http::IHttpHandlerPtr customHttpHandler = nullptr
    );

    //! Send realtime usage
    //! @deprecated Use SaasClient::PostUserUsage instead
    //! @param[in] accessToken OIDC or SAML token of user to track usage against. OIDC is preferred
    //! @param[in] version version for this usage
    //! @param[in] projectId projectId of this usage
    //! @param[in] authType optional - specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
    //! @param[in] productId optional - specify the productId for this usage
    //! @param[in] deviceId optional - specify the deviceId for this usage, required for multi-product id tracking
    //! @param[in] usageType optional - specify the usage type for this usage
    //! @param[in] correlationId optional - specify the correlationId for this usage, must be a GUID
    LICENSING_EXPORT folly::Future<BentleyStatus> TrackUsage
        (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType = AuthType::OIDC,
        int productId = -1,
        Utf8StringCR deviceId = "",
        UsageType usageType = UsageType::Production,
        Utf8StringCR correlationId = ""
        ){
        try { PostUserUsage(accessToken, version, projectId, authType, productId, deviceId, usageType, correlationId).get(); }
        catch(...) { return folly::makeFuture(BentleyStatus::ERROR); }
        return folly::makeFuture(BentleyStatus::SUCCESS);
        };

    

    // Takes in a product or products, and determines if ANY of the products passed has an entitlement, 
    // If so, starts RealTimeTrackUsage on that product.
    // Prioritizes Allowed over Trial over Evaluation entitlements
    // Returns TrackUsageStatus 
    //! @param[in] accessToken OIDC or SAML token of user to track usage against. OIDC is preferred
    //! @param[in] version version for this usage
    //! @param[in] projectId projectId of this usage
    //! @param[in] authType specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
    //! @param[in] productIds specify the productIds for this usage
    //! @param[in] deviceId specify the deviceId for this usage, required for multi-product id tracking
    //! @param[in] correlationId specify the correlationId for this usage, must be a GUID
    LICENSING_EXPORT folly::Future<TrackUsageStatus> EntitlementWorkflow
    (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType,
        std::vector<int> productIds,
        Utf8StringCR deviceId,
        Utf8StringCR correlationId
    );

    /**
     * Sends realtime user usage
     * @throw Http::HttpError if the request is rejected
     * @param accessToken OIDC or SAML token of user to track usage against. OIDC is preferred
     * @param version version for this usage
     * @param projectId projectId of this usage
     * @param authType optional - specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
     * @param productId optional - specify the productId for this usage
     * @param deviceId optional - specify the deviceId for this usage, required for multi-product id tracking
     * @param usageType optional - specify the usage type for this usage
     * @param correlationId optional - specify the correlationId for this usage, must be a GUID
     */
    LICENSING_EXPORT folly::Future<folly::Unit> PostUserUsage
        (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType = AuthType::OIDC,
        int productId = -1,
        Utf8StringCR deviceId = "",
        UsageType usageType = UsageType::Production,
        Utf8StringCR correlationId = "",
        Utf8StringCR principalId = ""
        );

    //! Mark realtime feature
    //! @deprecated Use SaasClient::PostFeatureUsage instead
    //! @param[in] accessToken OIDC or SAML token of user to mark feature against. OIDC is preferred
    //! @param[in] featureEvent The feature event to mark
    //! @param[in] authType optional - specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
    //! @param[in] productId optional - specify the productId for this usage
    //! @param[in] deviceId optional - specify the deviceId for this usage
    //! @param[in] usageType optional - specify the usage type for this usage
    //! @param[in] correlationId optional - specify the correlationId for this usage, must be a GUID
    LICENSING_EXPORT folly::Future<BentleyStatus> MarkFeature
        (
        Utf8StringCR accessToken,
        FeatureEvent featureEvent,
        AuthType authType = AuthType::OIDC,
        int productId = -1,
        Utf8StringCR deviceId = "",
        UsageType usageType = UsageType::Production,
        Utf8StringCR correlationId = ""
        ){
        try { PostFeatureUsage(accessToken, featureEvent, authType, productId, deviceId, usageType, correlationId).get(); }
        catch(...) { return folly::makeFuture(BentleyStatus::ERROR); }
        return folly::makeFuture(BentleyStatus::SUCCESS);
        };

    /**
     * Sends realtime feature usage
     * @throw Http::HttpError if the request is rejected
     * @param accessToken OIDC or SAML token of user to mark feature against. OIDC is preferred
     * @param featureEvent The feature event to mark
     * @param authType optional - specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
     * @param productId optional - specify the productId for this usage
     * @param deviceId optional - specify the deviceId for this usage
     * @param usageType optional - specify the usage type for this usage
     * @param correlationId optional - specify the correlationId for this usage, must be a GUID
     */
    LICENSING_EXPORT folly::Future<folly::Unit> PostFeatureUsage
        (
        Utf8StringCR accessToken,
        FeatureEvent featureEvent,
        AuthType authType = AuthType::OIDC,
        int productId = -1,
        Utf8StringCR deviceId = "",
        UsageType usageType = UsageType::Production,
        Utf8StringCR correlationId = "",
        Utf8StringCR principalId = ""
        );

    //! Check entitlement status for a user, important to cache result recommended for session or 4 hours.
    //! @param[in] accessToken OIDC or SAML token of user to check entitlement of. OIDC is preferred
    //! @param[in] version - the version of the product to check entitlements of
    //! @param[in] projectId projectId to consider when checking entitlements (overrides from constructor)
    //! @param[in] authType optional - specify whether the accessToken is OIDC or SAML. Defaults to OIDC which is preferred
    //! @param[in] productId optional - specify the productId to check the entitlement for (overrides from constructor if provided)
    //! @param[in] deviceId optional - specify the deviceId (overrides from constructor)
    //! @param[in] correlationId optional - specify the correlationId that will be used on downstream requests
    LICENSING_EXPORT folly::Future<EntitlementResult> CheckEntitlement
        (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType = AuthType::OIDC,
        int productId = -1,
        Utf8StringCR deviceId = "",
        Utf8StringCR correlationId = ""
        );
    };

END_BENTLEY_LICENSING_NAMESPACE
