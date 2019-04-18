/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ClassTests : ECTestFixture
    {
    ECSchemaPtr m_schema;

    //---------------------------------------------------------------------------------------//
    // Stores the format of the test schema xml as a string
    // @bsimethod                             Prasanna.Prakash                       02/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    static Utf8CP   TestSchemaXMLString()
        {
        Utf8CP format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"ClassA\" displayLabel=\"Regular Class\" isDomainClass=\"True\" >"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassB\" displayLabel=\"Regular Class\" isStruct=\"True\">"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassC\" displayLabel=\"Regular Class\" isCustomAttributeClass=\"True\">"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "</ECSchema>";

        return format;
        }

    //---------------------------------------------------------------------------------------//
    // Creates the test schema from the string xml format
    // @bsimethod                             Prasanna.Prakash                       02/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    void CreateTestSchema()
        {
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, TestSchemaXMLString(), *schemaContext))
            << "Test schema failed to read";
        ASSERT_TRUE(m_schema.IsValid())
            << "Test Schema is not valid";
        }
    };

//---------------------------------------------------------------------------------------//
// Test to check that Class may  not have a relationship class as a base class and may 
// not be a base class for a relationship class
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, CheckBaseClasses)
    {
    CreateTestSchema();

    ECClassP testClass = m_schema->GetClassP("ClassA");
    ASSERT_NE(nullptr, testClass) << "Cannot find 'ClassA' in test Schema";

    ECRelationshipClassP relClass;
    ASSERT_EQ(ECObjectsStatus::Success, m_schema->CreateRelationshipClass(relClass, "Relationship"))
        << "Cannot create a relationship class in the test schema";

    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, testClass->AddBaseClass(*relClass))
        << "Regular class cannot have a RelationshipClass as a base class";
    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, relClass->AddBaseClass(*testClass))
        << "RelationshipClass cannot have a regular class as a base class";
    }

//---------------------------------------------------------------------------------------//
// Test to check that Abstract classes cannot be instantiated
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, CheckAbstractness)
    {
    CreateTestSchema();

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"AbstractClass\" displayLabel=\"Abstract Class\" isDomainClass=\"False\" isStruct=\"False\" isCustomAttributeClass=\"False\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Test Schema was not read from the schemaXml";

    ECClassP abstractClass = schema->GetClassP("AbstractClass");
    ASSERT_NE(nullptr, abstractClass) << "Cannot find 'AbstractClass' from the test schema";

    StandaloneECEnablerPtr classEnabler1 = abstractClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(classEnabler1.IsValid()) << "Abstract Class enabler is not valid";

    StandaloneECInstancePtr classInstance1 = classEnabler1->CreateInstance();
    ASSERT_FALSE(classInstance1.IsValid()) << "Abstract Classses may not be instantiated";

    ECClassP testClass = m_schema->GetClassP("ClassA");
    ASSERT_NE(nullptr, testClass) << "Cannot find 'ClassA' in test schema";

    testClass->SetClassModifier(ECClassModifier::Abstract);
    ASSERT_EQ(ECClassModifier::Abstract, testClass->GetClassModifier())
        << "Cannot convert the test class into an abstract class";

    StandaloneECEnablerPtr classEnabler2 = testClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(classEnabler2.IsValid()) << "Abstract class enabler is not valid";

    StandaloneECInstancePtr classInstance2 = classEnabler2->CreateInstance();
    ASSERT_FALSE(classInstance2.IsValid()) << "Abstract Classses may not be instantiated";
    }

