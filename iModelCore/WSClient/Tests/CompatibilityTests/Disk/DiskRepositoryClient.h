/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSRepositoryClient.h>
#include <ECObjects/ECSchema.h>
#include <BeHttp/HttpRequest.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY

#define FBC_NOT_IMPLEMENTED(T) \
    BeAssert(false && "Not implemented!"); \
    return CreateCompletedAsyncTask(T::Error({}));

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
struct DiskClient : public IWSClient
    {
    public:
        virtual Utf8String GetServerUrl() const override { return nullptr; }
        virtual void RegisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) override {};
        virtual void UnregisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) override {};

        virtual AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            return CreateCompletedAsyncTask(WSInfoResult::Success(WSInfo({2, 0}, {2, 0}, WSInfo::Type::BentleyWSG)));
            }

        virtual AsyncTaskPtr<WSInfoResult> SendGetInfoRequest
            (
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            return GetServerInfo(ct);
            }

        virtual AsyncTaskPtr<WSVoidResult> VerifyConnection() const override
            {
            return CreateCompletedAsyncTask(WSVoidResult::Success());
            }

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSRepositoriesResult);
            }

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSRepositoriesResult);
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
struct DiskRepositoryClient : public IWSRepositoryClient
    {
    private:
        IWSClientPtr m_client = std::make_shared<DiskClient>();
        BeFileName m_schemasDir;
        bmap<ECN::SchemaKey, BeFileName> m_schemaPaths;
        Utf8String m_id = "DiskRepository";

    public:
        DiskRepositoryClient(BeFileName schemasDir);

        virtual IWSClientPtr GetWSClient() const override
            {
            return m_client;
            }

        virtual Utf8StringCR GetRepositoryId() const override
            {
            return m_id;
            }

        virtual void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic) override
            {
            BeAssert(false);
            }

        virtual AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const override
            {
            FBC_NOT_IMPLEMENTED(WSVoidResult);
            }

        virtual AsyncTaskPtr<WSVoidResult> VerifyAccessWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const override
            {
            FBC_NOT_IMPLEMENTED(WSVoidResult);
            }

        virtual void RegisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) override {};

        virtual void UnregisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) override {};

        virtual AsyncTaskPtr<WSRepositoryResult> GetInfo(ICancellationTokenPtr ct) const override
            {
            WSRepository repo;
            repo.SetServerUrl("FakeUrl");
            repo.SetId("FakeId");
            return CreateCompletedAsyncTask(WSRepositoryResult::Success(repo));
            }

        virtual AsyncTaskPtr<WSRepositoryResult> GetInfoWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const override
            {
            WSRepository repo;
            repo.SetServerUrl("FakeUrl");
            repo.SetId("FakeId");
            return CreateCompletedAsyncTask(WSRepositoryResult::Success(repo));
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            HttpBodyPtr bodyResponseOut,
            Utf8StringCR eTag = nullptr,
            Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSResult);
            }

        virtual AsyncTaskPtr<WSResult> SendGetFileRequestWithOptions
            (
            ObjectIdCR objectId,
            Http::HttpBodyPtr responseBodyOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequestWithOptions
            (
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequestWithOptions
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSChangesetResult);
            }

        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequestWithOptions
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSChangesetResult);
            }

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSCreateObjectResult);
            }

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSCreateObjectResult);
            }

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSUpdateObjectResult);
            }

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSUpdateObjectResult);
            }

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSDeleteObjectResult);
            }

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSDeleteObjectResult);
            }

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSUpdateFileResult);
            }

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequestWithOptions
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSUpdateFileResult);
            }

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSCreateObjectResult);
            }

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSCreateObjectResult);
            }
    };
