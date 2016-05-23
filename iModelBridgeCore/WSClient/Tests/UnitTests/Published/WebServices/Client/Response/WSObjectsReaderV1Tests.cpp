/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/UnitTests/Published/WebServices/Client/Response/WSObjectsReaderV1Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSObjectsReaderV1Tests.h"

#include <WebServices/Client/Response/WSObjectsReaderV1.h>

TEST_F(WSObjectsReaderV1Tests, InstancesIsValid_JsonAsNullValue_ReturnsFalse)
    {
    BeTest::SetFailOnAssert(false);
    auto json = std::make_shared<rapidjson::Document>();
    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsValid());
    EXPECT_TRUE(instances.IsEmpty());
    EXPECT_EQ(0, instances.Size());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV1Tests, InstancesIsValid_JsonAsEmptyObject_ReturnsTrue)
    {
    auto json = ToRapidJson("{}");

    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_TRUE(instances.IsEmpty());
    EXPECT_TRUE(instances.begin() == instances.end());
    EXPECT_EQ(0, instances.Size());
    }

TEST_F(WSObjectsReaderV1Tests, InstancesIsValid_JsonAsInstance_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "$id" : "A" })");

    auto reader = WSObjectsReaderV1::Create("TestSchema", "TestClass");
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE(instances.begin() == instances.end());
    EXPECT_EQ(1, instances.Size());
    }

TEST_F(WSObjectsReaderV1Tests, InstancesIsValid_JsonAsInstanceByClass_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "TestClass" : [{ "$id" : "A" }] })");

    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE(instances.begin() == instances.end());
    EXPECT_EQ(1, instances.Size());
    }

TEST_F(WSObjectsReaderV1Tests, InstanceIsValid_InstanceWithoutId_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({ "notId" : "A" })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV1::Create("TestSchema", "TestClass");
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE((*instances.begin()).IsValid());
    EXPECT_EQ(1, instances.Size());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV1Tests, InstanceIsValid_InstanceInClassWithoutId_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({ "TestClass1" : [{ "notId" : "A" }] })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE((*instances.begin()).IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV1Tests, ReadInstances_JsonInstanceWithoutETag_ReturnsEmptyETag)
    {
    auto json = ToRapidJson(R"({ "TestClass1" : [{ "notId" : "A" }] })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_STREQ("", (*instances.begin()).GetETag().c_str());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV1Tests, RelationshipInstancesIsValid_V1FormatDoesNotHaveRelationships_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "$id" : "A" })");

    auto reader = WSObjectsReaderV1::Create("TestSchema", "TestClass");
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE((*instances.begin()).GetRelationshipInstances().IsValid());
    EXPECT_TRUE((*instances.begin()).GetRelationshipInstances().IsEmpty());
    EXPECT_EQ(0, (*instances.begin()).GetRelationshipInstances().Size());
    }

TEST_F(WSObjectsReaderV1Tests, End_IsValid_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({})");

    auto reader = WSObjectsReaderV1::Create("TestSchema", "TestClass");
    auto instances = reader->ReadInstances(json);

    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE((*instances.end()).IsValid());
    EXPECT_FALSE((*(*instances.end()).GetRelationshipInstances().end()).IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV1Tests, BeginIncrement_JsonAsThreeInstancesByTwoClasses_IncrementsThreeTimes)
    {
    auto json = ToRapidJson(R"({ "TestClass1" : [{ "$id" : "A" }, { "$id" : "B" }], "TestClass2" : [{ "$id" : "C" }] })");

    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto instances = reader->ReadInstances(json);

    auto it = instances.begin();
    EXPECT_FALSE(it == instances.end());
    ++it;
    EXPECT_FALSE(it == instances.end());
    ++it;
    EXPECT_FALSE(it == instances.end());
    ++it;
    EXPECT_TRUE(it == instances.end());
    }

TEST_F(WSObjectsReaderV1Tests, Begin_JsonAsInstance_ReturnsInstance)
    {
    auto json = ToRapidJson(R"({ "$id" : "A", "Property" : "Value" })");

    auto reader = WSObjectsReaderV1::Create("TestSchema", "TestClass");
    auto instances = reader->ReadInstances(json);

    WSObjectsReader::Instance instance = *instances.begin();

    EXPECT_EQ(ObjectId("TestSchema", "TestClass", "A"), instance.GetObjectId());
    EXPECT_EQ("Value", Utf8String(instance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(instance.GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV1Tests, Begin_JsonAsInstanceByClass_ReturnsInstance)
    {
    auto json = ToRapidJson(R"({ "TestClass" : [{ "$id" : "A", "Property" : "Value" }] })");

    auto reader = WSObjectsReaderV1::Create("TestSchema");
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_EQ(ObjectId("TestSchema", "TestClass", "A"), instance.GetObjectId());
    EXPECT_EQ("Value", Utf8String(instance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(instance.GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV1Tests, BeginAndIncrement_JsonAsTwoInstancesByTwoClass_ReturnsSecondInstance)
    {
    auto json = ToRapidJson(R"({ "TestClass1" : [{ "$id" : "A" }], "TestClass2" : [{ "$id" : "B" }] })");

    auto reader = WSObjectsReaderV1::Create("TestSchema");
    auto it = reader->ReadInstances(json).begin();

    ++it;
    WSObjectsReader::Instance instance = *it;

    EXPECT_EQ(ObjectId("TestSchema", "TestClass2", "B"), instance.GetObjectId());
    }
