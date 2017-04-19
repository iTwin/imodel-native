/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/CachingDataSourceProgressTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceProgressTests.h"

#include <WebServices/Cache/ICachingDataSource.h>
#include "CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(CachingDataSourceProgressTests, StateCtor_Default_ReturnsEmptyValues)
    {
    ICachingDataSource::Progress::State state;

    EXPECT_EQ(0, state.current);
    EXPECT_EQ(0, state.total);
    }

TEST_F(CachingDataSourceProgressTests, StateCtor_ValuesPassed_ReturnsValues)
    {
    double current = 20.5;
    double total = 50;
    auto state = ICachingDataSource::Progress::State(current, total);

    EXPECT_EQ(current, state.current);
    EXPECT_EQ(total, state.total);
    }

TEST_F(CachingDataSourceProgressTests, Ctor_Default_ReturnsEmptyValues)
    {
    ICachingDataSource::Progress progress;

    EXPECT_EQ(0, progress.GetSynced());
    EXPECT_EQ("", progress.GetLabel());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesPassed_ReturnsBytesValuesOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    auto progress = ICachingDataSource::Progress(transfered);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(0, progress.GetSynced());
    EXPECT_EQ("", progress.GetLabel());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesAndLabelPassed_ReturnsBytesValuesAndLabelOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    auto progress = ICachingDataSource::Progress(transfered, std::make_shared<Utf8String>(label));

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(0, progress.GetSynced());    
    }

TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesAndLabelAndSyncedPassed_ReturnsBytesValuesAndLabelAndSyncedOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(transfered, std::make_shared<Utf8String>(label), synced);
        
    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());    
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesAndLabelPassed_ReturnsBytesValuesAndLabelPtrOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    auto labelPtr = std::make_shared<Utf8String>(label);
    auto progress = ICachingDataSource::Progress(transfered, labelPtr);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(labelPtr, progress.GetLabelPtr());
    EXPECT_EQ(0, progress.GetSynced());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_InstancesValuesAndBytesValuesAndLabelAndSyncedPassed_ReturnsInstancesValuesAndBytesValuesAndLabelAndSyncedOnly)
    {
    double current = 20;
    double total = 50;
    auto instances = ICachingDataSource::Progress::State(current, total);
    auto transfered = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    auto labelPtr = std::make_shared<Utf8String>(label);
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(synced, instances, transfered, labelPtr);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetInstances());
    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_InstancesValuesAndLabelAndSyncedPassed_ReturnsInstancesValuesAndLabelAndSyncedOnly)
    {
    double current = 20;
    double total = 50;
    auto instances = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    auto labelPtr = std::make_shared<Utf8String>(label);
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(synced, instances, {}, labelPtr);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetInstances());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    }

TEST_F(CachingDataSourceProgressTests, Ctor_InstancesValuesAndSyncedPassed_ReturnsInstancesValuesAndSyncedOnly)
    {
    double current = 20;
    double total = 50;
    auto instances = ICachingDataSource::Progress::State(current, total);
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(synced, instances);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetInstances());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    EXPECT_EQ("", progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    }