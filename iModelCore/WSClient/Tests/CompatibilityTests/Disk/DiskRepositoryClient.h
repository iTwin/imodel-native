/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Disk/DiskRepositoryClient.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSRepositoryClient.h>
#include <ECObjects/ECSchema.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

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
            };

        virtual Utf8StringCR GetRepositoryId() const override
            {
            return m_id;
            }

        virtual void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic) override
            {
            BeAssert(false);
            };

        virtual AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const override
            {
            FBC_NOT_IMPLEMENTED(WSVoidResult);
            };

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
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
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSObjectsResult);
            }

        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSChangesetResult);
            }

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override
            {
            FBC_NOT_IMPLEMENTED(WSCreateObjectResult);
            }
    };