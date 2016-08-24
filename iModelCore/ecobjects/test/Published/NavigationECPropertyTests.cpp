/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/NavigationECPropertyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct NavigationECPropertyTests : ECTestFixture { };

void CreateNavProp(ECEntityClassP ecClass, Utf8StringCR propName, ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, NavigationECPropertyP& navProp, PrimitiveType type = PRIMITIVETYPE_String)
    {
    ECObjectsStatus status = ecClass->CreateNavigationProperty(navProp, propName, relClass, direction, type);
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

void ValidateRoundTripEC3Serialization(ECSchemaPtr schema, bvector<NavigationECPropertyCP> expectedNavProps)
    {
    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, 3, 0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema with navigation property";

    for (auto const& navProp : expectedNavProps)
        ValidateDeserializedNavProp(deserializedSchema, navProp, navProp->GetClass().GetName(), navProp->GetName());
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
    ECRelationshipClassP relClass2;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->SetAlias("navt");
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass2->GetSource().AddClass(*sourceClass);
    relClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());


    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);
    NavigationECPropertyP navProp2Source;
    CreateNavProp(sourceClass, "MyTarget2", *relClass2, ECRelatedInstanceDirection::Forward, navProp2Source);
    NavigationECPropertyP navProp2Target;
    CreateNavProp(targetClass, "MySource2", *relClass2, ECRelatedInstanceDirection::Backward, navProp2Target);


    bvector<NavigationECPropertyCP> expectedNavProps;
    expectedNavProps.push_back(navPropSource);
    expectedNavProps.push_back(navPropTarget);
    expectedNavProps.push_back(navProp2Source);
    expectedNavProps.push_back(navProp2Target);
    ValidateRoundTripEC3Serialization(schema, expectedNavProps);
    }

TEST_F(NavigationECPropertyTests, NavPropUsingRelationshipWithMultipleConstraintClasses)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP source2Class;
    ECEntityClassP targetClass;
    ECEntityClassP target2Class;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->SetAlias("navt");
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(source2Class, "Source2");
    schema->CreateEntityClass(targetClass, "Target");
    schema->CreateEntityClass(target2Class, "Target2");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().AddClass(*source2Class);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().AddClass(*target2Class);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget2Constraints", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource2Constraints", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    bvector<NavigationECPropertyCP> expectedNavProps;
    expectedNavProps.push_back(navPropSource);
    expectedNavProps.push_back(navPropTarget);
    ValidateRoundTripEC3Serialization(schema, expectedNavProps);
    }

TEST_F(NavigationECPropertyTests, InvalidNavigationProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    // Test wrong direction fails
    NavigationECPropertyP failure;
    ECObjectsStatus status = sourceClass->CreateNavigationProperty(failure, "MyFailure", *relClass, ECRelatedInstanceDirection::Backward);
    ASSERT_NE(ECObjectsStatus::Success, status) << "Expected failure when creating a navigation property with direction wrong";
    }

TEST_F(NavigationECPropertyTests, InvalidXml)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECEntityClassP otherClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP otherRelClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(otherRelClass, "OtherRelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");
    schema->CreateEntityClass(otherClass, "Other");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, 3, 0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    Utf8String invalidSchemaString = schemaString.copy();
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("direction=\"forward\"", "direction=\"backward\"")) << "Failed to replace forward with backward";
    VerifyFailedToDeserialize(invalidSchemaString, "forward direction set to backward");

    invalidSchemaString = schemaString.copy();
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("direction=\"backward\"", "direction=\"forward\"")) << "Failed to replace backward with forward";
    VerifyFailedToDeserialize(invalidSchemaString, "backward direction set to forward");

    invalidSchemaString = schemaString.copy();
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("Class class=\"Source\"", "Class class=\"Other\"")) << "Failed to replace Source class with Other class";
    VerifyFailedToDeserialize(invalidSchemaString, "Relationship source constraint changed to 'Other'");

    invalidSchemaString = schemaString.copy();
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("Class class=\"Target\"", "Class class=\"Other\"")) << "Failed to replace Target class with Other class";
    VerifyFailedToDeserialize(invalidSchemaString, "Relationship target constraint changed to 'Other'");

    invalidSchemaString = schemaString.copy();
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("relationshipName=\"RelClass\"", "relationshipName=\"OtherRelClass\"")) << "Failed to replace RelClass with OtherRelClass";
    VerifyFailedToDeserialize(invalidSchemaString, "Replaced navigation property relationship class 'RelClass' name with 'OtherRelClass'");
    }

TEST_F(NavigationECPropertyTests, RoundtripToEC2Xml)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

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

