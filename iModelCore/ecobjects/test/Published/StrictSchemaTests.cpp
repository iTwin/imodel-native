/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/StrictSchemaTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;
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
// Test to check that Only Domain and Custom Attribute classes may be used to create 
// standalone instances
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, CheckClassTypeForStandAloneInstances)
    {
    CreateTestSchema();

    ECClassCP domainClass = m_schema->GetClassCP("ClassA");
    ASSERT_NE(nullptr, domainClass) << "Cannot find 'ClassA' in the test schema";
    EXPECT_EQ(ECClassType::Entity, domainClass->GetClassType());

    StandaloneECEnablerPtr doaminClassEnabler = domainClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(doaminClassEnabler.IsValid()) << "Cannot create an enabler for Domain Class";

    StandaloneECInstancePtr domainClassInstance = doaminClassEnabler->CreateInstance();
    ASSERT_TRUE(domainClassInstance.IsValid()) << "Cannot create an instance for the Domain class";

    ECClassCP customAttributeClass = m_schema->GetClassCP("ClassC");
    ASSERT_NE(nullptr, customAttributeClass) << "Cannot find 'ClassC' in the test schema";
    EXPECT_EQ(ECClassType::CustomAttribute, customAttributeClass->GetClassType());

    StandaloneECEnablerPtr customAttributeClassEnabler = customAttributeClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(customAttributeClassEnabler.IsValid()) << "Cannot create an enabler for CustomAttribute class";

    StandaloneECInstancePtr customAttributeClassInstance = customAttributeClassEnabler->CreateInstance();
    ASSERT_TRUE(customAttributeClassInstance.IsValid())
        << "Cannot create a standAlone instance of the CustomAttribute class";

    ECClassCP structClass = m_schema->GetClassCP("ClassB");
    ASSERT_NE(nullptr, structClass) << "Cannot find 'ClassB' in the test schema";
    EXPECT_EQ(ECClassType::Struct, structClass->GetClassType());

    StandaloneECEnablerPtr structClassEnabler = structClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(structClassEnabler.IsValid()) << "Cannot create an enabler for Struct class";

    StandaloneECInstancePtr structClassInstance = structClassEnabler->CreateInstance();
    ASSERT_FALSE(structClassInstance.IsValid())
        << "Only Domain and Custom Attribute classes may be used to create standalone instances";
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
// Test to check that An abstract class may only have derived classes of one type 
// @bsimethod                             Prasanna.Prakash                       02/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ClassTests, DerivedClassTypesOfAbstractClass)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "   <ECEntityClass typeName=\"AbstractClass\" displayLabel=\"Abstract Class\" modifier=\"Abstract\">"
        "   </ECEntityClass>"
        "   <ECCustomAttributeClass typeName=\"CustomAttributeClass\" displayLabel=\"Custom Attribute Class\" modifier=\"None\" appliesTo=\"Any\">"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName=\"DomainClassA\" displayLabel=\"Domain Class A\" modifier=\"None\">"
        "       <BaseClass>AbstractClass</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName=\"DomainClassB\" displayLabel=\"Domain Class B\" modifier=\"None\">"
        "       <BaseClass>AbstractClass</BaseClass>"
        "   </ECEntityClass>"
        "   <ECStructClass typeName=\"StructClassA\" displayLabel=\"Struct Class A\" modifier=\"None\">"
        "   </ECStructClass>"
        "   <ECStructClass typeName=\"StructClassB\" displayLabel=\"Struct Class B\" modifier=\"None\">"
        "   </ECStructClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unable to read the schema file";

    ECClassP abstractClass = schema->GetClassP("AbstractClass");
    ASSERT_NE(nullptr, abstractClass) << "Cannot find Abstract Class in the test schema";
    EXPECT_EQ(ECClassModifier::Abstract, abstractClass->GetClassModifier());

    ECEntityClassP domainClass1 = schema->GetClassP("DomainClassA")->GetEntityClassP();
    ASSERT_NE(nullptr, domainClass1) << "Cannot find Domain class in the test schema";

    ECEntityClassP domainClass2 = schema->GetClassP("DomainClassB")->GetEntityClassP();
    ASSERT_NE(nullptr, domainClass2) << "Cannot find Domain class in the test schema";

    ECCustomAttributeClassP customAttributeClass = schema->GetClassP("CustomAttributeClass")->GetCustomAttributeClassP();
    ASSERT_NE(nullptr, customAttributeClass) << "CustomAttribute Class could not be found in the test schema";
    ASSERT_NE(ECObjectsStatus::Success, customAttributeClass->AddBaseClass(*abstractClass))
        << "An abstract class may only have derived classes of one type";

    ECStructClassP structClass1 = schema->GetClassP("StructClassA")->GetStructClassP();
    ASSERT_NE(nullptr, structClass1) << "Cannot find Struct class in the test schema";
    ASSERT_NE(ECObjectsStatus::Success, structClass1->AddBaseClass(*abstractClass))
        << "An abstract class may only have derived classes of one type";

    ASSERT_EQ(ECObjectsStatus::Success, domainClass1->RemoveBaseClass(*abstractClass))
        << "Cannot remove the Abstract base class form Entity class";
    ASSERT_EQ(ECObjectsStatus::Success, domainClass2->RemoveBaseClass(*abstractClass))
        << "Cannot remove the abstract base class from Entity Class";

    ASSERT_EQ(ECObjectsStatus::Success, structClass1->AddBaseClass(*abstractClass))
        << "Cannot add the Abstract Class as a base class of Struct Class";

    ECStructClassP structClass2 = schema->GetClassP("StructClassB")->GetStructClassP();
    ASSERT_NE(nullptr, structClass2) << "Cannot find Struct class in the test schema";
    ASSERT_EQ(ECObjectsStatus::Success, structClass2->AddBaseClass(*abstractClass))
        << "Cannot add the Abstract Class as a base class of Struct Class";

    ASSERT_NE(ECObjectsStatus::Success, customAttributeClass->AddBaseClass(*abstractClass))
        << "An abstract class may only have derived classes of one type";
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
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
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
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema must have well-defined constraints for RelationshipClass";

    CreateTestSchema();

    ECRelationshipClassP relClass1 = m_schema->GetClassP("RelationshipA")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass1) << "Cannot find 'RealtionshipA' in the test schema";
    ECRelationshipClassCP relClass2 = m_schema->GetClassP("RelationshipB")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relClass2) << "Cannot find 'RealtionshipB' in the test schema";
    ECEntityClassCP regularClass = m_schema->GetClassP("RegularClass")->GetEntityClassCP();
    ASSERT_NE(nullptr, regularClass) << "Cannot find 'RegularClass' in the test schema";

    EXPECT_EQ(ECClassModifier::None, relClass1->GetClassModifier());

    ASSERT_TRUE(relClass1->GetSource().GetClasses().size() > 0)
             << "'RelationshipClass SourceConstraint does not have a well-defined class";

    ASSERT_TRUE(relClass1->GetTarget().GetClasses().size() > 0)
             << "'RelationshipClass TargetConstraint does not have a well-defined class";

    for (auto source : relClass1->GetSource().GetClasses())
        {        
        ASSERT_NE(ECClassType::Relationship, source->GetClassType()) 
               << "'RelationshipConstraint may not have RelationshipClass";

        ASSERT_TRUE(ECClassModifier::Abstract == source->GetClassModifier() || ECClassModifier::None == source->GetClassModifier())
                 << "'RealtioshipConstraint classes may only be Domain or Abstract";
        }

    ASSERT_EQ(ECObjectsStatus::Success, relClass1->GetTarget().AddClass(*regularClass))
           << "Regular class cannot be added to RelationshipConstraint";
    ASSERT_NE(ECObjectsStatus::Success, relClass1->GetTarget().AddClass(*relClass2))
           << "RelationshipClass may not be added to RelationshipConstraint";
    }

