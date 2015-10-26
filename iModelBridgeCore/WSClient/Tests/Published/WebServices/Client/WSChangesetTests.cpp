/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/WSChangesetTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSChangesetTests.h"

#include <WebServices/Client/WSChangeset.h>
#include <Bentley/BeTimeUtilities.h>

using namespace ::testing;

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(WSChangesetTests, ToString_EmptyChangeset_ReturnsEmptyChangesetAndSizeIsSameAsEmptyChangesetJson)
    {
    WSChangeset changeset;
    EXPECT_EQ(Utf8String(R"({"instances":[]})").size(), changeset.CalculateSize());
    EXPECT_EQ(Utf8String(R"({"instances":[]})"), changeset.ToString());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneExistingInstance_ReturnsChangesetJsonWithNotPropertiesAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "schemaName":"TestSchema",
            "className":"TestClass",
            "remoteId":"Foo"
            }
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneCreatedInstance_ReturnsChangesetJsonWithPropertiesAndWithoutIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Created, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"created",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":{"TestProperty":"TestValue"}
            }
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneModifiedInstance_ReturnsChangesetJsonWithPropertiesAndIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Modified, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "remoteId":"Foo",
            "properties":
                {
                "TestProperty" : "TestValue"
                }
            }
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneDeletedInstance_ReturnsChangesetJsonWithoutPropertiesAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Deleted, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "remoteId":"Foo"
            }
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneForwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA","IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB","IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","IgnoredId"}, WSChangeset::Created, propertiesC);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"created",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeStatus":"created",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeStatus":"created",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_OneBackwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA","IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB","IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Backward,
        {"TestSchemaC.TestClassC","IgnoredId"}, WSChangeset::Created, propertiesC);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"created",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeStatus":"created",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"backward",
                "relatedInstance":
                    {
                    "changeStatus":"created",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_TwoRelatedInstances_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));
    auto propertiesE = std::make_shared<Json::Value>(ToJson(R"({"Foo":"E"})"));

    WSChangeset changeset;
    auto& instance = changeset.AddInstance({"TestSchemaA.TestClassA","IgnoredId"}, WSChangeset::Created, propertiesA);
    instance.AddRelatedInstance({"TestSchemaB.TestClassB","IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC","IgnoredId"}, WSChangeset::Created, propertiesC);
    instance.AddRelatedInstance({"TestSchemaD.TestClassD","IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE","IgnoredId"}, WSChangeset::Created, propertiesE);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"created",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeStatus":"created",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeStatus":"created",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                },
                {
                "changeStatus":"created",
                "schemaName":"TestSchemaD",
                "className":"TestClassD",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeStatus":"created",
                    "schemaName":"TestSchemaE",
                    "className":"TestClassE",
                    "properties":{"Foo": "E"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(3, changeset.GetInstanceCount());
    EXPECT_EQ(2, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToString_ThreeInstances_ReturnsChangesetAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesB = std::make_shared<Json::Value>(ToJson(R"({"Foo":"B"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","A"}, WSChangeset::Modified, propertiesA);
    changeset.AddInstance({"TestSchema.TestClass","B"}, WSChangeset::Created, propertiesB);
    changeset.AddInstance({"TestSchema.TestClass","C"}, WSChangeset::Deleted, propertiesC);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "remoteId":"A",
            "properties":
                {
                "Foo" : "A"
                }
            },
            {
            "changeStatus":"created",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":
                {
                "Foo" : "B"
                }
            },
            {
            "changeStatus":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "remoteId":"C"
            }
        ]})");

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(3, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, RemoveInstance_OneInstanceInstance_ReturnsTrueAndLeavesChangesetEmpty)
    {
    WSChangeset changeset;
    auto& instance = changeset.AddInstance({"TestSchema.TestClassA","A"}, WSChangeset::Modified, nullptr);

    EXPECT_EQ(true, changeset.RemoveInstance(instance));

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(R"({"instances":[]})", changesetStr);
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, RemoveInstance_ThreeInstances_ReturnsTrueAndLeavesOtherTwoInstances)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchema.TestClassA","A"}, WSChangeset::Modified, nullptr);
    auto& instanceB = changeset.AddInstance({"TestSchema.TestClassB","B"}, WSChangeset::Created, nullptr);
    auto& instanceC = changeset.AddInstance({"TestSchema.TestClassC","C"}, WSChangeset::Deleted, nullptr);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"modified",
            "schemaName":"TestSchema",
            "className":"TestClassA",
            "remoteId":"A"
            },
            {
            "changeStatus":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClassC",
            "remoteId":"C"
            }
        ]})");

    EXPECT_EQ(true, changeset.RemoveInstance(instanceB));

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());

    // Check if other instances are alive
    EXPECT_EQ(ObjectId("TestSchema.TestClassA", "A"), instanceA.GetId());
    EXPECT_EQ(ObjectId("TestSchema.TestClassC", "C"), instanceC.GetId());
    }

