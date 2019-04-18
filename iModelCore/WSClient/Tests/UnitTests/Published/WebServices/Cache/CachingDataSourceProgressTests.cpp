/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceProgressTests.h"

#include <WebServices/Cache/ICachingDataSource.h>
#include "CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, StateCtor_Default_ReturnsEmptyValues)
    {
    ICachingDataSource::Progress::State state;

    EXPECT_EQ(0, state.current);
    EXPECT_EQ(0, state.total);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, StateCtor_ValuesPassed_ReturnsValues)
    {
    double current = 20.5;
    double total = 50;
    auto state = ICachingDataSource::Progress::State(current, total);

    EXPECT_EQ(current, state.current);
    EXPECT_EQ(total, state.total);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_Default_ReturnsEmptyValues)
    {
    ICachingDataSource::Progress progress;

    EXPECT_EQ(0, progress.GetSynced());
    EXPECT_EQ("", progress.GetLabel());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesPassed_ReturnsBytesValuesOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    auto progress = ICachingDataSource::Progress(transfered);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(0, progress.GetSynced());
    EXPECT_EQ("", progress.GetLabel());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_BytesValuesAndLabelPassed_ReturnsBytesValuesAndLabelPtrOnly)
    {
    double current = 20.5;
    double total = 50;
    auto transfered = ICachingDataSource::Progress::State(current, total);
    Utf8String label = "FooBar";
    auto labelPtr = std::make_shared<Utf8String>(label);
    auto progress = ICachingDataSource::Progress(transfered, labelPtr);

    EXPECT_EQ(ICachingDataSource::Progress::State(current, total), progress.GetBytes());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(0, progress.GetSynced());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_InstancesValuesAndLabelAndSyncedPassed_ReturnsInstancesValuesAndLabelAndSyncedOnly)
    {
    auto instances = ICachingDataSource::Progress::State(20, 50);
    Utf8String label = "FooBar";
    auto labelPtr = std::make_shared<Utf8String>(label);
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(synced, instances, {}, labelPtr);

    EXPECT_EQ(instances, progress.GetInstances());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Vilius.Kazlauskas               04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_InstancesValuesAndSyncedPassed_ReturnsInstancesValuesAndSyncedOnly)
    {
    auto instances = ICachingDataSource::Progress::State(20, 50);
    double synced = 0.99;
    auto progress = ICachingDataSource::Progress(synced, instances);

    EXPECT_EQ(instances, progress.GetInstances());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetBytes());
    EXPECT_EQ("", progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
    EXPECT_EQ(ECInstanceKey(), progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, Ctor_BytesAndLabelAndSyncedAndClassIdAndInstanceIdAndFileBytesPassed_ReturnsAllExceptInstances)
    {
    auto state = ICachingDataSource::Progress::State(20, 50);
    double synced = 0.99;
    auto label = Utf8String("FooBar");
    auto labelPtr = std::make_shared<Utf8String>(label);
    ECInstanceKey ecKey = StubECInstanceKey(500, 3200);
    auto progress = ICachingDataSource::Progress(state, labelPtr, synced, ecKey, state);

    EXPECT_EQ(state, progress.GetBytes());
    EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetInstances());
    EXPECT_EQ(label, progress.GetLabel());
    EXPECT_EQ(synced, progress.GetSynced());
    EXPECT_EQ(state, progress.GetCurrentFileBytes());
    EXPECT_EQ(ecKey, progress.GetCurrentFileKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, EqualOperator_CompareEqualObjects_ReturnTrue)
    {
    auto state = ICachingDataSource::Progress::State(20, 50);
    auto labelPtr = std::make_shared<Utf8String>(Utf8String("FooBar"));
    auto progress = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(500, 3200), state);

    EXPECT_EQ(progress, progress);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, NotEqualOperator_CompareDifferentObjects_ReturnTrue)
    {
    auto state1 = ICachingDataSource::Progress::State(20, 50);
    auto state2 = ICachingDataSource::Progress::State(30, 100);
    auto labelPtr1 = std::make_shared<Utf8String>(Utf8String("FooBar"));
    auto labelPtr2 = std::make_shared<Utf8String>(Utf8String("BarFoo"));
    auto progress1 = ICachingDataSource::Progress(state1, labelPtr1, 0.99, StubECInstanceKey(500, 3200), state1);
    auto progress2 = ICachingDataSource::Progress(state2, labelPtr2, 0.75, StubECInstanceKey(100, 999), state2);

    EXPECT_NE(progress1, progress2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, NotEqualOperator_CompareSameObjectsExceptClassId_ReturnTrue)
    {
    auto state = ICachingDataSource::Progress::State(20, 50);
    auto labelPtr = std::make_shared<Utf8String>(Utf8String("FooBar"));
    auto progress1 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(500, 3200), state);
    auto progress2 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(123, 3200), state);
    EXPECT_NE(progress1, progress2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, NotEqualOperator_CompareSameObjectsExceptInstanceId_ReturnTrue)
    {
    auto state = ICachingDataSource::Progress::State(20, 50);
    auto labelPtr = std::make_shared<Utf8String>(Utf8String("FooBar"));
    auto progress1 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(500, 3200), state);
    auto progress2 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(500, 1269), state);
    EXPECT_NE(progress1, progress2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         Petras.Sukys               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceProgressTests, NotEqualOperator_CompareSameObjectsExceptCurrentFileBytes_ReturnTrue)
    {

    auto state = ICachingDataSource::Progress::State(20, 50);
    auto labelPtr = std::make_shared<Utf8String>(Utf8String("FooBar"));
    auto progress1 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(50, 100), {50, 350});
    auto progress2 = ICachingDataSource::Progress(state, labelPtr, 0.99, StubECInstanceKey(50, 100), {100, 200});
    EXPECT_NE(progress1, progress2);
    }
    