//---------------------------------------------------------------------------------------//
// Test to check that It is only valid to set one of the following flags to true: 
// isDomainClass, isStruct and isCustomAttributeClass 
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, ValidateOnlyOneFlagTrue)
    {
    CreateTestSchema();

    ECClassCP domainClass = m_schema->GetClassCP("ClassA");
    ASSERT_NE(nullptr, domainClass) << "Cannot find 'ClassA' in the test schema";
    ASSERT_TRUE(domainClass->IsEntityClass() && !domainClass->IsCustomAttributeClass() && !domainClass->IsStructClass())
        << "Only isDomainClass property should be true for the domain class";

    ECClassCP structClass = m_schema->GetClassCP("ClassB");
    ASSERT_NE(nullptr, structClass) << "Cannot find 'ClassB' in the test schema";
    ASSERT_TRUE(!structClass->IsEntityClass() && !structClass->IsCustomAttributeClass() && structClass->IsStructClass())
        << "Only isStruct property should be true for the struct class";

    ECClassCP customAttributeClass = m_schema->GetClassCP("ClassC");
    ASSERT_NE(nullptr, customAttributeClass) << "Cannot find 'ClassC' in the test schema";
    ASSERT_TRUE(!customAttributeClass->IsEntityClass() && customAttributeClass->IsCustomAttributeClass() && !customAttributeClass->IsStructClass())
        << "Only isCustomAttributeClass property should be true for CustomAttribute class";

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"RegularClassA\" displayLabel=\"Regular Class A\" isDomainClass=\"True\" isStruct=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"RegularClassB\" displayLabel=\"Regular Class B\" isDomainClass=\"True\" isCustomAttributeClass=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"RegularClassC\" displayLabel=\"Regular Class C\" isStruct=\"True\" isCustomAttributeClass=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"RegularClassD\" displayLabel=\"Regular Class D\" isDomainClass=\"True\" isStruct=\"True\" isCustomAttributeClass=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unable to read the schema file";

    structClass = schema->GetClassCP("RegularClassA");
    ASSERT_NE(nullptr, structClass) << "Cannot find 'RegularClassA' in the test schema";
    ASSERT_TRUE(!structClass->IsEntityClass() && !structClass->IsCustomAttributeClass() && structClass->IsStructClass())
        << "Only isStruct property should be true for a class with isDomainClass and isStruct equal to true";

    customAttributeClass = schema->GetClassCP("RegularClassB");
    ASSERT_NE(nullptr, customAttributeClass) << "Cannot find 'RegularClassB' in the test schema";
    ASSERT_TRUE(!customAttributeClass->IsEntityClass() && customAttributeClass->IsCustomAttributeClass() && !customAttributeClass->IsStructClass())
        << "Only isCustomAttributeClass property should be true for a class with isDomainClass and isCustomAttribute equal to true";

    structClass = schema->GetClassCP("RegularClassC");
    ASSERT_NE(nullptr, structClass) << "Cannot find 'RegularClassC' in the test schema";
    ASSERT_TRUE(!structClass->IsEntityClass() && !structClass->IsCustomAttributeClass() && structClass->IsStructClass())
        << "Only isStruct property should be true for a class with isStruct and isCustomAttributeClass equal to true";

    structClass = schema->GetClassCP("RegularClassD");
    ASSERT_NE(nullptr, structClass) << "Cannot find 'RegularClassD' in the test schema";
    ASSERT_TRUE(!structClass->IsEntityClass() && !structClass->IsCustomAttributeClass() && structClass->IsStructClass())
        << "Only isStruct property should be true for a class with isDomainClass and isStruct and isCustomAttributeClass equal to true";
    }

