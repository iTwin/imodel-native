/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaRules_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECDbSchemaRules : SchemaImportTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, Casing)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TESTCLASS\" >"
                 "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Classes with names differing only by case."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Properties only differing by case within a class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"SubClass\" >"
                 "    <BaseClass>TestClass</BaseClass>"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Properties only differing by case in a sub and base class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" >"
                 "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"TestClass2\" >"
                 "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 true, "Properties differing only by case in two unrelated classes."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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


    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, SchemaNamespacePrefix)
    {
    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='123' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix has to be an ECName.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix must not be unset.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECClass>"
                      "</ECSchema>",
                      false, "Namespace prefix must not be empty.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem({"<ECSchema schemaName='Schema1' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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
    SchemaItem firstSchemaTestItem("<ECSchema schemaName='Schema1' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                                 "  <ECClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECClass>"
                                 "</ECSchema>",
                                 true, "");

    SchemaItem secondSchemaTestItem("<ECSchema schemaName='Schema2' nameSpacePrefix='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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
TEST_F(ECDbSchemaRules, Instantiability)
    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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
TEST_F(ECDbSchemaRules, PropertyOfSameTypeAsClass)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of same type as class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\" >"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" isStruct=\"true\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of subtype of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is array of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECClass>"
                 "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "Property is of array of subclass of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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


    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, Relationship)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='False' isStruct='False' isCustomAttribute='False' />"
                "</ECSchema>",
                true, "For abstract relationship class no constraints must be specified."),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, RelationshipCardinality)
    {
            {
            //(1,1):(1,1)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(1,1):(1,1)]> Min of (1,1) cardinality constraint is not enforced yet. (1,1) is interpreted as (0,1)";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(1,1):(1,1)]> Min of (1,1) cardinality constraint is not enforced yet. (1,1) is interpreted as (0,1)";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();

            //Test that child can have one parent at most (enforce (0,1) parent cardinality)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a2Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1,1):(1,1)]> Max of (1,1) cardinality constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b2Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1,1):(1,1)]> Max of (1,1) cardinality constraint is expected to be enforced";
            }

            {
            //(1,1):(1,N)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(1,1):(1,N)]> Min of (1,1) cardinality constraint is not enforced yet. (1,1) is interpreted as (0,1)";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(1,1):(1,N)]> Min of (1,N) cardinality constraint is not enforced yet. (1,1) is interpreted as (0,1)";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();

            //Test that child can have one parent at most
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a2Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1,1):(1,N)]> Max of (1,1) cardinality constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b2Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step()) << "[(1,1):(1,N)]> (1,N) cardinality is not expected to be violated by a second child" << relStmt.GetNativeSql() << " Error:" << ecdb.GetLastError().c_str();
            }

            {
            //(0,1):(0,1)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(0,1):(0,1)]> Child without parent is allowed to exist";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(0,1):(0,1)]> Child without parent is allowed to exist";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();

            //Test that child can have one parent at most (enforce (0,1) parent cardinality)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a2Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,1)]> Max of (0,1) cardinality constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b2Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,1)]> Max of (0,1) cardinality constraint is expected to be enforced";
            }

            {
            //(0,1):(0,N)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='B'>"
                "    <ECProperty propertyName='Id' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(0,1):(0,N)]> Child without parent is allowed to exist";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(0,1):(0,N)]> Child without parent is allowed to exist";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();

            //Test that child can have one parent at most (enforce (0,1) parent cardinality)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a2Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b1Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,N)]> Max of (0,1) cardinality constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, b2Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step()) << "[(0,1):(0,N)]> More than one child can exist";
            }

        //** Unenforced cardinality for self-joins
            {
            ECDbR ecdb = SetupECDb("relcardinality_selfjoins.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='Rel' isDomainClass='True'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key, a3Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a3Key));
            aStmt.Reset();

            //Test that child can have one parent at most (enforce (0,1) parent cardinality)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a2Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, a2Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a1Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a3Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, a3Key.GetECClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a3Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(2, a3Key.GetECClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a1Key.GetECInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindInt64(4, a1Key.GetECClassId()));
            //THIS SHOULD ACTUALLY FAIL, but we cannot enforce that in ECDb
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, ConsistentClassHierarchy)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isDomainClass='True'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isStruct='True' isDomainClass='False'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A domain base class must not have struct subclasses."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isDomainClass='True'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isDomainClass='True' isStruct='True'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A domain base class must not have struct subclasses, even if the subclass is a struct and a domain class at the same time."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECClass typeName=\"A\" isStruct='True' isDomainClass='False'>"
                 "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                 "  </ECClass>"
                 "  <ECClass typeName=\"B\" isDomainClass='True'>"
                 "    <BaseClass>A</BaseClass>"
                 "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
                 "  </ECClass>"
                 "</ECSchema>",
                 false, "A struct base class must have only struct subclasses."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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

    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaRules, RelationshipKeyProperties)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='Authority' >"
                      "    <ECProperty propertyName='Name' typeName='string' />"
                      "  </ECClass>"
                      "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                      "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                      "    <ECProperty propertyName='Namespace' typeName='string' />"
                      "    <ECProperty propertyName='Code' typeName='string' />"
                      "  </ECClass>"
                      "  <ECClass typeName='Element' >"
                      "    <ECProperty propertyName='ModelId' typeName='long' />"
                      "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                      "  </ECClass>"
                      "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                      "    <Source cardinality='(1,1)' polymorphic='False'>"
                      "        <Class class='Authority' />"
                      "     </Source>"
                      "     <Target cardinality='(0,N)' polymorphic='True'>"
                      "         <Class class='Element'>"
                      "             <Key>"
                      "                 <Property name='Code.AuthorityId'/>"
                      "             </Key>"
                      "         </Class>"
                      "     </Target>"
                      "  </ECRelationshipClass>"
                      "</ECSchema>",
                 true, ""),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "  <ECClass typeName='Authority' >"
                      "    <ECProperty propertyName='Name' typeName='string' />"
                      "  </ECClass>"
                      "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                      "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                      "    <ECProperty propertyName='Namespace' typeName='string' />"
                      "    <ECProperty propertyName='Code' typeName='string' />"
                      "  </ECClass>"
                      "  <ECClass typeName='Element' >"
                      "    <ECProperty propertyName='ModelId' typeName='long' />"
                      "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                      "  </ECClass>"
                      "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                      "    <Source cardinality='(1,1)' polymorphic='False'>"
                      "        <Class class='Authority' />"
                      "     </Source>"
                      "     <Target cardinality='(0,N)' polymorphic='True'>"
                      "         <Class class='Element'>"
                      "             <Key>"
                      "                 <Property name='Code'/>"
                      "             </Key>"
                      "         </Class>"
                      "     </Target>"
                      "  </ECRelationshipClass>"
                      "</ECSchema>",
                 false, "Struct property not allowed as key property"),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "  <ECClass typeName='Authority' >"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                 "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                 "    <ECProperty propertyName='Namespace' typeName='string' />"
                 "    <ECProperty propertyName='Code' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Element' >"
                 "    <ECProperty propertyName='ModelId' typeName='long' />"
                 "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                 "    <Source cardinality='(1,1)' polymorphic='False'>"
                 "        <Class class='Authority' />"
                 "     </Source>"
                 "     <Target cardinality='(0,N)' polymorphic='True'>"
                 "         <Class class='Element'>"
                 "             <Key>"
                 "                 <Property name='AuthorityId'/>"
                 "             </Key>"
                 "         </Class>"
                 "     </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "Property does not exist"),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "  <ECClass typeName='Authority' >"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                 "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                 "    <ECProperty propertyName='Namespace' typeName='string' />"
                 "    <ECProperty propertyName='Code' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Element' >"
                 "    <ECProperty propertyName='ModelId' typeName='long' />"
                 "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                 "    <Source cardinality='(1,1)' polymorphic='False'>"
                 "        <Class class='Authority' />"
                 "     </Source>"
                 "     <Target cardinality='(0,N)' polymorphic='True'>"
                 "         <Class class='Element'>"
                 "             <Key>"
                 "                 <Property name='CodeBla.AuthorityId'/>"
                 "             </Key>"
                 "         </Class>"
                 "     </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "Property path does not exist"),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='Authority' >"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                "    <ECProperty propertyName='Namespace' typeName='string' />"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='Element' >"
                "    <ECProperty propertyName='ModelId' typeName='long' />"
                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                "    <Source cardinality='(1,1)' polymorphic='False'>"
                "        <Class class='Authority' />"
                "     </Source>"
                "     <Target cardinality='(0,N)' polymorphic='True'>"
                "         <Class class='Element'>"
                "             <Key>"
                "                 <Property name='Code.AuthorityIdBla'/>"
                "             </Key>"
                "         </Class>"
                "     </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "Property path does not exist"),

    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "  <ECClass typeName='Authority' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
            "    <ECProperty propertyName='AuthorityId' typeName='int' />"
            "    <ECProperty propertyName='Namespace' typeName='string' />"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Element' >"
            "    <ECProperty propertyName='ModelId' typeName='long' />"
            "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='False'>"
            "        <Class class='Authority' />"
            "     </Source>"
            "     <Target cardinality='(0,N)' polymorphic='True'>"
            "         <Class class='Element'>"
            "             <Key>"
            "                 <Property name='Code.AuthorityId.Bla'/>"
            "             </Key>"
            "         </Class>"
            "     </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>",
             false, "Property path does not exist"),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='Authority' >"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                "    <ECProperty propertyName='Namespace' typeName='string' />"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='Element' >"
                "    <ECProperty propertyName='ModelId' typeName='long' />"
                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                "    <Source cardinality='(1,1)' polymorphic='False'>"
                "        <Class class='Authority' />"
                "     </Source>"
                "     <Target cardinality='(0,N)' polymorphic='True'>"
                "         <Class class='Element'>"
                "             <Key>"
                 "                 <Property name='ModelId'/>"
                 "                 <Property name='Code.AuthorityId'/>"
                "             </Key>"
                "         </Class>"
                "     </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "Multiple key properties are not supported"),

        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "  <ECClass typeName='Authority' >"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                "    <ECProperty propertyName='Namespace' typeName='string' />"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='Element' >"
                "    <ECProperty propertyName='ModelId' typeName='long' />"
                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                "     <Source cardinality='(1,1)' polymorphic='False'>"
                "        <Class class='Authority' />"
                "     </Source>"
                "     <Target cardinality='(0,N)' polymorphic='True'>"
                "         <Class class='Element'>"
                "             <Key>"
                "                 <Property name='ModelId'/>"
                "             </Key>"
                "         </Class>"
                 "        <Class class='Authority' />"
                 "     </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>",
                false, "Multiple classes in a constraint with key properties are not supported")
        };

    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }
    }


END_ECDBUNITTESTS_NAMESPACE
