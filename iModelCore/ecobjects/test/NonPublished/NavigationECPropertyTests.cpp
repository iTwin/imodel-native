/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECClassLocater : IECClassLocater
{
    ECSchemaCR m_schema;
    ECClassLocater(ECSchemaCR schema) : m_schema(schema) {}

protected:
    ECClassCP _LocateClass(ECClassId const& classId) override
        {
        auto const& ecClasses = m_schema.GetClasses();

        auto const it = std::find_if(std::begin(ecClasses), std::end(ecClasses),
            [&classId](auto const& ecClass)
            {
            return nullptr != ecClass && ecClass->HasId() && ecClass->GetId() == classId;
            });

        if (std::end(ecClasses) == it)
            return nullptr;
        return *it;
        }

    ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override
        {
        return m_schema.LookupClass(className);
        }
};

struct NavigationECPropertyTests : ECTestFixture
{
    void ValidateDeserializedNavProp(ECSchemaPtr deserializedSchema, NavigationECPropertyCP expectedNavProp, Utf8StringCR className, Utf8StringCR propName);
    void ValidateRoundTripEC3Serialization(ECSchemaPtr schema, bvector<NavigationECPropertyCP> expectedNavProps);
    void VerifyFailedToDeserialize(Utf8StringCR invalidSchemaString, Utf8StringCR failureMessage);
};