//---------------------------------------------------------------------------------------//
// Test to check that Class hierarchies must set these flags in a consistent manner
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, ClassHierarchiesFlagsConsistent)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"AbstractClass\" displayLabel=\"Abstract Class\" isDomainClass=\"False\" isStruct=\"False\" isCustomAttributeClass=\"False\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassA\" displayLabel=\"Regular Class\" isDomainClass=\"True\">"
        "        <BaseClass>AbstractClass</BaseClass>"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassB\" displayLabel=\"Regular Class\" isDomainClass=\"True\">"
        "        <BaseClass>AbstractClass</BaseClass>"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unable to read the schema file from schemaXml";

    ECClassP abstractClass = schema->GetClassP("AbstractClass");
    ASSERT_NE(nullptr, abstractClass) << "Cannot find 'AbstractClass' in test schema";
    EXPECT_EQ(ECClassModifier::Abstract, abstractClass->GetClassModifier());

    ECStructClassP structClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateStructClass(structClass, "ClassC"))
        << "Cannot create the struct class into the schema";

    ASSERT_NE(ECObjectsStatus::Success, structClass->AddBaseClass(*abstractClass))
        << "Class hierarchies must set these flags in a consistent manner.";

    ECStructClassP testBaseClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateStructClass(testBaseClass, "StructClass"))
        << "Cannot create the struct base class into the schema";

    ECClassP testClass1 = schema->GetClassP("ClassA");
    ASSERT_NE(nullptr, testClass1) << "Cannot find 'ClassA' in the test schema";
    EXPECT_EQ(ECClassType::Entity, testClass1->GetClassType());
    ASSERT_EQ(ECObjectsStatus::Success, testClass1->RemoveBaseClass(*abstractClass))
        << "Cannot remove the base class of 'ClassA'";
    ASSERT_NE(ECObjectsStatus::Success, testClass1->AddBaseClass(*testBaseClass))
        << "Cannot add struct base class to 'ClassA' in the test schema";

    ECClassP testClass2 = schema->GetClassP("ClassB");
    ASSERT_NE(nullptr, testClass2) << "Cannot find 'ClassA' in the test schema";
    EXPECT_EQ(ECClassType::Entity, testClass2->GetClassType());
    ASSERT_EQ(ECObjectsStatus::Success, testClass2->RemoveBaseClass(*abstractClass))
        << "Cannot remove the base class of 'ClassB'";
    ASSERT_NE(ECObjectsStatus::Success, testClass2->AddBaseClass(*testBaseClass))
        << "Cannot add struct base class to 'ClassB' in the test schema";
    }

struct RelationshipClassTests : ECTestFixture
    {
    ECSchemaPtr m_schema;

    //---------------------------------------------------------------------------------------//
    // Stores the format of the test schema xml as a string
    // @bsimethod                             Prasanna.Prakash                       01/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    static Utf8CP   TestSchemaXMLString()
        {
        Utf8CP format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"RegularClass\" displayLabel=\"Regular Class\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
            "        <BaseClass>RegularClass</BaseClass>"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
            "        <BaseClass>RegularClass</BaseClass>"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"Property2\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECRelationshipClass typeName=\"RelationshipA\" displayLabel=\"Source contains Target\" strength=\"embedding\">"
            "        <Source cardinality=\"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
            "            <Class class=\"SourceClass\" />"
            "            <Class class=\"RegularClass\" />"
            "        </Source>"
            "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
            "            <Class class=\"TargetClass\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "    <ECRelationshipClass typeName=\"RelationshipB\" displayLabel=\"Target contains Source\" strength=\"referencing\">"
            "        <Source cardinality = \"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
            "            <Class class=\"TargetClass\" />"
            "        </Source>"
            "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
            "            <Class class=\"SourceClass\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "</ECSchema>";

        return format;
        }
    
    //---------------------------------------------------------------------------------------//
    // Creates the test schema using the TestSchemaXml string
    // @bsimethod                             Prasanna.Prakash                       01/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    void CreateTestSchema()
        {
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, TestSchemaXMLString(), *schemaContext))
               << "Failed to read the test schema from xml string";
        ASSERT_TRUE(m_schema.IsValid()) << "Test Schema is not valid";
        ASSERT_TRUE(m_schema->IsECVersion(ECVersion::Latest)) << "Test Schema is not the latest ECVersion, " << ECSchema::GetECVersionString(ECVersion::Latest) << ".";
        }
    };

