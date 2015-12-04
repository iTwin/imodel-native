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
        static const BeVersion s_maxTestedWebApi;

    private:
        WSInfo m_info;

    private:
        Utf8String GetWebApiUrl() const;
        Utf8String GetRepositoryUrl(Utf8StringCR repositoryId) const;
        Utf8String GetUrl(Utf8StringCR path, Utf8StringCR queryString = "") const;

        Utf8String CreateObjectSubPath(ObjectIdCR objectId) const;
        Utf8String CreateFileSubPath(ObjectIdCR objectId) const;
        Utf8String CreateClassSubPath(Utf8StringCR schemaName, Utf8StringCR className) const;
        Utf8String CreateNavigationSubPath(ObjectIdCR parentId) const;

        Utf8String CreateSelectPropertiesQuery(const bset<Utf8String>& properties) const;

        std::shared_ptr<WSObjectsReader> CreateJsonInstancesReader() const;
        static Utf8String GetNullableString(RapidJsonValueCR jsonValue);

        WSRepositoriesResult ResolveGetRepositoriesResponse(HttpResponse& response) const;
        WSCreateObjectResult ResolveCreateObjectResponse(HttpResponse& response) const;
        WSUpdateObjectResult ResolveUpdateObjectResponse(HttpResponse& response) const;
        WSObjectsResult ResolveObjectsResponse(HttpResponse& response, const ObjectId* objectId = nullptr) const;
        WSFileResult ResolveFileResponse(HttpResponse& response, BeFileName filePath) const;

    public:
        WebApiV2(std::shared_ptr<const ClientConfiguration> configuration, WSInfo info);
        virtual ~WebApiV2();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr ct
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
