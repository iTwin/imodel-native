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

TEST_F(WSChangesetTests, ToRequestString_EmptyChangeset_ReturnsEmptyChangesetAndSizeIsSameAsEmptyChangesetJson)
    {
    WSChangeset changeset;
    EXPECT_EQ(Utf8String(R"({"instances":[]})").size(), changeset.CalculateSize());
    EXPECT_EQ(Utf8String(R"({"instances":[]})"), changeset.ToRequestString());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetIsEmpty_ReturnsEmptyChangesetAndSizeIsSameAsEmptyChangesetJson)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    EXPECT_EQ(Utf8String(R"({"instancs":{}})").size(), changeset.CalculateSize());
    EXPECT_EQ(Utf8String(R"({"instance":{}})"), changeset.ToRequestString());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAddingMoreThanOneInstance_DoesNotAddAdditionalInstance)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);

    changeset.AddInstance({"TestSchema.TestClass","A"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    BeTest::SetFailOnAssert(false);
    changeset.AddInstance({"TestSchema.TestClass","B"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));
    BeTest::SetFailOnAssert(true);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"A"
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneExistingInstance_ReturnsChangesetJsonWithNotPropertiesAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo"
            }
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneExistingInstance_ReturnsChangesetJsonWithNotPropertiesAndCalculateSizeMatches)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo"
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneCreatedInstance_ReturnsChangesetJsonWithPropertiesAndWithoutIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Created, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState":"new",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":{"TestProperty":"TestValue"}
            }
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneCreatedInstance_ReturnsChangesetJsonWithPropertiesAndWithoutIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Created, properties);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"new",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":{"TestProperty":"TestValue"}
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneModifiedInstance_ReturnsChangesetJsonWithPropertiesAndIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Modified, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo",
            "properties":
                {
                "TestProperty" : "TestValue"
                }
            }
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneModifiedInstance_ReturnsChangesetJsonWithPropertiesAndIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Modified, properties);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo",
            "properties":
                {
                "TestProperty" : "TestValue"
                }
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneDeletedInstance_ReturnsChangesetJsonWithoutPropertiesAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Deleted, properties);

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo"
            }
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneDeletedInstance_ReturnsChangesetJsonWithoutPropertiesAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Deleted, properties);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo"
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneForwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
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
            "changeState":"new",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeState":"new",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneForwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset
        .AddInstance({"TestSchemaA.TestClassA","IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB","IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","IgnoredId"}, WSChangeset::Created, propertiesC);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"new",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeState":"new",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                }]
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_OneBackwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
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
            "changeState":"new",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeState":"new",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"backward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_TwoRelatedInstances_ReturnsChangesetJsonAndCalculateSizeMatches)
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
            "changeState":"new",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "properties":{"Foo": "A"},
            "relationshipInstances":[
                {
                "changeState":"new",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC",
                    "properties":{"Foo": "C"}
                    }
                },
                {
                "changeState":"new",
                "schemaName":"TestSchemaD",
                "className":"TestClassD",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaE",
                    "className":"TestClassE",
                    "properties":{"Foo": "E"}
                    }
                }
            ]}
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(3, changeset.GetInstanceCount());
    EXPECT_EQ(2, changeset.GetRelationshipCount());
    }

TEST_F(WSChangesetTests, ToRequestString_ThreeInstances_ReturnsChangesetAndCalculateSizeMatches)
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
            "changeState":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"A",
            "properties":
                {
                "Foo" : "A"
                }
            },
            {
            "changeState":"new",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":
                {
                "Foo" : "B"
                }
            },
            {
            "changeState":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"C"
            }
        ]})");

    Utf8String changesetStr = changeset.ToRequestString();
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

    Utf8String changesetStr = changeset.ToRequestString();
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
            "changeState":"modified",
            "schemaName":"TestSchema",
            "className":"TestClassA",
            "instanceId":"A"
            },
            {
            "changeState":"deleted",
            "schemaName":"TestSchema",
            "className":"TestClassC",
            "instanceId":"C"
            }
        ]})");

    EXPECT_EQ(true, changeset.RemoveInstance(instanceB));

    Utf8String changesetStr = changeset.ToRequestString();
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
            "changeState":"new",
            "schemaName":"TestSchemaA",
            "className":"TestClassA",
            "relationshipInstances":[
                {
                "changeState":"new",
                "schemaName":"TestSchemaB",
                "className":"TestClassB",
                "direction":"forward",
                "relatedInstance":
                    {
                    "changeState":"new",
                    "schemaName":"TestSchemaC",
                    "className":"TestClassC"
                    }
                }
            ]}
        ]})");

    EXPECT_EQ(true, changeset.RemoveInstance(instanceE));

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(2, changeset.GetInstanceCount());
    EXPECT_EQ(1, changeset.GetRelationshipCount());

    // Check if other instances are alive
    EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), instanceA.GetId());
    EXPECT_EQ(ObjectId("TestSchemaC.TestClassC", "C"), instanceC.GetId());
    }

TEST_F(WSChangesetTests, FindInstance_EmptyChangeset_ReturnsNullptr)
    {
    WSChangeset changeset;
    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchema.TestClass","Foo"}));
    }

TEST_F(WSChangesetTests, FindInstance_NonExistingInstance_ReturnsNullptr)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Created, nullptr);
    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchema.TestClass","Other"}));
    }

TEST_F(WSChangesetTests, FindInstance_RelationshipInstanceId_ReturnsNull)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","C"}, WSChangeset::Created, nullptr);

    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchemaB.TestClassB","B"}));
    }

