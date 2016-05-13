/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/UnitTests/Published/WebServices/Client/Response/WSObjectsReaderV2Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSObjectsReaderV2Tests.h"

#include <WebServices/Client/Response/WSObjectsReaderV2.h>

TEST_F(WSObjectsReaderV2Tests, InstancesIsValid_JsonAsNullValue_ReturnsFalse)
    {
    BeTest::SetFailOnAssert(false);
    auto json = ToRapidJson("null");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsValid());
    EXPECT_TRUE(instances.IsEmpty());
    EXPECT_TRUE(instances.begin() == instances.end());
    EXPECT_EQ(0, instances.Size());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, InstancesIsValid_JsonAsEmptyObject_ReturnsTrue)
    {
    auto json = ToRapidJson("{}");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsValid());
    EXPECT_TRUE(instances.IsEmpty());
    EXPECT_TRUE(instances.begin() == instances.end());
    EXPECT_EQ(0, instances.Size());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, InstancesIsValid_JsonAsEmptyInstanceList_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "instances" : [] })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_TRUE(instances.IsEmpty());
    EXPECT_TRUE(instances.begin() == instances.end());
    EXPECT_EQ(0, instances.Size());
    }

TEST_F(WSObjectsReaderV2Tests, InstancesIsValid_JsonWithOneInstance_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "instances" : [ {} ] })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE(instances.begin() == instances.end());
    EXPECT_EQ(1, instances.Size());
    }

TEST_F(WSObjectsReaderV2Tests, InstancesSize_JsonWithThreeInstances_ReturnsThree)
    {
    auto json = ToRapidJson(R"({ "instances" : [ {}, {}, {} ] })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE(instances.begin() == instances.end());
    EXPECT_EQ(3, instances.Size());
    }

TEST_F(WSObjectsReaderV2Tests, InstancesSize_JsonWithOneInstanceWithRelatedInstance_ReturnsOnlyOneForMainInstance)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "IdA",
            "className" : "ClassA",
            "schemaName" : "SchemaA",
            "direction" : "forward",
            "relatedInstance": 
                {
                "instanceId" : "IdB",
                "className" : "ClassB",
                "schemaName" : "SchemaB",
                "properties" : {}
                }
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE(instances.IsValid());
    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE(instances.begin() == instances.end());
    EXPECT_EQ(1, instances.Size());
    }

TEST_F(WSObjectsReaderV2Tests, InstanceIsValid_JsonWithOneInstanceButWithNoMetaData_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({ "instances" : [ {} ] })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE((*instances.begin()).IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, InstanceIsValid_JsonWithInstanceWithoutProperties_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({ "instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "eTag" : "A"
        }]
    })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_FALSE((*instances.begin()).IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, ReadInstance_JsonInstanceWithoutETag_ReturnsEmptyETag)
    {
    auto json = ToRapidJson(R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {}
        }]
        })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_FALSE(instances.IsEmpty());
    EXPECT_STREQ("", (*instances.begin()).GetETag().c_str());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstancesIsValid_JsonWithInstanceWithoutRelationships_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({ "instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {}
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);

    EXPECT_TRUE((*instances.begin()).GetRelationshipInstances().IsValid());
    EXPECT_TRUE((*instances.begin()).GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstancesIsValid_JsonWithRelationshipsButNotArray_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {} ,
        "relationshipInstances": {}
        }]
    })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_FALSE(instance.GetRelationshipInstances().IsValid());
    EXPECT_TRUE(instance.GetRelationshipInstances().IsEmpty());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstancesIsValid_JsonWithRelationshipsWithArray_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {} ,
        "relationshipInstances": [{}]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_TRUE(instance.GetRelationshipInstances().IsValid());
    EXPECT_FALSE(instance.GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstanceIsValid_JsonWithRelationshipInstanceWithoutMetaData_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{}]
        }]
    })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_FALSE(instance.GetRelationshipInstances().IsEmpty());
    EXPECT_FALSE((*instance.GetRelationshipInstances().begin()).IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstanceIsValid_JsonWithRelationshipInstanceWithoutProperties_ReturnsTrue)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "B",
            "className" : "B",
            "schemaName" : "B",
            "direction" : "backward",
            "relatedInstance": {}
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_FALSE(instance.GetRelationshipInstances().IsEmpty());
    EXPECT_TRUE((*instance.GetRelationshipInstances().begin()).IsValid());
    }