void VerifyNavPropStringValue(IECInstanceR instance, Utf8CP propertyAccessor, Utf8CP expectedValue)
    {
    ECValueAccessor accessor;
    ECValueAccessor::PopulateValueAccessor(accessor, instance, propertyAccessor);
    ECValue myTarget;
    ASSERT_EQ(ECObjectsStatus::Success, instance.GetValueUsingAccessor(myTarget, accessor)) << "Failed to get ECValue for '" << propertyAccessor << "' Navigation Propertyt";
    ASSERT_FALSE(myTarget.IsNull()) << "Expected Navigation Property '" << propertyAccessor << "' to be not null but it was";
    EXPECT_STREQ(expectedValue, myTarget.GetUtf8CP()) << "Value of '" << propertyAccessor << "' nav property value from instance not as expected";
    }

void TestSettingNavPropStringValues(IECInstanceR instance)
    {
    ECValue myTargetValue("IdOfTarget");
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValue("MyTarget", myTargetValue)) << "Failed to set the value of MyTarget nav prop to a string";
    EXPECT_STREQ("IdOfTarget", myTargetValue.GetUtf8CP()) << "Value of MyTarget nav property not as expected";

    VerifyNavPropStringValue(instance, "MyTarget", "IdOfTarget");

    ECValueAccessor accessor;
    ECValueAccessor::PopulateValueAccessor(accessor, instance, "MyTargetMult[0]");
    ECValue myTargetValueMult("IdOfTarget");
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValueUsingAccessor(accessor, myTargetValueMult)) << "Failed to set the value of MyTargetMult nav prop to a string";
    EXPECT_STREQ("IdOfTarget", myTargetValueMult.GetUtf8CP()) << "Value of MyTargetMult nav property not as expected";

    VerifyNavPropStringValue(instance, "MyTargetMult[0]", "IdOfTarget");
    
    ECValueAccessor::PopulateValueAccessor(accessor, instance, "MyTargetMult[1]");
    ECValue myTargetValueMult1("IdOfTarget1");
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValueUsingAccessor(accessor, myTargetValueMult1)) << "Failed to set the value of MyTargetMult nav prop to a string";
    EXPECT_STREQ("IdOfTarget1", myTargetValueMult1.GetUtf8CP()) << "Value of MyTargetMult nav property not as expected";

    VerifyNavPropStringValue(instance, "MyTargetMult[1]", "IdOfTarget1");
    }

void VerifyNavPropLongValue(IECInstanceR instance, Utf8CP propertyAccessor, int64_t expectedValue)
    {
    ECValueAccessor accessor;
    ECValueAccessor::PopulateValueAccessor(accessor, instance, propertyAccessor);
    ECValue myTarget;
    ASSERT_EQ(ECObjectsStatus::Success, instance.GetValueUsingAccessor(myTarget, accessor)) << "Failed to get ECValue for '" << propertyAccessor << "' Navigation Propertyt";
    ASSERT_FALSE(myTarget.IsNull()) << "Expected Navigation Property '" << propertyAccessor << "' to be not null but it was";
    EXPECT_EQ(expectedValue, myTarget.GetLong()) << "Value of '" << propertyAccessor << "' nav property value from instance not as expected";
    }

void TestSettingNavPropLongValues(IECInstanceR instance)
    {
    ECValue myTargetValue(42LL);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValue("MyTarget", myTargetValue)) << "Failed to set the value of MyTarget nav prop to a long";
    EXPECT_EQ(42LL, myTargetValue.GetLong()) << "Value of MyTarget nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTarget", 42LL);

    ECValueAccessor accessor;
    ECValueAccessor::PopulateValueAccessor(accessor, instance, "MyTargetMult[0]");
    ECValue myTargetValueMult(42LL);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValueUsingAccessor(accessor, myTargetValueMult)) << "Failed to set the value of MyTargetMult nav prop to a long";
    EXPECT_EQ(42LL, myTargetValueMult.GetLong()) << "Value of MyTargetMult nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTargetMult[0]", 42LL);

    ECValueAccessor::PopulateValueAccessor(accessor, instance, "MyTargetMult[1]");
    ECValue myTargetValueMult1(43LL);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValueUsingAccessor(accessor, myTargetValueMult1)) << "Failed to set the value of MyTargetMult1 nav prop to a long";
    EXPECT_EQ(43LL, myTargetValueMult1.GetLong()) << "Value of MyTargetMult1 nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTargetMult[1]", 43LL);
    }

