/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "WebApi.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApiV2 : public WebApi
    {
    private:
        Utf8String GetUrl (Utf8StringCR path, Utf8StringCR queryString = "", Utf8StringCR webApiVersion = "v2.0") const;

        Utf8String CreateObjectSubPath (ObjectIdCR objectId) const;
        Utf8String CreateFileSubPath (ObjectIdCR objectId) const;
        Utf8String CreateClassSubPath (Utf8StringCR schemaName, Utf8StringCR className) const;
        Utf8String CreateNavigationSubPath (ObjectIdCR parentId) const;

        Utf8String CreateSelectPropertiesQuery (const bset<Utf8String>& properties) const;

        static Utf8String GetNullableString (RapidJsonValueCR jsonValue);

        static WSRepositoriesResult ResolveGetRepositoriesResponse (MobileDgn::Utils::HttpResponse& response);
        static WSCreateObjectResult ResolveCreateObjectResponse (MobileDgn::Utils::HttpResponse& response);
        static WSUpdateObjectResult ResolveUpdateObjectResponse (MobileDgn::Utils::HttpResponse& response);
        static WSObjectsResult ResolveObjectsResponse (MobileDgn::Utils::HttpResponse& response, const ObjectId* objectId = nullptr);
        static WSFileResult ResolveFileResponse (MobileDgn::Utils::HttpResponse& response, BeFileName filePath);

    public:
        WebApiV2 (std::shared_ptr<const ClientConfiguration> configuration);
        virtual ~WebApiV2 ();

        virtual MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