struct NavigationPropertyValueTests : ECTestFixture
{
    void InstanceWithNavProp();
    void DeserializeAndVerifyInstanceJson(ECSchemaPtr schema, IECInstanceR sourceInstance, JsonValueCR instanceJson);
    void DeserializeAndVerifyInstanceXml(ECSchemaPtr schema, IECInstanceR sourceInstance, Utf8StringCR instanceXml);
    void VerifyInstance(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized);
    void VerifyInstanceJson(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized);
    void VerifyInstanceXml(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized);
    void TestSettingNavPropLongValuesWithId(IECInstanceR instance);
    void TestSettingNavPropLongValuesWithRel(IECInstanceR instance, ECRelationshipClassCP expectedRelClass);
    void VerifyNavPropLongValue(IECInstanceR instance, Utf8CP propertyAccessor, BeInt64Id expectedValue, ECRelationshipClassCP expectedRelClass = nullptr, ECClassId expectedRelClassId = ECClassId());
};

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CreateNavProp(ECEntityClassP ecClass, Utf8StringCR propName, ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, NavigationECPropertyP& navProp)
    {
    ECObjectsStatus status = ecClass->CreateNavigationProperty(navProp, propName, relClass, direction);
    ASSERT_EQ(ECObjectsStatus::Success, status) << "Failed to create navigation property '" << ecClass->GetFullName() << "." << propName << "'.";
    ASSERT_NE(nullptr, navProp) << "Navigation property " << propName << " null though success returned";
    ASSERT_EQ(propName, navProp->GetName()) << "Navigation property " << propName << " does not have expected name";
    ASSERT_EQ(relClass.GetName(), navProp->GetRelationshipClass()->GetName()) << "Navigation property " << propName << " does not have expected relationship";
    ASSERT_EQ(direction, navProp->GetDirection()) << "Navigation property " << propName << " does not have expected direction";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationECPropertyTests::ValidateDeserializedNavProp(ECSchemaPtr deserializedSchema, NavigationECPropertyCP expectedNavProp, Utf8StringCR className, Utf8StringCR propName)
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

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationECPropertyTests::ValidateRoundTripEC3Serialization(ECSchemaPtr schema, bvector<NavigationECPropertyCP> expectedNavProps)
    {
    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, ECVersion::V3_0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema with navigation property";

    for (auto const& navProp : expectedNavProps)
        ValidateDeserializedNavProp(deserializedSchema, navProp, navProp->GetClass().GetName(), navProp->GetName());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationECPropertyTests::VerifyFailedToDeserialize(Utf8StringCR invalidSchemaString, Utf8StringCR failureMessage)
    {
    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, invalidSchemaString.c_str(), *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, readStatus) << "Successfully deserialized schema with error: " << failureMessage;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                               Colin.Kerr                         12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, CreateAndRoundTripNavigationProperty)
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relClass2;
    ECRelationshipClassP relClass3;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->SetAlias("navt");
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");
    schema->CreateRelationshipClass(relClass3, "RelClass3");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass2->GetSource().AddClass(*sourceClass);
    relClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass3->GetSource().AddClass(*sourceClass);
    relClass3->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass3->GetTarget().AddClass(*targetClass);
    relClass3->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);
    NavigationECPropertyP navProp2Source;
    CreateNavProp(sourceClass, "MyTarget2", *relClass2, ECRelatedInstanceDirection::Forward, navProp2Source);
    NavigationECPropertyP navProp3Target;
    CreateNavProp(targetClass, "MySource3", *relClass3, ECRelatedInstanceDirection::Backward, navProp3Target);

    bvector<NavigationECPropertyCP> expectedNavProps;
    expectedNavProps.push_back(navPropSource);
    expectedNavProps.push_back(navPropTarget);
    expectedNavProps.push_back(navProp2Source);
    expectedNavProps.push_back(navProp3Target);
    ValidateRoundTripEC3Serialization(schema, expectedNavProps);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Colin.Kerr                         12/2015
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
//@bsimethod                                Colin.Kerr                         12/2015
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, InvalidReferencedRelationshipWithBaseClasses)
    {
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP derivedRelClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(derivedRelClass, "DerivedRelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    derivedRelClass->GetSource().AddClass(*sourceClass);
    derivedRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    derivedRelClass->GetTarget().AddClass(*targetClass);
    derivedRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());
    derivedRelClass->AddBaseClass(*relClass);

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, sourceClass->CreateNavigationProperty(navProp, "BadNavProp", *derivedRelClass, ECRelatedInstanceDirection::Forward));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Source">
                <NavigationECProperty propertyName="targetValue" relationshipName="DerivedRelClass"/>
            </ECEntityClass>
            <ECEntityClass typeName="Target"/>
            <ECRelationshipClass typeName="DerivedRelClass" strength="Referencing" strengthDirection="forward">
                <BaseClass>RelClass</BaseClass>
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="source">
                    <Class class="Source"/>
                </Source>
                <Target multiplicity="(1..1)" polymorphic="true" roleLabel="target">
                    <Class class="Target"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelClass" strength="Referencing" strengthDirection="forward">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="source">
                    <Class class="Source"/>
                </Source>
                <Target multiplicity="(1..1)" polymorphic="true" roleLabel="target">
                    <Class class="Target"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, InvalidOverrideWithDifferentRelationshipClass)
    {
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECEntityClassP derivedSource;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relClass2;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");

    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");

    schema->CreateEntityClass(derivedSource, "DerivedSource");
    derivedSource->AddBaseClass(*sourceClass);

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass2->GetSource().AddClass(*derivedSource);
    relClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    CreateNavProp(sourceClass, "NavProp", *relClass, ECRelatedInstanceDirection::Forward, navProp);

    NavigationECPropertyP derivedNavProp;
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, derivedSource->CreateNavigationProperty(derivedNavProp, "NavProp", *relClass2, ECRelatedInstanceDirection::Forward))
        << "The navigation property should fail to be created because the overriding navigation property has a different relationship than the overridden property.";
    }

    {
    ECSchemaPtr schema;
    ECEntityClassP baseClass;

    ECEntityClassP sourceClass;
    ECEntityClassP derivedSource;
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relClass2;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");
    schema->CreateEntityClass(baseClass, "BaseClass");
    schema->CreateEntityClass(targetClass, "Target");

    schema->CreateEntityClass(sourceClass, "Source");
    sourceClass->AddBaseClass(*baseClass);
    schema->CreateEntityClass(derivedSource, "DerivedSource");
    derivedSource->AddBaseClass(*baseClass);

    relClass->GetSource().SetAbstractConstraint(*baseClass);
    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    relClass2->GetSource().AddClass(*derivedSource);
    relClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    CreateNavProp(sourceClass, "NavProp", *relClass, ECRelatedInstanceDirection::Forward, navProp);

    NavigationECPropertyP derivedNavProp;
    CreateNavProp(derivedSource, "NavProp", *relClass2, ECRelatedInstanceDirection::Forward, derivedNavProp);

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, derivedSource->AddBaseClass(*sourceClass)) << "The base class should fail to be added because the derived class is changing the relationship.";
    }

    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECEntityClassP derivedSource;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");
    schema->CreateEntityClass(derivedSource, "DerivedSource");
    derivedSource->AddBaseClass(*sourceClass);

    relClass->GetSource().SetAbstractConstraint(*sourceClass);
    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().AddClass(*derivedSource);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    CreateNavProp(sourceClass, "NavProp", *relClass, ECRelatedInstanceDirection::Forward, navProp);

    NavigationECPropertyP derivedNavProp;
    EXPECT_EQ(ECObjectsStatus::Success, derivedSource->CreateNavigationProperty(derivedNavProp, "NavProp", *relClass, ECRelatedInstanceDirection::Forward))
        << "The navigation property should be successfully created because the overriding navigation property has the same relationship as the overridden property.";
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="SourceClass" >
                <ECNavigationProperty propertyName="NavProp" relationshipName="RelClass" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="DerivedSource">
                <BaseClass>SourceClass</BaseClass>
                <ECNavigationProperty propertyName="NavProp" relationshipName="DerivedRelClass" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" />
            <ECRelationshipClass typeName="RelClass" strength="Referencing" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass"/>
                </Source>
                <Target multiplicity="(1..1)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelClass2" strength="Referencing" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass"/>
                </Source>
                <Target multiplicity="(1..1)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "The schema should fail to deserialize because the overriding navigation property has a different relationship than the overridden property.";
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="SourceClass" >
                <ECNavigationProperty propertyName="NavProp" relationshipName="RelClass" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="DerivedSource">
                <BaseClass>SourceClass</BaseClass>
                <ECNavigationProperty propertyName="NavProp" relationshipName="RelClass" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" />
            <ECRelationshipClass typeName="RelClass" strength="Referencing" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="source" polymorphic="true" abstractConstraint="SourceClass">
                    <Class class="SourceClass"/>
                    <Class class="DerivedSource"/>
                </Source>
                <Target multiplicity="(1..1)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context)) << "The schema should successfully deserialize because the overriding navigation property has the same relationship as the overridden property.";
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, MustPointToSingularEndpoint)
    {
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, source->CreateNavigationProperty(navProp, "BadNavProp", *relClass, ECRelatedInstanceDirection::Forward));
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 1);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, target->CreateNavigationProperty(navProp, "BadNavProp", *relClass, ECRelatedInstanceDirection::Backward));
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 2);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::Success, source->CreateNavigationProperty(navProp, "GoodNavProp", *relClass, ECRelatedInstanceDirection::Forward));
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 3);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::Success, target->CreateNavigationProperty(navProp, "GoodNavProp", *relClass, ECRelatedInstanceDirection::Backward));
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, MustBeAddedToConcreteConstraintClass)
    {
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP derivedFromSource;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(derivedFromSource, "DerivedFromSource");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    derivedFromSource->AddBaseClass(*source);

    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, derivedFromSource->CreateNavigationProperty(navProp, "BadNavProp", *relClass, ECRelatedInstanceDirection::Forward));
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP derivedFromSource;
    ECEntityClassP target;
    ECRelationshipClassP relClass;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(derivedFromSource, "DerivedFromSource");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship");

    derivedFromSource->AddBaseClass(*source);

    relClass->GetSource().SetAbstractConstraint(*source);
    relClass->GetSource().AddClass(*source);
    relClass->GetSource().AddClass(*derivedFromSource);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::Success, derivedFromSource->CreateNavigationProperty(navProp, "BadNavProp", *relClass, ECRelatedInstanceDirection::Forward));
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationECPropertyTests, EndpointsWithRelationshipsCannotBeUsed)
    {
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relWithRelEndpoint;
    NavigationECPropertyP navProp;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship", *source, "Source", *target, "Target");

    schema->CreateRelationshipClass(relWithRelEndpoint, "RelWithRelEndpoint");
    relWithRelEndpoint->GetSource().SetRoleLabel("Source");
    relWithRelEndpoint->GetSource().AddClass(*source);
    relWithRelEndpoint->GetTarget().SetRoleLabel("Target");
    relWithRelEndpoint->GetTarget().AddClass(*relClass);

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, source->CreateNavigationProperty(navProp, "BadNavProp", *relWithRelEndpoint, ECRelatedInstanceDirection::Forward));
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP source;
    ECEntityClassP target;
    ECRelationshipClassP relClass;
    ECRelationshipClassP relWithRelEndpoint;
    NavigationECPropertyP navProp;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(source, "Source");
    schema->CreateEntityClass(target, "Target");
    schema->CreateRelationshipClass(relClass, "TestRelationship", *source, "Source", *target, "Target");

    schema->CreateRelationshipClass(relWithRelEndpoint, "RelWithRelEndpoint");
    relWithRelEndpoint->SetStrengthDirection(ECRelatedInstanceDirection::Backward);
    relWithRelEndpoint->GetSource().SetRoleLabel("Source");
    relWithRelEndpoint->GetSource().AddClass(*relClass);
    relWithRelEndpoint->GetTarget().SetRoleLabel("Target");
    relWithRelEndpoint->GetTarget().AddClass(*source);

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, source->CreateNavigationProperty(navProp, "BadNavProp", *relWithRelEndpoint, ECRelatedInstanceDirection::Forward));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NavigationECPropertyTests, RelationshipPointsToEntity)
    {
    ECSchemaPtr schema;
    ECEntityClassP classA;
    ECEntityClassP classB;
    ECEntityClassP navPropEndpoint;
    ECRelationshipClassP relAToB;
    ECRelationshipClassP relAToBRelatesToNavPropEndpoint;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 1, 0, 0);
    schema->CreateEntityClass(classA, "Source");
    schema->CreateEntityClass(classB, "Target");
    schema->CreateRelationshipClass(relAToB, "relAToB", *classA, "Source", *classB, "Target");

    schema->CreateEntityClass(navPropEndpoint, "NavPropEndpoint");
    schema->CreateRelationshipClass(relAToBRelatesToNavPropEndpoint, "AtoBRelatesToNavPropEndpoint");
    relAToBRelatesToNavPropEndpoint->GetSource().AddClass(*relAToB);
    relAToBRelatesToNavPropEndpoint->GetSource().SetRoleLabel("Source");
    relAToBRelatesToNavPropEndpoint->GetTarget().AddClass(*navPropEndpoint);
    relAToBRelatesToNavPropEndpoint->GetTarget().SetRoleLabel("Target");

    NavigationECPropertyP navProp;
    EXPECT_EQ(ECObjectsStatus::Success, relAToB->CreateNavigationProperty(navProp, "Test", *relAToBRelatesToNavPropEndpoint, ECRelatedInstanceDirection::Forward));

    bvector<NavigationECPropertyCP> expectedNavProps;
    expectedNavProps.push_back(navProp);
    ValidateRoundTripEC3Serialization(schema, expectedNavProps);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Colin.Kerr                         12/2015
