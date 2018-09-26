/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/WSChangesetTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSChangesetTests.h"

#include <WebServices/Client/WSChangeset.h>
#include <Bentley/BeTimeUtilities.h>

using namespace ::testing;

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, IsEmpty_Empty_True)
    {
    WSChangeset changeset;
    EXPECT_TRUE(changeset.IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, IsEmpty_InstanceAdded_False)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));
    EXPECT_FALSE(changeset.IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_EmptyChangeset_ReturnsEmptyChangesetAndSizeIsSameAsEmptyChangesetJson)
    {
    WSChangeset changeset;
    EXPECT_EQ(Utf8String(R"({"instances":[]})").size(), changeset.CalculateSize());
    EXPECT_EQ(Utf8String(R"({"instances":[]})"), changeset.ToRequestString());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetIsEmpty_ReturnsEmptyChangesetAndSizeIsSameAsEmptyChangesetJson)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    EXPECT_EQ(Utf8String(R"({"instancs":{}})").size(), changeset.CalculateSize());
    EXPECT_EQ(Utf8String(R"({"instance":{}})"), changeset.ToRequestString());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAddingMoreThanOneInstance_DoesNotAddAdditionalInstance)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);

    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    BeTest::SetFailOnAssert(false);
    changeset.AddInstance({"TestSchema.TestClass", "B"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));
    BeTest::SetFailOnAssert(true);

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"existing",
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_NullProperties_DoesNotAddPropertiesMember)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Created, nullptr);

    Json::Value changesetJson;
    changeset.ToRequestJson(changesetJson);
    ASSERT_TRUE(changesetJson.isMember("instance"));
    EXPECT_FALSE(changesetJson["instance"].isMember("properties"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_EmptyObjectValueProperties_DoesNotAddPropertiesMember)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Created, std::make_shared<Json::Value>(Json::objectValue));

    Json::Value changesetJson;
    changeset.ToRequestJson(changesetJson);
    ASSERT_TRUE(changesetJson.isMember("instance"));
    EXPECT_FALSE(changesetJson["instance"].isMember("properties"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_NullValueProperties_DoesNotAddPropertiesMember)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Created, std::make_shared<Json::Value>());

    Json::Value changesetJson;
    changeset.ToRequestJson(changesetJson);
    ASSERT_TRUE(changesetJson.isMember("instance"));
    EXPECT_FALSE(changesetJson["instance"].isMember("properties"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneExistingInstance_ReturnsChangesetJsonWithNotPropertiesAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState":"existing",
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneExistingInstance_ReturnsChangesetJsonWithNotPropertiesAndCalculateSizeMatches)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"existing",
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneCreatedInstance_ReturnsChangesetJsonWithPropertiesAndWithoutIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Created, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneCreatedInstance_ReturnsChangesetJsonWithPropertiesAndWithoutIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Created, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneModifiedInstance_ReturnsChangesetJsonWithPropertiesAndIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Modified, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneModifiedInstance_ReturnsChangesetJsonWithPropertiesAndIdAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Modified, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneDeletedInstance_ReturnsChangesetJsonWithoutPropertiesAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Deleted, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneDeletedInstance_ReturnsChangesetJsonWithoutPropertiesAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Deleted, properties);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetWithRequestOptions_ReturnsChangesetWithRequestOptionsAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Created, properties);
    changeset.SetRequestOptions(RequestOptions());

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"new",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":{"TestProperty":"TestValue"}
            },
        "requestOptions":{
            "FailureStrategy" : "Stop",
            "ResponseContent" : "FullInstance",
            "RefreshInstances" : false
        }})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneCreatedInstanceWithRequestOptions_ReturnsChangesetWithRequestOptionsAndCalculateSizeMatches)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Created, properties);
    changeset.SetRequestOptions(RequestOptions());

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState":"new",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "properties":{"TestProperty":"TestValue"}
            }
        ],
        "requestOptions":{
            "FailureStrategy" : "Stop",
            "ResponseContent" : "FullInstance",
            "RefreshInstances" : false
        }})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, AddInstance_SingleInstanceChangesetAndSecondInstance_ReturnsInvalidInstanceAndDoesNotAddIt)
    {
    auto properties = std::make_shared<Json::Value>(ToJson(R"({"TestProperty":"TestValue"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);

    auto& instance1 = changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Modified, properties);
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), instance1.GetId());

    BeTest::SetFailOnAssert(false);
    auto& instance2 = changeset.AddInstance({"TestSchema.TestClass", "B"}, WSChangeset::Modified, properties);
    BeTest::SetFailOnAssert(true);
    ASSERT_NE(nullptr, &instance2);
    EXPECT_EQ(ObjectId(), instance2.GetId());

    BeTest::SetFailOnAssert(false);
    auto& relInstance2 = instance2.AddRelatedInstance({"TestSchema.TestClass", "BC"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchema.TestClass", "C"}, WSChangeset::Created, properties);
    BeTest::SetFailOnAssert(true);
    ASSERT_NE(nullptr, &relInstance2.GetId());
    EXPECT_EQ(ObjectId(), relInstance2.GetId());

    auto expectedJson = ToJson(R"({
        "instance":
            {
            "changeState":"modified",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"A",
            "properties":{"TestProperty":"TestValue"}
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(1, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneForwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "IgnoredId"}, WSChangeset::Created, propertiesC);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_SingleInstanceChangesetAndOneForwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "IgnoredId"}, WSChangeset::Created, propertiesC);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OneBackwardRelatedInstance_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "IgnoredId"}, WSChangeset::Created, propertiesA)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Backward,
        {"TestSchemaC.TestClassC", "IgnoredId"}, WSChangeset::Created, propertiesC);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_TwoRelatedInstances_ReturnsChangesetJsonAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));
    auto propertiesE = std::make_shared<Json::Value>(ToJson(R"({"Foo":"E"})"));

    WSChangeset changeset;
    auto& instance = changeset.AddInstance({"TestSchemaA.TestClassA", "IgnoredId"}, WSChangeset::Created, propertiesA);
    instance.AddRelatedInstance({"TestSchemaB.TestClassB", "IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC", "IgnoredId"}, WSChangeset::Created, propertiesC);
    instance.AddRelatedInstance({"TestSchemaD.TestClassD", "IgnoredId"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE", "IgnoredId"}, WSChangeset::Created, propertiesE);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_ThreeInstances_ReturnsChangesetAndCalculateSizeMatches)
    {
    auto propertiesA = std::make_shared<Json::Value>(ToJson(R"({"Foo":"A"})"));
    auto propertiesB = std::make_shared<Json::Value>(ToJson(R"({"Foo":"B"})"));
    auto propertiesC = std::make_shared<Json::Value>(ToJson(R"({"Foo":"C"})"));

    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "A"}, WSChangeset::Modified, propertiesA);
    changeset.AddInstance({"TestSchema.TestClass", "B"}, WSChangeset::Created, propertiesB);
    changeset.AddInstance({"TestSchema.TestClass", "C"}, WSChangeset::Deleted, propertiesC);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_GetRequestOptions_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions();

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":{}
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_RemoveRequestOptions_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions();
    changeset.RemoveRequestOptions();

    auto expectedJson = ToJson(R"({
        "instances":[]
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(WSChangeset::Options::FailureStrategy::Null, changeset.GetRequestOptions().GetFailureStrategy());
    EXPECT_EQ(WSChangeset::Options::ResponseContent::Null, changeset.GetRequestOptions().GetResponseContent());
    EXPECT_TRUE(changeset.GetRequestOptions().GetCustomOptions().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsFailureStrategySet_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Continue);

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "FailureStrategy":"Continue"
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(WSChangeset::Options::FailureStrategy::Continue, changeset.GetRequestOptions().GetFailureStrategy());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsFailureStrategyRemoved_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
    changeset.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Null);

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":{}
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(WSChangeset::Options::FailureStrategy::Null, changeset.GetRequestOptions().GetFailureStrategy());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsResponseContentSet_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::InstanceId);

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "ResponseContent":"InstanceId"
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(WSChangeset::Options::ResponseContent::InstanceId, changeset.GetRequestOptions().GetResponseContent());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsRefreshInstanceSet_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetRefreshInstances(true);

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "RefreshInstances":true
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(WSChangeset::Options::RefreshInstances::Refresh, changeset.GetRequestOptions().GetRefreshInstances());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsTwoCustomOptionsSet_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetCustomOption("A", "1");
    changeset.GetRequestOptions().SetCustomOption("B", "2");

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "CustomOptions":{"A":"1","B":"2"}
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(ToJson(R"({"A":"1","B":"2"})"), changeset.GetRequestOptions().GetCustomOptions());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsCustomOptionsOwerwriten_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetCustomOption("A", "1");
    changeset.GetRequestOptions().SetCustomOption("A", "2");

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "CustomOptions":{"A":"2"}
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(ToJson(R"({"A":"2"})"), changeset.GetRequestOptions().GetCustomOptions());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsCustomOptionsRemoved_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetCustomOption("A", "1");
    changeset.GetRequestOptions().RemoveCustomOption("A");

    auto expectedJson = ToJson(R"({
        "instances":[],"requestOptions":{}})");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_TRUE(changeset.GetRequestOptions().GetCustomOptions().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsManyOptionsSet_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetRefreshInstances(true);  
    changeset.GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::InstanceId);
    changeset.GetRequestOptions().SetCustomOption("A", "1");

    auto expectedJson = ToJson(R"({
        "instances":[],
        "requestOptions":
            {
            "ResponseContent":"InstanceId",
            "CustomOptions":{"A":"1"},
            "RefreshInstances":true
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ToRequestString_OptionsSetAndInstanceAdded_ReturnsChangesetAndCalculateSizeMatches)
    {
    WSChangeset changeset;
    changeset.GetRequestOptions().SetRefreshInstances(true);
    changeset.AddInstance({"TestSchema.TestClass","Foo"}, WSChangeset::Existing, std::make_shared<Json::Value>(Json::objectValue));

    auto expectedJson = ToJson(R"({
        "instances":[
            {
            "changeState" : "existing",
            "schemaName":"TestSchema",
            "className":"TestClass",
            "instanceId":"Foo"
            }],
        "requestOptions":
            {
            "RefreshInstances":true
            }
        })");

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(expectedJson, ToJson(changesetStr));
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, RemoveInstance_OneInstanceInstance_ReturnsTrueAndLeavesChangesetEmpty)
    {
    WSChangeset changeset;
    auto& instance = changeset.AddInstance({"TestSchema.TestClassA", "A"}, WSChangeset::Modified, nullptr);

    EXPECT_EQ(true, changeset.RemoveInstance(instance));

    Utf8String changesetStr = changeset.ToRequestString();
    EXPECT_EQ(R"({"instances":[]})", changesetStr);
    EXPECT_EQ(changesetStr.size(), changeset.CalculateSize());
    EXPECT_EQ(0, changeset.GetInstanceCount());
    EXPECT_EQ(0, changeset.GetRelationshipCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, RemoveInstance_ThreeInstances_ReturnsTrueAndLeavesOtherTwoInstances)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchema.TestClassA", "A"}, WSChangeset::Modified, nullptr);
    auto& instanceB = changeset.AddInstance({"TestSchema.TestClassB", "B"}, WSChangeset::Created, nullptr);
    auto& instanceC = changeset.AddInstance({"TestSchema.TestClassC", "C"}, WSChangeset::Deleted, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, RemoveInstance_TwoRelatedInstances_ReturnsRemovedIdAndLeavesOtherInstances)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr);
    auto& instanceC = instanceA.AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC", "C"}, WSChangeset::Created, nullptr);
    auto& instanceE = instanceA.AddRelatedInstance({"TestSchemaD.TestClassD", "D"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE", "E"}, WSChangeset::Created, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, FindInstance_EmptyChangeset_ReturnsNullptr)
    {
    WSChangeset changeset;
    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchema.TestClass", "Foo"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, FindInstance_NonExistingInstance_ReturnsNullptr)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchema.TestClass", "Foo"}, WSChangeset::Created, nullptr);
    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchema.TestClass", "Other"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, FindInstance_RelationshipInstanceId_ReturnsNull)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Created, nullptr);

    EXPECT_EQ(nullptr, changeset.FindInstance({"TestSchemaB.TestClassB", "B"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, FindInstance_RootAndRelatedInstance_ReturnsInstance)
    {
    WSChangeset changeset;
    auto& instanceA = changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr);
    auto& instanceC = instanceA.AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaC.TestClassC", "C"}, WSChangeset::Created, nullptr);
    auto& instanceE = instanceC.AddRelatedInstance({"TestSchemaD.TestClassD", "D"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
    {"TestSchemaE.TestClassE", "E"}, WSChangeset::Created, nullptr);

    EXPECT_EQ(&instanceA, changeset.FindInstance({"TestSchemaA.TestClassA", "A"}));
    EXPECT_EQ(&instanceC, changeset.FindInstance({"TestSchemaC.TestClassC", "C"}));
    EXPECT_EQ(&instanceE, changeset.FindInstance({"TestSchemaE.TestClassE", "E"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_TwoCreatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr);
    changeset.AddInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_ModifiedDeletedInstances_DoesNotCallHandle)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Modified, nullptr);
    changeset.AddInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Deleted, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_ExistingRelationshipAndModifiedInstances_DoesNotCallHandler)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Modified, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Existing, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Modified, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_CreatedRelatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Created, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_SingleInstanceChangesetCreatedRelatedInstances_CallsHandleOnEachCreatedInstance)
    {
    WSChangeset changeset(WSChangeset::SingeInstance);
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Created, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_CreatedRelationship_CallsHandleForRelationship)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Existing, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Existing, nullptr);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894648
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_OneFailedAndOneSuccessInstance_CallsErrorAndSuccessHandler)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr);
    changeset.AddInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "error":
                    {
                    "httpStatusCode": 404,
                    "errorId": "InstanceNotFound",
                    "errorMessage": "MESSAGE",
                    "errorDescription": "DESCRIPTION"
                    }
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

    int successCount = 0;
    auto successHandler =
        [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
        EXPECT_EQ(ObjectId("NewSchemaB.NewClassB", "NewIdB"), newId);
        successCount++;
        return SUCCESS;
        };

    int errorCount = 0;
    auto errorHandler = [&] (ObjectIdCR oldId, WSErrorCR error)
        {
        EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
        EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
        EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
        EXPECT_NE("", error.GetDisplayMessage());
        EXPECT_EQ("MESSAGE\nDESCRIPTION", error.GetDisplayDescription());
        errorCount++;
        return SUCCESS;
        };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(1, successCount);
    EXPECT_EQ(1, errorCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_OneFailedModifiedInstance_CallsErrorHandler)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Modified, ToJsonPtr(R"({"TestProperty" : "TestValue"})"));

    auto response = ToRapidJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "error":
                    {
                    "httpStatusCode": 404,
                    "errorId": "InstanceNotFound",
                    "errorMessage": "MESSAGE",
                    "errorDescription": "DESCRIPTION"
                    }
                }
            }]
        })");

    int successCount = 0;
    auto successHandler =
        [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        successCount++;
        return SUCCESS;
        };

    int errorCount = 0;
    auto errorHandler = [&] (ObjectIdCR oldId, WSErrorCR error)
        {
        EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
        EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
        EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
        EXPECT_NE("", error.GetDisplayMessage());
        EXPECT_EQ("MESSAGE\nDESCRIPTION", error.GetDisplayDescription());
        errorCount++;
        return SUCCESS;
        };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(0, successCount);
    EXPECT_EQ(1, errorCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_OneFailedDeletedInstance_CallsErrorHandler)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Deleted, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "error":
                    {
                    "httpStatusCode": 404,
                    "errorId": "InstanceNotFound",
                    "errorMessage": "MESSAGE",
                    "errorDescription": "DESCRIPTION"
                    }
                }
            }]
        })");

    int successCount = 0;
    auto successHandler =
        [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        successCount++;
        return SUCCESS;
        };

    int errorCount = 0;
    auto errorHandler = [&] (ObjectIdCR oldId, WSErrorCR error)
        {
        EXPECT_EQ(ObjectId("TestSchemaA.TestClassA", "A"), oldId);
        EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
        EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
        EXPECT_NE("", error.GetDisplayMessage());
        EXPECT_EQ("MESSAGE\nDESCRIPTION", error.GetDisplayDescription());
        errorCount++;
        return SUCCESS;
        };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(0, successCount);
    EXPECT_EQ(1, errorCount);
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_OneFailedInstanceAndDefaultErrorHandler_ReturnsError)
    {
    WSChangeset changeset;
    changeset.AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Created, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "instanceId" : "NewIdA",
                "className" : "NewClassA",
                "schemaName" : "NewSchemaA",
                "error":
                    {
                    "httpStatusCode": 404,
                    "errorId": "InstanceNotFound",
                    "errorMessage": "MESSAGE",
                    "errorDescription": "DESCRIPTION"
                    }
                }
            }]
        })");

    auto successHandler = [] (ObjectIdCR, ObjectIdCR) { return SUCCESS; };
    auto errorHandler = [] (ObjectIdCR, WSErrorCR) { return SUCCESS; };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(ERROR, changeset.ExtractNewIdsFromResponse(*response, successHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894648
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_FailedRelationshipInstance_CallsErrorHandler)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Existing, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Existing, nullptr);

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
                    "error":
                        {
                        "httpStatusCode": 404,
                        "errorId": "InstanceNotFound",
                        "errorMessage": "MESSAGE",
                        "errorDescription": "DESCRIPTION"
                        },
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

    int successCount = 0;
    auto successHandler =
        [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        successCount++;
        return SUCCESS;
        };

    int errorCount = 0;
    auto errorHandler = [&] (ObjectIdCR oldId, WSErrorCR error)
        {
        EXPECT_EQ(ObjectId("TestSchemaB.TestClassB", "B"), oldId);
        EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
        EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
        EXPECT_NE("", error.GetDisplayMessage());
        EXPECT_EQ("MESSAGE\nDESCRIPTION", error.GetDisplayDescription());
        errorCount++;
        return SUCCESS;
        };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(0, successCount);
    EXPECT_EQ(1, errorCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, ExtractNewIdsFromResponse_MultipleErrorInstances_CallsErrorHandlerForEach)
    {
    WSChangeset changeset;
    changeset
        .AddInstance({"TestSchemaA.TestClassA", "A"}, WSChangeset::Existing, nullptr)
        .AddRelatedInstance({"TestSchemaB.TestClassB", "B"}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchemaC.TestClassC", "C"}, WSChangeset::Existing, nullptr);

    auto response = ToRapidJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "instanceId" : "ExistingIdA",
                "className" : "ExistingClassA",
                "schemaName" : "ExistingSchemaA",
                "error":
                    {
                    "httpStatusCode": 404,
                    "errorId": "InstanceNotFound",
                    "errorMessage": "MESSAGE",
                    "errorDescription": "DESCRIPTION"
                    },
                "relationshipInstances" :
                    [{
                    "instanceId" : "NewIdB",
                    "className" : "NewClassB",
                    "schemaName" : "NewSchemaB",
                    "error":
                        {
                        "httpStatusCode": 404,
                        "errorId": "InstanceNotFound",
                        "errorMessage": "MESSAGE",
                        "errorDescription": "DESCRIPTION"
                        },
                    "relatedInstance" :
                        {
                        "instanceId" : "ExistingIdC",
                        "className" : "ExistingClassC",
                        "schemaName" : "ExistingSchemaC",
                        "error":
                            {
                            "httpStatusCode": 404,
                            "errorId": "InstanceNotFound",
                            "errorMessage": "MESSAGE",
                            "errorDescription": "DESCRIPTION"
                            }
                        }
                    }]
                }
            }]
        })");

    int successCount = 0;
    auto successHandler =
        [&] (ObjectIdCR oldId, ObjectIdCR newId)
        {
        successCount++;
        return SUCCESS;
        };

    bvector<ObjectId> expectedIds = {
            {"TestSchemaA.TestClassA", "A"},
            {"TestSchemaB.TestClassB", "B"},
            {"TestSchemaC.TestClassC", "C"}
        };

    size_t errorCount = 0;
    auto errorHandler = [&] (ObjectIdCR oldId, WSErrorCR error)
        {
        errorCount++;
        if (errorCount > expectedIds.size())
            return ERROR;

        EXPECT_EQ(expectedIds[errorCount - 1], oldId);
        EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
        EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
        EXPECT_NE("", error.GetDisplayMessage());
        EXPECT_EQ("MESSAGE\nDESCRIPTION", error.GetDisplayDescription());
        return SUCCESS;
        };

    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(*response, successHandler, errorHandler));
    EXPECT_EQ(0, successCount);
    EXPECT_EQ(3, errorCount);
    }
#endif
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSChangesetTests, CalculateSize_LotsOfIntsances_PerformanceBetterThanDoingToRequestString)
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
    int testInstanceCount = 100;
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
    printf("Create changeset for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations);

    // Measure CalculateSize()
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset([&] (WSChangeset& changeset)
        {
        changeset.CalculateSize();
        }, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf("CalculateSize() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations);

    // Measure ToRequestString().size()
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset([&] (WSChangeset& changeset)
        {
        changeset.ToRequestString().size();
        }, nullptr);
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf("ToString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations);

    // Measure CalculateSize() incrementally
    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    testChangeset(nullptr, [&] (WSChangeset& changeset)
        {
        changeset.CalculateSize();
        });
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    printf("Incremental CalculateSize() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations);

    // Measure ToRequestString().size() incrementally
    //start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //testChangeset(nullptr, [&] (WSChangeset& changeset)
    //    {
    //    changeset.ToRequestString().size();
    //    });
    //end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    //printf("Incremental ToString().size() for ~%i instances took: %.2f ms\n", testInstanceCount, (end - start) / testIterations);
    }
