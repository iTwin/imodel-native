/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/SaasClient.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__


// TODO: check includes to make sure we need everything
#include <Licensing/Licensing.h>

#include "LicenseStatus.h"

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include <Licensing/Utils/FeatureEvent.h>
//#include <Licensing/Utils/FeatureUserDataMap.h>

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
    //! @param[in] accessToken OIDC token of user to track usage against
    //! @param[in] version version for this usage
    //! @param[in] projectId projectId of this usage
    LICENSING_EXPORT folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId);

    //! Mark realtime feature
    //! @param[in] accessToken OIDC token of user to mark feature against
    //! @param[in] featureEvent The feature event to mark
    LICENSING_EXPORT folly::Future<BentleyStatus> MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent);
    };

END_BENTLEY_LICENSING_NAMESPACE
