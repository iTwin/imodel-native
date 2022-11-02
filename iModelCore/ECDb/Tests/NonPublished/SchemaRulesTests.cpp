/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct SchemaRulesTestFixture : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Casing)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECEntityClass typeName="TestClass" >
                        <ECProperty propertyName="TestProperty" typeName="string" />
                      </ECEntityClass>
                      <ECEntityClass typeName="TESTCLASS" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml"))) << "Classes with names differing only by case.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml"))) << "Properties only differing by case within a class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="SubClass">
                     <BaseClass>TestClass</BaseClass>
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>"
                 </ECSchema>)xml"))) << "Properties only differing by case in a sub and base class.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                 <ECEntityClass typeName="TestClass" >
                     <ECProperty propertyName="TestProperty" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="TestClass2" >
                     <ECProperty propertyName="TESTPROPERTY" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml"))) << "Properties differing only by case in two unrelated classes.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   <ECEntityClass typeName="TESTClass" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="Foo" >
                     <ECProperty propertyName="Property" typeName="string" />
                   </ECEntityClass>
                   <ECEntityClass typeName="FOO" >
                     <ECProperty propertyName="Property" typeName="string" />
                     <ECProperty propertyName="PROPerty" typeName="string" />
                     <ECProperty propertyName="PROPERTY" typeName="string" />
                     <ECProperty propertyName="Prop2" typeName="string" />
                     <ECProperty propertyName="PROP2" typeName="string" />
                   </ECEntityClass>
                 </ECSchema>)xml"))) << "Class and properties only differing by case within a class.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, SchemaAlias)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema' alias='123' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>"))) << "Alias has to be an ECName.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>"))) << "Alias must not be unset.";
    
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema' alias='' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                      "  <ECEntityClass typeName='TestClass' >"
                      "    <ECProperty propertyName='TestProperty' typeName='string' />"
                      "  </ECEntityClass>"
                      "</ECSchema>"))) << "Alias must not be empty";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({SchemaItem("<ECSchema schemaName='Schema1' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                 "  <ECEntityClass typeName='TestClass1' >"
                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                 "  </ECEntityClass>"
                                 "</ECSchema>"),
                        SchemaItem("<ECSchema schemaName='Schema2' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                        "  <ECEntityClass typeName='TestClass2' >"
                        "    <ECProperty propertyName='TestProperty' typeName='string' />"
                        "  </ECEntityClass>"
                        "</ECSchema>")})) << "Two schemas with same alias is not supported.";


    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_duplicateschemaaliases.ecdb", SchemaItem("<ECSchema schemaName='Schema1' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                                           "  <ECEntityClass typeName='TestClass1' >"
                                                                                           "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                                                                           "  </ECEntityClass>"
                                                                                           "</ECSchema>")));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem("<ECSchema schemaName='Schema2' alias='ns' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                 "  <ECEntityClass typeName='TestClass2' >"
                                                                 "    <ECProperty propertyName='TestProperty' typeName='string' />"
                                                                 "  </ECEntityClass>"
                                                                 "</ECSchema>"))) << "Two schemas with same aliases is not supported.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, BaseClasses)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for abstract entity classes is not supported";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Implementing multiple mixins is supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for abstract struct classes is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for abstract custom attribute classes is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for non-abstract entity classes is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for non-abstract struct classes is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
               </ECSchema>)xml"))) << "Multi-inheritance for non-abstract custom attribute classes is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Base" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECEntityClass>
               </ECSchema>)xml"))) << "Abstract entity class may not inherit concrete entity class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECStructClass typeName="Base" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECStructClass>
                    <ECStructClass typeName="Sub" modifier="Abstract">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECStructClass>
               </ECSchema>)xml"))) << "Abstract struct class may not inherit concrete struct class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECCustomAttributeClass typeName="Base" modifier="None" appliesTo="Schema">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECCustomAttributeClass>
                    <ECCustomAttributeClass typeName="Sub" modifier="Abstract" appliesTo="Schema">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                    </ECCustomAttributeClass>
               </ECSchema>)xml"))) << "Abstract CA class may not inherit concrete CA class";
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                   <ECEntityClass typeName="B" modifier="None">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="RelBase1" modifier="Abstract">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                    <ECRelationshipClass typeName="RelBase2" modifier="Abstract">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                    <ECRelationshipClass typeName="Rel" modifier="Abstract">
                        <BaseClass>RelBase1</BaseClass>
                        <BaseClass>RelBase2</BaseClass>
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="roleLabel">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                </ECSchema>)xml"))) << "Multi-inheritance for relationships is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, MixinsAndECDbMapCAs)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
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
               </ECSchema>)xml"))) << "Mixin may not have ClassMap CA";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
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
               </ECSchema>)xml"))) << "Mixin may not have ClassMap CA";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
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
               </ECSchema>)xml"))) << "Mixin may not have DbIndexList CA";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
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
               </ECSchema>)xml"))) << "Mixin may not have PropertyMap CA";
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Instantiability)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                      </ECSchema>)xml")));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.AbstractClass (Name) VALUES('bla')")) << "INSERT with abstract class should fail";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.DomainClass (Name) VALUES('bla')")) << "INSERT with domain class should succeed";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "INSERT with domain class should succeed";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.AbstractRel (SourceECInstanceId, TargetECInstanceId) VALUES(1,2)")) << "INSERT with abstract relationship should fail";
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, KindOfQuantities)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_koq.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT" />
                    <ECStructClass typeName="TestStruct" >
                       <ECProperty propertyName="Name" typeName="string"/>
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass" >
                       <ECProperty propertyName="Prop1" typeName="double" kindOfQuantity="MyKoq" />
                       <ECArrayProperty propertyName="Prop2" typeName="double" kindOfQuantity="MyKoq"/>
                       <ECStructProperty propertyName="Prop3" typeName="TestStruct" kindOfQuantity="MyKoq"/>
                       <ECStructArrayProperty propertyName="Prop4" typeName="TestStruct" kindOfQuantity="MyKoq"/>
                     </ECEntityClass>
                   </ECSchema>)xml"))) << "KOQ can be applied to prim, struct, and array properties.";

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "TestClass");
    ASSERT_TRUE(testClass != nullptr);
    KindOfQuantityCP expectedKoq = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKoq");
    ASSERT_TRUE(expectedKoq != nullptr);
    for (ECPropertyCP prop : testClass->GetProperties(true))
        {
        ASSERT_EQ(expectedKoq, prop->GetKindOfQuantity()) << prop->GetName().c_str();
        }
    m_ecdb.CloseDb();

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   </ECSchema>)xml"))) << "KOQ cannot be applied to a nav prop.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="Bla" />
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Code" typeName="string" kindOfQuantity="MyKoq"/>
                    </ECEntityClass>
                   </ECSchema>)xml"))) << "Invalid KOQ persistence unit";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <KindOfQuantity typeName="MyKoq" relativeError="0.1" persistenceUnit="FT"
                    presentationUnits="FT(real);Bla(real);M(real)" />
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Code" typeName="string" kindOfQuantity="MyKoq"/>
                    </ECEntityClass>
                   </ECSchema>)xml"))) << "Invalid KOQ presentation unit";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, PropertyOfSameTypeAsClass)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                                                            "  <ECStructClass typeName=\"TestClass\" >"
                                                            "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
                                                            "  </ECStructClass>"
                                                            "</ECSchema>"))) << "Property is of same type as class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                                                            "  <ECStructClass typeName=\"Base\" >"
                                                            "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" />"
                                                            "  </ECStructClass>"
                                                            "  <ECStructClass typeName=\"Sub\" >"
                                                            "     <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                                                            "  </ECStructClass>"
                                                            "</ECSchema>"))) << "Property is of subtype of class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                                                            "  <ECStructClass typeName=\"TestClass\" >"
                                                            "    <ECStructArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                                                            "  </ECStructClass>"
                                                            "</ECSchema>"))) << "Property is array of class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName=\"InvalidSchema\" alias=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                                                            "  <ECStructClass typeName=\"Base\" >"
                                                            "    <ECStructArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                                                            "  </ECStructClass>"
                                                            "  <ECStructClass typeName=\"Sub\" >"
                                                            "     <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
                                                            "  </ECStructClass>"
                                                            "</ECSchema>"))) << "Property is of array of subclass of class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="InvalidSchema" alias="is" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                            <ECStructClass typeName="Base" >
                                                              <ECStructArrayProperty propertyName="Prop1" typeName="Sub" minOccurs="0" maxOccurs="unbounded"/>
                                                            </ECStructClass>
                                                            <ECStructClass typeName="Sub" >
                                                               <BaseClass>Base</BaseClass>
                                                              <ECProperty propertyName="PROP1" typeName="string" />
                                                            </ECStructClass>
                                                            <ECEntityClass typeName="SUB" >
                                                              <ECProperty propertyName="PROP1" typeName="string" />
                                                            </ECEntityClass>
                                                          </ECSchema>)xml"))) << "Case-sensitive class and prop names and property is of array of subclass of class.";

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, CircularStructReferences)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECStructClass typeName="StructA" >
                       <ECStructArrayProperty propertyName="B" typeName="StructB" />
                     </ECStructClass>
                    <ECStructClass typeName="StructB" >
                       <ECStructArrayProperty propertyName="A" typeName="StructA" />
                     </ECStructClass>
                    <ECEntityClass typeName="Foo" >
                       <ECStructProperty propertyName="A" typeName="StructA" />
                       <ECStructProperty propertyName="B" typeName="StructB" />
                     </ECEntityClass>
                    </ECSchema>)xml"))) << "Circular references of two structs.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECStructClass typeName="StructA" >
                       <ECStructArrayProperty propertyName="B" typeName="StructB" />
                     </ECStructClass>
                    <ECStructClass typeName="StructB" >
                       <ECStructArrayProperty propertyName="A" typeName="StructA" />
                     </ECStructClass>
                    <ECEntityClass typeName="Foo" >
                       <ECStructArrayProperty propertyName="A" typeName="StructA" />
                       <ECStructArrayProperty propertyName="B" typeName="StructB" />
                     </ECEntityClass>
                    </ECSchema>)xml"))) << "Circular references of two structs.";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECStructClass typeName="StructA" >
                       <ECStructProperty propertyName="A2" typeName="StructA" />
                     </ECStructClass>
                    </ECSchema>)xml"))) << "Circular references of two structs.";


    //Following is allowed given struct can only have value of struct specified. It does not have polymorphic behaviour
    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECStructClass typeName="StructA" >
                       <ECStructProperty propertyName="B" typeName="StructB" />
                     </ECStructClass>
                    <ECStructClass typeName="StructB" >
                       <ECProperty propertyName="Name" typeName="string" />
                   </ECStructClass>
                    <ECStructClass typeName="SubStructB" >
                        <BaseClass>StructB</BaseClass>
                       <ECStructProperty propertyName="A" typeName="StructA" />
                   </ECStructClass>
                    <ECEntityClass typeName="Foo" >
                       <ECStructProperty propertyName="A" typeName="StructA" />
                       <ECStructProperty propertyName="B" typeName="SubStructB" />
                     </ECEntityClass>
                    </ECSchema>)xml"))) << "Circular references of two structs (via class hierarchy).";
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, NavigationProperties)
    {
     ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A nav prop cannot be applied for a link table rel (implied by M:N cardinality)";

     ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                       <ECProperty propertyName="Order" typeName="int" />
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A nav prop cannot be applied for a link table rel (implied as rel has a property)";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <ECCustomAttributes>
                            <LinkTableRelationshipMap xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A nav prop cannot be applied for a link table rel (because of LinkTableRelationshipMap CA)";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema2a" alias="ts2a" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="MyChild" relationshipName="Rel" direction="Forward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <ECCustomAttributes>
                            <LinkTableRelationshipMap xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A nav prop cannot be applied for a link table rel (because of LinkTableRelationshipMap CA)";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   </ECSchema>)xml"))) << "A nav prop must not point to a relationship end with multiplicity greater than 1";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="MyChild" relationshipName="Rel" direction="Forward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A nav prop can only be defined from FK end to referenced end and not vice versa";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Person" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MySibling" relationshipName="RelSub" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="None" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Person"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Person"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="RelSub" modifier="Sealed" strength="Referencing">
                        <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Person"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Person"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A navigation property must be on the relationship root class, not on relationship sub classes";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties for the same relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Person" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MySibling1" relationshipName="Rel" direction="Backward" />
                       <ECNavigationProperty propertyName="MySibling2" relationshipName="Rel" direction="Forward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Person"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Person"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties for the same relationship, even if direction differs";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema5" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
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
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties for the same relationship (even if one is inherited from a base class)";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema6" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Foo" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
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
                   </ECSchema>)xml"))) << "A class can have two navigation properties for the same relationship (if one is inherited from a base class) if the direction is different";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema7" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties to the same relationship hierarchy";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema8" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties to the same relationship hierarchy";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema9" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
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
                   </ECSchema>)xml"))) << "A class cannot have two navigation properties to the same relationship hierarchy";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema10" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent1" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation property must not be on the abstract constraint class";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema11" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation property is on the constraint class (not on the abstract constraint class)";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema11a" alias="ts11a" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="string" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                        <BaseClass>Child</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"))) << "Navigation property overrides are allowed if the property names are the same";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema11b" alias="ts11b" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="string" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                        <BaseClass>Child</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                        <ECNavigationProperty propertyName="MyParent_1" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"))) << "Navigation property overrides are allowed if the property names are the same";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema11c" alias="ts11c" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="string" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                        <BaseClass>Child</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="RelDer" direction="Backward" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="RelDer" modifier="Abstract" strength="Referencing">
                        <BaseClass>Rel</BaseClass>
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                            <Class class="Child"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"))) << "Navigation property overrides may not change the relationship to a derived relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema12" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="string" />
                        <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                        <BaseClass>Child</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                        <BaseClass>Child</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"))) << "Navigation property must not be on the abstract constraint class";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema13" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties on all constraint classes, having the same name";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema14" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties must be on all constraint classes.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema15" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent1" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent2" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties on all constraint classes must have same name.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema20" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties where one has ForeignKeyConstraint CA and other doesn't";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema21" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties where one has ForeignKeyConstraint CA and other doesn't";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema22" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                <OnDeleteAction>NoAction</OnDeleteAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties with different ForeignKeyConstraint CAs";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema23" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                <OnDeleteAction>NoAction</OnDeleteAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties with different ForeignKeyConstraint CAs";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema24" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub1" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                <OnDeleteAction>Cascade</OnDeleteAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                    <ECEntityClass typeName="ChildSub2" >
                       <BaseClass>Child</BaseClass>
                       <ECProperty propertyName="Cost" typeName="double" />
                       <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" >
                         <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                <OnDeleteAction>SetNull</OnDeleteAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECEntityClass>
                     <ECRelationshipClass typeName="Rel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" abstractConstraint="Child" roleLabel="is owned by">
                            <Class class="ChildSub1"/>
                            <Class class="ChildSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Navigation properties with different ForeignKeyConstraint CAs";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="MyParent" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Nav prop on the source class for a 1:1 cardinality is expected to fail.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Forward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with a Nav prop on the FK end with 'Forward' direction is expected to fail.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with a Nav prop on the FK end with 'Backward' direction is expected to succedd.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class doesn't support the nav property to be mapped to the source class for a 1:N cardinality.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with a Nav prop on the FK end with 'Backward' direction and a 1:N cardinality is expected to succedd.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with an N:1 cardinality expects the Nav property to be on the source class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Backward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with an N:1 cardinality and Nav prop with direction 'Backward' on the source class is expected to fail.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" >
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECNavigationProperty propertyName="NavProp" relationshipName="ParentHasChild" direction="Forward" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Child" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="ParentHasChild" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                         <Class class="Parent"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is referenced by">
                            <Class class="Child"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Relationship class with an N:1 cardinality and Nav prop on the source class is expected to succeed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, Relationship)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "AnyClass is not supported by ECDb";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema2' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "AnyClass is not supported by ECDb";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "AnyClass is not supported by ECDb";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "RelationshipClass constraint must not specify a relationship class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema5' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "RelationshipClass constraint must not specify a relationship class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema6' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "RelationshipClass constraint must not specify a relationship class.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema7' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "Sealed RelationshipClass must at least specify one constraint class each.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema8' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "RelationshipClass constraint must not be left out.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema9' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                            "</ECSchema>"))) << "Abstract relationship class must have fully defined constraints";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema10' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "Abstract relationship class must have fully defined constraints";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema11' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "Abstract relationship class must have fully defined constraints";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema12' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "RelationshipClass cannot have multiple base classes.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipCardinality)
    {
            {
            //(1,1):(1,1) Physical FK
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "    <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward' >"
                "       <ECCustomAttributes>"
                "          <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "       </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(1..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, bStmt.Step(b1Key)) << "Multiplicity of (1,1) means that a B instance cannot be created without assigning it an A instance";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, bStmt.Step(b2Key))<< "A1 is already used by B1 so would violate 1:1 cardinality";
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key)) << "Multiplicity of (1,1) for referenced end is not enforced yet";
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();
            bStmt.ClearBindings();
            }

            {
            //(1,1):(1,1) Logical FK
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
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
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "Multiplicity of (1,1) implies NOT NULL which is not enforced for logical FK though";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key)) << "This is a cardinality violation, but as it is a logical FK it is not enforced";
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key)) << "Multiplicity of (1,1) for referenced end is not enforced yet";
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a2Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();
            bStmt.ClearBindings();
            }

            {
            //(1,1):(1,N) (Physical FK)
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='CodeId' typeName='string' />"
                "    <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward' >"
                "       <ECCustomAttributes>"
                "          <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "       </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(1..*)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, bStmt.Step(b1Key)) << "Multiplicity of (1,1) implies NOT NULL";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key)) << "[(1..1):(1..*)]> (1..*) multiplicity is not expected to be violated by a second child" << bStmt.GetNativeSql() << " Error:" << m_ecdb.GetLastError().c_str();
            bStmt.Reset();
            bStmt.ClearBindings();
            }
            {
            //(1,1):(1,N) (logical FK)
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
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
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(MyA.Id) VALUES (?)"));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "Multiplicity of (1,1) implies NOT NULL which is not enforced for logical FK";
            bStmt.Reset();
            bStmt.ClearBindings();
            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key));
            bStmt.Reset();
            bStmt.ClearBindings();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();

            ASSERT_EQ(ECSqlStatus::Success, bStmt.BindId(1, a1Key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key)) << "[(1..1):(1..*)]> (1..*) multiplicity is not expected to be violated by a second child" << bStmt.GetNativeSql() << " Error:" << m_ecdb.GetLastError().c_str();
            bStmt.Reset();
            bStmt.ClearBindings();
            }

            {
            //(0,1):(0,1)
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' >"
                "       <ECCustomAttributes>"
                "          <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "       </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(0,1):(0,1)]> Child without parent is allowed to exist";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(0,1):(0,1)]> Child without parent is allowed to exist";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();
            }

            {
            //(0,1):(0,N)
            ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemarules_cardinality.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='B'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "   <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'>"
                "       <ECCustomAttributes>"
                "          <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "       </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key;
            ECInstanceKey b1Key, b2Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(ECInstanceId) VALUES (NULL)"));

            ECSqlStatement bStmt;
            ASSERT_EQ(ECSqlStatus::Success, bStmt.Prepare(m_ecdb, "INSERT INTO ts.B(ECInstanceId) VALUES (NULL)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key)) << "[(0,1):(0,N)]> Child without parent is allowed to exist";
            aStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b1Key)) << "[(0,1):(0,N)]> Child without parent is allowed to exist";
            bStmt.Reset();

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            ASSERT_EQ(BE_SQLITE_DONE, bStmt.Step(b2Key));
            bStmt.Reset();
            }

        //**  self-joins
            {
            ASSERT_EQ(SUCCESS, SetupECDb("relcardinality_selfjoins.ecdb", SchemaItem(
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "  <ECEntityClass typeName='A'>"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "   <ECNavigationProperty propertyName='Partner' relationshipName='Rel' direction='Backward'>"
                "       <ECCustomAttributes>"
                "          <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "       </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='Referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Target'>"
                "      <Class class='A'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));
            ECInstanceKey a1Key, a2Key, a3Key;

            ECSqlStatement aStmt;
            ASSERT_EQ(ECSqlStatus::Success, aStmt.Prepare(m_ecdb, "INSERT INTO ts.A(Partner.Id) VALUES (?)"));

            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a1Key));
            aStmt.Reset();
            aStmt.BindId(1, a1Key.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_DONE, aStmt.Step(a2Key));
            aStmt.Reset();
            aStmt.ClearBindings();

            aStmt.BindId(1, a1Key.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, aStmt.Step(a3Key)) << "A1 already referenced by A2";
            aStmt.Reset();
            aStmt.ClearBindings();
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipWithMultipleConstraintClasses)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Foo' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Base' modifier='Abstract'>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Goo' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Hoo' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Width' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                          "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                          "        <Class class='Foo' />"
                                                          "     </Source>"
                                                          "     <Target multiplicity='(0..*)' polymorphic='True' abstractConstraint='Base' roleLabel='Target'>"
                                                          "         <Class class='Goo'/>"
                                                          "         <Class class='Hoo'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Multi-constraint class rel which implicitly maps to link table because of missing nav prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Foo' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Base' modifier='Abstract'>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Goo' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Hoo' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Width' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                          "     <Source multiplicity='(0..1)' polymorphic='False' abstractConstraint='Base' roleLabel='Source'>"
                                                          "        <Class class='Foo' />"
                                                          "         <Class class='Hoo'/>"
                                                          "     </Source>"
                                                          "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                                                          "         <Class class='Goo'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Multiple constraint classes which map to more than one table on referenced side is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                          "</ECSchema>"))) << "Multiple constraint classes without specifying abstract constraint base class";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "  <ECEntityClass typeName='Base' >"
                                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                                            "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Sub' >"
                                                            "    <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName='Length' typeName='long' />"
                                                            "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
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
                                                            "</ECSchema>"))) << "Nav prop overriding in class hierarchy is fine";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo2' relationshipName='Rel' direction='Backward'/>"
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
                                                          "</ECSchema>"))) << "Duplicate nav props in class hierarchy";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema5' alias='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "  <ECEntityClass typeName='Base' >"
                                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Sub1' >"
                                                            "    <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName='Length' typeName='long' />"
                                                            "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Sub2' >"
                                                            "    <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName='Length' typeName='long' />"
                                                            "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Hoo' >"
                                                            "    <ECProperty propertyName='Width' typeName='long' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                            "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                            "         <Class class='Hoo'/>"
                                                            "     </Source>"
                                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                                                            "         <Class class='Sub1'/>"
                                                            "         <Class class='Sub2'/>"
                                                            "     </Target>"
                                                            "  </ECRelationshipClass>"
                                                            "</ECSchema>"))) << "Every constraint class has a nav prop with same name";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema6' alias='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub1' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub2' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Hoo' >"
                                                          "    <ECProperty propertyName='Width' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                          "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                          "         <Class class='Hoo'/>"
                                                          "     </Source>"
                                                          "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                                                          "         <Class class='Sub1'/>"
                                                          "         <Class class='Sub2'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Abstract constraint class must not have a nav prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema7' alias='ts7' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub1' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub2' >"
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
                                                          "         <Class class='Sub1'/>"
                                                          "         <Class class='Sub2'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Abstract constraint class must not have a nav prop, constraint classes must have them";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema8' alias='ts8' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub1' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub2' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Hoo' >"
                                                          "    <ECProperty propertyName='Width' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                          "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                          "         <Class class='Hoo'/>"
                                                          "     </Source>"
                                                          "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                                                          "         <Class class='Sub1'/>"
                                                          "         <Class class='Sub2'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Not all constraint classes have a nav prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema9' alias='ts9' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub1' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub2' >"
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
                                                          "         <Class class='Sub1'/>"
                                                          "         <Class class='Sub2'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "Not all constraint classes have a nav prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema10' alias='ts10' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                          "  <ECEntityClass typeName='Base' >"
                                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub1' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo1' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Sub2' >"
                                                          "    <BaseClass>Base</BaseClass>"
                                                          "    <ECProperty propertyName='Length' typeName='long' />"
                                                          "    <ECNavigationProperty propertyName='Hoo2' relationshipName='Rel' direction='Backward'/>"
                                                          "  </ECEntityClass>"
                                                          "  <ECEntityClass typeName='Hoo' >"
                                                          "    <ECProperty propertyName='Width' typeName='long' />"
                                                          "  </ECEntityClass>"
                                                          "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                          "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                          "         <Class class='Hoo'/>"
                                                          "     </Source>"
                                                          "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                                                          "         <Class class='Sub1'/>"
                                                          "         <Class class='Sub2'/>"
                                                          "     </Target>"
                                                          "  </ECRelationshipClass>"
                                                          "</ECSchema>"))) << "all constraint classes must have a nav prop with the same name";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema11' alias='ts11' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "  <ECEntityClass typeName='Base' >"
                                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Sub' >"
                                                            "    <BaseClass>Base</BaseClass>"
                                                            "    <ECProperty propertyName='Length' typeName='long' />"
                                                            "    <ECNavigationProperty propertyName='Hoo' relationshipName='Rel' direction='Backward'/>"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Hoo' >"
                                                            "    <ECProperty propertyName='Width' typeName='long' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECRelationshipClass typeName='Rel' strength='referencing' modifier='Sealed'>"
                                                            "     <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Source'>"
                                                            "         <Class class='Hoo'/>"
                                                            "     </Source>"
                                                            "     <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target' abstractConstraint='Base'>"
                                                            "         <Class class='Sub'/>"
                                                            "         <Class class='Sub'/>"
                                                            "     </Target>"
                                                            "  </ECRelationshipClass>"
                                                            "</ECSchema>"))) << "Duplicate constraint classes are already merged by ECObjects, so this is fine";
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipAsConstraintClass)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("RelationshipAsConstraintClass.ecdb",SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Definition" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                        <ECNavigationProperty propertyName="Definition" relationshipName="LinkTableRelIsDefinedBy" direction="Backward"/>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelIsDefinedBy" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Definition"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Logical FK Nav prop Rel has link table rel as constraint class";

            ASSERT_EQ(ExpectedColumns({ExpectedColumn("ts1_LinkTableRel","DefinitionId"), ExpectedColumn("ts1_LinkTableRel","DefinitionRelECClassId", Virtual::Yes)}),
                      GetHelper().GetPropertyMapColumns(AccessString("TestSchema1", "LinkTableRel", "Definition")));
            }


            {
            ASSERT_EQ(SUCCESS, SetupECDb("RelationshipAsConstraintClass.ecdb",
                                         SchemaItem(R"xml(<ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                     <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Definition" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                        <ECNavigationProperty propertyName="Definition" relationshipName="LinkTableRelIsDefinedBy" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelIsDefinedBy" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Definition"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Physical FK Nav prop Rel has link table rel as constraint class";

            ASSERT_EQ(ExpectedColumns({ExpectedColumn("ts2_LinkTableRel","DefinitionId"), ExpectedColumn("ts2_LinkTableRel","DefinitionRelECClassId", Virtual::Yes)}),
                      GetHelper().GetPropertyMapColumns(AccessString("TestSchema2", "LinkTableRel", "Definition")));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("RelationshipAsConstraintClass.ecdb",
                SchemaItem(R"xml(<ECSchema schemaName="TestSchema3" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Definition" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelHasDefinition" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="Definition"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "LinkTable Rel has link table rel as constraint class";

            ASSERT_EQ(ExpectedColumn("ts3_LinkTableRel","SourceId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema3", "LinkTableRel", "SourceECInstanceId")));
            ASSERT_EQ(ExpectedColumn("ts3_LinkTableRel","TargetId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema3", "LinkTableRel", "TargetECInstanceId")));
            ASSERT_EQ(ExpectedColumn("ts3_LinkTableRelHasDefinition","SourceId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema3", "LinkTableRelHasDefinition", "SourceECInstanceId")));
            ASSERT_EQ(ExpectedColumn("ts3_LinkTableRelHasDefinition","TargetId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema3", "LinkTableRelHasDefinition", "TargetECInstanceId")));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("RelationshipAsConstraintClass.ecdb",
                    SchemaItem(R"xml(<ECSchema schemaName="TestSchema4" alias="ts4" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel1" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRel2" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelHasDefinition" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel1"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "LinkTable Rel has link table rel as constraint class on both sides";

            ASSERT_EQ(ExpectedColumn("ts4_LinkTableRelHasDefinition","SourceId"),
                      GetHelper().GetPropertyMapColumn(AccessString("TestSchema4", "LinkTableRelHasDefinition", "SourceECInstanceId")));

            ASSERT_EQ(ExpectedColumn("ts4_LinkTableRel1","ECClassId", Virtual::Yes),
                      GetHelper().GetPropertyMapColumn(AccessString("TestSchema4", "LinkTableRelHasDefinition", "SourceECClassId")));

            ASSERT_EQ(ExpectedColumn("ts4_LinkTableRelHasDefinition","TargetId"),
                      GetHelper().GetPropertyMapColumn(AccessString("TestSchema4", "LinkTableRelHasDefinition", "TargetECInstanceId")));

            ASSERT_EQ(ExpectedColumn("ts4_LinkTableRel2","ECClassId", Virtual::Yes),
                      GetHelper().GetPropertyMapColumn(AccessString("TestSchema4", "LinkTableRelHasDefinition", "TargetECClassId")));

            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("RelationshipAsConstraintClass.ecdb",
                     SchemaItem(R"xml(<ECSchema schemaName="TestSchema5" alias="ts5" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Definition" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel" modifier="Abstract" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelSub1" modifier="Sealed" strength="Referencing">
                        <BaseClass>LinkTableRel</BaseClass>
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelSub2" modifier="Sealed" strength="Referencing">
                        <BaseClass>LinkTableRel</BaseClass>
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelHasDefinition" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="Definition"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has" abstractConstraint="LinkTableRel">
                            <Class class="LinkTableRelSub1"/>
                            <Class class="LinkTableRelSub2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "LinkTable Rel has two link table rels as constraint class";


            ASSERT_EQ(ExpectedColumn("ts5_LinkTableRelHasDefinition","SourceId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema5", "LinkTableRelHasDefinition", "SourceECInstanceId")));
            ASSERT_EQ(ExpectedColumn("ts5_LinkTableRelHasDefinition","TargetId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema5", "LinkTableRelHasDefinition", "TargetECInstanceId")));
            }

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Definition" >
                       <ECProperty propertyName="Name" typeName="string" />
                        <ECNavigationProperty propertyName="LinkTableRel" relationshipName="LinkTableRelIsDefinedBy" direction="Backward"/>
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelIsDefinedBy" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="Definition"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Nav prop must not point to relationship";

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema5" alias="ts5" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECEntityClass typeName="A" >
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="B" >
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                     <ECRelationshipClass typeName="LinkTableRel1" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRel2" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B"/>
                        </Target>
                        <ECNavigationProperty propertyName="LinkTableRel1" relationshipName="LinkTableRelIsDefinedBy" direction="Backward"/>
                     </ECRelationshipClass>
                     <ECRelationshipClass typeName="LinkTableRelIsDefinedBy" modifier="Sealed" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel1"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="LinkTableRel2"/>
                        </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml"))) << "Nav prop must point to entity class";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, LinkTableRelationshipMapStrategy)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("rellinktablemapstrategy.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                  "</ECSchema>")));

    ASSERT_EQ(MapStrategyInfo(MapStrategy::TablePerHierarchy), GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test1", "Rel1"))) << "Rel class with modifier Abstract";
    ASSERT_EQ(MapStrategyInfo(MapStrategy::TablePerHierarchy), GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test1", "Rel2"))) << "Rel class with modifier None";
    ASSERT_EQ(MapStrategyInfo(MapStrategy::OwnTable), GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test1", "Rel3"))) << "Rel class with modifier Sealed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipInheritance)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                          "</ECSchema>"))) << "Partially defined abstract relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                          "</ECSchema>"))) << "Relationship multi inheritance";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                            "</ECSchema>"), "relinheritance3.ecdb")) << "Inheriting relationship class with modifier None";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                            "</ECSchema>"), "relinheritance4.ecdb")) << "Additional properties on root rel class";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                            "</ECSchema>"), "relinheritance5.ecdb")) << "Additional properties on rel subclass"; //WIP: want to disallow this eventually
    }



