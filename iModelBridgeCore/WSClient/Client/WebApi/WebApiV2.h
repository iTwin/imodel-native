/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "WebApi.h"
#include "WebApiV2Utils/JobApi.h"
#include "WebApiV2Utils/ActivityLogger.h"
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
        uint64_t GetMaxUploadSize(Http::Response& response, ActivityLoggerR activityLogger, uint64_t defaultMaxUploadSize = 0) const;
        BeVersion GetMaxWebApiVersion() const;
        Utf8String GetVersionedUrl() const;
        Utf8String GetRepositoryUrl(Utf8StringCR repositoryId) const;
        Utf8String GetUrl(Utf8StringCR path, Utf8StringCR queryString = "") const;
        Utf8String GetUrlWithoutLengthWarning(Utf8StringCR path, Utf8StringCR queryString = "") const;

        Utf8String CreateObjectSubPath(ObjectIdCR objectId) const;
        Utf8String CreateFileSubPath(ObjectIdCR objectId) const;
        Utf8String CreateClassSubPath(Utf8StringCR schemaName, Utf8StringCR className) const;
        Utf8String CreatePostQueryPath(Utf8StringCR classSubPath) const;
        Utf8String CreateNavigationSubPath(ObjectIdCR parentId) const;

        ActivityLogger CreateActivityLogger(Utf8StringCR activityName, IWSRepositoryClient::RequestOptionsPtr options = nullptr) const;
        void SetActivityIdToRequest(ActivityLoggerR activityLogger, Http::RequestR request) const;
        void SetActivityIdToRequest(ActivityLoggerR activityLogger, ChunkedUploadRequestR request) const;
        void CheckResponseActivityId(Http::Response& httpResponse, ActivityLoggerR activityLogger) const;

        Utf8String CreateSelectPropertiesQuery(const bset<Utf8String>& properties) const;
        Http::Request CreateGetRepositoryRequest() const;
        Http::Request CreateQueryRequest(WSQueryCR query) const;
        WSError CreateError(Http::Response& response, Utf8StringCR activityHeaderName = "") const;
        WSError CreateServerNotSupportedError(Http::Response& response, Utf8StringCR activityHeaderName = "") const;
        WSError CreateErrorFromAzzureError(AzureErrorCR azureError, Utf8StringCR activityId = "") const;

        std::shared_ptr<WSObjectsReader> CreateJsonInstancesReader() const;
        Utf8String GetNullableString(RapidJsonValueCR object, Utf8CP member) const;

        WSRepositoriesResult ResolveGetRepositoriesResponse(Http::Response& response, Utf8StringCR activityHeaderName) const;
        WSUpdateObjectResult ResolveUpdateObjectResponse(Http::Response& response, Utf8StringCR activityHeaderName) const;
        WSUploadResponse ResolveUploadResponse(Http::Response& response) const;
        WSObjectsResult ResolveObjectsResponse(Http::Response& response, Utf8StringCR activityHeaderName) const;

        BeVersion GetRepositoryPluginVersion(Http::Response& response, Utf8StringCR pluginId) const;

        Http::Request CreateFileDownloadRequest
            (
            Utf8StringCR url,
            HttpBodyPtr responseBody,
            Utf8StringCR eTag,
            ActivityLoggerR activityLogger,
            Http::Request::ProgressCallbackCR onProgress,
            ICancellationTokenPtr ct
            ) const;
        WSResult ResolveFileDownloadResponse(Http::Response& response, Utf8StringCR activityHeaderName) const;

        AsyncTaskPtr<WSUpdateFileResult> ResolveUpdateFileResponse
            (
            Http::Response& httpResponse,
            Utf8StringCR url,
            BeFileNameCR filePath,
            ActivityLoggerR activityLogger,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ) const;

    public:
        WebApiV2(std::shared_ptr<const ClientConfiguration> configuration, WSInfo info);
        virtual ~WebApiV2();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSRepositoryResult> SendGetRepositoryInfoRequest
            (
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        // TODO: SendGetRepositoriesRequest should have RequestOptions to specify activity id related options
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
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            HttpBodyPtr bodyResponseOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
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