void DeserializeAndVerifyInstanceXml(ECSchemaPtr schema, IECInstanceR sourceInstance, Utf8StringCR instanceXml, PrimitiveType navPropType)
    {
    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext(*schema);
    IECInstancePtr sourceDeserialized;
    InstanceReadStatus readStatus = StandaloneECInstance::ReadFromXmlString(sourceDeserialized, instanceXml.c_str(), *readContext);
    ASSERT_EQ(InstanceReadStatus::Success, readStatus) << "Failed to deserialize an instance with a nav property";

    ECValue stringValue;
    sourceInstance.GetValue(stringValue, "sProp");
    ECValue stringValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceDeserialized->GetValue(stringValueDes, "sProp")) << "Failed to get standard string property value after deserialization";
    ASSERT_STREQ(stringValue.GetUtf8CP(), stringValueDes.GetUtf8CP()) << "Standard String property value failed to deserialize correctly";

    ECValue intValue;
    sourceInstance.GetValue(intValue, "iProp");
    ECValue intValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceDeserialized->GetValue(intValueDes, "iProp")) << "Failed to get standard int property value after deserialization";
    ASSERT_EQ(intValue.GetInteger(), intValueDes.GetInteger()) << "Standard Int property value failed to deserialize correctly";

    if (PrimitiveType::PRIMITIVETYPE_String == navPropType)
        {
        VerifyNavPropStringValue(*sourceDeserialized, "MyTarget", "IdOfTarget");
        VerifyNavPropStringValue(*sourceDeserialized, "MyTargetMult[0]", "IdOfTarget");
        VerifyNavPropStringValue(*sourceDeserialized, "MyTargetMult[1]", "IdOfTarget1");
        }
    else // type is PrimitiveType::PRIMITIVETYPE_Long
        {
        VerifyNavPropLongValue(*sourceDeserialized, "MyTarget", 42LL);
        VerifyNavPropLongValue(*sourceDeserialized, "MyTargetMult[0]", 42LL);
        VerifyNavPropLongValue(*sourceDeserialized, "MyTargetMult[1]", 43LL);
        }
    }

void InstanceWithNavProp(PrimitiveType navPropType)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relClass2;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass2->GetSource().AddClass(*sourceClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource, navPropType);
    NavigationECPropertyP navPropSourceMult;
    CreateNavProp(sourceClass, "MyTargetMult", *relClass2, ECRelatedInstanceDirection::Forward, navPropSourceMult, navPropType);
    PrimitiveECPropertyP stringProp;
    sourceClass->CreatePrimitiveProperty(stringProp, "sProp", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP intProp;
    sourceClass->CreatePrimitiveProperty(intProp, "iProp", PrimitiveType::PRIMITIVETYPE_Integer);

    StandaloneECEnablerPtr enabler = sourceClass->GetDefaultStandaloneEnabler();
    StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();

    ECValue stringValue("SomeValue");
    sourceInstance->SetValue("sProp", stringValue);
    ECValue intValue(42);
    sourceInstance->SetValue("iProp", intValue);

    ECValue myTargetValueFromInst;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetValueFromInst, "MyTarget")) << "Failed to get ECValue for MyTarget Navigation Property";
    ASSERT_TRUE(myTargetValueFromInst.IsNull()) << "Expected Navigation Property MyTarget to be null but it was not";
    ASSERT_EQ(navPropType, myTargetValueFromInst.GetPrimitiveType()) << "Navigation property did not have the expected type";

    if (PrimitiveType::PRIMITIVETYPE_String == navPropType)
        {
        TestSettingNavPropStringValues(*sourceInstance);
        }
    else // type is PrimitiveType::PRIMITIVETYPE_Long
        {
        TestSettingNavPropLongValues(*sourceInstance);
        }

    Utf8String xmlString;
    InstanceWriteStatus writeStatus = sourceInstance->WriteToXmlString(xmlString, true, false);
    ASSERT_EQ(InstanceWriteStatus::Success, writeStatus) << "Failed to serilaize an instance with a nav property";
    DeserializeAndVerifyInstanceXml(schema, *sourceInstance, xmlString, navPropType);

    if (PrimitiveType::PRIMITIVETYPE_Long == navPropType)
        ASSERT_NE(0, xmlString.ReplaceAll("long", "string")) << "Failed to replace 'long' with 'string' in the serialzied xml";
    DeserializeAndVerifyInstanceXml(schema, *sourceInstance, xmlString, navPropType);
    }

TEST_F(NavigationECPropertyTests, InstanceWithNavProp_Long)
    {
    InstanceWithNavProp(PrimitiveType::PRIMITIVETYPE_Long);
    }

TEST_F(NavigationECPropertyTests, InstanceWithNavProp_String)
    {
    InstanceWithNavProp(PrimitiveType::PRIMITIVETYPE_String);
    }
END_BENTLEY_ECN_TEST_NAMESPACE

