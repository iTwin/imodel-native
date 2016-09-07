/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ECInstanceKeyFilterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECInstanceKeyFilterTests.h"

#include <WebServices/Cache/Util/ECInstanceKeyFilter.h>

#include <Bentley/BeDebugLog.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST

TEST_F(ECInstanceKeyFilterTests, FilterByClass_OneClassEmptyInput_EmptyResults)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");

    ECInstanceKeyMultiMap instances;
    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter(*ecClass);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(0, result.size());
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_MultipleClassesEmptyInput_EmptyResults)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");
    auto ecClass2 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass2");

    ECInstanceKeyMultiMap instances;
    ECInstanceKeyMultiMap result;
    bset<ECClassCP> classes;
    classes.insert(ecClass);
    classes.insert(ecClass2);
    ECInstanceKeyFilter filter(classes);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(0, result.size());
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_MultipleInvalidClasses_SkipsInvalidClasses)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(2))});

    ECInstanceKeyMultiMap result;
    bset<ECClassCP> classes;
    classes.insert(nullptr);
    classes.insert(ecClass);
    ECInstanceKeyFilter filter(classes);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(2, result.size());
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(2))));
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_OneClass_FiltersByClass)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");
    auto ecClass2 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass2");

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(2))});
    instances.insert({ecClass2->GetId(), ECInstanceId(UINT64_C(1))});

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter(*ecClass);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(2, result.size());
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(2))));
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_MultipleClasses_FiltersByClasses)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");
    auto ecClass2 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass2");
    auto ecClass3 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass3");

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass2->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass2->GetId(), ECInstanceId(UINT64_C(2))});
    instances.insert({ecClass3->GetId(), ECInstanceId(UINT64_C(1))});

    ECInstanceKeyMultiMap result;
    bset<ECClassCP> classes;
    classes.insert(ecClass);
    classes.insert(ecClass2);
    ECInstanceKeyFilter filter(classes);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(3, result.size());
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass2->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass2->GetId(), ECInstanceId(UINT64_C(2))));
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_OneClassPolymorphicaly_FiltersByClassAndItsDerived)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");
    auto ecClass2 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass2");
    auto ecClassDerived = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestDerivedClass");
       

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass2->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClassDerived->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClassDerived->GetId(), ECInstanceId(UINT64_C(2))});

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter(*ecClass, true);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(3, result.size());
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClassDerived->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClassDerived->GetId(), ECInstanceId(UINT64_C(2))));
    }

TEST_F(ECInstanceKeyFilterTests, FilterByClass_MultipleClassesPolymorphicaly_FiltersByClassesAndItsDerived)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");
    auto ecClass2 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass2");
    auto ecClass3 = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass3");
    auto ecClassDerived = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestDerivedClass");

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(2))});
    instances.insert({ecClass2->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass3->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClassDerived->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClassDerived->GetId(), ECInstanceId(UINT64_C(2))});

    ECInstanceKeyMultiMap result;
    bset<ECClassCP> classes;
    classes.insert(ecClass);
    classes.insert(ecClass2);
    ECInstanceKeyFilter filter(classes, true);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(5, result.size());
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass->GetId(), ECInstanceId(UINT64_C(2))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClass2->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClassDerived->GetId(), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(result, ECInstanceKeyMultiMapPair(ecClassDerived->GetId(), ECInstanceId(UINT64_C(2))));
    }

TEST_F(ECInstanceKeyFilterTests, AddLabelFilter_EmptyLabelPassed_FindsNoInstances)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    StubInstances instances;
    instances.Add({"TestSchema.TestLabeledClass", "1"}, {{"Name", "Label1"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "1"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter;
    filter.AddLabelFilter("");
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(0, result.size());
    }

TEST_F(ECInstanceKeyFilterTests, AddLabelFilter_MultipleClasses_FiltersByLabel)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    StubInstances instances;
    instances.Add({"TestSchema.TestLabeledClass", "1"}, {{"Name", "Label1"}});
    instances.Add({"TestSchema.TestLabeledClass", "2"}, {{"Name", "Label2"}});
    instances.Add({"TestSchema.TestFileClass4", "1"}, {{"Name", "Label1"}});
    instances.Add({"TestSchema.TestFileClass4", "2"}, {{"Name", "Label2"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    
    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "1"}));
    auto instanceKeyPair2 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "2"}));
    auto instanceKeyPair3 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestFileClass4", "1"}));
    auto instanceKeyPair4 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestFileClass4", "2"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);
    instancesToFilter.insert(instanceKeyPair2);
    instancesToFilter.insert(instanceKeyPair3);
    instancesToFilter.insert(instanceKeyPair4);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter;
    filter.AddLabelFilter("Label1");
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(2, result.size());
    EXPECT_CONTAINS(result, instanceKeyPair1);
    EXPECT_CONTAINS(result, instanceKeyPair3);
    }

