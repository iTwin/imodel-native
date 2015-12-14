/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/NavigationECPropertyTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct NavigationECPropertyTests : ECTestFixture { };

void CreateNavProp(ECEntityClassP ecClass, Utf8StringCR propName, ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, NavigationECPropertyP& navProp)
    {
    ECObjectsStatus status = ecClass->CreateNavigationProperty(navProp, propName, relClass, direction);
    ASSERT_EQ(ECObjectsStatus::Success, status) << "Failed to create navigation property " << propName;
    ASSERT_NE(nullptr, navProp) << "Navigation property " << propName << " null though success returned";
    ASSERT_EQ(propName, navProp->GetName()) << "Navigation property " << propName << " does not have expected name";
    ASSERT_EQ(relClass.GetName(), navProp->GetRelationshipClass()->GetName()) << "Navigation property " << propName << " does not have expected relationship";
    ASSERT_EQ(direction, navProp->GetDirection()) << "Navigation property " << propName << " does not have expected direction";
    }

void ValidateDeserializedNavProp(ECSchemaPtr deserializedSchema, NavigationECPropertyCP expectedNavProp, Utf8StringCR className, Utf8StringCR propName)
    {
    ECClassCP deserialized = deserializedSchema->GetClassCP(className.c_str());
    ASSERT_TRUE(nullptr != deserialized) << "Class '" << className << "' containing navigation property not found in deserialized schema";

    ECPropertyP ecProp = deserialized->GetPropertyP(propName);
    ASSERT_TRUE(nullptr != ecProp) << "Navigation property '" << propName << "' not found in deserialized schema";
    NavigationECPropertyP navProp = ecProp->GetAsNavigationPropertyP();
    ASSERT_TRUE(nullptr != navProp) << "Navigation property '" << propName << "' is not of type NavigationECProperty in deserialized schema";

    ASSERT_EQ(expectedNavProp->GetName(), navProp->GetName()) << "Navigation property '" << propName << "' does not have correct name in deserialized schema";
    ASSERT_EQ(expectedNavProp->GetRelationshipClass()->GetName(), navProp->GetRelationshipClass()->GetName()) << "Navigation property '" << propName << "' does not have correct relationship class in deserialized schema";
    ASSERT_EQ(expectedNavProp->GetDirection(), navProp->GetDirection()) << "Navigation property '" << propName << "' does not have correct direction in deserialized schema";
    }

void ValidateRoundTripEC3Serialization(ECSchemaPtr schema, NavigationECPropertyCP expectedNavProp1, NavigationECPropertyCP expectedNavProp2)
    {
    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, 3, 0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema with navigation property";

    ValidateDeserializedNavProp(deserializedSchema, expectedNavProp1, expectedNavProp1->GetClass().GetName(), expectedNavProp1->GetName());
    if (nullptr != expectedNavProp2)
        ValidateDeserializedNavProp(deserializedSchema, expectedNavProp2, expectedNavProp2->GetClass().GetName(), expectedNavProp2->GetName());
    }

void VerifyFailedToDeserialize(Utf8StringCR invalidSchemaString, Utf8StringCR failureMessage)
    {
    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, invalidSchemaString.c_str(), *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, readStatus) << "Successfully deserialized schema with error: " << failureMessage;
    }

TEST_F(NavigationECPropertyTests, CreateAndRoundTripNavigationProperty)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);
    ValidateRoundTripEC3Serialization(schema, navPropSource, navPropTarget);

    // Test with one many cardinality on one endpoint
    sourceClass->RemoveProperty(navPropSource->GetName());
    targetClass->RemoveProperty(navPropTarget->GetName());

    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneMany());

    // Should succeed when nav property points to singular endpoint
    CreateNavProp(targetClass, "MySourceWhenTargetMany", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);
    ValidateRoundTripEC3Serialization(schema, navPropTarget, nullptr);
    }

TEST_F(NavigationECPropertyTests, NavPropUsingRelationshipWithMultipleConstraintClasses)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP source2Class;
    ECEntityClassP targetClass;
    ECEntityClassP target2Class;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(source2Class, "Source2");
    schema->CreateEntityClass(targetClass, "Target");
    schema->CreateEntityClass(target2Class, "Target2");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().AddClass(*source2Class);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().AddClass(*target2Class);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget2Constraints", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource2Constraints", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    ValidateRoundTripEC3Serialization(schema, navPropSource, navPropTarget);
    }

TEST_F(NavigationECPropertyTests, InvalidNavigationProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    // Test wrong direction fails
    NavigationECPropertyP failure;
    ECObjectsStatus status = sourceClass->CreateNavigationProperty(failure, "MyFailure", *relClass, ECRelatedInstanceDirection::Backward);
    ASSERT_NE(ECObjectsStatus::Success, status) << "Expected failure when creating a navigation property with direction wrong";

    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneMany());

    // Should fail when nav property points to a many endpoint (NOTE: We may allow this in the future but not in first release)
    status = sourceClass->CreateNavigationProperty(failure, "MyFail", *relClass, ECRelatedInstanceDirection::Forward);
    ASSERT_NE(ECObjectsStatus::Success, status) << "Expected failure when creating a navigation property when cardinality has upper limit greater than 1";
    }