//+---------------+---------------+---------------+---------------+---------------+------
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
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, ECVersion::V3_0);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with navigation property";

    Utf8String invalidSchemaString = schemaString;
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("direction=\"forward\"", "direction=\"backward\"")) << "Failed to replace forward with backward";
    VerifyFailedToDeserialize(invalidSchemaString, "forward direction set to backward");

    invalidSchemaString = schemaString;
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("direction=\"backward\"", "direction=\"forward\"")) << "Failed to replace backward with forward";
    VerifyFailedToDeserialize(invalidSchemaString, "backward direction set to forward");

    invalidSchemaString = schemaString;
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("Class class=\"Source\"", "Class class=\"Other\"")) << "Failed to replace Source class with Other class";
    VerifyFailedToDeserialize(invalidSchemaString, "Relationship source constraint changed to 'Other'");

    invalidSchemaString = schemaString;
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("Class class=\"Target\"", "Class class=\"Other\"")) << "Failed to replace Target class with Other class";
    VerifyFailedToDeserialize(invalidSchemaString, "Relationship target constraint changed to 'Other'");

    invalidSchemaString = schemaString;
    ASSERT_NE(0, invalidSchemaString.ReplaceAll("relationshipName=\"RelClass\"", "relationshipName=\"OtherRelClass\"")) << "Failed to replace RelClass with OtherRelClass";
    VerifyFailedToDeserialize(invalidSchemaString, "Replaced navigation property relationship class 'RelClass' name with 'OtherRelClass'");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Colin.Kerr                         12/2015