//---------------------------------------------------------------------------------------//
// Test to check that RelationshipClass may not regular class as base class and that
// RegularClass may not have relationship class as a base class
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, RelationshipClassBaseClass)
    {
    CreateTestSchema();
    
    ECClassP regularClass = m_schema->GetClassP("RegularClass");
    ASSERT_NE(nullptr, regularClass) << "Cannot find 'RegularClass' in the test schema";

    ECRelationshipClassP relClass = m_schema->GetClassP("RelationshipA")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'RelationshipA' in the test schema";

    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, relClass->AddBaseClass(*regularClass))
           << "RelationshipClass may not have regular class as a base class";

    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, regularClass->AddBaseClass(*relClass))
           << "Regular Class may not have RelationshipClass as a base class";
    }


//---------------------------------------------------------------------------------------//
// Test to check that Abstract RelationshipClass may not be instatiated
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, RelationshipClassAbstractness)
    {
    CreateTestSchema();

    ECRelationshipClassP relClass = m_schema->GetClassP("RelationshipA")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'RelationshipA' in the test schema";

    relClass->SetClassModifier(ECClassModifier::Abstract);
    ASSERT_EQ(ECClassModifier::Abstract, relClass->GetClassModifier())
           << "'RelationshipA' is not Abstract";

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass);
    ASSERT_TRUE(relationshipEnabler.IsValid()) << "RelationshipClassEnabler is not valid for 'RelationshipA'";

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
    ASSERT_FALSE(relationshipInstance.IsValid()) << "Instance may not be created for the Abstract Relationship Class";
    }

//---------------------------------------------------------------------------------------//
// Test to check that for non-abstract RelationshipClass constraint must be well-defined
// i.e, constraint must have atleast one class defined and these classes may only be
// regular class and not relationship class
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, NonAbstractRelationshipClassConstraintsDefinition)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"Property2\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"RelationshipA\" displayLabel=\"Source contains Target\" strength=\"embedding\">"
        "        <Source cardinality=\"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Schema could not be deserialized.";
    ASSERT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "Schema must have well-defined constraints for RelationshipClass";

    CreateTestSchema();

    ECRelationshipClassP relClass1 = m_schema->GetClassP("RelationshipA")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass1) << "Cannot find 'RealtionshipA' in the test schema";
    ECRelationshipClassCP relClass2 = m_schema->GetClassP("RelationshipB")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relClass2) << "Cannot find 'RealtionshipB' in the test schema";
    ECEntityClassCP regularClass = m_schema->GetClassP("RegularClass")->GetEntityClassCP();
    ASSERT_NE(nullptr, regularClass) << "Cannot find 'RegularClass' in the test schema";

    EXPECT_EQ(ECClassModifier::None, relClass1->GetClassModifier());

    ASSERT_TRUE(relClass1->GetSource().GetConstraintClasses().size() > 0)
             << "'RelationshipClass SourceConstraint does not have a well-defined class";

    ASSERT_TRUE(relClass1->GetTarget().GetConstraintClasses().size() > 0)
             << "'RelationshipClass TargetConstraint does not have a well-defined class";

    for (auto source : relClass1->GetSource().GetConstraintClasses())
        {        
        ASSERT_NE(ECClassType::Relationship, source->GetClassType()) 
               << "'RelationshipConstraint may not have RelationshipClass";

        ASSERT_TRUE(ECClassModifier::Abstract == source->GetClassModifier() || ECClassModifier::None == source->GetClassModifier())
                 << "'RealtioshipConstraint classes may only be Domain or Abstract";
        }

    // Need to set the abstract constraint to regular class before it can be set. Otherwise, the target constraint is too narrow
    ASSERT_EQ(ECObjectsStatus::Success, relClass1->GetTarget().SetAbstractConstraint(*regularClass))
           <<  "Cannot set the abstract constraint of the Target-Constraint on " << relClass1->GetFullName() << ".";
    ASSERT_EQ(ECObjectsStatus::Success, relClass1->GetTarget().AddClass(*regularClass))
           << "Regular class cannot be added to RelationshipConstraint";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
