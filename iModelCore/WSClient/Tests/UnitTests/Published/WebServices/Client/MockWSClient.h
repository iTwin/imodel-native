/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/MockWSClient.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <WebServices/Client/WSClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockWSClient : public IWSClient
    {
    public:
        static std::shared_ptr<NiceMock<MockWSClient>> Create()
            {
            // Set the default return values for tasks so they would crash with stack trace in unit tests
            DefaultValue<AsyncTaskPtr<WSInfoResult>>::Set(AsyncTaskPtr<WSInfoResult>());
            DefaultValue<AsyncTaskPtr<WSRepositoriesResult>>::Set(AsyncTaskPtr<WSRepositoriesResult>());

            auto client = std::make_shared<NiceMock<MockWSClient>>();

            ON_CALL(*client, RegisterServerInfoListener(_)).WillByDefault(Return());
            ON_CALL(*client, UnregisterServerInfoListener(_)).WillByDefault(Return());
            ON_CALL(*client, GetServerUrl()).WillByDefault(Return("MockServerUrl"));

            ON_CALL(*client, GetServerUrl()).WillByDefault(Return("TestServerUrl"));

            ON_CALL(*client, GetServerInfo(_)).WillByDefault(
                Return(CreateCompletedAsyncTask(WSInfoResult::Success(WSInfo()))));

            return client;
            };

        MOCK_CONST_METHOD0(GetServerUrl, Utf8String());

        MOCK_CONST_METHOD0(VerifyConnection, AsyncTaskPtr<WSVoidResult>());

        MOCK_METHOD1(RegisterServerInfoListener, void(std::weak_ptr<IServerInfoListener> listener));

        MOCK_METHOD1(UnregisterServerInfoListener, void(std::weak_ptr<IServerInfoListener> listener));

        MOCK_METHOD1(EnableWsgServerHeader, void(bool enable));

        MOCK_CONST_METHOD1(GetServerInfo, AsyncTaskPtr<WSInfoResult>
            (
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD1(SendGetInfoRequest, AsyncTaskPtr<WSInfoResult>
            (
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD1(SendGetRepositoriesRequest, AsyncTaskPtr<WSRepositoriesResult>
            (
            ICancellationTokenPtr ct
            ));

        MOCK_CONST_METHOD3(SendGetRepositoriesRequest, AsyncTaskPtr<WSRepositoriesResult>
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr ct
            ));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