//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_UnsupportedCases)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema7' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>"))) << "Referenced end maps to more than one table";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema8' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>"))) << "Referenced end maps to more than one table";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema9' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>"))) << "Referenced end maps to more than one table";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema10' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>"))) << "Target end of link table maps to more than one table";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema11' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>"))) << "Source end of link table maps to more than one table";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_SupportedCases)
    {
    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
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
        "</ECSchema>"))) << "1:N and holding";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<ECSchema schemaName='TestSchema4' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                           "</ECSchema>")));

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
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
        "</ECSchema>"))) << "N:N and holding";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_relwithoutnavprop.ecdb", SchemaItem("<ECSchema schemaName='TestSchema1' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                                                                   "</ECSchema>"))) << "Rel w/o nav prop";

    ASSERT_EQ(2, GetHelper().GetColumnCount("ts_Parent"));
    ASSERT_EQ(2, GetHelper().GetColumnCount("ts_Child"));
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_ParentHasChildren"));

    ASSERT_EQ(ExpectedColumn("ts_ParentHasChildren","Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema1", "ParentHasChildren", "ECInstanceId"))) << "Rel without nav prop -> link table";
    ASSERT_EQ(ExpectedColumn("ts_ParentHasChildren","SourceId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema1", "ParentHasChildren", "SourceECInstanceId"))) << "Rel without nav prop -> link table";
    ASSERT_EQ(ExpectedColumn("ts_ParentHasChildren","TargetId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema1", "ParentHasChildren", "TargetECInstanceId"))) << "Rel without nav prop -> link table";
    }

    auto assertRelationship = [](ECDbCR ecdb, Utf8CP schemaName, Utf8CP relationshipClassName, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey)
        {
        //select
        Utf8String ecsql;
        ecsql.Sprintf("SELECT SourceECInstanceId, SourceECClassId FROM %s.%s WHERE TargetECInstanceId=%s",
                      schemaName, relationshipClassName, targetKey.GetInstanceId().ToString().c_str());

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(sourceKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(sourceKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << ": ECSQL: " << ecsql.c_str();
        stmt.Finalize();

        ecsql.Sprintf("SELECT TargetECInstanceId, TargetECClassId FROM %s.%s WHERE SourceECInstanceId=%s",
                      schemaName, relationshipClassName, sourceKey.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(targetKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << ": ECSQL: " << ecsql.c_str();
        ASSERT_EQ(targetKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << ": ECSQL: " << ecsql.c_str();
        };


    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_childtph.ecdb", SchemaItem(
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
        "</ECSchema>"))) << "Child hierarchy in TPH";

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Parent(ParentProp) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ChildA(ChildAProp,ChildProp, Parent.Id) VALUES(2,2,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }
    m_ecdb.SaveChanges();
    assertRelationship(m_ecdb, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }


    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_parenthierarchyinsharedtable.ecdb", SchemaItem(
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
        "</ECSchema>"))) << "Parent hierarchy in TPH";

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ParentA(ParentAProp,ParentProp) VALUES(1,1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Child(ChildProp, Parent.Id) VALUES(2,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }

    assertRelationship(m_ecdb, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }


    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_childreninseparatejoinedtables_fknotinjoinedtable.ecdb",
                                 SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                            "</ECSchema>")));

    ECInstanceKey parentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Parent(ParentProp) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }

    ECInstanceKey childKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ChildA(ChildAProp, Parent.Id) VALUES(2,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }

    assertRelationship(m_ecdb, "TestSchema", "ParentHasChildren", parentKey, childKey);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRulesTestFixture, RelationshipMappingLimitations_InvalidInECSql)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_childreninseparatetables.ecdb", SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECEntityClass typeName='Parent' >"
        "    <ECProperty propertyName='ParentProp' typeName='int' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child' >"
        "    <ECProperty propertyName='ChildProp' typeName='int' />"
        "    <ECNavigationProperty propertyName='Parent' relationshipName='NavPropRel' direction='Backward'/>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='NavPropRel' strength='embedding' modifier='Sealed'>"
        "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
        "         <Class class='Parent' />"
        "     </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
        "        <Class class='Child' />"
        "     </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='LinkTableRel' strength='embedding' modifier='Sealed'>"
        "     <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Has Children'>"
        "         <Class class='Parent' />"
        "     </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Parent Has Children (Reversed)'>"
        "        <Class class='Child' />"
        "     </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>")));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ts.NavPropRel"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.NavPropRel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.NavPropRel SET SourceECInstanceId=?, TargetECInstanceId=?"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ts.NavPropRel"));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ts.LinkTableRel"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.LinkTableRel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.LinkTableRel SET SourceECInstanceId=?, TargetECInstanceId=?"));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.LinkTableRel"));
    }

    }
END_ECDBUNITTESTS_NAMESPACE
