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

    {
    TestItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='123' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix has to be an ECName.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    TestItem testItem("<ECSchema schemaName='TestSchema' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix must not be unset.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    TestItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix must not be empty.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    TestItem testItem({"<ECSchema schemaName='Schema1' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                                 "  <ECClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECClass>"
                                 "</ECSchema>",
                        "<ECSchema schemaName='Schema2' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                        "  <ECClass typeName='TestClass2' >"
                        "    <ECProperty propertyName='TestProperty' typeName='string' />"
                        "  </ECClass>"
                        "</ECSchema>"}, 
                       false, "Two schemas with same namespace prefix is not supported.");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "ecdbschemarules_duplicatensprefixes.ecdb");
    ASSERT_FALSE(asserted);
    }

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
                                  false, "Two schemas with same namespace prefix is not supported.");

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
TEST_F(SchemaImportTestFixture, ECDbSchemaRules_Instantiability)
    {
    ECDbTestProject::Initialize();

    TestItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "<ECClass typeName='AbstractClass' isDomainClass='False' isStruct='False' isCustomAttributeClass='False' >"
                      "    <ECProperty propertyName='Name' typeName='string' />"
                      "</ECClass>"
                      "<ECClass typeName='DomainClass' isDomainClass='True' isStruct='False' isCustomAttributeClass='False' >"
                      "    <ECProperty propertyName='Name' typeName='string' />"
                      "</ECClass>"
                      "<ECClass typeName='Struct' isDomainClass='False' isStruct='True' isCustomAttributeClass='False' >"
                      "    <ECProperty propertyName='Name' typeName='string' />"
                      "</ECClass>"
                      "<ECRelationshipClass typeName = 'AbstractRel' isDomainClass = 'False' isStruct='False' isCustomAttributeClass='False' />"
                      "</ECSchema>",
                      true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "ecdbschemarules.ecdb");
    ASSERT_FALSE(asserted);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.AbstractClass (Name) VALUES('bla')")) << "INSERT with abstract class should fail";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.DomainClass (Name) VALUES('bla')")) << "INSERT with domain class should succeed";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "INSERT with domain class should succeed";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.AbstractRel1 (SourceECInstanceId, TargetECInstanceId) VALUES(1,2)")) << "INSERT with abstract relationship should fail";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.AbstractRel2 (SourceECInstanceId, TargetECInstanceId) VALUES(1,2)")) << "INSERT with abstract relationship should fail";
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
        "  <ECRelationshipClass typeName='Rel1' isDomainClass='True' isStruct='True'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='A'/>"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='B'/>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>",
        true, "Will fail in the future as in a ECRelationshipClass isStruct should not be set to true."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "  <ECClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel1' isDomainClass='True' isCustomAttributeClass='True'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                 true, "Will fail in the future as in a ECRelationshipClass isCustomAttributeClass must not be set to true."),

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
                 false, "RelationshipClass constraint must not specify a relationship class."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                    "  <ECClass typeName='A'>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECClass>"
                    "  <ECClass typeName='B'>"
                    "    <ECProperty propertyName='Id' typeName='string' />"
                    "  </ECClass>"
                    "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "RelationshipClass constraint must always specify at least one class."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                    "  <ECClass typeName='A'>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECClass>"
                    "  <ECClass typeName='B'>"
                    "    <ECProperty propertyName='Id' typeName='string' />"
                    "  </ECClass>"
                    "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                    "    <Source polymorphic='True'>"
                    "      <Class class='A'/>"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    true, "Succeeds right now, but will fail in the future as cardinality should always be specified in RelationshipClass constraint."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                    "  <ECClass typeName='A'>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECClass>"
                    "  <ECClass typeName='B'>"
                    "    <ECProperty propertyName='Id' typeName='string' />"
                    "  </ECClass>"
                    "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "RelationshipClass constraint must not be left out."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='False' isStruct='False' isCustomAttribute='False' />"
                "</ECSchema>",
                true, "For abstract relationship class no constraints must be specified."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
                 "    <Source cardinality='(0,1)' polymorphic='True'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "For abstract relationship class no constraints must be specified."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "For abstract relationship class no constraints must be specified."),

        TestItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "For abstract relationship class no constraints must be specified.")};

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
