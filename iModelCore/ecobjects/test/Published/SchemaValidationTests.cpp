/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaValidationTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//---------------------------------------------------------------------------------------//
// Tests that strict schema rules are complied with
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
struct SchemaValidationTests : ECTestFixture
    {
    StandaloneECEnablerP    m_customAttributeEnabler;
    ECSchemaPtr             m_schema;

    ECClassCP           CreateClass(Utf8CP className, bool hasInstanceLabelAttribute, Utf8CP instanceLabelPropertyName)
        {
        ECEntityClassP ecClass;
        m_schema->CreateEntityClass(ecClass, className);

        PrimitiveECPropertyP prop;
        ecClass->CreatePrimitiveProperty(prop, instanceLabelPropertyName, PRIMITIVETYPE_String);

        if (hasInstanceLabelAttribute)
            {
            IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance();
            ECValue v;
            if (instanceLabelPropertyName)
                v.SetUtf8CP(instanceLabelPropertyName, false);

            labelAttr->SetValue("PropertyName", v);
            ecClass->SetCustomAttribute(*labelAttr);
            }

        return ecClass;
        }

    static Utf8String    GetTestSchemaXMLString()
        {
        Utf8Char schemaFragment[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"ExtendedTypeTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"ClassA\" displayLabel=\"Class B\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"knownExtendedType\" extendedTypeName=\"color\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"unknownExtendedType\" extendedTypeName=\"banana\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"withoutExtendedType\" typeName=\"int\" />"
            "    </ECClass>"
            "</ECSchema>";

        return schemaFragment;
        }
    };

//---------------------------------------------------------------------------------------//
// Tests that names (SchemaName, ClassName, PropertyName) do not differ by case.
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestNamesMayNotDifferByCase)
    {
    ECSchema::CreateSchema(m_schema, "MySchema", "ts", 1, 0, 0);

    ECEntityClassP ecClass;
    m_schema->CreateEntityClass(ecClass, "MyClass");

    ECClassP testClass = m_schema->GetClassP("myClass");
    EXPECT_TRUE(testClass != nullptr) << "Expected 'myClass' to return 'MyClass'";

    PrimitiveECPropertyP prop;
    ecClass->CreatePrimitiveProperty(prop, "MyProperty", PRIMITIVETYPE_String);

    ECPropertyP testProp = ecClass->GetPropertyP("myProperty");
    EXPECT_TRUE(testProp != nullptr) << "Expected 'myProperty' to return 'MyProperty'";
    }

//---------------------------------------------------------------------------------------//
// Tests that names (SchemaName, ClassName, PropertyName) are valid. For instance they   //
// may not differ by case                                                                //
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestValidSchemaNames)
    {
    ECSchemaPtr schemaA;
    ECSchemaPtr schemaB;

    ECObjectsStatus statusA = ECSchema::CreateSchema(schemaA, "somethingsomethingdarkside", "ts", 2, 0, 3);
    ECObjectsStatus statusB = ECSchema::CreateSchema(schemaB, "MySchema5&!(/$§!$", "ts", 2, 0, 3);

    EXPECT_TRUE(statusA == ECObjectsStatus::Success) << "Expected to return success when a valid schema name is given";
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the schema name contains invalid characters";
    ECEntityClassP ecClass;
    statusB = schemaA->CreateEntityClass(ecClass, "MyClass""§$%&/()");
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the class name contains invalid characters";

    ASSERT_EQ(ECObjectsStatus::Success, schemaA->CreateEntityClass(ecClass, "MyClass"));
    PrimitiveECPropertyP prop;
    statusB = ecClass->CreatePrimitiveProperty(prop, "MyProperty!§$%&/()=", PRIMITIVETYPE_String);
    EXPECT_TRUE(statusB == ECObjectsStatus::InvalidName) << "Expected InvalidName because the property name contains invalid characters";
    }

//---------------------------------------------------------------------------------------//
// Tests that Aliases are valid. For instance they may not differ by case                //
// @bsimethod                                    Andreas.Kurka                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidationTests, TestAliases)
    {
    ECSchemaPtr schemaA;

    ECObjectsStatus status = ECSchema::CreateSchema(schemaA, "somethingsomethingdarkside", "testalias", 3, 0, 1);
    EXPECT_TRUE(status == ECObjectsStatus::Success) << "Expected success because a valid alias was given";

    status = schemaA->SetAlias("invalid&/(!$&(/§!$");
    EXPECT_TRUE(status == ECObjectsStatus::InvalidName) << "Expected InvalidName because the alias contains invalid characters";

    status = schemaA->SetAlias("");
    EXPECT_TRUE(status == ECObjectsStatus::InvalidName) << "Expected InvalidName because the alias is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidationTests, TestClassConstraintDelayedValidation)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'></ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid()) << "The schema should have successfully been read from the xml string.";

    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema should stay a 3.0 schema and not be upgraded to a the latest EC version.";

    // Attempt to validate the schema, should remain a 3.0 schema
    EXPECT_TRUE(schema->Validate());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema should still not be upgraded after the validation is ran.";

    // Update the schema to now be a validate 3.2 schema
    ECClassCP baseClass = schema->GetClassCP("B");
    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassP("C")->AddBaseClass(*baseClass)) << "Adding a base class to C to make the relationship constraints valid.";

    schema->Validate();
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema should now be upgraded to the latest ECVersion.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidationTests, TestMultiplicityConstraintDelayedValidation)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema should have been read as a 3.0 schema. It fails validation.";
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());

    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(UINT_MAX, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());

    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassP("ARelB")->GetRelationshipClassP()->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne())) << "Fixing the Source multiplicity to not violate the narrowing rule.";
    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassP("ARelB")->GetRelationshipClassP()->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany())) << "Fixing the Target multiplicity to not violate the narrowing rule.";

    EXPECT_TRUE(schema->Validate());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "Since the validation passed, the schema should be upgraded to the latest EC version.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     04/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidationTests, TestSchemaValidationCanBeDelayed)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
           <ECEntityClass typeName='A' modifier='abstract' />
           <ECEntityClass typeName='B' modifier='abstract' />
           <ECEntityClass typeName='C' modifier='abstract' />
           <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward' modifier='abstract'>
               <Source multiplicity='(1..1)' polymorphic='True' roleLabel='source'>
                   <Class class='A' />
               </Source>
               <Target multiplicity='(1..1)' polymorphic='True' roleLabel='target'>
                   <Class class='B' />
               </Target>
           </ECRelationshipClass>
           <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward' modifier='abstract'>
               <BaseClass>ARelB</BaseClass>
               <Source multiplicity='(0..1)' roleLabel='source' polymorphic='True'>
                   <Class class='A' />
               </Source>
               <Target multiplicity='(0..*)' roleLabel='target' polymorphic='True'>
                   <Class class='C' />
               </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Relationship constraints do not narrow but ECSchema::ReadFromXmlString did not return the expected error status";
    
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetSkipValidation(true);
    status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Skip validation was set on the schema read context but relationship constraints that do not narrow still caused a schema read error.";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
