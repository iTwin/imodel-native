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

typedef std::shared_ptr<struct WebApi> WebApiPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApi : public std::enable_shared_from_this<WebApi>
    {
    protected:
        const std::shared_ptr<const ClientConfiguration> m_configuration;

    public:
        WebApi (std::shared_ptr<const ClientConfiguration> configuration);
        virtual ~WebApi ();

        virtual MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