TEST_F(WSChangesetTests, RemoveInstance_TwoRelatedInstances_ReturnsRemovedIdAndLeavesOtherInstances)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr);
    auto& instanceC = instanceA.AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC","C"}, WSChangeset::Created, nullptr);
    auto& instanceE = instanceA.AddRelatedInstance({"TestSchemaD.TestClassD","D"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE","E"}, WSChangeset::Created, nullptr);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeStatus":"created",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "relationshipInstances":[
                {
                "changeStatus":"created",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeStatus":"created",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC"
                    }
                }
            ]}
        ]})");

    EXPECT_EQ(true, changeset.RemoveInstance(instanceE));

    Utf8String changesetStr = changeset.ToString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());

    // Check if other instances are alive
    EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), instanceA.GetId());
    EXPECT_EQ(ObjectId("TestSchemaC.TestClassC", "C"), instanceC.GetId());
    }

TEST_F(WSChangesetTests, DISABLED_CalculateSize_LotsOfIntsances_PerformanceBetterThanDoingToString)
    {
    auto testProperties = std::make_shared<Json::Value>(ToJson(R"(
        {
        "Test1":"A - Nullam lobortis sollicitudin massa, ultricies euismod enim tempor vitae. Cum.", 
        "Test2":"B - Nulla id ultrices nibh. Morbi rhoncus leo in euismod finibus.", 
        "Test3":"C - Donec lacinia vitae nunc auctor sodales. Duis in massa consectetur.", 
        "Test4":"D - Quisque nec est semper, congue felis at, lacinia ante. Mauris.", 
        "Test5":"E - Ut maximus elit sit amet molestie faucibus. Duis pharetra, urna."
        })"));
    ObjectId testId("SomeTestSchema", "SomeTestClass", "TestId");
    int testInstanceCount = 2500;
    int testIterations = 3;

    auto testChangeset = [&] (std::function<void(WSChangeset&)> completeCallback, std::function<void(WSChangeset&)> incrementalCallback)
        {
        for (int i = 0; i < testIterations; i++)
            {
            WSChangeset changeset;
            while ((int)changeset.GetInstanceCount() < testInstanceCount)
                {
                auto& instance = changeset.AddInstance(testId, WSChangeset::Created, testProperties);

                if (incrementalCallback)
                    {
                    incrementalCallback(changeset);
                    }

                instance.AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties)
                    .AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties)
                    .AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties);

                if (incrementalCallback)
                    {
                    incrementalCallback(changeset);
                    }

                instance.AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties)
                    .AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties)
                    .AddRelatedInstance(testId, WSChangeset::Created, ECRelatedInstanceDirection::Forward, testId, WSChangeset::Created, testProperties);

                if (incrementalCallback)
                    {
                    incrementalCallback(changeset);
                    }
                }
            if (completeCallback)
                {
                completeCallback(changeset);
                }
            }
        };

    double start, end;

    // Measure empty
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset(nullptr, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("Create changeset for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure CalculateSize()
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset([&] (WSChangeset& changeset)
        {
        changeset.CalculateSize();
        }, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("CalculateSize() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure ToString().size()
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset([&] (WSChangeset& changeset)
        {
        changeset.ToString().size();
        }, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("ToString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure CalculateSize() incrementally
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset(nullptr, [&] (WSChangeset& changeset)
        {
        changeset.CalculateSize();
        });
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("Incremental CalculateSize() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure ToString().size() incrementally
    //start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //testChangeset(nullptr, [&] (WSChangeset& changeset)
    //    {
    //    changeset.ToString().size();
    //    });
    //end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //printf(Utf8PrintfString("Incremental ToString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));
    }
