/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/MockWSRepositoryClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        typedef std::shared_ptr<PackagedAsyncTask<AsyncResult<void, WSError>>> PackagedAsyncTaskVoidWSError;

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

        MOCK_METHOD1(SetCredentials, void(Credentials credentials));

        MOCK_CONST_METHOD1(VerifyAccess, PackagedAsyncTaskVoidWSError
            (
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD3(SendGetObjectRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag,
            ICancellationTokenPtr canncelationToken
            ));

        MOCK_CONST_METHOD3(SendGetChildrenRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendGetChildrenRequest, AsyncTaskPtr<WSObjectsResult>
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendGetFileRequest, AsyncTaskPtr<WSFileResult>
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            HttpRequest::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD2(SendGetSchemasRequest, AsyncTaskPtr<WSObjectsResult>
            (
            Utf8StringCR eTag,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendQueryRequest, AsyncTaskPtr<WSObjectsResult>
            (
            WSQueryCR query,
            Utf8StringCR eTag,
            Utf8StringCR skipToken,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD3(SendChangesetRequest, AsyncTaskPtr<WSChangesetResult>
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR,
            ICancellationTokenPtr
            ));

        MOCK_CONST_METHOD4(SendCreateObjectRequest, AsyncTaskPtr<WSCreateObjectResult>
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD5(SendUpdateObjectRequest, AsyncTaskPtr<WSUpdateObjectResult>
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag,
            HttpRequest::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD2(SendDeleteObjectRequest, AsyncTaskPtr<WSDeleteObjectResult>
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD4(SendUpdateFileRequest, AsyncTaskPtr<WSUpdateFileResult>
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback,
            ICancellationTokenPtr ct
            ));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
