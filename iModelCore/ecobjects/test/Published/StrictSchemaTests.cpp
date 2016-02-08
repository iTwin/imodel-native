/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/StrictSchemaTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

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
// Test to check that isStruct is set to false for RelationshipClass as well  as
// RelationshipConstraint classes
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, RelationshipClassIsStruct)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isStruct=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isStruct=\"True\" >"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"RelationshipA\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
        "        <Source cardinality=\"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "            <Class class=\"TargetClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) 
           << "Schema should not have Struct class as RelationshipConstraint class";

    CreateTestSchema();

    ECRelationshipClassCP relClass = m_schema->GetClassP("RelationshipA")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'RelationshipA' in test schema";

    ASSERT_FALSE(relClass->IsStructClass()) << "IsStruct property must be false for RelationshipClass";

    for (auto constraintClass : relClass->GetSource().GetConstraintClasses())
        {
        ASSERT_FALSE(constraintClass->GetClass().IsStructClass()) 
                  << "RelationshipConstraint may not have StructClasses";
        }

    for (auto constraintClass : relClass->GetTarget().GetConstraintClasses())
        {
        ASSERT_FALSE(constraintClass->GetClass().IsStructClass())
                  << "RelationshipConstraint may not have StructClasses";
        }
    }

//---------------------------------------------------------------------------------------//
// Test to check that isCustomAttributeClass is set to false for RelationshipClass 
// as well RelationshipConstraint classes
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(RelationshipClassTests, RelationshipClassIsCustomAttribute)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"SourceClass\" displayLabel=\"Source Class\" isCustomAttributeClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"TargetClass\" displayLabel=\"Target Class\" isCustomAttributeClass=\"True\">"
        "        <ECProperty propertyName=\"Property1\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"Property2\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"RelationshipA\" displayLabel=\"Source contains Target\" strength=\"referencing\">"
        "        <Source cardinality=\"(1,1)\" roleLabel=\"contains\" polymorphic=\"False\">"
        "            <Class class=\"SourceClass\" />"
        "        </Source>"
        "        <Target cardinality=\"(1,1)\" roleLabel=\"is contained by\" polymorphic=\"True\">"
        "            <Class class=\"TargetClass\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) 
           << "Schema should not have CustomAttribute class as RelationshipConstraintClass";

    CreateTestSchema();

    ECRelationshipClassCP relClass = m_schema->GetClassP("RelationshipA")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relClass) << "Cannot find 'RealtionshipA' in test schema";

    ASSERT_FALSE(relClass->IsCustomAttributeClass()) 
              << "IsCustomAttributeClass property must be false for RelationshipClass";

    for (auto constraintClass : relClass->GetSource().GetConstraintClasses())
        {
        ASSERT_FALSE(constraintClass->GetClass().IsCustomAttributeClass())
                  << "RelationshipConstraint may not have a CustomAttribute Class";
        }

    for (auto constraintClass : relClass->GetTarget().GetConstraintClasses())
        {
        ASSERT_FALSE(constraintClass->GetClass().IsStructClass())
                  << "RelationshipConstraint may not have a CustomAttribute Class";
        }
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
// Test to check that Struct property can’t be a key property in a relationship class
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
