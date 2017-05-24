/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SchemaRulesTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct SchemaRulesTestFixture : DbMappingTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Casing)
    {
    std::vector<SchemaItem> testItems {
        SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECEntityClass typeName="TestClass" >
                        <ECProperty propertyName="TestProperty" typeName="string" />
                      </ECEntityClass>
                      <ECEntityClass typeName="TESTCLASS" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml",
                 false, "Classes with names differing only by case."),

        SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml",
                 false, "Properties only differing by case within a class."),

        SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">"
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="SubClass">
                     <BaseClass>TestClass</BaseClass>
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>"
                 </ECSchema>)xml",
                 false, "Properties only differing by case in a sub and base class."),

        SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">"
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="TestClass2" >
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml",
                 true, "Properties differing only by case in two unrelated classes."),

        SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">"
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="SubClass">
                     <BaseClass>TestClass</BaseClass>
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="TESTCLASS" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName=\"TESTClass\" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName=\"Foo\" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName=\"FOO\" >
                     <ECProperty propertyName="Property" typeName="string" />
                     <ECProperty propertyName="PROPerty" typeName="string" />
                     <ECProperty propertyName="PROPERTY" typeName="string" />
                     <ECProperty propertyName="Prop2" typeName="string" />
                     <ECProperty propertyName="PROP2" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml",
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
TEST_F(SchemaRulesTestFixture, SchemaAlias)
    {
    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' alias='123' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>",
                      false, "Alias has to be an ECName.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>",
                      false, "Alias must not be unset.");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' alias='' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>",
                      false, "Alias must not be empty");
    AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
    }

    {
    SchemaItem testItem({"<ECSchema schemaName='Schema1' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                 "  <ECEntityClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECEntityClass>"
                                 "</ECSchema>",
                        "<ECSchema schemaName='Schema2' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                        "  <ECEntityClass typeName='TestClass2' >"
                        "    <ECProperty propertyName='TestProperty' typeName='string' />"
                        "  </ECEntityClass>"
                        "</ECSchema>"}, 
                       false, "Two schemas with same alias is not supported.");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "ecdbschemarules_duplicateschemaaliases.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    SchemaItem firstSchemaTestItem("<ECSchema schemaName='Schema1' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                 "  <ECEntityClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECEntityClass>"
                                 "</ECSchema>",
                                 true, "");

    SchemaItem secondSchemaTestItem("<ECSchema schemaName='Schema2' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECEntityClass typeName='TestClass2' >"
                                  "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "</ECSchema>",
                                  false, "Two schemas with same aliases is not supported.");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, firstSchemaTestItem, "ecdbschemarules_duplicateschemaaliases.ecdb");
    ASSERT_FALSE(asserted);

    AssertSchemaImport(asserted, ecdb, secondSchemaTestItem);
    ASSERT_FALSE(asserted);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, BaseClasses)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Base1" modifier="Abstract">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Base2" modifier="Abstract">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                     </ECEntityClass>
               </ECSchema>)xml", false, "Multi-inheritance for abstract entity classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                   <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IMixin1" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IMixin2" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop3" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base</BaseClass>
                      <BaseClass>IMixin1</BaseClass>
                      <BaseClass>IMixin2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                     </ECEntityClass>
               </ECSchema>)xml", true, "Implementing multiple mixins is supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECStructClass typeName="Base1" modifier="Abstract">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Base2" modifier="Abstract">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECStructClass>
               </ECSchema>)xml", false, "Multi-inheritance for abstract struct classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECCustomAttributeClass typeName="Base1" modifier="Abstract" appliesTo="Schema">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Base2" modifier="Abstract" appliesTo="Schema">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Sub" modifier="Abstract" appliesTo="Schema">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECCustomAttributeClass>
               </ECSchema>)xml", false, "Multi-inheritance for abstract custom attribute classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Base1" modifier="Abstract">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Base2" modifier="Abstract">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="None">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                     </ECEntityClass>
               </ECSchema>)xml", false, "Multi-inheritance for non-abstract entity classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECStructClass typeName="Base1" modifier="Abstract">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Base2" modifier="Abstract">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Sub" modifier="None">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECStructClass>
               </ECSchema>)xml", false, "Multi-inheritance for non-abstract struct classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECCustomAttributeClass typeName="Base1" modifier="Abstract" appliesTo="Schema">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Base2" modifier="Abstract" appliesTo="Schema">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Sub" modifier="None" appliesTo="Schema">
                      <BaseClass>Base1</BaseClass>
                      <BaseClass>Base2</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECCustomAttributeClass>
               </ECSchema>)xml", false, "Multi-inheritance for non-abstract custom attribute classes is not supported"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Base" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECEntityClass>
               </ECSchema>)xml", false, "Abstract entity class may not inherit concrete entity class"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECStructClass typeName="Base" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECStructClass>
               </ECSchema>)xml", false, "Abstract struct class may not inherit concrete struct class"));

    testSchemas.push_back(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECCustomAttributeClass typeName="Base" modifier="None" appliesTo="Schema">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Sub" modifier="Abstract" appliesTo="Schema">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECCustomAttributeClass>
               </ECSchema>)xml", false, "Abstract CA class may not inherit concrete CA class"));

    AssertSchemaImport(testSchemas, "ecdbschemarules_baseclasses.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, MixinsAndECDbMapCAs)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract"/>
                   <ECEntityClass typeName="IMixin" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>NotMapped</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
               </ECSchema>)xml", false, "Mixin may not have ClassMap CA"));

    testSchemas.push_back(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract"/>
                   <ECEntityClass typeName="IMixin" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
               </ECSchema>)xml", false, "Mixin may not have ClassMap CA"));

    testSchemas.push_back(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract"/>
                   <ECEntityClass typeName="IMixin" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                <Indexes>
                                    <DbIndex>
                                       <IsUnique>False</IsUnique>
                                       <Name>myindex</Name>
                                       <Properties>
                                          <string>Prop1</string>
                                       </Properties>
                                    </DbIndex>
                                </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
               </ECSchema>)xml", true, "Mixin may have DbIndexList CA"));


    testSchemas.push_back(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract"/>
                   <ECEntityClass typeName="IMixin" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>Base</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" >
                        <ECCustomAttributes>
                            <PropertyMap xmlns="ECDbMap.02.00">
                                <IsNullable>false</IsNullable>
                            </PropertyMap>
                        </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
               </ECSchema>)xml", false, "Mixin may not have PropertyMap CA"));

    AssertSchemaImport(testSchemas, "ecdbschemarules_mixins_ecdbmapcas.ecdb");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Instantiability)
    {
    SchemaItem testItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECEntityClass typeName="AbstractClass" modifier="Abstract">
                          <ECProperty propertyName="Name" typeName="string" />
                      </ECEntityClass>
                      <ECEntityClass typeName="DomainClass" modifier="Sealed">
                          <ECProperty propertyName="Name" typeName="string" />
                          <ECNavigationProperty propertyName="Parent" relationshipName="AbstractRel" direction="Backward"/>
                      </ECEntityClass>
                      <ECStructClass typeName="Struct" >
                          <ECProperty propertyName="Name" typeName="string" />
                      </ECStructClass>
                      <ECRelationshipClass typeName="AbstractRel" modifier="Abstract">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="DomainClass"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="DomainClass"/>
                            </Target>
                          </ECRelationshipClass>
                      </ECSchema>)xml");

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
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.AbstractRel (SourceECInstanceId, TargetECInstanceId) VALUES(1,2)")) << "INSERT with abstract relationship should fail";
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, KindOfQuantities)
    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT" />
                    <ECStructClass typeName="TestStruct" >
                       <ECProperty propertyName="Name" typeName="string"/>
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass" >
                       <ECProperty propertyName="Prop1" typeName="double" kindOfQuantity="MyKoq" />
                       <ECPrimitiveArrayProperty propertyName="Prop2" typeName="double" kindOfQuantity="MyKoq"/>
                       <ECStructProperty propertyName="Prop3" typeName="TestStruct" kindOfQuantity="MyKoq"/>
                       <ECStructArrayProperty propertyName="Prop4" typeName="TestStruct" kindOfQuantity="MyKoq"/>
                     </ECEntityClass>
                   </ECSchema>)xml", true, "KOQ can be applied to prim, struct, and array properties."), "ecdbschemarules_koq.ecdb");
    ASSERT_FALSE(asserted);

    ECClassCP testClass = ecdb.Schemas().GetClass("TestSchema", "TestClass");
    ASSERT_TRUE(testClass != nullptr);
    KindOfQuantityCP expectedKoq = ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKoq");
    ASSERT_TRUE(expectedKoq != nullptr);
    for (ECPropertyCP prop : testClass->GetProperties())
        {
        ASSERT_EQ(expectedKoq, prop->GetKindOfQuantity()) << prop->GetName().c_str();
        }
    ecdb.CloseDb();

    std::vector<SchemaItem> invalidSchemas {
        SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string"/>
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string"/>
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" kindOfQuantity="MyKoq"/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing" >
                       <Source multiplicity="(0..1)" polymorphic="true" roleLabel="Parent">
                            <Class class="Parent"/>
                       </Source>
                       <Target multiplicity="(0..*)" polymorphic="true" roleLabel="Child">
                            <Class class="Child"/>
                       </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "KOQ cannot be applied to a nav prop."),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="Bla" />
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Code" typeName="string" kindOfQuantity="MyKoq"/>
                    </ECEntityClass>
                   </ECSchema>)xml", false, "Invalid KOQ persistence unit"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT"
                    presentationUnits="FT(real);Bla(real);M(real)" />
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Code" typeName="string" kindOfQuantity="MyKoq"/>
                    </ECEntityClass>
                   </ECSchema>)xml", false, "Invalid KOQ presentation unit")
        };

    AssertSchemaImport(invalidSchemas, "ecdbschemarules_koq.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, PropertyOfSameTypeAsClass)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
        "  <ECStructClass typeName=\"TestClass\" >"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
                 "  </ECStructClass>"
                 "</ECSchema>",
                 false, "Property is of same type as class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                 "  <ECStructClass typeName=\"Base\" >"
                 "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" />"
                 "  </ECStructClass>"
                 "  <ECStructClass typeName=\"Sub\" >"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECStructClass>"
                 "</ECSchema>",
                 false, "Property is of subtype of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                 "  <ECStructClass typeName=\"TestClass\" >"
                 "    <ECStructArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECStructClass>"
                 "</ECSchema>",
                 false, "Property is array of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                 "  <ECStructClass typeName=\"Base\" >"
                 "    <ECStructArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECStructClass>"
                 "  <ECStructClass typeName=\"Sub\" >"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                 "  </ECStructClass>"
                 "</ECSchema>",
                 false, "Property is of array of subclass of class."),

        SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                 "  <ECStructClass typeName=\"Base\" >"
                 "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                 "  </ECStructClass>"
                 "  <ECStructClass typeName=\"Sub\" >"
                 "     <BaseClass>Base</BaseClass>"
                 "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
                 "  </ECStructClass>"
                 "  <ECEntityClass typeName=\"SUB\" >"
                 "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
                 "  </ECEntityClass>"
                 "</ECSchema>",
                 false, "Case-sensitive class and prop names and property is of array of subclass of class.")
        };

    AssertSchemaImport(testItems, "ecdbschemarules.ecdb");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, NavigationProperties)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..2)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A nav prop must not point to a relationship end with multiplicity greater than 1"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="Rel" direction="Forward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Foo"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Foo"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", true, "A class can have two navigation properties for the same relationship with different direction"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class cannot have two navigation properties for the same relationship"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class cannot have two navigation properties for the same relationship (even if one is inherited from a base class)"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema5" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Foo" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="FooSub" >
                       <BaseClass>Foo</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="Rel" direction="Forward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Foo"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Foo"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class can have two navigation properties for the same relationship (if one is inherited from a base class) if the direction is different"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema6" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="RelSub" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="RelSub" modifier="Sealed" strength="Referencing">
                       <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class cannot have two navigation properties to the same relationship hierarchy"),

        SchemaItem(R"xml(<ECSchema schemaName="TestSchema7" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="RelSub1" direction="Backward" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="RelSub2" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="RelSub1" modifier="Sealed" strength="Referencing">
                       <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="RelSub2" modifier="Sealed" strength="Referencing">
                       <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class cannot have two navigation properties to the same relationship hierarchy"),

    SchemaItem(R"xml(<ECSchema schemaName="TestSchema8" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="MyParent2" relationshipName="RelSub" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="RelSub" modifier="Sealed" strength="Referencing">
                       <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="ChildSub"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml", false, "A class cannot have two navigation properties to the same relationship hierarchy")
    };

    AssertSchemaImport(testItems, "ecdbschemarules_navprops.ecdb");

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Relationship)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='Bentley_Standard_Classes' version='01.00' alias='bsc' />"
        "  <ECEntityClass typeName='A'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='B'>"
        "    <ECProperty propertyName='CodeId' typeName='string' />"
        "    <ECNavigationProperty propertyName='Any' relationshipName='Rel' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
        "      <Class class='bsc:AnyClass'/>"
        "    </Source>"
        "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
        "      <Class class='B'/>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>",
        false, "AnyClass is not supported by ECDb"),

        SchemaItem("<ECSchema schemaName='TestSchema2' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "<ECSchemaReference name='Bentley_Standard_Classes' version='01.00' alias='bsc' />"
                   "  <ECEntityClass typeName='A'>"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='B'>"
                   "    <ECProperty propertyName='CodeId' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='Rel'  modifier='Sealed' strength='Referencing'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                   "      <Class class='A'/>"
                   "    </Source>"
                   "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                   "      <Class class='bsc:AnyClass'/>"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>",
                   false, "AnyClass is not supported by ECDb"),

        SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "<ECSchemaReference name='Bentley_Standard_Classes' version='01.00' alias='bsc' />"
                   "  <ECEntityClass typeName='A'>"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='B'>"
                   "    <ECProperty propertyName='CodeId' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='Rel'  modifier='Sealed' strength='Referencing'>"
                   "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Source' >"
                   "      <Class class='A'/>"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                   "      <Class class='B'/>"
                   "      <Class class='bsc:AnyClass'/>"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>",
                   false, "AnyClass is not supported by ECDb"),

        SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "  <ECEntityClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECEntityClass>"
                 "  <ECEntityClass typeName='B'>"
                 "    <ECProperty propertyName='CodeId' typeName='string' />"
                 "   <ECNavigationProperty propertyName='A' relationshipName='Rel1' direction='Backward' />"
                 "  </ECEntityClass>"
                 "  <ECRelationshipClass typeName='Rel1' modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2' modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='Rel1'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class."),

        SchemaItem("<ECSchema schemaName='TestSchema5' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "  <ECEntityClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECEntityClass>"
                 "  <ECEntityClass typeName='B'>"
                 "    <ECProperty propertyName='CodeId' typeName='string' />"
                 "   <ECNavigationProperty propertyName='A' relationshipName='Rel1' direction='Backward' />"
                   "   <ECNavigationProperty propertyName='A2' relationshipName='Rel2' direction='Backward' />"
                   "  </ECEntityClass>"
                 "  <ECRelationshipClass typeName='Rel1' modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2' modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='Rel1'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class."),

        SchemaItem("<ECSchema schemaName='TestSchema6' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "  <ECEntityClass typeName='A'>"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECEntityClass>"
                 "  <ECEntityClass typeName='B'>"
                 "    <ECProperty propertyName='CodeId' typeName='string' />"
                 "   <ECNavigationProperty propertyName='A' relationshipName='Rel1' direction='Backward' />"
                   "   <ECNavigationProperty propertyName='A2' relationshipName='Rel2' direction='Backward' />"
                   "  </ECEntityClass>"
                 "  <ECRelationshipClass typeName='Rel1'  modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "  <ECRelationshipClass typeName='Rel2'  modifier='Sealed' strength='Referencing'>"
                 "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "      <Class class='Rel1'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                 "      <Class class='B'/>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>",
                 false, "RelationshipClass constraint must not specify a relationship class."),

        SchemaItem("<ECSchema schemaName='TestSchema7' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "  <ECEntityClass typeName='A'>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='B'>"
                    "    <ECProperty propertyName='CodeId' typeName='string' />"
                    "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                    "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                    "    </Source>"
                    "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "Sealed RelationshipClass must at least specify one constraint class each."),

        SchemaItem("<ECSchema schemaName='TestSchema8' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "  <ECEntityClass typeName='A'>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='B'>"
                    "    <ECProperty propertyName='CodeId' typeName='string' />"
                   "     <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                   "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                    "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "RelationshipClass constraint must not be left out."),

        SchemaItem("<ECSchema schemaName='TestSchema9' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Abstract' strength='Referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                 "      <Class class='A'/>"
                 "    </Source>"
                 "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "Abstract relationship class must have fully defined constraints"),

        SchemaItem("<ECSchema schemaName='TestSchema10' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Abstract' strength='Referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "  </ECRelationshipClass>"
                "</ECSchema>", false, "Abstract relationship class must have fully defined constraints"),

        SchemaItem("<ECSchema schemaName='TestSchema11' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Abstract' strength='Referencing'>"
                "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", false, "Abstract relationship class must have fully defined constraints"),

        SchemaItem("<ECSchema schemaName='TestSchema12' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Abstract' strength='Referencing'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                   "      <Class class='A'/>"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='Rel2' modifier='Abstract' strength='Referencing'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                   "      <Class class='A'/>"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Source'>"
                   "      <Class class='B'/>"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ConcreteRel' modifier='Sealed' strength='Referencing'>"
                   "    <BaseClass>Rel</BaseClass>"
                   "    <BaseClass>Rel2</BaseClass>"
                   "    <Source multiplicity='(0..1)' polymorphic='True'>"
                   "      <Class class='A'/>"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True'>"
                   "      <Class class='B'/>"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "RelationshipClass cannot have multiple base classes.")
                };

    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, "ecdbschemarules.ecdb");
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipCardinality)
    {
            {
            //(1,1):(1,1)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "    <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(1..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, bStmt.Step(b1Key)) << "Multiplicity of (1,1) means that a B instance cannot be created without assigning it an A instance";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key)) << "Multiplicity of (1,1) for referenced end is not enforced yet";
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            //Test that child can have one parent at most (enforce (0..1) parent multiplicity)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a2Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1..1):(1..1)]> Max of (1..1) multiplicity constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b2Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1..1):(1..1)]> Max of (1..1) multiplicity constraint is expected to be enforced";
            }

            {
            //(1,1):(1,N)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "    <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(1..*)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, bStmt.Step(b1Key)) << "Multiplicity of (1,1) means that a B instance cannot be created without assigning it an A instance";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key)) << "[(1..1):(1..*)]> (1..*) multiplicity is not expected to be violated by a second child" << bStmt.GetNativeSql() << " Error:" << ecdb.GetLastError().c_str();
            bStmt.Reset();
            bStmt.ClearBindings();

            //Test that child can have one parent at most
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a2Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(1..1):(1..*)]> Max of (1..1) multiplicity constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();
            }

            {
            //(0,1):(0,1)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <ECCustomAttributes>"
                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "    </ECCustomAttributes>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
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

            //Test that child can have one parent at most (enforce (0,1) parent multiplicity)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a2Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,1)]> Max of (0,1) multiplicity constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b2Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,1)]> Max of (0,1) multiplicity constraint is expected to be enforced";
            }

            {
            //(0,1):(0,N)
            ECDbR ecdb = SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <ECCustomAttributes>"
                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "    </ECCustomAttributes>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
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

            //Test that child can have one parent at most (enforce (0,1) parent multiplicity)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a2Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b1Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step()) << "[(0,1):(0,N)]> Max of (0,1) multiplicity constraint is expected to be enforced";
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, b2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, b2Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step()) << "[(0,1):(0,N)]> More than one child can exist";
            }

        //** Unenforced cardinality for self-joins
            {
            ECDbR ecdb = SetupECDb("relcardinality_selfjoins.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "   <ECNavigationProperty propertyName='Partner' relationshipName='Rel' direction='Backward' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <ECCustomAttributes>"
                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "    </ECCustomAttributes>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
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

            //Test that child can have one parent at most (enforce (0,1) parent multiplicity)
            ECSqlStatement relStmt;
            ASSERT_EQ(ECSqlStatus::Success, relStmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a2Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, a2Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a1Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a3Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, a3Key.GetClassId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(1, a3Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(2, a3Key.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(3, a1Key.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, relStmt.BindId(4, a1Key.GetClassId()));
            //THIS SHOULD ACTUALLY FAIL, but we cannot enforce that in ECDb, because the unique index is only on the FK column,
            //but it would have to be on FK and PK at the same time.
            ASSERT_EQ(BE_SQLITE_DONE, relStmt.Step());
            relStmt.Reset();
            relStmt.ClearBindings();
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipWithMultipleConstraintClasses)
    {
    std::vector <SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECEntityClass typeName='Foo' >"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Goo' >"
                    "    <ECProperty propertyName='Length' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Hoo' >"
                    "    <ECProperty propertyName='Width' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                    "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                    "        <Class class='Foo' />"
                    "     </Source>"
                    "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                    "         <Class class='Goo'/>"
                    "         <Class class='Hoo'/>"
                    "     </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false),
        SchemaItem("<ECSchema schemaName='TestSchema2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "  <ECEntityClass typeName='Foo' >"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Goo' >"
                    "    <ECProperty propertyName='Length' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Hoo' >"
                    "    <ECProperty propertyName='Width' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                    "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                    "        <Class class='Foo' />"
                    "         <Class class='Hoo'/>"
                    "     </Source>"
                    "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                    "         <Class class='Goo'/>"
                    "     </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "Multiple constraint classes which map to more than one table on referenced side is not supported"),
        SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "  <ECEntityClass typeName='Base' >"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Sub' >"
                    "    <BaseClass>Base</BaseClass>"
                    "    <ECProperty propertyName='Length' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Hoo' >"
                    "    <ECProperty propertyName='Width' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                    "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                    "         <Class class='Hoo'/>"
                    "     </Source>"
                    "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                    "         <Class class='Base'/>"
                    "         <Class class='Sub'/>"
                    "     </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                    false, "Multiple constraint classes without specifying abstract constraint base class"), 
        SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "  <ECEntityClass typeName='Base' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Sub' >"
                   "    <BaseClass>Base</BaseClass>"
                   "    <ECProperty propertyName='Length' typeName='long' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Hoo' >"
                   "    <ECProperty propertyName='Width' typeName='long' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                   "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                   "         <Class class='Hoo'/>"
                   "     </Source>"
                   "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                   "         <Class class='Base'/>"
                   "         <Class class='Sub'/>"
                   "     </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>",
                   true, "Multiple constraint classes with abstract constraint class"),
        SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                    "  <ECEntityClass typeName='Base' >"
                    "     <ECCustomAttributes>"
                    "        <ClassMap xmlns='ECDbMap.02.00'>"
                    "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "        </ClassMap>"
                    "     </ECCustomAttributes>"
                    "    <ECProperty propertyName='Name' typeName='string' />"
                   "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                   "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Sub' >"
                    "    <BaseClass>Base</BaseClass>"
                    "    <ECProperty propertyName='Length' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECEntityClass typeName='Hoo' >"
                    "    <ECProperty propertyName='Width' typeName='long' />"
                    "  </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                    "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                    "         <Class class='Hoo'/>"
                    "     </Source>"
                    "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                    "         <Class class='Base'/>"
                    "         <Class class='Sub'/>"
                    "     </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>",
                   true, "Multiple constraint classes")
        };

    AssertSchemaImport(testItems, "ecdbschemarules.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, LinkTableRelationshipMapStrategy)
    {
    ECDbCR ecdb = SetupECDb("rellinktablemapstrategy.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECEntityClass typeName='A' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='B' >"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='Rel1' modifier='Abstract' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='A'>"
                                  "      <Class class='A' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                                  "      <Class class='B' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='Rel2' modifier='None' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='A'>"
                                  "      <Class class='A' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                                  "      <Class class='B' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='Rel3' modifier='Sealed' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='A'>"
                                  "      <Class class='A' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                                  "      <Class class='B' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    MapStrategyInfo actualMapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(actualMapStrategy, ecdb, ecdb.Schemas().GetClassId("Test1", "Rel1")));
    ASSERT_EQ(MapStrategyInfo::Strategy::TablePerHierarchy, actualMapStrategy.m_strategy) << "Rel class with modifier Abstract";

    ASSERT_TRUE(TryGetMapStrategyInfo(actualMapStrategy, ecdb, ecdb.Schemas().GetClassId("Test1", "Rel2")));
    ASSERT_EQ(MapStrategyInfo::Strategy::TablePerHierarchy, actualMapStrategy.m_strategy) << "Rel class with modifier None";

    ASSERT_TRUE(TryGetMapStrategyInfo(actualMapStrategy, ecdb, ecdb.Schemas().GetClassId("Test1", "Rel3")));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, actualMapStrategy.m_strategy) << "Rel class with modifier Sealed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipInheritance)
    {
    AssertSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECEntityClass typeName='Model' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='Element' >"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'/>"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PhysicalElement' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='Geometry' typeName='binary' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                  "   <BaseClass>ModelHasElements</BaseClass>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Physical Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements (Reversed)'>"
                                  "      <Class class='PhysicalElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", false, "Partially defined abstract relationship"),
                       "reliinheritance1.ecdb");

    AssertSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECEntityClass typeName='Model' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='Element' >"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "    <ECNavigationProperty propertyName='Model1' relationshipName='ModelHasElements' direction='Backward'/>"
                                  "    <ECNavigationProperty propertyName='Model2' relationshipName='ModelHasElements2' direction='Backward'/>"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PhysicalElement' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='Geometry' typeName='binary' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements2' modifier='Abstract' strength='embedding'>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                  "   <BaseClass>ModelHasElements</BaseClass>"
                                  "   <BaseClass>ModelHasElements2</BaseClass>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Physical Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements (Reversed)'>"
                                  "      <Class class='PhysicalElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", false, "Relationship multi inheritance"), 
                       "reliinheritance2.ecdb");

    AssertSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECEntityClass typeName='Model' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='Element' >"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'/>"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PhysicalElement' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='Geometry' typeName='binary' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements' modifier='None' strength='embedding'>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                  "   <BaseClass>ModelHasElements</BaseClass>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model has Physical Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements (Reversed)'>"
                                  "      <Class class='PhysicalElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", true, "Inheriting relationship class with modifier None"), 
                       "relinheritance3.ecdb");

    AssertSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                  "  <ECEntityClass typeName='Model' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='Element' modifier='Abstract'>"
                                  "    <ECCustomAttributes>"
                                  "        <ClassMap xmlns='ECDbMap.02.00'>"
                                  "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                  "        </ClassMap>"
                                  "    </ECCustomAttributes>"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PhysicalElement' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='Geometry' typeName='binary' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "    <ECProperty propertyName='Ordinal' typeName='int' />"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
                                  "   <BaseClass>ModelHasElements</BaseClass>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements (Reversed)'>"
                                  "      <Class class='PhysicalElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", true, "Additional properties on root rel class"),
                       "relinheritance4.ecdb");

    AssertSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test5' alias='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                  "  <ECEntityClass typeName='Model' >"
                                  "    <ECProperty propertyName='Name' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='Element' modifier='Abstract'>"
                                  "    <ECCustomAttributes>"
                                  "        <ClassMap xmlns='ECDbMap.02.00'>"
                                  "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                  "        </ClassMap>"
                                  "    </ECCustomAttributes>"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PhysicalElement' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='Geometry' typeName='binary' />"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
                                  "   <BaseClass>ModelHasElements</BaseClass>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements'>"
                                  "      <Class class='Model' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model has Physical Elements (Reversed)'>"
                                  "      <Class class='PhysicalElement' />"
                                  "    </Target>"
                                  "    <ECProperty propertyName='Ordinal' typeName='int' />"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", true, "Additional properties on rel subclass"), //WIP: want to disallow this eventually
                       "relinheritance5.ecdb");

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void AssertRelationship(ECDbCR ecdb, ECDbTestFixture::SchemaItem const& schemaItem, Utf8CP schemaName, Utf8CP relationshipClassName, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey)
    {
    //insert relationship
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s.%s(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(%s,%s,%s,%s)",
                  schemaName, relationshipClassName, sourceKey.GetInstanceId().ToString().c_str(), sourceKey.GetClassId().ToString().c_str(),
                  targetKey.GetInstanceId().ToString().c_str(), targetKey.GetClassId().ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    stmt.Finalize();

    const_cast<ECDbR>(ecdb).SaveChanges();
    //select
    ecsql.Sprintf("SELECT SourceECInstanceId, SourceECClassId FROM %s.%s WHERE TargetECInstanceId=%s",
                  schemaName, relationshipClassName, targetKey.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(sourceKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(sourceKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    stmt.Finalize();

    ecsql.Sprintf("SELECT TargetECInstanceId, TargetECClassId FROM %s.%s WHERE SourceECInstanceId=%s",
                  schemaName, relationshipClassName, sourceKey.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(targetKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(targetKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    stmt.Finalize();

    //delete relationship
    ecsql.Sprintf("DELETE FROM %s.%s WHERE SourceECInstanceId=%s AND SourceECClassId=%s AND TargetECInstanceId=%s AND TargetECClassId=%s",
                  schemaName, relationshipClassName, sourceKey.GetInstanceId().ToString().c_str(), sourceKey.GetClassId().ToString().c_str(),
                  targetKey.GetInstanceId().ToString().c_str(), targetKey.GetClassId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    ASSERT_EQ(1, ecdb.GetModifiedRowCount()) << schemaItem.m_name.c_str() << ": ECSQL: " << ecsql.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_UnsupportedCases)
    {
    std::vector<SchemaItem> unsupportedSchemas;
    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='ParentProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='ChildProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "End table relationship requires a navigation property on FK end"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema2' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='ParentProp' typeName='long' />"
                                            "    <ECNavigationProperty propertyName='Child' relationshipName='ParentHasChildren' direction='Forward'/>"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='ChildProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "End table relationship requires a navigation property on FK end"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Foo' >"
                                            "    <ECProperty propertyName='FooProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Goo' >"
                                            "    <ECProperty propertyName='GooProp' typeName='long' />"
                                            "    <ECNavigationProperty propertyName='Foo' relationshipName='FooHasGoo' direction='Forward'/>"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='FooHasGoo' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Foo' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Goo' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "End table relationship requires a navigation property on FK end"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='ParentProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='ChildProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Cardinality N:N and Embedding is not supported"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema5' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='ParentProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='ChildProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                                            "     <ECCustomAttributes>"
                                            "         <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                                            "     </ECCustomAttributes>"
                                            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "ForeignKeyConstraint CA cannot applied to link table (as implied from cardinality N:N)"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema6' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='ParentProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='ChildProp' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                                            "     <ECCustomAttributes>"
                                            "         <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                                            "     </ECCustomAttributes>"
                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "    <ECProperty propertyName='RelProp' typeName='long' />"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "ForeignKeyConstraint CA cannot applied to link table (as implied from additional property on relationship class)"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema7' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentA' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PA' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentB' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PB' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='C1' typeName='long' />"
                                            "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Referenced end maps to more than one table"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema8' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent1' >"
                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Parent2' >"
                                            "    <ECProperty propertyName='PA' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='C1' typeName='long' />"
                                            "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                                            "    <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Parent Has Children'>"
                                            "        <Class class='Parent1' />"
                                            "        <Class class='Parent2' />"
                                            "     </Source>"
                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                            "         <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Referenced end maps to more than one table"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema9' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentA' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PA' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentB' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PB' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='C1' typeName='long' />"
                                            "    <ECNavigationProperty propertyName='Parent' relationshipName='ChildHasParent' direction='Forward'/>"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ChildHasParent' strength='embedding' strengthDirection='backward' modifier='Sealed'>"
                                            "     <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Children Has Parent'>"
                                            "         <Class class='Child' />"
                                            "     </Source>"
                                            "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Children Has Parent (Reversed)'>"
                                            "        <Class class='Parent' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Referenced end maps to more than one table"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema10' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentA' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PA' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentB' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PB' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='C1' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ChildHasParent' strength='referencing' modifier='Sealed'>"
                                            "     <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Children Has Parent'>"
                                            "         <Class class='Child' />"
                                            "     </Source>"
                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children Has Parent (Reversed)'>"
                                            "        <Class class='Parent' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Target end of link table maps to more than one table"));

    unsupportedSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema11' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Parent' >"
                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentA' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PA' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='ParentB' >"
                                            "     <BaseClass>Parent</BaseClass>"
                                            "    <ECProperty propertyName='PB' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='Child' >"
                                            "    <ECProperty propertyName='C1' typeName='long' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='ChildHasParent' strength='referencing' modifier='Sealed'>"
                                            "     <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Children Has Parent'>"
                                            "         <Class class='Parent' />"
                                            "     </Source>"
                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children Has Parent (Reversed)'>"
                                            "        <Class class='Child' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>", false, "Source end of link table maps to more than one table"));

    AssertSchemaImport(unsupportedSchemas, "ecdbrelationshipmappingrules.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_SupportedCases)
    {
    {
    std::vector<SchemaItem> supportedSchemas;
    supportedSchemas.push_back(SchemaItem("1:N and holding",
                          "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECEntityClass typeName='Geometry' >"
                          "    <ECProperty propertyName='Type' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='GeometryPart' >"
                          "    <ECProperty propertyName='Stream' typeName='binary' />"
                          "    <ECNavigationProperty propertyName='Geometry' relationshipName='GeometryHoldsParts' direction='Forward'/>"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='holding' modifier='Sealed'>"
                          "     <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Geometry Holds Parts'>"
                          "         <Class class='GeometryPart' />"
                          "     </Source>"
                          "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Geometry Holds Parts (Reversed)'>"
                          "        <Class class='Geometry' />"
                          "     </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>"));

    supportedSchemas.push_back(SchemaItem("N:N and holding",
                                            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "  <ECEntityClass typeName='Geometry' >"
                                            "    <ECProperty propertyName='Type' typeName='string' />"
                                            "  </ECEntityClass>"
                                            "  <ECEntityClass typeName='GeometryPart' >"
                                            "    <ECProperty propertyName='Stream' typeName='binary' />"
                                            "  </ECEntityClass>"
                                            "  <ECRelationshipClass typeName='GeometryHasParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                                            "     <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Geometry Has Parts'>"
                                            "         <Class class='Geometry' />"
                                            "     </Source>"
                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Geometry Has Parts (Reversed)'>"
                                            "        <Class class='GeometryPart' />"
                                            "     </Target>"
                                            "  </ECRelationshipClass>"
                                            "</ECSchema>"));

    AssertSchemaImport(supportedSchemas, "ecdbrelationshipmappingrules.ecdb");
    }

    {
    SchemaItem testSchema("Child hierarchy in SharedTable", 
                          "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='Parent' >"
                          "    <ECProperty propertyName='ParentProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Child' >"
                          "     <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "         </ClassMap>"
                          "     </ECCustomAttributes>"
                          "    <ECProperty propertyName='ChildProp' typeName='int' />"
                          "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ChildA' >"
                          "     <BaseClass>Child</BaseClass>"
                          "    <ECProperty propertyName='ChildAProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ChildB' >"
                          "     <BaseClass>Child</BaseClass>"
                          "    <ECProperty propertyName='ChildBProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                          "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                          "         <Class class='Parent' />"
                          "     </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                          "        <Class class='Child' />"
                          "     </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");
    
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipmappingrules_childhierarchyinsharedtable.ecdb");
    ASSERT_FALSE(asserted);

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Parent(ParentProp) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ChildA(ChildAProp,ChildProp) VALUES(2,2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }
    ecdb.SaveChanges();
    //WIP_REL: Fails because ECSQL DELETE is incorrectly prepared (exp: 126=129)
    //ECSQL: DELETE FROM TestSchema.ParentHasChildren WHERE SourceECInstanceId=1 AND SourceECClassId=129 AND TargetECInstanceId=2 AND TargetECClassId=127
    //->SQL: UPDATE [ts_Child] SET [FK_ts_ParentHasChildren] = NULL
    //       WHERE [ts_Child].[FK_ts_ParentHasChildren] = 1 AND 126 = 129 AND [ts_Child].[ECInstanceId] = 2 AND [ts_Child].[ECClassId] = 127
    AssertRelationship(ecdb, testSchema, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }

    {
    SchemaItem testSchema("Parent hierarchy in SharedTable",
                          "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='Parent' >"
                          "     <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "         </ClassMap>"
                          "     </ECCustomAttributes>"
                          "    <ECProperty propertyName='ParentProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ParentA' >"
                          "     <BaseClass>Parent</BaseClass>"
                          "    <ECProperty propertyName='ParentAProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ParentB' >"
                          "     <BaseClass>Parent</BaseClass>"
                          "    <ECProperty propertyName='ParentBProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Child' >"
                          "    <ECProperty propertyName='ChildProp' typeName='int' />"
                          "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                          "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                          "         <Class class='Parent' />"
                          "     </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                          "        <Class class='Child' />"
                          "     </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipmappingrules_parenthierarchyinsharedtable.ecdb");
    ASSERT_FALSE(asserted);

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ParentA(ParentAProp,ParentProp) VALUES(1,1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Child(ChildProp) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }

    AssertRelationship(ecdb, testSchema, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }


    {
    SchemaItem testSchema("Children in different joined tables, but FK in primary table",
                          "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='Parent' >"
                          "    <ECProperty propertyName='ParentProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Child' >"
                          "     <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "         </ClassMap>"
                          "         <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "     </ECCustomAttributes>"
                          "    <ECProperty propertyName='ChildProp' typeName='int' />"
                          "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ChildA' >"
                          "     <BaseClass>Child</BaseClass>"
                          "    <ECProperty propertyName='ChildAProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='ChildB' >"
                          "     <BaseClass>Child</BaseClass>"
                          "    <ECProperty propertyName='ChildBProp' typeName='int' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' strength='embedding' modifier='Sealed'>"
                          "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                          "         <Class class='Parent' />"
                          "     </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                          "        <Class class='Child' />"
                          "     </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipmappingrules_childreninseparatejoinedtables_fknotinjoinedtable.ecdb");
    ASSERT_FALSE(asserted);

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Parent(ParentProp) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ChildA(ChildAProp) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }

    //WIP_REL: Fails because ECSQL DELETE is incorrectly prepared (exp: 126=129)
    //ECSQL: DELETE FROM TestSchema.ParentHasChildren WHERE SourceECInstanceId=1 AND SourceECClassId=129 AND TargetECInstanceId=2 AND TargetECClassId=127
    AssertRelationship(ecdb, testSchema, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_InvalidInECSql)
    {
    bvector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("Children in different tables",
                                     "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                     "  <ECEntityClass typeName='Parent' >"
                                     "    <ECProperty propertyName='ParentProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Child' >"
                                     "    <ECProperty propertyName='ChildProp' typeName='int' />"
                                     "    <ECNavigationProperty propertyName='Parent' relationshipName='Rel' direction='Backward'/>"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ChildA' >"
                                     "     <BaseClass>Child</BaseClass>"
                                     "    <ECProperty propertyName='ChildAProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ChildB' >"
                                     "     <BaseClass>Child</BaseClass>"
                                     "    <ECProperty propertyName='ChildBProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                                     "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
                                     "         <Class class='Parent' />"
                                     "     </Source>"
                                     "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
                                     "        <Class class='Child' />"
                                     "     </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem("Children in different joined tables, FK in joined tables",
                                     "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                     "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                     "  <ECEntityClass typeName='Parent' >"
                                     "    <ECProperty propertyName='ParentProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Child' >"
                                     "     <ECCustomAttributes>"
                                     "         <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "         </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "    <ECProperty propertyName='ChildProp' typeName='int' />"
                                     "    <ECNavigationProperty propertyName='Parent' relationshipName='Rel' direction='Backward'/>"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GrandchildA' >"
                                     "     <BaseClass>Child</BaseClass>"
                                     "    <ECProperty propertyName='GrandchildAProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GrandchildB' >"
                                     "     <BaseClass>Child</BaseClass>"
                                     "    <ECProperty propertyName='GrandchildBProp' typeName='int' />"
                                     "  </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                     "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Grandchildren'>"
                                     "         <Class class='Parent' />"
                                     "     </Source>"
                                     "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Grandchildren (Reversed)' abstractConstraint='Child'>"
                                     "        <Class class='GrandchildA' />"
                                     "        <Class class='GrandchildB' />"
                                     "     </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    for (SchemaItem const& testSchema : testSchemas)
        {
        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipmappingrules_childreninseparatetables.ecdb");
        ASSERT_FALSE(asserted);

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT SourceECInstance,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ts.Rel"));
        }
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstance,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
        }
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "UPDATE ts.Rel SET SourceECInstanceId=?, TargetECInstanceId=?"));
        }
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "DELETE FROM ts.Rel"));
        }
        }
    }
END_ECDBUNITTESTS_NAMESPACE
