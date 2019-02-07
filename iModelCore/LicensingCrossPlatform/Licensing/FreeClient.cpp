/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/FreeClient.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// TODO: check includes to make sure we need everything
#include <Licensing/FreeClient.h>
//#include "ClientImpl.h"
#include "FreeClientImpl.h"
#include "ClientWithKeyImpl.h"

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
    return std::shared_ptr<FreeClient>(new FreeClient(std::make_shared<FreeClientImpl>(featureString, httpHandler)));
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> FreeClient::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
{
    return m_impl->TrackUsage(accessToken, version, projectId);
}