//+---------------+---------------+---------------+---------------+---------------+------
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
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropTarget;
    CreateNavProp(targetClass, "MySource", *relClass, ECRelatedInstanceDirection::Backward, navPropTarget);

    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, ECVersion::V2_0);
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
    ASSERT_EQ("long", navPropSourceDeserialized->GetTypeName()) << "Expected navigation property to be of type string after roundtripped to ECXml 2.0";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::VerifyNavPropLongValue(IECInstanceR instance, Utf8CP propertyAccessor, BeInt64Id expectedValue, ECRelationshipClassCP expectedRelClass, ECClassId expectedRelClassId)
    {
    ECValueAccessor accessor;
    ECValueAccessor::PopulateValueAccessor(accessor, instance, propertyAccessor);
    ECValue myTarget;
    ASSERT_EQ(ECObjectsStatus::Success, instance.GetValueUsingAccessor(myTarget, accessor)) << "Failed to get ECValue for '" << propertyAccessor << "' Navigation Property";
    ASSERT_FALSE(myTarget.IsNull()) << "Expected Navigation Property '" << propertyAccessor << "' to be not null but it was";
    EXPECT_TRUE(myTarget.IsNavigation()) << "Expected Navigation Property '" << propertyAccessor << "' to be ValueKind Navigation but it was not.";
    EXPECT_EQ(expectedValue.GetValue(), myTarget.GetNavigationInfo().GetId<BeInt64Id>().GetValue()) << "Value of '" << propertyAccessor << "' nav property value from instance not as expected";

    if (!expectedRelClassId.IsValid())
        {
        ECRelationshipClassCP relClass = myTarget.GetNavigationInfo().GetRelationshipClass();

        if (nullptr != expectedRelClass)
            {
            ASSERT_TRUE(nullptr != relClass) << "The relationship for '" << propertyAccessor << "' is null when it should not be.";
            EXPECT_EQ(expectedRelClass->GetId(), myTarget.GetNavigationInfo().GetRelationshipClassId()) << "The relationship class id of '" << propertyAccessor << "' is different than expected.";
            EXPECT_TRUE(ECClass::ClassesAreEqualByName(expectedRelClass, relClass)) << "The relationship for '" << propertyAccessor << "' was not the expected relationship.";
            }
        else
            {
            EXPECT_EQ(expectedRelClass, relClass) << "The relationship class of '" << propertyAccessor << "' is not as expected.";
            EXPECT_FALSE(myTarget.GetNavigationInfo().GetRelationshipClassId().IsValid()) << "The relationship class id of '" << propertyAccessor << "' should be invalid but is valid.";
            }
        }
    else
        {
        EXPECT_EQ(nullptr, myTarget.GetNavigationInfo().GetRelationshipClass()) << "The relationship class pointer of '" << propertyAccessor << "' was not nullptr but should be.";
        EXPECT_EQ(expectedRelClassId, myTarget.GetNavigationInfo().GetRelationshipClassId()) << "The relationship class id for '" << propertyAccessor << "' was not the expected value.";
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::TestSettingNavPropLongValuesWithRel(IECInstanceR instance, ECRelationshipClassCP expectedRelClass)
    {
    BeInt64Id navId(42);
    ECValue myTargetValue(navId, expectedRelClass);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValue("MyTarget", myTargetValue)) << "Failed to set the value of MyTarget nav prop to a long";
    EXPECT_EQ(navId, myTargetValue.GetNavigationInfo().GetId<BeInt64Id>()) << "Id value of MyTarget nav property not as expected";
    EXPECT_EQ(expectedRelClass, myTargetValue.GetNavigationInfo().GetRelationshipClass()) << "Relationship Class of MyTarget nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTarget", navId, expectedRelClass);

    BeInt64Id noRelNavId (50);
    ECValue myTargetNoRel;
    myTargetNoRel.SetNavigationInfo(noRelNavId);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValue("MyTargetNoRel", myTargetNoRel)) << "Failed to set the value of MyTargetNoRel nav prop to a long";
    EXPECT_EQ(noRelNavId, myTargetNoRel.GetNavigationInfo().GetId<BeInt64Id>()) << "Id value of MyTargetNoRel nav property not as expected";
    EXPECT_EQ(nullptr, myTargetNoRel.GetNavigationInfo().GetRelationshipClass()) << "Relationship Class of MyTargetNoRel nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTargetNoRel", noRelNavId, nullptr);

    /*
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
    */
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::TestSettingNavPropLongValuesWithId(IECInstanceR instance)
    {
    BeInt64Id navId(42);
    ECClassId relClassId((uint64_t) 25);
    ECValue myTargetValue;
    myTargetValue.SetNavigationInfo(navId, relClassId);
    ASSERT_EQ(ECObjectsStatus::Success, instance.SetValue("MyTargetWithRelId", myTargetValue)) << "Failed to set the value of MyTargetWithRelId nav prop to a long";
    EXPECT_EQ(navId, myTargetValue.GetNavigationInfo().GetId<BeInt64Id>()) << "Id value of MyTargetWithRelId nav property not as expected";
    EXPECT_TRUE(myTargetValue.GetNavigationInfo().GetRelationshipClassId().IsValid());
    EXPECT_EQ(relClassId, myTargetValue.GetNavigationInfo().GetRelationshipClassId()) << "Relationship Class Id of MyTargetWithRelId nav property not as expected";

    VerifyNavPropLongValue(instance, "MyTargetWithRelId", navId, nullptr, relClassId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::VerifyInstance(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized)
    {
    ECValue intValue;
    sourceInstance.GetValue(intValue, "iProp");
    ECValue intValueDes;
    ASSERT_EQ(ECObjectsStatus::Success, sourceDeserialized.GetValue(intValueDes, "iProp")) << "Failed to get standard int property value after deserialization";
    ASSERT_EQ(intValue.GetInteger(), intValueDes.GetInteger()) << "Standard Int property value failed to deserialize correctly";

    VerifyNavPropLongValue(sourceDeserialized, "MyTargetNoRel", BeInt64Id(50), nullptr);
    VerifyNavPropLongValue(sourceDeserialized, "MyTargetWithRelId", BeInt64Id(42), nullptr, ECClassId((uint64_t) 25));
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::VerifyInstanceXml(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized)
    {
    VerifyInstance(schema, sourceInstance, sourceDeserialized);
    VerifyNavPropLongValue(sourceDeserialized, "MyTarget", BeInt64Id(42), schema->GetClassCP("DerivedRelClass")->GetRelationshipClassCP());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::VerifyInstanceJson(ECSchemaPtr schema, IECInstanceR sourceInstance, IECInstanceR sourceDeserialized)
    {
    VerifyInstance(schema, sourceInstance, sourceDeserialized);
    VerifyNavPropLongValue(sourceDeserialized, "MyTarget", BeInt64Id(42), nullptr, ECClassId((uint64_t) 25));
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::DeserializeAndVerifyInstanceXml(ECSchemaPtr schema, IECInstanceR sourceInstance, Utf8StringCR instanceXml)
    {
    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext(*schema);
    IECInstancePtr sourceDeserialized;
    InstanceReadStatus readStatus = StandaloneECInstance::ReadFromXmlString(sourceDeserialized, instanceXml.c_str(), *readContext);
    ASSERT_EQ(InstanceReadStatus::Success, readStatus) << "Failed to deserialize an instance with a nav property";

    VerifyInstanceXml(schema, sourceInstance, *sourceDeserialized);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::DeserializeAndVerifyInstanceJson(ECSchemaPtr schema, IECInstanceR sourceInstance, JsonValueCR instanceJson)
    {
    IECInstancePtr sourceDeserialized = sourceInstance.GetClass().GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(sourceDeserialized.IsValid());
    ECClassLocater classLocater(*schema);
    BentleyStatus readStatus = JsonECInstanceConverter::JsonToECInstance(*sourceDeserialized, instanceJson, classLocater);
    ASSERT_EQ(BentleyStatus::SUCCESS, readStatus);

    VerifyInstanceJson(schema, sourceInstance, *sourceDeserialized);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyValueTests::InstanceWithNavProp()
    {
    ECSchemaPtr schema;
    ECEntityClassP sourceClass;
    ECEntityClassP targetClass;
    ECEntityClassP derivedTargetClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP derivedRelClass;
    ECRelationshipClassP relClass2;

    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(derivedRelClass, "DerivedRelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");
    schema->CreateEntityClass(sourceClass, "Source");
    schema->CreateEntityClass(targetClass, "Target");
    schema->CreateEntityClass(derivedTargetClass, "DerivedTarget");

    derivedTargetClass->AddBaseClass(*targetClass);

    relClass->GetSource().AddClass(*sourceClass);
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    derivedRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    derivedRelClass->GetSource().AddClass(*sourceClass);
    derivedRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());
    derivedRelClass->GetTarget().AddClass(*derivedTargetClass);
    derivedRelClass->AddBaseClass(*relClass);
    derivedRelClass->SetId(ECClassId((uint64_t) 25));

    relClass2->GetSource().AddClass(*sourceClass);
    relClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relClass2->GetTarget().AddClass(*targetClass);
    relClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);
    NavigationECPropertyP navPropSourceNoRel;
    CreateNavProp(sourceClass, "MyTargetNoRel", *relClass, ECRelatedInstanceDirection::Forward, navPropSourceNoRel);
    NavigationECPropertyP navPropSourceRelId;
    CreateNavProp(sourceClass, "MyTargetWithRelId", *relClass, ECRelatedInstanceDirection::Forward, navPropSourceRelId);
    PrimitiveECPropertyP intProp;
    sourceClass->CreatePrimitiveProperty(intProp, "iProp", PrimitiveType::PRIMITIVETYPE_Integer);

    StandaloneECEnablerPtr enabler = sourceClass->GetDefaultStandaloneEnabler();
    StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();

    ECValue intValue(42);
    sourceInstance->SetValue("iProp", intValue);

    ECValue myTargetValueFromInst;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetValueFromInst, "MyTarget")) << "Failed to get ECValue for MyTarget Navigation Property";
    ASSERT_TRUE(myTargetValueFromInst.IsNull()) << "Expected Navigation Property MyTarget to be null but it was not";
    ASSERT_TRUE(myTargetValueFromInst.IsNavigation()) << "Expected the value to be a navigation type but it was not";

    ECValue myTargetNoRelValueFromInst;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetNoRelValueFromInst, "MyTargetNoRel")) << "Failed to get ECValue for MyTargetNoRel Navigation Property";
    ASSERT_TRUE(myTargetNoRelValueFromInst.IsNull()) << "Expected Navigation Property MyTargetNoRel to be null but it was not";
    ASSERT_TRUE(myTargetNoRelValueFromInst.IsNavigation()) << "Expected the value to be a navigation type but it was not";

    ECValue myTargetRelIdValueFromInst;
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->GetValue(myTargetRelIdValueFromInst, "MyTargetWithRelId")) << "Failed to get ECValue for MyTargetWithRelId Navigation Property";
    ASSERT_TRUE(myTargetRelIdValueFromInst.IsNull()) << "Expected Navigation Property MyTargetWithRelId to be null but it was not";
    ASSERT_TRUE(myTargetRelIdValueFromInst.IsNavigation()) << "Expected the value to be a navigation type but it was not";

    TestSettingNavPropLongValuesWithRel(*sourceInstance, derivedRelClass);
    TestSettingNavPropLongValuesWithId(*sourceInstance);

    Utf8String xmlString;
    InstanceWriteStatus writeStatus = sourceInstance->WriteToXmlString(xmlString, true, false);
    ASSERT_EQ(InstanceWriteStatus::Success, writeStatus) << "Failed to serilaize an instance to xml with a nav property";
    DeserializeAndVerifyInstanceXml(schema, *sourceInstance, xmlString);

    Json::Value jsonValue;
    ECClassLocater classLocator(*schema.get());
    StatusInt jsonWriteStatus = JsonEcInstanceWriter::WriteInstanceToJson(jsonValue, *sourceInstance, "Source", true, false, &classLocator);
    ASSERT_EQ(0, jsonWriteStatus) << "Failed to serialize an instance to Json with a nav property";
    DeserializeAndVerifyInstanceJson(schema, *sourceInstance, jsonValue["Source"]);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Colin.Kerr                         01/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationPropertyValueTests, InstanceWithNavProp_Long)
    {
    InstanceWithNavProp();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         11/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationPropertyValueTests, ValueCopyTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    ECRelationshipClassP relClass;
    schema->CreateRelationshipClass(relClass, "RelClass");

    BeInt64Id navId(42);

    ECValue v;
    v = ECValue(navId, relClass);

    ASSERT_FALSE(v.IsNull()) << "The value is null after the copy when it shouldn't be.";
    ASSERT_TRUE(v.IsNavigation()) << "The value is not a navigation";
    EXPECT_EQ(navId, v.GetNavigationInfo().GetId<BeInt64Id>()) << "Value did not have expected value";
    EXPECT_STREQ(relClass->GetName().c_str(), v.GetNavigationInfo().GetRelationshipClass()->GetName().c_str()) << "Value did not have the expected relationship";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationPropertyValueTests, TestNavigationValueEquality)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "NavTest", "ts", 4, 0, 2);
    ECRelationshipClassP relClass;
    ECRelationshipClassP relClass2;
    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateRelationshipClass(relClass2, "RelClass2");

    BeInt64Id navId(42);
    ECClassId relClassId((uint64_t) 10);
    relClass->SetId(relClassId);

    {
    ECValue v;
    v.SetNavigationInfo(navId, relClass);

    ECValue v2(navId, relClass);
    EXPECT_TRUE(v.Equals(v2)) << "Should not have failed with navigation id and relationship class set with valid. ";
    }

    {
    ECValue v;
    v.SetNavigationInfo(navId, relClass->GetId());

    ECValue v2 (navId, relClassId);
    EXPECT_TRUE(v.Equals(v2)) << "Should not have failed with navigation id and relationship class id set. Relationship class is nullptr";
    }

    {
    ECValue v;
    v.SetNavigationInfo(navId);

    ECValue v2(navId);
    EXPECT_TRUE(v.Equals(v2)) << "Should not have failed with navigation id set and relationship class nullptr";
    }

    {
    ECValue v;
    v.SetNavigationInfo(navId, relClass2);

    ECValue v2(navId, relClass2);
    EXPECT_TRUE(v.Equals(v2)) << "Should not have failed with navigation id and relationship class set with no class id set.";
    }

    {
    ECValue v(navId, relClass);
    ECValue v2(navId, relClass2);

    EXPECT_FALSE(v.Equals(v2)) << "Should not be equal because the relationships used to set the value are different.";
    }

    {
    BeInt64Id diffNavId(15);
    ECValue v(navId, relClass);
    ECValue v2(diffNavId, relClass);

    EXPECT_FALSE(v.Equals(v2)) << "Should not be equal because the navigation values are different.";
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Caleb.Shafer                         12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(NavigationPropertyValueTests, TestNullNavigationValue)
    {
    ECValue v;
    EXPECT_TRUE(v.IsNull()) << "The value should be null but it is not.";
    EXPECT_FALSE(v.IsNavigation()) << "The value should not be a navigation value but it is.";

    v.SetNavigationInfo();
    EXPECT_TRUE(v.IsNull()) << "The value should be null but it is not";
    EXPECT_TRUE(v.IsNavigation()) << "The value should be a navigation value but it is not";
    EXPECT_FALSE(v.GetNavigationInfo().GetId<BeInt64Id>().IsValid()) << "The navigation value should be invalid but it is not";
    EXPECT_FALSE(v.GetNavigationInfo().GetRelationshipClassId().IsValid()) << "The relationship class id should be invalid but it is not.";
    EXPECT_EQ(nullptr, v.GetNavigationInfo().GetRelationshipClass()) << "The relationship class should be null but it is not.";

    BeInt64Id navId(12);
    v.SetNavigationInfo(navId);
    EXPECT_FALSE(v.IsNull()) << "The value should not be null but it is ";
    EXPECT_TRUE(v.IsNavigation()) << "The value should be a navigation value but it is not";
    EXPECT_EQ(navId, v.GetNavigationInfo().GetId<BeInt64Id>()) << "The value was not as expected";

    v.Clear();
    EXPECT_TRUE(v.IsNull()) << "The value should be null but it is not.";
    EXPECT_FALSE(v.IsNavigation()) << "The value should not be a navigation value but it is.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   08/2017
//---------------------------------------------------------------------------------------
TEST_F(NavigationPropertyValueTests, JsonRelatedInstanceIdSerialization)
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
    relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relClass->GetTarget().AddClass(*targetClass);
    relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());

    NavigationECPropertyP navPropSource;
    CreateNavProp(sourceClass, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navPropSource);

    StandaloneECEnablerPtr enabler = sourceClass->GetDefaultStandaloneEnabler();
    StandaloneECInstancePtr sourceInstance = enabler->CreateInstance();

    BeInt64Id navId(50);
    ECValue myTarget;
    myTarget.SetNavigationInfo(navId);
    ASSERT_EQ(ECObjectsStatus::Success, sourceInstance->SetValue("MyTarget", myTarget)) << "Failed to set the value of MyTargetNoRel nav prop to a long";
    EXPECT_EQ(navId, myTarget.GetNavigationInfo().GetId<BeInt64Id>()) << "Id value of MyTargetNoRel nav property not as expected";
    EXPECT_EQ(nullptr, myTarget.GetNavigationInfo().GetRelationshipClass()) << "Relationship Class of MyTargetNoRel nav property not as expected";

    Json::Value jsonValue;
    StatusInt jsonWriteStatus = JsonEcInstanceWriter::WriteInstanceToJson(jsonValue, *sourceInstance, "Source", true);
    ASSERT_EQ(0, jsonWriteStatus) << "Failed to serialize an instance to Json with a nav property";

    ASSERT_TRUE(jsonValue.isObject());
    ASSERT_TRUE(jsonValue["Source"].isObject());
    ASSERT_TRUE(jsonValue["Source"]["MyTarget"].isObject());
    EXPECT_STREQ(navId.ToHexStr().c_str(), jsonValue["Source"]["MyTarget"][ECJsonUtilities::json_navId()].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Victor.Cushman     02/2018
//---------------------------------------------------------------------------------------
TEST_F(NavigationPropertyValueTests, TestRelClassNameJsonNotSerializedForRelationshipClassesWithSealedModifier)
    {
    Utf8CP const sourceStr = "Source";
    Utf8CP const targetStr = "Target";
    Utf8CP const navPropStr = "NavProp";

    static auto const ValidateTestNavProp = [&](ECEntityClassP const& sourceClass, ECRelationshipClassP const& navPropRelClass, bool shouldRelClassNameBeSerialized) -> void
        {
        StandaloneECInstancePtr instance = sourceClass->GetDefaultStandaloneEnabler()->CreateInstance();
        BeInt64Id navId(42);
        ECValue navPropValue;
        navPropValue.SetNavigationInfo(navId, navPropRelClass);
        EC_ASSERT_SUCCESS(instance->SetValue(navPropStr, navPropValue));
        EC_ASSERT_SUCCESS(instance->GetValue(navPropValue, navPropStr));
        ASSERT_FALSE(navPropValue.IsNull());
        ASSERT_TRUE(navPropValue.IsNavigation());

        Json::Value json;
        ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(json, *instance, "Source", true));
        ASSERT_TRUE(json.isObject());
        ASSERT_TRUE(json[sourceStr].isObject());
        ASSERT_TRUE(json[sourceStr][navPropStr].isObject());
        auto const& membs = json[sourceStr][navPropStr].getMemberNames();
        EXPECT_TRUE((membs.end() == std::find_if(
            membs.begin(),
            membs.end(),
            [](Utf8StringCR _) -> bool {return _ == ECJsonSystemNames::Navigation::RelClassName();}
        )) == !shouldRelClassNameBeSerialized);
        };

    static auto const ValidateShouldRelClassNameSerialized_NonInherited = [&](ECClassModifier modifier, bool shouldRelClassNameBeSerialized) -> void
        {
        ECSchemaPtr schema;
        ECEntityClassP sourceClass;
        ECEntityClassP targetClass;
        ECRelationshipClassP relClass;
        NavigationECPropertyP navProp;

        ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::Latest);
        EC_ASSERT_SUCCESS(schema->CreateEntityClass(sourceClass, sourceStr));
        EC_ASSERT_SUCCESS(schema->CreateEntityClass(targetClass, targetStr));
        EC_ASSERT_SUCCESS(schema->CreateRelationshipClass(relClass, "TestRelClass", *sourceClass, sourceStr, *targetClass, targetStr));
        relClass->SetClassModifier(modifier);
        relClass->SetId(ECClassId((uint64_t)101));
        CreateNavProp(sourceClass, navPropStr, *relClass, ECRelatedInstanceDirection::Forward, navProp);
        ValidateTestNavProp(sourceClass, relClass, shouldRelClassNameBeSerialized);
        };

    static auto const ValidateShouldRelClassNameSerialized_Inherited = [&](ECClassModifier childModifier, bool shouldRelClassNameBeSerialized) -> void
        {
        ECSchemaPtr schema;
        ECEntityClassP sourceClass;
        ECEntityClassP targetClass;
        ECRelationshipClassP parentRelClass;
        ECRelationshipClassP childRelClass;
        NavigationECPropertyP navProp;

        ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::Latest);
        EC_ASSERT_SUCCESS(schema->CreateEntityClass(sourceClass, sourceStr));
        EC_ASSERT_SUCCESS(schema->CreateEntityClass(targetClass, targetStr));
        EC_ASSERT_SUCCESS(schema->CreateRelationshipClass(parentRelClass, "TestParentRelClass", *sourceClass, sourceStr, *targetClass, targetStr));
        parentRelClass->SetClassModifier(ECClassModifier::Abstract);
        parentRelClass->SetId(ECClassId((uint64_t)101));
        EC_ASSERT_SUCCESS(schema->CreateRelationshipClass(childRelClass, "TestChildRelClass", *sourceClass, sourceStr, *targetClass, targetStr));
        childRelClass->SetClassModifier(childModifier);
        childRelClass->SetId(ECClassId((uint64_t)102));
        childRelClass->AddBaseClass(*parentRelClass);
        CreateNavProp(sourceClass, navPropStr, *parentRelClass, ECRelatedInstanceDirection::Forward, navProp);
        ValidateTestNavProp(sourceClass, parentRelClass, shouldRelClassNameBeSerialized);
        };

    // Test Non Inherited Relationship Classes.
    ValidateShouldRelClassNameSerialized_NonInherited(ECClassModifier::None, true);
    ValidateShouldRelClassNameSerialized_NonInherited(ECClassModifier::Abstract, true);
    ValidateShouldRelClassNameSerialized_NonInherited(ECClassModifier::Sealed, false);

    // Test Relationship Class Deriving from another relationship class.
    ValidateShouldRelClassNameSerialized_Inherited(ECClassModifier::None, true);
    ValidateShouldRelClassNameSerialized_Inherited(ECClassModifier::Abstract, true);
    ValidateShouldRelClassNameSerialized_Inherited(ECClassModifier::Sealed, true);

    }

END_BENTLEY_ECN_TEST_NAMESPACE