//---------------------------------------------------------------------------------------//
// Test to check that cyclic holding and embedding relationships are not allowed
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, CyclicRelationships)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "            <Class class=\"TargetClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName=\"RelationshipB\" displayLabel=\"Target contains Source\" strength=\"embedding\">"
        "        <Source cardinality = \"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"TargetClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "            <Class class=\"SourceClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName=\"RelationshipC\" displayLabel=\"Target contains Source\" strength=\"holding\">"
        "        <Source cardinality = \"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"TargetClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "            <Class class=\"SourceClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName=\"RelationshipD\" displayLabel=\"Target contains Source\" strength=\"holding\">"
        "        <Source cardinality = \"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "            <Class class=\"TargetClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema may not have cyclic holding and embedding relationships";

    CreateTestSchema();

    ECRelationshipClassP relClass1 = m_schema->GetClassP("RelationshipA")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass1) << "Could not find 'RelationshipA' in the test schema";
    ECRelationshipClassP relClass2 = m_schema->GetClassP("RelationshipB")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass2) << "Could not find 'RelationshipB' in the test schema";

    ASSERT_FALSE(StrengthType::Embedding == relClass1->GetStrength() && StrengthType::Embedding == relClass2->GetStrength())
              << "Cyclic relationship encountered in the test schema";
    
    ASSERT_FALSE(StrengthType::Holding == relClass1->GetStrength() && StrengthType::Holding == relClass2->GetStrength())
              << "Cyclic relationship encountered in the test schema";
    
    ASSERT_NE(ECObjectsStatus::Success, relClass2->SetStrength(StrengthType::Embedding))
           << "Cyclic Embedding relationship encountered";

    ASSERT_EQ(ECObjectsStatus::Success, relClass1->SetStrength(StrengthType::Holding))
           << "Cannot change the strength of 'RelationshipA' in test schema";

    ASSERT_NE(ECObjectsStatus::Success, relClass2->SetStrength(StrengthType::Holding))
           << "Cyclic Holding relationship encountered";
    }

