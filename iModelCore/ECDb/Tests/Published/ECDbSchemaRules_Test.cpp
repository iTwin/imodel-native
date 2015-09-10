/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaRules_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_Casing)
    {
    ECDbTestProject::Initialize();

    std::vector <TestItem> testItems {
        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TESTCLASS\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Classes with names differing only by case."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Properties only differing by case within a class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"SubClass\" >"
                 "    <BaseClass>TestClass</BaseClass>"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Properties only differing by case in a sub and base class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TestClass2\" >"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 true, "Properties differing only by case in two unrelated classes."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"SubClass\" >"
                 "    <BaseClass>TestClass</BaseClass>"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TESTCLASS\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TESTClass\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Foo\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"FOO\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"PROPerty\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"PROPERTY\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"PROP2\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Class and properties only differing by case within a class.")
        };


    for (TestItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_SchemaNamespacePrefix)
    {
    ECDbTestProject::Initialize();

    TestItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='123' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      true, "Namespace prefix doesn't have to be an ECName.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");

    {
    TestItem firstSchemaTestItem("<ECSchema schemaName='Schema1' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                                 "  <ECClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECClass>"
                                 "</ECSchema>",
                                 true, "");

    TestItem secondSchemaTestItem("<ECSchema schemaName='Schema2' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                                  "  <ECClass typeName='TestClass2' >"
                                  "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                  "  </ECClass>"
                                  "</ECSchema>",
                                  true, "Two schemas with same namespace prefix is supported, second prefix is suffixed with a number.");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, firstSchemaTestItem, "ecdbschemarules_duplicatensprefixes.ecdb");
    ASSERT_FALSE(asserted);

    AssertSchemaImport(asserted, ecdb, secondSchemaTestItem);
    ASSERT_FALSE(asserted);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_PropertyOfSameTypeAsClass)
    {
    ECDbTestProject::Initialize();

    std::vector <TestItem> testItems {
        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of same type as class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\" >"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" isStruct=\"true\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of subtype of class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is array of class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of array of subclass of class."),

        TestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"SUB\" >"
                 "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Case-sensitive class and prop names and property is of array of subclass of class.")
        };


    for (TestItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_Relationship)
    {
    ECDbTestProject::Initialize();

    std::vector <TestItem> testItems {
        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                 "  <ECClass typeName='B'>"
                 "    <ECProperty propertyName='Id' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='Rel1' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='Rel1'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "  <ECClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='B'>"
                 "    <ECProperty propertyName='Id' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='Rel1' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='Rel1'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "  <ECClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='B'>"
                 "    <ECProperty propertyName='Id' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='Rel1' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2' isDomainClass='True'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "      <Class class='Rel1'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class.")};

    for (TestItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_ConsistentClassHierarchy)
    {
    ECDbTestProject::Initialize();

    std::vector <TestItem> testItems {
        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" >"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" >"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Base\" >"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName = \"Rel\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
                 "    <BaseClass>Base</BaseClass>"
                 "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
                 "      <Class class = \"A\" />"
                 "    </Source>"
                 "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
                 "      <Class class = \"B\" />"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass with non-relationship base class is not expected to be supported."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" >"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" >"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName = \"RelBase\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
                 "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
                 "      <Class class = \"A\" />"
                 "    </Source>"
                 "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
                 "      <Class class = \"B\" />"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECClass typeName=\"Cl\" >"
                 "    <BaseClass>RelBase</BaseClass>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Non-relationship class with a relationship base class is not expected to be supported."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" >"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" >"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName = \"RelBase\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
                 "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
                 "      <Class class = \"A\" />"
                 "    </Source>"
                 "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
                 "      <Class class = \"B\" />"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECClass typeName=\"Cl\" >"
                 "    <BaseClass>RelBase</BaseClass>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Non-relationship class with a relationship base class is not expected to be supported."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isDomainClass='True'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isStruct='True' isDomainClass='False'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A domain base class must not have struct subclasses."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isDomainClass='True'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isDomainClass='True'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"C\" isStruct='True' isDomainClass='False'>"
                 "    <BaseClass>B</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A domain base class must not have struct subclasses."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isDomainClass='True'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isDomainClass='True' isStruct='True'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A domain base class must not have struct subclasses, even if the subclass is a struct and a domain class at the same time."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isStruct='True' isDomainClass='False'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isDomainClass='True'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A struct base class must have only struct subclasses."),

        TestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isStruct='True' isDomainClass='False'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isStruct='True' isDomainClass='False'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"C\" isDomainClass='True'>"
                 "    <BaseClass>B</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A struct base class must have only struct subclasses.")
        };

    for (TestItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

END_ECDBUNITTESTS_NAMESPACE
