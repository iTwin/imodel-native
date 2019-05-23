/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "SyncNotifierTests.h"

#include <WebServices/Cache/SyncNotifier.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsitest                                          Benediktas.Lipnickas        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SyncNotifierTests, OnComplete_NoTasksAdded_ReturnsNotSynced)
    {
    auto notifier = SyncNotifier::Create();

    auto result = notifier->OnComplete()->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                          Benediktas.Lipnickas        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SyncNotifierTests, OnComplete_AllTasksSynced_ReturnsSynced)
    {
    auto notifier = SyncNotifier::Create();
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::Synced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::Synced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::Synced)));

    auto result = notifier->OnComplete()->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                          Benediktas.Lipnickas        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SyncNotifierTests, OnComplete_OneOfTheTasksSynced_ReturnsSynced)
    {
    auto notifier = SyncNotifier::Create();
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotModified)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::Synced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotSynced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::SyncError)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Error(AsyncError())));

    auto result = notifier->OnComplete()->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                          Benediktas.Lipnickas        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SyncNotifierTests, OnComplete_AllTasksNotSynced_ReturnsNotSynced)
    {
    auto notifier = SyncNotifier::Create();
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotSynced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotSynced)));
    notifier->AddTask(CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotSynced)));

    auto result = notifier->OnComplete()->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue());
    }