TEST_F(WSObjectsReaderV2Tests, RelatedInstanceIsValid_JsonWithNoRelatedInstance_ReturnsFalse)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "B",
            "className" : "B",
            "schemaName" : "B",
            "direction" : "backward"
            }]
        }]
    })");

    BeTest::SetFailOnAssert(false);
    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_FALSE(instance.GetRelationshipInstances().IsEmpty());
    EXPECT_FALSE((*instance.GetRelationshipInstances().begin()).GetRelatedInstance().IsValid());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsReaderV2Tests, Begin_JsonWithOneInstance_ReturnsInstance)
    {
    auto json = ToRapidJson(R"({ "instances" : 
        [{
        "instanceId" : "A",
        "className" : "TestClass",
        "schemaName" : "TestSchema",
        "properties" : 
            {
            "Property": "Value"
            }
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), instance.GetObjectId());
    EXPECT_EQ("Value", Utf8String(instance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(instance.GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV2Tests, BeginIncrement_JsonWithTwoInstances_ReturnsSecondInstance)
    {
    auto json = ToRapidJson(R"({ "instances" : 
        [{
        "instanceId" : "A",
        "className" : "TestClass",
        "schemaName" : "TestSchema",
        "properties" : {}
        },
        {
        "instanceId" : "B",
        "className" : "TestClassB",
        "schemaName" : "TestSchemaB",
        "properties" : 
            {
            "Property": "ValueB"
            }
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    auto it = reader->ReadInstances(json).begin();

    ++it;
    WSObjectsReader::Instance instance = *it;

    EXPECT_EQ(ObjectId("TestSchemaB", "TestClassB", "B"), instance.GetObjectId());
    EXPECT_EQ("ValueB", Utf8String(instance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(instance.GetRelationshipInstances().IsEmpty());
    }

TEST_F(WSObjectsReaderV2Tests, GetETag_QuoteEtagsSetToTrue_ReturnsETagWithQuotes)
    {
    auto json = ToRapidJson(R"({"instances" : 
            [{
            "instanceId" : "Foo",
            "className" : "Foo",
            "schemaName" : "Foo",
            "eTag" : "TestEtagA",
            "properties" : {},
            "relationshipInstances": [{
                "instanceId" : "Foo",
                "className" : "Foo",
                "schemaName" : "Foo",
                "eTag" : "TestEtagB",
                "direction" : "forward",
                "relatedInstance": 
                    {
                    "instanceId" : "Foo",
                    "className" : "Foo",
                    "schemaName" : "Foo",
                    "properties" : {}
                    }
                }]
            }]
        })");

    auto reader = WSObjectsReaderV2::Create(true);
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_STREQ("\"TestEtagA\"", instance.GetETag().c_str());
    EXPECT_STREQ("\"TestEtagB\"", (*instance.GetRelationshipInstances().begin()).GetETag().c_str());
    }

TEST_F(WSObjectsReaderV2Tests, GetETag_QuoteEtagsSetToFalse_ReturnsRawETag)
    {
    auto json = ToRapidJson(R"({"instances" : 
            [{
            "instanceId" : "Foo",
            "className" : "Foo",
            "schemaName" : "Foo",
            "eTag" : "TestEtagA",
            "properties" : {},
            "relationshipInstances": [{
                "instanceId" : "Foo",
                "className" : "Foo",
                "schemaName" : "Foo",
                "eTag" : "TestEtagB",
                "direction" : "forward",
                "relatedInstance": 
                    {
                    "instanceId" : "Foo",
                    "className" : "Foo",
                    "schemaName" : "Foo",
                    "properties" : {}
                    }
                }]
            }]
        })");

    auto reader = WSObjectsReaderV2::Create(false);
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_STREQ("TestEtagA", instance.GetETag().c_str());
    EXPECT_STREQ("TestEtagB", (*instance.GetRelationshipInstances().begin()).GetETag().c_str());
    }

TEST_F(WSObjectsReaderV2Tests, Begin_JsonWithOneInstanceWithOneRelatedInstance_ReturnsInstanceWithRelationship)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "TestClassA",
        "schemaName" : "TestSchemaA",
        "properties" : 
            {
            "Property": "ValueA"
            },
        "relationshipInstances": [{
            "instanceId" : "RelId",
            "className" : "TestRelClass",
            "schemaName" : "TestRelSchema",
            "direction" : "backward",
            "properties" : 
                {
                "Property": "RelValue"
                },
            "relatedInstance": 
                {
                "instanceId" : "B",
                "className" : "TestClassB",
                "schemaName" : "TestSchemaB",
                "properties" : 
                    {
                    "Property": "ValueB"
                    }
                }
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_EQ("A", instance.GetObjectId().remoteId);

    EXPECT_FALSE(instance.GetRelationshipInstances().IsEmpty());

    WSObjectsReader::RelationshipInstance relationshipInstance = *instance.GetRelationshipInstances().begin();
    EXPECT_EQ(ObjectId("TestRelSchema", "TestRelClass", "RelId"), relationshipInstance.GetObjectId());
    EXPECT_EQ("RelValue", Utf8String(relationshipInstance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(BentleyApi::ECN::ECRelatedInstanceDirection::Backward == relationshipInstance.GetDirection());
    EXPECT_TRUE(relationshipInstance.IsValid());

    WSObjectsReader::Instance relatedInstance = relationshipInstance.GetRelatedInstance();
    EXPECT_EQ(ObjectId("TestSchemaB", "TestClassB", "B"), relatedInstance.GetObjectId());
    EXPECT_EQ("ValueB", Utf8String(relatedInstance.GetProperties()["Property"].GetString()));
    EXPECT_TRUE(relatedInstance.GetRelationshipInstances().IsEmpty());
    EXPECT_TRUE(relatedInstance.IsValid());
    }

TEST_F(WSObjectsReaderV2Tests, BeginIncrement_JsonWithOneInstanceWithTwoRelatedInstances_ReturnsInstanceWithAllRelationships)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "IdA",
            "className" : "ClassA",
            "schemaName" : "SchemaA",
            "direction" : "forward",
            "relatedInstance": 
                {
                "instanceId" : "IdB",
                "className" : "ClassB",
                "schemaName" : "SchemaB",
                "properties" : {}
                }
            },
            {
            "instanceId" : "IdC",
            "className" : "ClassC",
            "schemaName" : "SchemaC",
            "direction" : "backward",
            "relatedInstance": 
                {
                "instanceId" : "IdD",
                "className" : "ClassD",
                "schemaName" : "SchemaD",
                "properties" : {}
                }
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    auto it = instance.GetRelationshipInstances().begin();

    WSObjectsReader::RelationshipInstance relationshipInstance = *it;
    EXPECT_EQ(ObjectId("SchemaA", "ClassA", "IdA"), relationshipInstance.GetObjectId());
    EXPECT_EQ(ObjectId("SchemaB", "ClassB", "IdB"), relationshipInstance.GetRelatedInstance().GetObjectId());
    EXPECT_TRUE(BentleyApi::ECN::ECRelatedInstanceDirection::Forward == relationshipInstance.GetDirection());
    EXPECT_TRUE(relationshipInstance.IsValid());

    ++it;

    relationshipInstance = *it;
    EXPECT_EQ(ObjectId("SchemaC", "ClassC", "IdC"), relationshipInstance.GetObjectId());
    EXPECT_EQ(ObjectId("SchemaD", "ClassD", "IdD"), relationshipInstance.GetRelatedInstance().GetObjectId());
    EXPECT_TRUE(BentleyApi::ECN::ECRelatedInstanceDirection::Backward == relationshipInstance.GetDirection());
    EXPECT_TRUE(relationshipInstance.IsValid());

    ++it;

    EXPECT_TRUE(it == instance.GetRelationshipInstances().end());
    }

TEST_F(WSObjectsReaderV2Tests, Begin_JsonWithInstanceAndRelationshipWithoutProperties_ReturnsEmptyObjectsForProperties)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "B",
            "className" : "B",
            "schemaName" : "B",
            "direction" : "forward",
            "relatedInstance": 
                {
                "instanceId" : "C",
                "className" : "C",
                "schemaName" : "C",
                "properties" : {}
                }
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    WSObjectsReader::Instance instance = *reader->ReadInstances(json).begin();

    EXPECT_TRUE(instance.GetProperties().IsObject());
    EXPECT_TRUE(instance.GetProperties().MemberBegin() == instance.GetProperties().MemberEnd());

    WSObjectsReader::RelationshipInstance relInstance = *instance.GetRelationshipInstances().begin();
    EXPECT_TRUE(relInstance.GetProperties().IsObject());
    EXPECT_TRUE(relInstance.GetProperties().MemberBegin() == relInstance.GetProperties().MemberEnd());

    EXPECT_TRUE(relInstance.GetRelatedInstance().GetProperties().IsObject());
    EXPECT_TRUE(relInstance.GetRelatedInstance().GetProperties().MemberBegin() == relInstance.GetRelatedInstance().GetProperties().MemberEnd());
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstancesSize_JsonWithInstanceWithoutRelationships_ReturnsZero)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {}
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);
    ASSERT_EQ(1, instances.Size());

    auto instance = *instances.begin();
    auto relationships = instance.GetRelationshipInstances();

    EXPECT_TRUE(relationships.IsValid());
    EXPECT_TRUE(relationships.IsEmpty());
    EXPECT_EQ(0, relationships.Size());
    }

TEST_F(WSObjectsReaderV2Tests, RelationshipInstancesSize_JsonWithInstanceWithRelationships_ReturnsRelationshipCount)
    {
    auto json = ToRapidJson(R"({"instances" : 
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {},
        "relationshipInstances": [{
            "instanceId" : "IdA",
            "className" : "ClassA",
            "schemaName" : "SchemaA",
            "direction" : "forward",
            "relatedInstance": 
                {
                "instanceId" : "IdB",
                "className" : "ClassB",
                "schemaName" : "SchemaB",
                "properties" : {}
                }
            },
            {
            "instanceId" : "IdC",
            "className" : "ClassC",
            "schemaName" : "SchemaC",
            "direction" : "backward",
            "relatedInstance": 
                {
                "instanceId" : "IdD",
                "className" : "ClassD",
                "schemaName" : "SchemaD",
                "properties" : {}
                }
            }]
        }]
    })");

    auto reader = WSObjectsReaderV2::Create();
    auto instances = reader->ReadInstances(json);
    ASSERT_EQ(1, instances.Size());

    auto instance = *instances.begin();
    auto relationships = instance.GetRelationshipInstances();

    EXPECT_TRUE(relationships.IsValid());
    EXPECT_FALSE(relationships.IsEmpty());
    EXPECT_EQ(2, relationships.Size());
    }