TEST_F(ECInstanceKeyFilterTests, AddLabelFilterAndByClass_MultipleClasses_FiltersByLabelAndClass)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestFileClass4");

    StubInstances instances;
    instances.Add({"TestSchema.TestLabeledClass", "1"}, {{"Name", "Label1"}});
    instances.Add({"TestSchema.TestLabeledClass", "2"}, {{"Name", "Label2"}});
    instances.Add({"TestSchema.TestFileClass4", "1"}, {{"Name", "Label1"}});
    instances.Add({"TestSchema.TestFileClass4", "2"}, {{"Name", "Label2"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "1"}));
    auto instanceKeyPair2 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "2"}));
    auto instanceKeyPair3 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestFileClass4", "1"}));
    auto instanceKeyPair4 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestFileClass4", "2"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);
    instancesToFilter.insert(instanceKeyPair2);
    instancesToFilter.insert(instanceKeyPair3);
    instancesToFilter.insert(instanceKeyPair4);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter(*ecClass);
    filter.AddLabelFilter("Label1");
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(1, result.size());
    EXPECT_CONTAINS(result, instanceKeyPair3);
    }

TEST_F(ECInstanceKeyFilterTests, AddAnyPropertiesLikeFilter_SingleClas_FiltersByLabel)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "1"}, {{"TestProperty", "BBBABBB"}, {"TestProperty2", "B"}});
    instances.Add({"TestSchema.TestClass", "2"}, {{"TestProperty", "B"}, {"TestProperty2", "BBBBABBBB"}});
    instances.Add({"TestSchema.TestClass", "3"}, {{"TestProperty", "B"}, {"TestProperty2", "B"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestClass", "1"}));
    auto instanceKeyPair2 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestClass", "2"}));
    auto instanceKeyPair3 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestClass", "3"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);
    instancesToFilter.insert(instanceKeyPair2);
    instancesToFilter.insert(instanceKeyPair3);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter;
    bset<Utf8String> propertiesToSearch;
    propertiesToSearch.insert("TestProperty");
    propertiesToSearch.insert("TestProperty2");
    filter.AddAnyPropertiesLikeFilter(propertiesToSearch, "A");
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(2, result.size());
    EXPECT_CONTAINS(result, instanceKeyPair1);
    EXPECT_CONTAINS(result, instanceKeyPair2);
    }

TEST_F(ECInstanceKeyFilterTests, AddAnyPropertiesLikeFilter_MultipleClasses_FiltersByLabel)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "1"}, {{"TestProperty", "BBABB"}, {"TestProperty2", "B"}});
    instances.Add({"TestSchema.TestFileClass3", "1"}, {{"TestName", "BBABB"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestClass", "1"}));
    auto instanceKeyPair2 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestFileClass3", "1"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);
    instancesToFilter.insert(instanceKeyPair2);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter;
    filter.AddAnyPropertiesLikeFilter([] (ECClassCR ecClass)
        {
        bset<Utf8String> propertiesToSearch;
        if (ecClass.GetName().Equals("TestClass"))
            {
            propertiesToSearch.insert("TestProperty");
            propertiesToSearch.insert("TestProperty2");
            }
        else
            {
            propertiesToSearch.insert("TestName");
            }

        return propertiesToSearch;
        }, "A");
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(2, result.size());
    EXPECT_CONTAINS(result, instanceKeyPair1);
    EXPECT_CONTAINS(result, instanceKeyPair2);
    }

TEST_F(ECInstanceKeyFilterTests, SetLimit_OnClassFilter_ReturnsLimitedInstances)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema", "TestClass");

    ECInstanceKeyMultiMap instances;
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    instances.insert({ecClass->GetId(), ECInstanceId(UINT64_C(2))});

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter(*ecClass);
    filter.SetLimit(1);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instances, result));

    txn.Commit();

    ASSERT_EQ(1, result.size());
    }

TEST_F(ECInstanceKeyFilterTests, SetLimit_OnLabelFilter_ReturnsLimitedInstances)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();

    StubInstances instances;
    instances.Add({"TestSchema.TestLabeledClass", "1"}, {{"Name", "Label1"}});
    instances.Add({"TestSchema.TestLabeledClass", "2"}, {{"Name", "Label2"}});

    CachedResponseKey key(txn.GetCache().FindOrCreateRoot("ResponceRoot"), "KeyName");
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instanceKeyPair1 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "1"}));
    auto instanceKeyPair2 = ECDbHelper::ToPair(txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "2"}));

    ECInstanceKeyMultiMap instancesToFilter;
    instancesToFilter.insert(instanceKeyPair1);
    instancesToFilter.insert(instanceKeyPair2);

    ECInstanceKeyMultiMap result;
    ECInstanceKeyFilter filter;
    filter.AddLabelFilter("Label1");
    filter.SetLimit(1);
    ASSERT_EQ(SUCCESS, filter.Filter(txn, instancesToFilter, result));

    txn.Commit();

    ASSERT_EQ(1, result.size());
    }

#endif