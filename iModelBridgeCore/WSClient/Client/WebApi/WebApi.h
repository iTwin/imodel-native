/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSQuery.h>
#include <WebServices/Client/WSRepositoryClient.h>

#include "../ClientConfiguration.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef std::shared_ptr<struct WebApi> WebApiPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApi : public std::enable_shared_from_this<WebApi>
    {
    protected:
        const std::shared_ptr<const ClientConfiguration> m_configuration;

    public:
        WebApi(std::shared_ptr<const ClientConfiguration> configuration);
        virtual ~WebApi();

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr cancellationToken
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