struct RelationshipConstraintTests : ECTestFixture
    {
    ECSchemaPtr m_schema;

    //---------------------------------------------------------------------------------------//
    // Stores the format of the test schema xml as a string
    // @bsimethod                             Prasanna.Prakash                       01/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    static Utf8CP   TestSchemaXMLString()
        {
        Utf8CP format= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"Property2\" typeName=\"int\" isStruct=\"True\" />"
            "    </ECClass>"
            "    <ECRelationshipClass typeName=\"Relationship\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
            "        <Source roleLabel=\"contains\" polymorphic=\"False\">"
            "            <Class class=\"SourceClass\">"
            "                <Key>"
            "                    <Property name = \"Property1\" />"
            "                </Key>"
            "            </Class>"        
            "        </Source>"
            "        <Target cardinality=\"(1,N)\" roleLabel=\"is contained by\" >"
            "            <Class class=\"TargetClass\">"
            "                <Key>"
            "                    <Property name = \"Property1\" />"
            "                </Key>"
            "            </Class>"
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
        }
    };

//---------------------------------------------------------------------------------------//
// Test to check that Struct property can�t be a key property in a relationship class
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipConstraintTests, StructKeyPropertiesNotSupported)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" isStruct=\"True\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"Relationship\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\">"
        "                <Key>"
        "                    <Property name = \"Property1\" />"
        "                </Key>"
        "            </Class>"
        "        </Source>"
        "        <Target cardinality=\"(1,N)\" roleLabel=\"is contained by\" polymorphic=\"True\" >"
        "            <Class class=\"TargetClass\">"
        "                <Key>"
        "                    <Property name = \"Property1\" />"
        "                </Key>"
        "            </Class>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_NE(SchemaReadStatus::Success, status) 
           << "Schema cannot have struct property as a key property in a relationship class";

    CreateTestSchema();

    ECRelationshipClassP relClass = m_schema->GetClassP("Relationship")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'Relationship' in test schema";

    ECClassCP sourceTestClass = m_schema->GetClassCP(relClass->GetSource().GetConstraintClasses()[0]->GetClass().GetName().c_str());
    ASSERT_FALSE(sourceTestClass->GetPropertyP(relClass->GetSource().GetConstraintClasses()[0]->GetKeys()[0])->GetIsStruct())
              << "Struct Property cannot be a key property in a Relationship Class";

    ECClassCP targetTestClass = m_schema->GetClassCP(relClass->GetTarget().GetConstraintClasses()[0]->GetClass().GetName().c_str());
    ASSERT_FALSE(targetTestClass->GetPropertyP(relClass->GetTarget().GetConstraintClasses()[0]->GetKeys()[0])->GetIsStruct())
              << "Struct Property cannot be a key property in a Relationship Class";

    ECRelationshipConstraintClassList *constraintClasses = nullptr;
    ECEntityClassP entity = m_schema->GetClassP("TargetClass")->GetEntityClassP();
    ECRelationshipConstraintClassP constraint;
    constraintClasses->Add(constraint, *entity);
    constraint->AddKey("Property2");

    ASSERT_TRUE(constraint->GetKeys()[0].Equals("Property2")) << "Key not added to the constraint";
    }

