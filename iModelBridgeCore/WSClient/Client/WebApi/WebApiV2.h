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
        WSUploadResponse ResolveUploadResponse(HttpResponse& response) const;
        WSObjectsResult ResolveObjectsResponse(HttpResponse& response, bool requestHadSkipToken = false, const ObjectId* objectId = nullptr) const;
        BeVersion GetRepositoryPluginVersion(HttpResponseCR response, Utf8StringCR pluginId) const;

        HttpRequest CreateFileDownloadRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            HttpRequest::ProgressCallbackCR onProgress,
            ICancellationTokenPtr ct
            ) const;
        WSFileResult ResolveFileDownloadResponse(HttpResponse& response, BeFileName filePath) const;

        AsyncTaskPtr<WSCreateObjectResult> ResolveUpdateFileResponse
            (
            HttpResponse& httpResponse,
            Utf8StringCR url,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ) const;

    public:
        WebApiV2(std::shared_ptr<const ClientConfiguration> configuration, WSInfo info);
        virtual ~WebApiV2();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSRepositoryResult> SendGetRepositoryRequest(ICancellationTokenPtr ct = nullptr) const override;

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
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
