/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/FreeClient.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <Licensing/FreeClient.h>
#include "FreeClientImpl.h"
#include "Providers/BuddiProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FreeClient::FreeClient
    (
    std::shared_ptr<struct IFreeClient> implementation
    )
    {
    m_impl = implementation;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FreeClientPtr FreeClient::Create
    (
    Utf8StringCR featureString,
    IHttpHandlerPtr httpHandler
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    return std::shared_ptr<FreeClient>(new FreeClient(std::make_shared<FreeClientImpl>(featureString, httpHandler, buddiProvider)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> FreeClient::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
    {
    return m_impl->TrackUsage(accessToken, version, projectId);
    }