// TODO/NEEDSWORK - Fails because we cannot validate nav props on deserialization
TEST_F(NavigationECPropertyTests, InvalidXml)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, 3, 0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    Utf8String invalidSchemaString = schemaString.copy();
    invalidSchemaString.ReplaceAll("direction=\"forward\"", "direction=\"backward\"");
    VerifyFailedToDeserialize(invalidSchemaString, "forward direction set to backward");

    invalidSchemaString = schemaString.copy();
    invalidSchemaString.ReplaceAll("direction=\"backward\"", "direction=\"forward\"");
    VerifyFailedToDeserialize(invalidSchemaString, "backward direction set to forward");

    invalidSchemaString = schemaString.copy();
    invalidSchemaString.ReplaceAll("cardinality = \"(0,1)\"", "cardinality = \"(0,N)\"");
    VerifyFailedToDeserialize(invalidSchemaString, "(0,1) cardinality set to (0,N)");

    invalidSchemaString = schemaString.copy();
    invalidSchemaString.ReplaceAll("cardinality = \"(1,1)\"", "cardinality = \"(1,N)\"");
    VerifyFailedToDeserialize(invalidSchemaString, "(1,1) cardinality set to (1,N)");
    }

TEST_F(NavigationECPropertyTests, RoundtripToEC2Xml)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, 2, 0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema to ECXml 2.0 with navigation property";

    ASSERT_FALSE(schemaString.Contains("ECNavigationProperty")) << "Schema serialized as ECXml 2.0 has navigation property when it should have a normal property";

    ECSchemaPtr schemaFromECXml2;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(schemaFromECXml2, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema from ECXml 2.0 that had navigation property";

    ECClassCP sourceDeserialized = schemaFromECXml2->GetClassCP("Source");
    ECPropertyP navPropSourceDeserialized = sourceDeserialized->GetPropertyP("MyTarget");
    ASSERT_NE(nullptr, navPropSourceDeserialized) << "Source.MyTarget property does not exist after deserialization from ECXml 2.0";
    ASSERT_FALSE(navPropSourceDeserialized->GetIsNavigation()) << "Source.MyTarget property is still navigation property after deserializing from ECXml 2.0"; // Not that it couldn't be just that we don't need it to
    ASSERT_EQ("string", navPropSourceDeserialized->GetTypeName()) << "Expected navigation property to be of type string after roundtripped to ECXml 2.0";
    }

TEST_F(NavigationECPropertyTests, InstanceWithNavProp)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", 4, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetCardinality(RelationshipCardinality::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    PrimitiveECPropertyP stringProp;
    sourceClass->CreatePrimitiveProperty(stringProp, "sProp", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP intProp;
    sourceClass->CreatePrimitiveProperty(intProp, "iProp", PrimitiveType::PRIMITIVETYPE_Integer);

    StandaloneECEnablerPtr enabler = sourceClass->GetDefaultStandaloneEnabler();
    StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();
    
    ECValue stringValue ("SomeValue");
    sourceInstance->SetValue("sProp", stringValue);
    ECValue intValue(42);
    sourceInstance->SetValue("iProp", intValue);

    ECValue myTargetValueFromInst;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetValueFromInst, "MyTarget")) << "Failed to get ECValue for MyTarget Navigation Property";
    ASSERT_TRUE(myTargetValueFromInst.IsNull()) << "Expected Navigation Property MyTarget to be null but it was not";
    ASSERT_TRUE(myTargetValueFromInst.IsString()) << "Expected Navigation Property to have an ECValue of type string because this is a standalone instance";

    ECValue myTargetValue("IdOfTarget");
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->SetValue("MyTarget", myTargetValue)) << "Failed to set the value of MyTarget nav prop to a string";
    EXPECT_STREQ("IdOfTarget", myTargetValue.GetUtf8CP()) << "Value of MyTarget nav property not as expected";

    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetValueFromInst, "MyTarget")) << "Failed to get ECValue for MyTarget Navigation Property after set";
    ASSERT_FALSE(myTargetValueFromInst.IsNull()) << "Expected Navigation Property MyTarget to be not null after set but it was";
    EXPECT_STREQ("IdOfTarget", myTargetValueFromInst.GetUtf8CP()) << "Value of MyTarget nav property gotten from instance not as expected";

    Utf8String xmlString;
    InstanceWriteStatus writeStatus = sourceInstance->WriteToXmlString(xmlString, true, false);
    ASSERT_EQ(InstanceWriteStatus::Success, writeStatus) << "Failed to serilaize an instance with a nav property";
    
    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext(*schema);
    IECInstancePtr sourceDeserialized;
    InstanceReadStatus readStatus = StandaloneECInstance::ReadFromXmlString(sourceDeserialized, xmlString.c_str(), *readContext);
    ASSERT_EQ(InstanceReadStatus::Success, readStatus) << "Failed to deserialize an instance with a nav property";

    ECValue stringValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(stringValueDes, "sProp")) << "Failed to get standard string property value after deserialization";
    ASSERT_STREQ(stringValue.GetUtf8CP(), stringValueDes.GetUtf8CP()) << "Standard String property value failed to deserialize correctly";

    ECValue intValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(intValueDes, "iProp")) << "Failed to get standard int property value after deserialization";
    ASSERT_EQ(intValue.GetInteger(), intValueDes.GetInteger()) << "Standard Int property value failed to deserialize correctly";

    ECValue myTargetValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetValueDes, "MyTarget")) << "Failed to get ECValue for MyTarget Navigation Property after deserialization";
    ASSERT_FALSE(myTargetValueDes.IsNull()) << "Expected Navigation Property MyTarget to be not null after deserialization but it was";
    ASSERT_TRUE(myTargetValueDes.IsString()) << "Expected Navigation Property to have an ECValue of type string because this is a standalone instance";

    EXPECT_STREQ("IdOfTarget", myTargetValueDes.GetUtf8CP()) << "Value of MyTarget nav property not as expected after deserialization";
    }

END_BENTLEY_ECN_TEST_NAMESPACE

