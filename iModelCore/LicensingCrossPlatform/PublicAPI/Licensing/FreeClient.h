/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/FreeClient.h $
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

#include <Licensing/Utils/FeatureUserDataMap.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct FreeClient> FreeClientPtr;

struct FreeClient
{
private:
    std::shared_ptr<struct IFreeClient> m_impl;

    FreeClient
    (
        std::shared_ptr<struct IFreeClient> implementation
    );

public:
    //! Client Creator
    /*!
    * Initializes an instance of Client, returns a ClientPtr to the prepared Client instance.
    *
    *
    */
    LICENSING_EXPORT static FreeClientPtr Create
    (
        Utf8StringCR featureString = "", /** FeatureString, defaults to an empty string */
        IHttpHandlerPtr customHttpHandler = nullptr /** CustomHttpHandler, defaults to a nullptr */
    );

    /*!
    * Send real time usage
    */
    LICENSING_EXPORT folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId);
};

END_BENTLEY_LICENSING_NAMESPACE
