/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "WebApi.h"
#include "WebApiV2Utils/JobApi.h"
#include <WebServices/Azure/AzureBlobStorageClient.h>

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
        IAzureBlobStorageClientPtr m_azureClient;
        JobApiPtr m_jobApi;

    private:
        BeVersion GetMaxWebApiVersion() const;
        Utf8String GetWebApiUrl(BeVersion webApiVersion = BeVersion()) const;
        Utf8String GetRepositoryUrl(Utf8StringCR repositoryId, BeVersion webApiVersion = BeVersion()) const;
        Utf8String GetUrl(Utf8StringCR path, Utf8StringCR queryString = "", BeVersion webApiVersion = BeVersion()) const;
        Utf8String GetUrlWithoutLengthWarning(Utf8StringCR path, Utf8StringCR queryString = "", BeVersion webApiVersion = BeVersion()) const;

        Utf8String CreateObjectSubPath(ObjectIdCR objectId) const;
        Utf8String CreateFileSubPath(ObjectIdCR objectId) const;
        Utf8String CreateClassSubPath(Utf8StringCR schemaName, Utf8StringCR className) const;
        Utf8String CreatePostQueryPath(Utf8StringCR classSubPath) const;
        Utf8String CreateNavigationSubPath(ObjectIdCR parentId) const;

        Utf8String CreateSelectPropertiesQuery(const bset<Utf8String>& properties) const;

        std::shared_ptr<WSObjectsReader> CreateJsonInstancesReader() const;
        static Utf8String GetNullableString(RapidJsonValueCR jsonValue);

        WSRepositoriesResult ResolveGetRepositoriesResponse(Http::Response& response) const;
        WSUpdateObjectResult ResolveUpdateObjectResponse(Http::Response& response) const;
        WSUploadResponse ResolveUploadResponse(Http::Response& response) const;
        WSObjectsResult ResolveObjectsResponse(Http::Response& response, bool requestHadSkipToken = false, const ObjectId* objectId = nullptr) const;

        Http::Request CreateFileDownloadRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            Http::Request::ProgressCallbackCR onProgress,
            ICancellationTokenPtr ct
            ) const;
        WSFileResult ResolveFileDownloadResponse(Http::Response& response, BeFileName filePath) const;

        AsyncTaskPtr<WSUpdateFileResult> WebApiV2::ResolveUpdateFileResponse
            (
            Http::Response& httpResponse,
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ) const;

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
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
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
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