TEST_F(WSChangesetTests, FindInstance_RootAndRelatedInstance_ReturnsInstance)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr);
    auto& instanceC = instanceA.AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC","C"}, WSChangeset::Created, nullptr);
    auto& instanceE = instanceC.AddRelatedInstance({"TestSchemaD.TestClassD","D"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE","E"}, WSChangeset::Created, nullptr);

    EXPECT_EQ(&instanceA, changeset.FindInstance({"TestSchemaA.TestClassA","A"}));
    EXPECT_EQ(&instanceC, changeset.FindInstance({"TestSchemaC.TestClassC","C"}));
    EXPECT_EQ(&instanceE, changeset.FindInstance({"TestSchemaE.TestClassE","E"}));
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_EmptyChangeset_DoesNotCallHandlerAndReturnsSuccess)
    {
    WSChangeset changeset;
    auto response = ToRapidJson(R"({"changedInstances":[]})");
    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(0, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_TwoCreatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr);
    changeset.AddInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" : 
            [{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA"
                }
            },{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdB",
                "className" : "NewClassB",
                "schemaName" : "NewSchemaB"
                }
            }]
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        switch (count)
            {
            case 0:
                EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaA.NewClassA", "NewIdA"), newId);
                break;
            case 1:
                EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaB.NewClassB", "NewIdB"), newId);
                break;
            }
        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(2, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_ModifiedDeletedInstances_DoesNotCallHandle)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Modified, nullptr);
    changeset.AddInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Deleted, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" : 
            [{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA"
                }
            },{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdB",
                "className" : "NewClassB",
                "schemaName" : "NewSchemaB"
                }
            }]
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(0, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_ExistingRelationshipAndModifiedInstances_DoesNotCallHandler)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Modified, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Existing, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","C"}, WSChangeset::Modified, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" : 
            [{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "relationshipInstances" : 
                    [{
                    "instanceId" : "NewIdB",
                    "className" : "NewClassB",
                    "schemaName" : "NewSchemaB",
                    "relatedInstance" : 
                        {
                        "instanceId" : "NewIdC",
                        "className" : "NewClassC",
                        "schemaName" : "NewSchemaC"
                        }
                    }]
                }
            }]
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(0, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_CreatedRelatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","C"}, WSChangeset::Created, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" : 
            [{
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "relationshipInstances" : 
                    [{
                    "instanceId" : "NewIdB",
                    "className" : "NewClassB",
                    "schemaName" : "NewSchemaB",
                    "relatedInstance" : 
                        {
                        "instanceId" : "NewIdC",
                        "className" : "NewClassC",
                        "schemaName" : "NewSchemaC"
                        }
                    }]
                }
            }]
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        switch (count)
            {
            case 0:
                EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaA.NewClassA", "NewIdA"), newId);
                break;
            case 1:
                EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaB.NewClassB", "NewIdB"), newId);
                break;
            case 2:
                EXPECT_EQ(ObjectId("TestSchemaC.TestClassC", "C"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaC.NewClassC", "NewIdC"), newId);
                break;
            }

        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(3, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_SingleInstanceChangesetCreatedRelatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset (WSChangeset::SingeInstance);
    changeset
        .AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","C"}, WSChangeset::Created, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstance" : 
            {
            "instanceAfterChange" : 
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "relationshipInstances" : 
                    [{
                    "instanceId" : "NewIdB",
                    "className" : "NewClassB",
                    "schemaName" : "NewSchemaB",
                    "relatedInstance" : 
                        {
                        "instanceId" : "NewIdC",
                        "className" : "NewClassC",
                        "schemaName" : "NewSchemaC"
                        }
                    }]
                }
            }
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        switch (count)
            {
            case 0:
                EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaA.NewClassA", "NewIdA"), newId);
                break;
            case 1:
                EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaB.NewClassB", "NewIdB"), newId);
                break;
            case 2:
                EXPECT_EQ(ObjectId("TestSchemaC.TestClassC", "C"), oldId);
                EXPECT_EQ(ObjectId("NewSchemaC.NewClassC", "NewIdC"), newId);
                break;
            }

        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(3, count);
    }

TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_CreatedRelationship_CallsHandleForRelationship)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA","A"}, WSChangeset::Existing, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB","B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC","C"}, WSChangeset::Existing, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" : 
            [{
            "instanceAfterChange" : 
                {
                "instanceId" : "ExistingIdA",
                "className" : "ExistingClassA",
                "schemaName" : "ExistingSchemaA",
                "relationshipInstances" : 
                    [{
                    "instanceId" : "NewIdB",
                    "className" : "NewClassB",
                    "schemaName" : "NewSchemaB",
                    "relatedInstance" : 
                        {
                        "instanceId" : "ExistingIdC",
                        "className" : "ExistingClassC",
                        "schemaName" : "ExistingSchemaC"
                        }
                    }]
                }
            }]
        })");

    int count = 0;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
        EXPECT_EQ(ObjectId("NewSchemaB.NewClassB", "NewIdB"), newId);
        count++;
        return SUCCESS;
        }));
    EXPECT_EQ(1, count);
    }

TEST_F(WSChangesetTests, DISABLED_CalculateSize_LotsOfIntsances_PerformanceBetterThanDoingToRequestString)
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
            while ((int) changeset.GetInstanceCount() < testInstanceCount)
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

    // Measure ToRequestString().size()
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset([&] (WSChangeset& changeset)
        {
        changeset.ToRequestString().size();
        }, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("ToRequestString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure CalculateSize() incrementally
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset(nullptr, [&] (WSChangeset& changeset)
        {
        changeset.CalculateSize();
        });
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf(Utf8PrintfString("Incremental CalculateSize() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));

    // Measure ToRequestString().size() incrementally
    //start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //testChangeset(nullptr, [&] (WSChangeset& changeset)
    //    {
    //    changeset.ToRequestString().size();
    //    });
    //end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //printf(Utf8PrintfString("Incremental ToRequestString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations));
    }
