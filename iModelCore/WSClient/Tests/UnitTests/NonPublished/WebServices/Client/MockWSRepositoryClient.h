/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include "MockWSClient.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockWSRepositoryClient : public IWSRepositoryClient
    {
    private:
        std::shared_ptr<MockWSClient> m_client;
        Utf8String m_id;

    public:
        MockWSRepositoryClient()
            {
            WSRepository repository;
            repository.SetId("testId");
            repository.SetServerUrl("testUrl");
            ON_CALL(*this, GetInfo(_)).WillByDefault(
                Return(CreateCompletedAsyncTask(WSRepositoryResult::Success(repository))));
            }

        static std::shared_ptr<NiceMock<MockWSRepositoryClient>> Create(Utf8StringCR id = "test")
            {
            DefaultValue<AsyncTaskPtr<WSObjectsResult>>::Set(CreateCompletedAsyncTask(WSObjectsResult()));
            DefaultValue<AsyncTaskPtr<WSFileResult>>::Set(CreateCompletedAsyncTask(WSFileResult()));
            DefaultValue<AsyncTaskPtr<WSChangesetResult>>::Set(CreateCompletedAsyncTask(WSChangesetResult()));
            DefaultValue<AsyncTaskPtr<WSCreateObjectResult>>::Set(CreateCompletedAsyncTask(WSCreateObjectResult()));
            DefaultValue<AsyncTaskPtr<WSUpdateObjectResult>>::Set(CreateCompletedAsyncTask(WSUpdateObjectResult()));
            DefaultValue<AsyncTaskPtr<WSDeleteObjectResult>>::Set(CreateCompletedAsyncTask(WSDeleteObjectResult()));
            DefaultValue<AsyncTaskPtr<WSUpdateFileResult>>::Set(CreateCompletedAsyncTask(WSUpdateFileResult()));

            auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
            client->m_client = MockWSClient::Create();
            client->m_id = id;
            return client;
            };

        Utf8StringCR GetRepositoryId() const override
            {
            return m_id;
            };

        IWSClientPtr GetWSClient() const override
            {
            return m_client;
            }

        MockWSClient& GetMockWSClient()
            {
            return *m_client;
            }

        MOCK_METHOD2(SetCredentials, void(Credentials, IWSRepositoryClient::AuthenticationType));

        MOCK_METHOD1(RegisterRepositoryInfoListener, void(std::weak_ptr<IRepositoryInfoListener>));
        MOCK_METHOD1(UnregisterRepositoryInfoListener, void(std::weak_ptr<IRepositoryInfoListener>));

        MOCK_CONST_METHOD1(GetInfo, AsyncTaskPtr<WSRepositoryResult>(ICancellationTokenPtr));

        MOCK_CONST_METHOD2(GetInfoWithOptions, AsyncTaskPtr<WSRepositoryResult>
            (
            RequestOptionsPtr options,
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD1(VerifyAccess, AsyncTaskPtr<WSVoidResult>
            (
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD2(VerifyAccessWithOptions, AsyncTaskPtr<WSVoidResult>
            (
            RequestOptionsPtr options,
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD3(SendGetObjectRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag,
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD4(SendGetObjectRequestWithOptions, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag,
            RequestOptionsPtr options,
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD3(SendGetChildrenRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendGetChildrenRequestWithOptions, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag,
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendGetChildrenRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendGetChildrenRequestWithOptions, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag,
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendGetFileRequest, AsyncTaskPtr<WSFileResult>
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            Http::Request::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendGetFileRequest, AsyncTaskPtr<WSResult>
            (
            ObjectIdCR objectId,
            HttpBodyPtr responseBody,
            Utf8StringCR eTag,
            Http::Request::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD6(SendGetFileRequestWithOptions, AsyncTaskPtr<WSResult>
            (
            ObjectIdCR objectId,
            HttpBodyPtr responseBody,
            Utf8StringCR eTag,
            Http::Request::ProgressCallbackCR downloadProgressCallback,
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD2(SendGetSchemasRequest, AsyncTaskPtr<WSObjectsResult>
            (
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD3(SendGetSchemasRequestWithOptions, AsyncTaskPtr<WSObjectsResult>
            (
            Utf8StringCR eTag,
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendQueryRequest, AsyncTaskPtr<WSObjectsResult>
            (
            WSQueryCR query,
            Utf8StringCR eTag,
            Utf8StringCR skipToken,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendQueryRequestWithOptions, AsyncTaskPtr<WSObjectsResult>
            (
            WSQueryCR query,
            Utf8StringCR eTag,
            Utf8StringCR skipToken,
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD3(SendChangesetRequest, AsyncTaskPtr<WSChangesetResult>
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR,
            ICancellationTokenPtr
            ));

        MOCK_CONST_METHOD4(SendChangesetRequestWithOptions, AsyncTaskPtr<WSChangesetResult>
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR,
            IWSRepositoryClient::RequestOptionsPtr,
            ICancellationTokenPtr
            ));

        MOCK_CONST_METHOD4(SendCreateObjectRequest, AsyncTaskPtr<WSCreateObjectResult>
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendCreateObjectRequestWithOptions, AsyncTaskPtr<WSCreateObjectResult>
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            IWSRepositoryClient::RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendCreateObjectRequest, AsyncTaskPtr<WSCreateObjectResult>
            (
            ObjectIdCR objectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD6(SendCreateObjectRequestWithOptions, AsyncTaskPtr<WSCreateObjectResult>
            (
            ObjectIdCR objectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            IWSRepositoryClient::RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD6(SendUpdateObjectRequest, AsyncTaskPtr<WSUpdateObjectResult>
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD7(SendUpdateObjectRequestWithOptions, AsyncTaskPtr<WSUpdateObjectResult>
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            IWSRepositoryClient::RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD2(SendDeleteObjectRequest, AsyncTaskPtr<WSDeleteObjectResult>
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD3(SendDeleteObjectRequestWithOptions, AsyncTaskPtr<WSDeleteObjectResult>
            (
            ObjectIdCR objectId,
            IWSRepositoryClient::RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendUpdateFileRequest, AsyncTaskPtr<WSUpdateFileResult>
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendUpdateFileRequestWithOptions, AsyncTaskPtr<WSUpdateFileResult>
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback,
            IWSRepositoryClient::RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
