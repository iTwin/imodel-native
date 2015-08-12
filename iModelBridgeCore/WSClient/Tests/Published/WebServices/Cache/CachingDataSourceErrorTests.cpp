/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/CachingDataSourceErrorTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceErrorTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <Bentley/BeDebugLog.h>
#include "CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(CachingDataSourceErrorTests, Ctor_DefaultWSError_EmptyMessageAndDescription)
    {
    auto error = CachingDataSource::Error(WSError());

    EXPECT_EQ("", error.GetDescription());
    EXPECT_EQ("", error.GetMessage());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_SuccessStatus_EmptyMessageAndDescription)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::Success);

    EXPECT_EQ("", error.GetDescription());
    EXPECT_EQ("", error.GetMessage());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_InternalCacheError_NotEmptyMessageAndEmptyDescription)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::InternalCacheError);

    EXPECT_EQ("", error.GetDescription());
    ASSERT_NE("", error.GetMessage());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_DataNotCached_NotEmptyMessageAndEmptyDescription)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::DataNotCached);

    EXPECT_EQ("", error.GetDescription());
    ASSERT_NE("", error.GetMessage());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_StringMessage_StatusIsInternalCacheErrorAndMessageSet)
    {
    auto error = CachingDataSource::Error("TestMessage");

    ASSERT_EQ(ICachingDataSource::Status::InternalCacheError, error.GetStatus());
    ASSERT_EQ("TestMessage", error.GetMessage());
    EXPECT_EQ("", error.GetDescription());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_WSError_NetworkError)
    {
    auto error = CachingDataSource::Error(WSError::CreateServerNotSupportedError());

    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, error.GetStatus());
    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetWSError().GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetWSError().GetId());
    EXPECT_EQ(WSError::CreateServerNotSupportedError().GetMessage(), error.GetMessage());
    EXPECT_EQ(WSError::CreateServerNotSupportedError().GetDescription(), error.GetDescription());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_AsyncErrorPassed_StatusIsInternalCacheErrorAndMessageAndDescriptionSet)
    {
    auto error = CachingDataSource::Error(AsyncError("A", "B"));

    ASSERT_EQ(ICachingDataSource::Status::InternalCacheError, error.GetStatus());
    ASSERT_EQ("A", error.GetMessage());
    EXPECT_EQ("B", error.GetDescription());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_AsyncErrorAndStatusPassed_StatusAndMessageAndDescriptionSet)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::FunctionalityNotSupported, AsyncError("A", "B"));
    ASSERT_EQ(ICachingDataSource::Status::FunctionalityNotSupported, error.GetStatus());
    ASSERT_EQ("A", error.GetMessage());
    EXPECT_EQ("B", error.GetDescription());

    error = CachingDataSource::Error(ICachingDataSource::Status::ApplicationError, AsyncError("A", "B"));
    ASSERT_EQ(ICachingDataSource::Status::ApplicationError, error.GetStatus());
    ASSERT_EQ("A", error.GetMessage());
    EXPECT_EQ("B", error.GetDescription());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_NullCancellationTokenPassed_SetsStatus)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::InternalCacheError, nullptr);

    ASSERT_EQ(ICachingDataSource::Status::InternalCacheError, error.GetStatus());
    EXPECT_FALSE(error.GetMessage().empty());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_NotCanceledCancellationTokenPassed_SetsStatus)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::InternalCacheError, SimpleCancellationToken::Create(false));

    ASSERT_EQ(ICachingDataSource::Status::InternalCacheError, error.GetStatus());
    EXPECT_FALSE(error.GetMessage().empty());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_CanceledCancellationTokenPassed_SetsStatusCanceledAndNoMessage)
    {
    auto error = CachingDataSource::Error(ICachingDataSource::Status::InternalCacheError, SimpleCancellationToken::Create(true));

    ASSERT_EQ(ICachingDataSource::Status::Canceled, error.GetStatus());
    EXPECT_TRUE(error.GetMessage().empty());
    EXPECT_TRUE(error.GetDescription().empty());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_CacheStatusOKStatusPassed_SetsSuccessWithoutMessage)
    {
    auto error = CachingDataSource::Error(CacheStatus::OK);

    ASSERT_EQ(ICachingDataSource::Status::Success, error.GetStatus());
    EXPECT_TRUE(error.GetMessage().empty());
    EXPECT_TRUE(error.GetDescription().empty());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_CacheStatusErrorStatusPassed_SetsInternalCacheErrorWithMessage)
    {
    auto error = CachingDataSource::Error(CacheStatus::Error);

    ASSERT_EQ(ICachingDataSource::Status::InternalCacheError, error.GetStatus());
    EXPECT_FALSE(error.GetMessage().empty());
    EXPECT_TRUE(error.GetDescription().empty());
    }

TEST_F(CachingDataSourceErrorTests, Ctor_CacheStatusDataNotCachedStatusPassed_SetsStatusDataNotCachedWithMessage)
    {
    auto error = CachingDataSource::Error(CacheStatus::DataNotCached);

    ASSERT_EQ(ICachingDataSource::Status::DataNotCached, error.GetStatus());
    EXPECT_FALSE(error.GetMessage().empty());
    EXPECT_TRUE(error.GetDescription().empty());
    }