//---------------------------------------------------------------------------------------//
// Test to check that Property OR property.path must exist to be added as key property
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipConstraintTests, KeyPropertiesPathDefined)
    {
    CreateTestSchema();

    ECRelationshipClassP relClass = m_schema->GetClassP("Relationship")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass);

    ECClassCP sourceTestClass = m_schema->GetClassCP(relClass->GetSource().GetConstraintClasses()[0]->GetClass().GetName().c_str());
    ASSERT_NE(nullptr, sourceTestClass->GetPropertyP(relClass->GetSource().GetConstraintClasses()[0]->GetKeys()[0]));

    ECClassCP targetTestClass = m_schema->GetClassCP(relClass->GetTarget().GetConstraintClasses()[0]->GetClass().GetName().c_str());
    ASSERT_NE(nullptr, targetTestClass->GetPropertyP(relClass->GetTarget().GetConstraintClasses()[0]->GetKeys()[0]));

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"Relationship\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\">"
        "                <Key>"
        "                    <Property name = \"Property1\" />"
        "                </Key>"
        "            </Class>"
        "        </Source>"
        "        <Target cardinality=\"(1,N)\" roleLabel=\"is contained by\" polymorphic=\"True\" >"
        "            <Class class=\"TargetClass\">"
        "                <Key>"
        "                    <Property name = \"Property2\" />"
        "                </Key>"
        "            </Class>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_NE(SchemaReadStatus::Success, status);
    }

//---------------------------------------------------------------------------------------//
// Test to check that Multiple key properties are not supported
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipConstraintTests, MultipleKeyPropertiesNotSupported)
    {
    CreateTestSchema();

    ECRelationshipClassP relClass = m_schema->GetClassP("Relationship")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'Relationship' in thest schema";
    
    ASSERT_TRUE(relClass->GetSource().GetConstraintClasses()[0]->GetKeys().size() < 1)
           << "Multiple key properties are not supported";

    ASSERT_TRUE(relClass->GetTarget().GetConstraintClasses()[0]->GetKeys().size() < 1)
           << "Multiple key properties are not supported";
    
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestStrictSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"Property2\" typeName=\"int\" isStruct=\"True\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"Relationship\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
        "        <Source roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\">"
        "                <Key>"
        "                    <Property name = \"Property1\" />"
        "                </Key>"
        "            </Class>"
        "        </Source>"
        "        <Target cardinality=\"(1,N)\" roleLabel=\"is contained by\" >"
        "            <Class class=\"TargetClass\">"
        "                <Key>"
        "                    <Property name = \"Property1\" />"
        "                    <Property name = \"Property2\" />"
        "                </Key>"
        "            </Class>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema cannot have multiple key properties for a RelationshipClass";
    }

//---------------------------------------------------------------------------------------//
// Test to check that If constraint has key property, multiple classes should not be 
// in constraint
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipConstraintTests, MultipleConstraintClassesWithKeyPropertyNotSupported)
    {
    CreateTestSchema();

    ECRelationshipClassP relClass = m_schema->GetClassP("Relationship")->GetRelationshipClassP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'Relationship' in the test schema";

    ASSERT_NE(0, relClass->GetSource().GetConstraintClasses()[0]->GetKeys().size())
           << "There are no primary keys in RelationshipClass";
    ASSERT_EQ(1, relClass->GetSource().GetConstraintClasses().size())
           << "If constraint has key property, multiple classes should not be in constraint";

    ASSERT_NE(0, relClass->GetTarget().GetConstraintClasses()[0]->GetKeys().size())
           << "There are no primary keys in RelationshipClass";
    ASSERT_EQ(1, relClass->GetTarget().GetConstraintClasses().size())
           << "If constraint has key property, multiple classes should not be in constraint";

    ECRelationshipConstraintClassP constraintClass;
    ECEntityClassP regularClass;
    ASSERT_EQ(ECObjectsStatus::Success, m_schema->CreateEntityClass(regularClass, "RegularClass"))
           << "Cannot create a DomainClass in the test schema";

    ASSERT_NE(ECObjectsStatus::Success, relClass->GetSource().GetConstraintClassesR().Add(constraintClass, *regularClass))
           << "If constraint has key property, multiple classes should not be in constraint";
    ASSERT_NE(ECObjectsStatus::Success, relClass->GetTarget().GetConstraintClassesR().Add(constraintClass, *regularClass))
           << "If constraint has key property, multiple classes should not be in constraint";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
