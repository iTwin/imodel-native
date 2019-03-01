/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/IndexTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct IndexTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, InvalidUserDefinedIndexes)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with empty Indexes property is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Empty DbIndexList CA is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                   </DbIndex>"
        "                  </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with empty DbIndex is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                      <Properties>"
        "                      </Properties>"
        "                   </DbIndex>"
        "                  </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with DbIndex with empty Properties is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                      <Properties>"
        "                          <string>Code</string>"
        "                      </Properties>"
        "                   </DbIndex>"
        "                  </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with DbIndex without Name is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Bla</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Property in index does not exist";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on struct prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructArrayProperty propertyName='Codes' typeName='ElementCode' minOccurs='0' maxOccurs='unbounded' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on struct array prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='DgnModel' modifier='None' >"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DgnElement' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_model</Name>"
        "                       <Properties>"
        "                          <string>Model</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
        "    </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' strength='embedding'>"
        "    <Source multiplicity='(1..1)' polymorphic='true' roleLabel='Model'>"
        "      <Class class='DgnModel' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='Element'>"
        "      <Class class='DgnElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on navigation prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECArrayProperty propertyName='Codes' typeName='string' minOccurs='0' maxOccurs='unbounded' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on primitive array prop";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
        "    <ECEntityClass typeName='Foo' modifier='None' />"
        "    <ECEntityClass typeName='MyMixin' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "            <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "                   <AppliesToEntityClass>Foo</AppliesToEntityClass>"
        "            </IsMixin>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_mymixin_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on mixin";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>mypoorlynamedindex</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>mypoorlynamedindex</Name>"
        "                       <Properties>"
        "                          <string>BB</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='BB' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Duplicate indexes";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='AProp' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>MyIndex</Name>"
        "                       <Properties>"
        "                          <string>AProp</string>"
        "                          <string>BProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>A</BaseClass>"
        "        <ECProperty propertyName='BProp' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Index with properties that map to different tables is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnEntityClassSystemProperties)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_entityclass.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_foo_id</Name>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_foo_id</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_foo_classid</Name>
                                    <Properties>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <IsUnique>true</IsUnique>
                                    <Name>uix_foo_classid</Name>
                                    <Properties>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <IsUnique>false</IsUnique>
                                    <Name>ix_foo_id_prop2</Name>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                        <string>Prop2</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <IsUnique>true</IsUnique>
                                    <Name>uix_foo_id_prop2</Name>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                        <string>Prop2</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <IsUnique>false</IsUnique>
                                    <Name>ix_foo_prop1_classid</Name>
                                    <Properties>
                                        <string>Prop1</string>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <IsUnique>true</IsUnique>
                                    <Name>uix_foo_prop1_classid</Name>
                                    <Properties>
                                        <string>Prop1</string>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Prop1" typeName="string" />
                    <ECProperty propertyName="Prop2" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml"))) << "Indexes on ECInstanceId and non-virtual ECClassId props";

            EXPECT_STRCASEEQ(IndexInfo("ix_foo_id", false, "ts_Foo", "Id").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo_id").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_foo_id", true, "ts_Foo", "Id").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_foo_id").c_str());

            EXPECT_FALSE(GetHelper().IndexExists("ix_foo_classid")) << "Is duplicate to system index on class id col";
            EXPECT_STRCASEEQ(IndexInfo("ix_ts_Foo_ecclassid", false, "ts_Foo", "ECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Foo_ecclassid").c_str()) << "System index on ECClassId col";

            EXPECT_STRCASEEQ(IndexInfo("uix_foo_classid", true, "ts_Foo", "ECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_foo_classid").c_str());

            EXPECT_STRCASEEQ(IndexInfo("ix_foo_id_prop2", false, "ts_Foo", std::vector<Utf8String>{"Id", "Prop2"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo_id_prop2").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_foo_id_prop2", true, "ts_Foo", std::vector<Utf8String>{"Id", "Prop2"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_foo_id_prop2").c_str());

            EXPECT_STRCASEEQ(IndexInfo("ix_foo_prop1_classid", false, "ts_Foo", std::vector<Utf8String>{"Prop1", "ECClassId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo_prop1_classid").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_foo_prop1_classid", true, "ts_Foo", std::vector<Utf8String>{"Prop1", "ECClassId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_foo_prop1_classid").c_str());
            }


            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECEntityClass typeName="Foo" modifier="Sealed">
            <ECCustomAttributes>
                <DbIndexList xmlns="ECDbMap.02.00">
                    <Indexes>
                        <DbIndex>
                            <Name>ix_foo_classid</Name>
                            <Properties>
                                <string>ECClassId</string>
                            </Properties>
                        </DbIndex>
                    </Indexes>
                </DbIndexList>
            </ECCustomAttributes>
            <ECProperty propertyName="Prop1" typeName="string" />
            <ECProperty propertyName="Prop2" typeName="int" />
        </ECEntityClass>
        </ECSchema>)xml"))) << "Cannot define index on virtual ECClassId col";

            {
            //index on nav prop (logical FK)
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_navprop.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_parentid</Name>
                                    <Properties>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_parentid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_parentrelclassid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_parentid_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.Id</string>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_name_parentid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Name</string>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml")));


            EXPECT_STRCASEEQ(IndexInfo("ix_parentid", false, "ts_Child", "ParentId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_parentid").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_parentid", true, "ts_Child", "ParentId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_parentid").c_str());

            EXPECT_STRCASEEQ(IndexInfo("ix_parentrelclassid", false, "ts_Child", "ParentRelECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_parentrelclassid").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_parentrelclassid", true, "ts_Child", "ParentRelECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_parentrelclassid").c_str());


            EXPECT_STRCASEEQ(IndexInfo("ix_parentid_parentrelclassid", false, "ts_Child", std::vector<Utf8String>{"ParentId", "ParentRelECClassId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_parentid_parentrelclassid").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_name_parentid", true, "ts_Child", std::vector<Utf8String>{"Name", "ParentId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_name_parentid").c_str());

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
        </ECSchema>)xml"))) << "Cannot define index on virtual RelECClassId col";

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>uix_parentrelclassid</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
        </ECSchema>)xml"))) << "Cannot define index on virtual RelECClassId col";
            }

                {
                //index on nav prop (physical FK)
                ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_navprop.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_parentid</Name>
                                    <Properties>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_parentid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_parentrelclassid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_parentid_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.Id</string>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_name_parentid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Name</string>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml")));


                EXPECT_FALSE(GetHelper().IndexExists("ix_parentid")) << "Is duplicate to system index on nav id col (by cardinality of the relationship)";
                EXPECT_STRCASEEQ(IndexInfo("ix_ts_Child_fk_ts_Rel_target", false, "ts_Child", "ParentId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Child_fk_ts_Rel_target").c_str()) << "System index on nav id col (by cardinality of the relationship)";
                EXPECT_STRCASEEQ(IndexInfo("uix_parentid", true, "ts_Child", "ParentId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_parentid").c_str());

                EXPECT_FALSE(GetHelper().IndexExists("ix_parentrelclassid")) << "Is duplicate to system index on nav relecclassid col";
                EXPECT_STRCASEEQ(IndexInfo("ix_ts_Child_ParentRelECClassId", false, "ts_Child", "ParentRelECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Child_ParentRelECClassId").c_str());
                EXPECT_STRCASEEQ(IndexInfo("uix_parentrelclassid", true, "ts_Child", "ParentRelECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_parentrelclassid").c_str());


                EXPECT_STRCASEEQ(IndexInfo("ix_parentid_parentrelclassid", false, "ts_Child", std::vector<Utf8String>{"ParentId", "ParentRelECClassId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_parentid_parentrelclassid").c_str());
                EXPECT_STRCASEEQ(IndexInfo("uix_name_parentid", true, "ts_Child", std::vector<Utf8String>{"Name", "ParentId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_name_parentid").c_str());

                ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                    R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
        </ECSchema>)xml"))) << "Cannot define index on virtual RelECClassId col";

                ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                    R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>uix_parentrelclassid</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
        </ECSchema>)xml"))) << "Cannot define index on virtual RelECClassId col";
                }

                    {
                    //index on nav prop (physical FK) with 1:1
                    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_navprop.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>uix_parentid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Parent.Id</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_parentrelclassid</Name>
                                    <Properties>
                                        <string>Parent.RelECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml")));


                    EXPECT_FALSE(GetHelper().IndexExists("uix_parentid")) << "Is duplicate to system index on nav id col (by cardinality of the relationship)";
                    EXPECT_STRCASEEQ(IndexInfo("uix_ts_Child_fk_ts_Rel_target", true, "ts_Child", "ParentId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_ts_Child_fk_ts_Rel_target").c_str()) << "System index on nav id col (by cardinality of the relationship)";

                    EXPECT_FALSE(GetHelper().IndexExists("ix_parentrelclassid")) << "Is duplicate to system index on nav relecclassid col";
                    EXPECT_STRCASEEQ(IndexInfo("ix_ts_Child_ParentRelECClassId", false, "ts_Child", "ParentRelECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Child_ParentRelECClassId").c_str());
                    }

                    {
                    //index Point members
                    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_points.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_llx</Name>
                                    <Properties>
                                        <string>LL.X</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_llx</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>LL.X</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_lly</Name>
                                    <Properties>
                                        <string>LL.Y</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_lly</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>LL.Y</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_originx</Name>
                                    <Properties>
                                        <string>Origin.X</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_originx</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Origin.X</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_originy</Name>
                                    <Properties>
                                        <string>Origin.Y</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_originy</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Origin.Y</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_originz</Name>
                                    <Properties>
                                        <string>Origin.Z</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_originz</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Origin.Z</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_llx_originz</Name>
                                    <Properties>
                                        <string>LL.X</string>
                                        <string>Origin.Z</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_originx_lly</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Origin.X</string>
                                        <string>LL.Y</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="LL" typeName="Point2d" />
                    <ECProperty propertyName="Origin" typeName="Point3d" />
                </ECEntityClass>
            </ECSchema>)xml")));


                    EXPECT_STRCASEEQ(IndexInfo("ix_llx", false, "ts_Foo", "LL_X").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_llx").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_llx", true, "ts_Foo", "LL_X").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_llx").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("ix_lly", false, "ts_Foo", "LL_Y").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_lly").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_lly", true, "ts_Foo", "LL_Y").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_lly").c_str());

                    EXPECT_STRCASEEQ(IndexInfo("ix_originx", false, "ts_Foo", "Origin_X").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_originx").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_originx", true, "ts_Foo", "Origin_X").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_originx").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("ix_originy", false, "ts_Foo", "Origin_Y").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_originy").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_originy", true, "ts_Foo", "Origin_Y").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_originy").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("ix_originz", false, "ts_Foo", "Origin_Z").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_originz").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_originz", true, "ts_Foo", "Origin_Z").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_originz").c_str());

                    EXPECT_STRCASEEQ(IndexInfo("ix_llx_originz", false, "ts_Foo", std::vector<Utf8String>{"LL_X", "Origin_Z"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_llx_originz").c_str());
                    EXPECT_STRCASEEQ(IndexInfo("uix_originx_lly", true, "ts_Foo", std::vector<Utf8String>{"Origin_X", "LL_Y"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_originx_lly").c_str());
                    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnFKRelSystemProperties)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_id</Name>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_classid</Name>
                                    <Properties>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel</Name>
                                    <Properties>
                                        <string>SourceECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel</Name>
                                    <Properties>
                                        <string>SourceECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel</Name>
                                    <Properties>
                                        <string>TargetECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel</Name>
                                    <Properties>
                                        <string>TargetECClassId</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on FK rel is not valid";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnLinkTableRelSystemProperties)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnSystemProperties_linktable.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_id</Name>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_id</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>ECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_rel_classid</Name>
                                    <Properties>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_classid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>ECClassId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_rel_sourceid</Name>
                                    <Properties>
                                        <string>SourceECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_sourceid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>SourceECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_rel_targetid</Name>
                                    <Properties>
                                        <string>TargetECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_targetid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>TargetECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_rel_sourceid_order</Name>
                                    <Properties>
                                        <string>SourceECInstanceId</string>
                                        <string>Order</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_sourceid_order</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>SourceECInstanceId</string>
                                        <string>Order</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>ix_rel_order_targetid</Name>
                                    <Properties>
                                        <string>Order</string>
                                        <string>TargetECInstanceId</string>
                                    </Properties>
                                </DbIndex>
                                <DbIndex>
                                    <Name>uix_rel_order_targetid</Name>
                                    <IsUnique>true</IsUnique>
                                    <Properties>
                                        <string>Order</string>
                                        <string>TargetECInstanceId</string>
                                    </Properties>
                                </DbIndex>                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                    <ECProperty propertyName="Order" typeName="int" />
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Indexes on all system props of a link table rel";

            EXPECT_STRCASEEQ(IndexInfo("ix_rel_id", false, "ts_Rel", "Id").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_rel_id").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_rel_id", true, "ts_Rel", "Id").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_rel_id").c_str());

            EXPECT_FALSE(GetHelper().IndexExists("ix_rel_classid")) << "Is duplicate to system index on class id col";
            EXPECT_STRCASEEQ(IndexInfo("ix_ts_Rel_ecclassid", false, "ts_Rel", "ECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Rel_ecclassid").c_str()) << "System index on ECClassId col";

            EXPECT_STRCASEEQ(IndexInfo("uix_rel_classid", true, "ts_Rel", "ECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_rel_classid").c_str());


            EXPECT_FALSE(GetHelper().IndexExists("ix_rel_sourceid")) << "Is duplicate to system index on sourceid col (because of cardinality)";
            EXPECT_STRCASEEQ(IndexInfo("ix_ts_Rel_source", false, "ts_Rel", "SourceId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_Rel_source").c_str()) << "System index on SourceId col (because of cardinality)";
            EXPECT_STRCASEEQ(IndexInfo("uix_rel_sourceid", true, "ts_Rel", "SourceId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_rel_sourceid").c_str());

            EXPECT_STRCASEEQ(IndexInfo("ix_rel_targetid", false, "ts_Rel", "TargetId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_rel_targetid").c_str());
            EXPECT_FALSE(GetHelper().IndexExists("uix_rel_targetid")) << "Is duplicate to system index on targetid col (because of cardinality)";
            EXPECT_STRCASEEQ(IndexInfo("uix_ts_Rel_target", true, "ts_Rel", "TargetId").ToDdl().c_str(), GetHelper().GetIndexDdl("uix_ts_Rel_target").c_str()) << "System index on TargetId col (because of cardinality)";


            EXPECT_STRCASEEQ(IndexInfo("ix_rel_sourceid_order", false, "ts_Rel", std::vector<Utf8String>{"SourceId", "Order"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_rel_sourceid_order").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_rel_sourceid_order", true, "ts_Rel", std::vector<Utf8String>{"SourceId", "Order"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_rel_sourceid_order").c_str());

            EXPECT_STRCASEEQ(IndexInfo("ix_rel_order_targetid", false, "ts_Rel", std::vector<Utf8String>{"Order", "TargetId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_rel_order_targetid").c_str());
            EXPECT_STRCASEEQ(IndexInfo("uix_rel_order_targetid", true, "ts_Rel", std::vector<Utf8String>{"Order", "TargetId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_rel_order_targetid").c_str());
            }

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_sourceclassid</Name>
                                    <Properties>
                                        <string>SourceECClassId</string>
                                    </Properties>
                                </DbIndex>
                             </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on SourceECClassId is expected to fail as SourceECClassId is never mapped to the link table";

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_sourceclassid</Name>
                                    <Properties>
                                        <string>SourceECClassId</string>
                                    </Properties>
                                </DbIndex>
                             </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on SourceECClassId is expected to fail as SourceECClassId is virtual column";

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_targetclassid</Name>
                                    <Properties>
                                        <string>TargetECClassId</string>
                                    </Properties>
                                </DbIndex>
                             </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on TargetECClassId is expected to fail as TargetECClassId is never mapped to the link table";

            ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_rel_targetclassid</Name>
                                    <Properties>
                                        <string>TargetECClassId</string>
                                    </Properties>
                                </DbIndex>
                             </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
            </ECSchema>)xml"))) << "Index on TargetECClassId is expected to fail as TargetECClassId is virtual column";
    }

    

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnStructMembers)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnStructMembers.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Code.AuthorityId</string>"
        "                          <string>Code.Namespace</string>"
        "                          <string>Code.Val</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Utf8CP indexName = "uix_element_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts_Element", {"Code_AuthorityId", "Code_Namespace", "Code_Val"}).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnTph)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest1.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_base_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "             </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Utf8CP indexName = "ix_base_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts1_Base", "Code").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str());


    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest2.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_base_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "             </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_sub1_prop</Name>"
        "                       <Properties>"
        "                          <string>Sub1_Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECClassId sub1Id = m_ecdb.Schemas().GetClassId("TestSchema", "Sub1");
    indexName = "ix_sub1_prop";
    ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts2_Base", "Sub1_Prop", IndexInfo::WhereClause(sub1Id)).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest3.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_sub1_code</Name>"
        "                       <IsUnique>true</IsUnique>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    indexName = "uix_sub1_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts3_Base", "Code", IndexInfo::WhereClause({m_ecdb.Schemas().GetClassId("TestSchema","Base")}, true)).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest4.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_base_code</Name>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub3' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_sub3_prop</Name>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Properties>"
        "                          <string>Sub3_Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub3_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    indexName = "uix_base_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts4_Base", "Code").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;



    indexName = "uix_sub3_prop";
    ECClassId sub3ClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Sub3");
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts4_Base", "Sub3_Prop", IndexInfo::WhereClause({sub3ClassId})).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;

    //after second import new subclass in hierarchy must be reflected by indices
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts42' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts4' />"
        "    <ECEntityClass typeName='Sub4' modifier='None'>"
        "        <BaseClass>ts4:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub4_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    indexName = "uix_base_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts4_Base", "Code").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName << "> This index is not affected as index is still applying to entire hierarchy";


    //This index must include the new subclass Sub4
    indexName = "uix_sub3_prop";
    ECClassId sub4ClassId = m_ecdb.Schemas().GetClassId("TestSchema2", "Sub4");
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts4_Base", "Sub3_Prop", IndexInfo::WhereClause({sub3ClassId, sub4ClassId})).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName << "> This index must include the new subclass Sub4";


    

    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest8.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts8' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' prefix='CoreCA' />"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Root' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_root</Name>"
        "                       <Properties>"
        "                          <string>RootProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='RootProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub</Name>"
        "                       <Properties>"
        "                          <string>SubProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Root</BaseClass>"
        "        <ECProperty propertyName='SubProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_subsub</Name>"
        "                       <Properties>"
        "                          <string>SubSubProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='SubSubProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub2</Name>"
        "                       <Properties>"
        "                          <string>Sub2Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Root</BaseClass>"
        "        <ECProperty propertyName='Sub2Prop' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Index on abstract classes - Schema 1";

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts82' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts8' />"
        "    <ECEntityClass typeName='Sub3'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub3</Name>"
        "                       <Properties>"
        "                          <string>Sub3Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>ts8:Root</BaseClass>"
        "        <ECProperty propertyName='Sub3Prop' typeName='int' />"
        "    </ECEntityClass>"
        " </ECSchema>"))) << "Index on abstract classes - Schema 2";

    //class hierarchy with shared table
    indexName = "uix_root";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts8_Root", "RootProp").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


    //index from Interface class is applied to Sub and Sub2 which are stored in joined tables
    indexName = "uix_sub";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts8_Sub", "SubProp").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;

    indexName = "uix_sub2";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts8_Sub2", "Sub2Prop").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;

    indexName = "uix_sub3";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts82_Sub3", "Sub3Prop").ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;

    ECClassCP subSubClass = m_ecdb.Schemas().GetClass("TestSchema", "SubSub");
    ASSERT_TRUE(subSubClass != nullptr);

    indexName = "uix_subsub";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts8_Sub", "SubSubProp", IndexInfo::WhereClause({subSubClass->GetId()})).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindextest9.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts9' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='DgnModel' modifier='None' >"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DgnElement' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_dgnelement_model_id_code</Name>"
        "                       <Properties>"
        "                          <string>Model.Id</string>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' strength='embedding' modifier='Sealed'>"
        "    <Source multiplicity='(1..1)' polymorphic='true' roleLabel='Model'>"
        "      <Class class='DgnModel' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='Element'>"
        "      <Class class='DgnElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>")));


    indexName = "ix_dgnelement_model_id_code";
    ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts9_DgnElement", std::vector<Utf8String>{"ModelId", "Code"}).ToDdl().c_str(),
                     GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedUniqueIndexesOnNonTph)
    {
    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract class";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="None">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on non-sealed class";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_A">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with single subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_A">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_B">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_sub</Name>
                               <Properties>
                                  <string>SubProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
               <BaseClass>Root</BaseClass>
                <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSubSub">
               <BaseClass>SubSub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_sub</Name>
                               <Properties>
                                  <string>SubProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
               <BaseClass>Root</BaseClass>
                <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub_A">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub_B">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Prop</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed class";

    ASSERT_TRUE(GetHelper().TableExists("ts_Foo"));
    ASSERT_STRCASEEQ(IndexInfo("ix_foo", true, "ts_Foo", "Prop").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Prop</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed class";

    ASSERT_TRUE(GetHelper().TableExists("ts_Foo"));
    ASSERT_STRCASEEQ(IndexInfo("ix_foo", true, "ts_Foo", "Prop").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="None">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Prop</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Class modifier changed from Sealed to None";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedNonUniqueIndexesOnNonTph)
    {
    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract class";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="None">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on non-sealed class";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_A">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with single subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_A">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub_B">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub">
               <BaseClass>Root</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_sub</Name>
                               <Properties>
                                  <string>SubProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
               <BaseClass>Root</BaseClass>
                <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSubSub">
               <BaseClass>SubSub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_sub</Name>
                               <Properties>
                                  <string>SubProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
               <BaseClass>Root</BaseClass>
                <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub_A">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropA" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="SubSub_B">
               <BaseClass>Sub</BaseClass>
               <ECProperty propertyName="PropB" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on abstract base class with multiple subclasses";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Prop</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed class";

    ASSERT_TRUE(GetHelper().TableExists("ts_Foo"));
    ASSERT_STRCASEEQ(IndexInfo("ix_foo", false, "ts_Foo", "Prop").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";
    }
    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexWhenClassModifierIsUpgraded)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexWhenClassModifierIsUpgraded.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Prop</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed class";

            EXPECT_TRUE(GetHelper().TableExists("ts_Foo"));
            EXPECT_STRCASEEQ(IndexInfo("ix_foo", false, "ts_Foo", "Prop").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="None">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <IsUnique>False</IsUnique>
                                    <Name>ix_foo</Name>
                                    <Properties>
                                        <string>Prop</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Prop" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml"))) << "Class modifier changed from Sealed to None -> index should not be allowed anymore";
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexWhenClassModifierIsUpgraded.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                 </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Code</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Code" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed subclass (TPH)";
            ECClassId fooId = m_ecdb.Schemas().GetClassId("TestSchema", "Foo");
            EXPECT_TRUE(GetHelper().TableExists("ts_Base"));
            EXPECT_STRCASEEQ(IndexInfo("ix_foo", false, "ts_Base", "Code", IndexInfo::WhereClause(fooId)).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";


            EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                 </ECCustomAttributes>
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo" modifier="None">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>False</IsUnique>
                               <Name>ix_foo</Name>
                               <Properties>
                                  <string>Code</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Code" typeName="int" />
            </ECEntityClass>
            </ECSchema>)xml"))) << "Unsealed subclass (TPH) -> index still valid";
            EXPECT_TRUE(GetHelper().TableExists("ts_Base"));
            EXPECT_STRCASEEQ(IndexInfo("ix_foo", false, "ts_Base", "Code", IndexInfo::WhereClause(fooId)).ToDdl().c_str(), GetHelper().GetIndexDdl("ix_foo").c_str()) << "index on sealed class";

            }
            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexWhenClassModifierIsUpgraded.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" modifier="Abstract">
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Abstract class w/o index";

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <IsUnique>False</IsUnique>
                                    <Name>ix_foo</Name>
                                    <Properties>
                                        <string>Prop</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Prop" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml"))) << "Class modifier changed from Abstract is generally not supported.";
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexWhenClassModifierIsUpgraded.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" modifier="None">
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Abstract class w/o index";

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00">
                            <Indexes>
                                <DbIndex>
                                    <IsUnique>False</IsUnique>
                                    <Name>ix_foo</Name>
                                    <Properties>
                                        <string>Prop</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Prop" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml"))) << "DbIndexes cannot be added during schema upgrade";
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexesOnSharedColumns)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexonsharedcolumns1.ecdb", SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
        <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
        <ECEntityClass typeName="Base" modifier="None">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00"/>
                <DbIndexList xmlns="ECDbMap.02.00">
                        <Indexes>
                        <DbIndex>
                            <Name>ix_base_prop2</Name>
                            <Properties>
                                <string>Prop2</string>
                            </Properties>
                        </DbIndex>
                        </Indexes>
                </DbIndexList>
            </ECCustomAttributes>
            <ECProperty propertyName="Prop1" typeName="string" />
            <ECProperty propertyName="Prop2" typeName="int" />
        </ECEntityClass>
        <ECEntityClass typeName="Sub1" modifier="None">
            <ECCustomAttributes>
                <DbIndexList xmlns="ECDbMap.02.00">
                        <Indexes>
                        <DbIndex>
                            <Name>ix_sub1_Prop1</Name>
                            <Properties>
                                <string>Sub_Prop1</string>
                            </Properties>
                        </DbIndex>
                        <DbIndex>
                            <Name>uix_sub1_Prop2</Name>
                            <IsUnique>true</IsUnique>
                            <Properties>
                                <string>Sub_Prop2</string>
                            </Properties>
                        </DbIndex>
                        </Indexes>
                </DbIndexList>
            </ECCustomAttributes>
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="Sub_Prop1" typeName="long" />
            <ECProperty propertyName="Sub_Prop2" typeName="long" />
        </ECEntityClass>
        <ECEntityClass typeName="Sub1_1" modifier="None">
            <BaseClass>Sub1</BaseClass>
            <ECProperty propertyName="Sub1_1_Prop1" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml")));

            ECClassId baseClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Base");
            ASSERT_TRUE(baseClassId.IsValid());

            Utf8CP expectedIndexName = "ix_Base_Prop2";
            ASSERT_STRCASEEQ(IndexInfo(expectedIndexName, false, "ts_Base", "ps2").ToDdl().c_str(), GetHelper().GetIndexDdl(expectedIndexName).c_str());
            expectedIndexName = "ix_Sub1_Prop1";
            ASSERT_STRCASEEQ(IndexInfo(expectedIndexName, false, "ts_Base", "ps3", IndexInfo::WhereClause(baseClassId, true)).ToDdl().c_str(), GetHelper().GetIndexDdl(expectedIndexName).c_str());
            expectedIndexName = "uix_Sub1_Prop2";
            ASSERT_STRCASEEQ(IndexInfo(expectedIndexName, true, "ts_Base", "ps4", IndexInfo::WhereClause(baseClassId, true)).ToDdl().c_str(),
                             GetHelper().GetIndexDdl(expectedIndexName).c_str());
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexonsharedcolumns2.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00">
                              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                   <DbIndex>
                                       <IsUnique>True</IsUnique>
                                       <Name>uix_sub1_aid</Name>
                                       <Properties>
                                          <string>AId</string>
                                       </Properties>
                                   </DbIndex>
                                 </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="AId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub3" modifier="None">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Name2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub11" modifier="None">
                        <BaseClass>Sub1</BaseClass>
                        <ECProperty propertyName="Cost" typeName="double" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Unique indices on shared columns are supported.";

            ECClassId sub1ClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Sub1");
            ECClassId sub11ClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Sub11");
            Utf8CP indexName = "uix_sub1_aid";
            ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts_Base", "ps1", IndexInfo::WhereClause({sub1ClassId, sub11ClassId})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexonsharedcolumns3.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Parent" modifier="None">
                        <ECProperty propertyName="Code" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Child" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None">
                         <Source multiplicity="(0..1)" polymorphic="True" roleLabel="owns">
                              <Class class="Parent"/>
                         </Source>
                         <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                              <Class class="Child"/>
                         </Target>
                     </ECRelationshipClass>   
                </ECSchema>)xml"))) << "Logical FK";

            std::vector<Utf8String> indexes = GetHelper().GetIndexNamesForTable("ts_Child");
            ASSERT_EQ(1, indexes.size()) << "Only expected index on ts_Child is the one on ECClassId";
            ASSERT_STRCASEEQ("ix_ts_Child_ecclassid", indexes[0].c_str()) << "Only expected index on ts_Child is the one on ECClassId";
            }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexWithWhereClauseAndPropertyMapCAIsNullable)
    {
        ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest1.ecdb", SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
            "    <ECEntityClass typeName='B' modifier='Sealed'>"
            "        <ECCustomAttributes>"
            "            <DbIndexList xmlns='ECDbMap.02.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_code</Name>"
            "                       <Properties>"
            "                           <string>Code</string>"
            "                       </Properties>"
            "                       <Where>IndexedColumnsAreNotNull</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </DbIndexList>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Code' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.02.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECEntityClass>"
            "</ECSchema>")));

        Utf8CP indexName = "ix_b_code";
        ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts1_B", "Code").ToDdl().c_str(),
                         GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


        ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest2.ecdb", SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
            "    <ECEntityClass typeName='B' modifier='Sealed'>"
            "        <ECCustomAttributes>"
            "            <DbIndexList xmlns='ECDbMap.02.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_code_name</Name>"
            "                       <Properties>"
            "                           <string>Code</string>"
            "                           <string>Name</string>"
            "                       </Properties>"
            "                       <Where>IndexedColumnsAreNotNull</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </DbIndexList>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Code' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.02.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECEntityClass>"
            "</ECSchema>")));

        indexName = "ix_b_code_name";
        ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts2_B", std::vector<Utf8String>{"Code", "Name"}, IndexInfo::WhereClause(true, {"Name"})).ToDdl().c_str(),
                         GetHelper().GetIndexDdl(indexName).c_str()) << indexName;


        ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest3.ecdb", SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
            "    <ECEntityClass typeName='B' modifier='Sealed'>"
            "        <ECCustomAttributes>"
            "            <DbIndexList xmlns='ECDbMap.02.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_code_name</Name>"
            "                       <Properties>"
            "                           <string>Code</string>"
            "                           <string>Name</string>"
            "                       </Properties>"
            "                       <Where>IndexedColumnsAreNotNull</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </DbIndexList>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Code' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.02.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string'>"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.02.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "    </ECEntityClass>"
            "</ECSchema>")));

        indexName = "ix_b_code_name";
        ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts3_B", std::vector<Utf8String>{"Code", "Name"}).ToDdl().c_str(),
                         GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
        }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin                         03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexWithWhereIsNotNull)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmapindextest.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                                     "   <ECSchemaReference name='ECDbMap' version='02.00' prefix ='ecdbmap' />"
                                                                     "   <ECEntityClass typeName = 'IndexClass' modifier='Sealed'>"
                                                                     "       <ECCustomAttributes>"
                                                                     "       <DbIndexList xmlns='ECDbMap.02.00'>"
                                                                     "           <Indexes>"
                                                                     "               <DbIndex>"
                                                                     "                   <Name>IDX_Partial</Name>"
                                                                     "                   <IsUnique>False</IsUnique>"
                                                                     "                   <Properties>"
                                                                     "                       <string>PropertyPartialIndex</string>"
                                                                     "                   </Properties>"
                                                                     "                   <Where>IndexedColumnsAreNotNull</Where>"
                                                                     "               </DbIndex>"
                                                                     "               <DbIndex>"
                                                                     "                   <Name>IDX_Full</Name>"
                                                                     "                   <IsUnique>False</IsUnique>"
                                                                     "                   <Properties>"
                                                                     "                       <string>PropertyFullIndex</string>"
                                                                     "                   </Properties>"
                                                                     "               </DbIndex>"
                                                                     "               <DbIndex>"
                                                                     "                   <Name>IDX_PartialMissing</Name>"
                                                                     "                   <IsUnique>False</IsUnique>"
                                                                     "                   <Properties>"
                                                                     "                       <string>PropertyPartialIndex</string>"
                                                                     "                   </Properties>"
                                                                     "                   <Where></Where>"
                                                                     "               </DbIndex>"
                                                                     "           </Indexes>"
                                                                     "       </DbIndexList>"
                                                                     "   </ECCustomAttributes>"
                                                                     "   <ECProperty propertyName ='PropertyFullIndex' typeName = 'string' />"
                                                                     "   <ECProperty propertyName ='PropertyPartialIndex' typeName = 'string' />"
                                                                     "   </ECEntityClass>"
                                                                     "</ECSchema>")));
    //Verify that one Partial index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass' AND name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Partial", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String sqlCmd = stmt.GetValueText(4);
    ASSERT_FALSE(sqlCmd.find("WHERE") == std::string::npos) << "IDX_Partial is a partial index and will have WHERE clause";
    //Verify that other index is not Partial as Where was not specified
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Full", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    sqlCmd = stmt.GetValueText(4);
    ASSERT_TRUE(sqlCmd.find("WHERE") == std::string::npos);

    //Verify that index with empty Where clause is treated as not-partial index
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_PartialMissing", Statement::MakeCopy::No));
    //IDX_PartialMissing will be skipped as it has empty WHERE clause
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin                         03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedUniqueIndex)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmapindextest.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                                     "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                     "<ECEntityClass typeName='IndexClass2' modifier='Sealed'>"
                                                                     "   <ECCustomAttributes>"
                                                                     "       <DbIndexList xmlns='ECDbMap.02.00'>"
                                                                     "           <Indexes>"
                                                                     "               <DbIndex>"
                                                                     "                   <Name>IDX_Unique</Name>"
                                                                     "                   <IsUnique>True</IsUnique>"
                                                                     "                   <Properties>"
                                                                     "                       <string>Property2</string>"
                                                                     "                   </Properties>"
                                                                     "               </DbIndex>"
                                                                     "               <DbIndex>"
                                                                     "                   <Name>IDX_NotUnique</Name>"
                                                                     "                   <IsUnique>False</IsUnique>"
                                                                     "                   <Properties>"
                                                                     "                       <string>Property2</string>"
                                                                     "                   </Properties>"
                                                                     "               </DbIndex>"
                                                                     "           </Indexes>"
                                                                     "       </DbIndexList>"
                                                                     "   </ECCustomAttributes>"
                                                                     "   <ECProperty propertyName='Property1' typeName='string' />"
                                                                     "   <ECProperty propertyName='Property2' typeName='string' />"
                                                                     "</ECEntityClass>"
                                                                     "</ECSchema>")));

    //Verify that one Unique index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass2' AND name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Unique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String sqlCmd = stmt.GetValueText(4);
    ASSERT_FALSE(sqlCmd.find("UNIQUE") == std::string::npos) << "IDX_Unique will have UNIQUE clause";

    //Verify that other indexes are not Unique
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_NotUnique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    sqlCmd = stmt.GetValueText(4);
    ASSERT_TRUE(sqlCmd.find("UNIQUE") == std::string::npos);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, DuplicateUserDefinedIndexes)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name and definition on same class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name and different definition on same class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                  <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name and different definition on same class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" >
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>Cost</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Cost" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name and different definition in subclass";

            {
            ASSERT_EQ(SUCCESS, SetupECDb("DuplicateUserDefinedIndexes.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml")));

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="B" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>Name</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name already exists";

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="B" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>AnotherIndex</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>Name</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="C"modifier="Sealed" >
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>AnotherIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Cost</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Cost" typeName="double" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Two indexes with same name in same schema";
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("DuplicateUserDefinedIndexes.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Type</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml")));

            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                    <ECEntityClass typeName="Sub" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                 <Indexes>
                                    <DbIndex>
                                    <Name>MyFunnyIndex</Name>
                                    <IsUnique>True</IsUnique>
                                    <Properties>
                                        <string>Cost</string>
                                    </Properties>
                                 </DbIndex>
                                 </Indexes>
                               </DbIndexList>
                            </ECCustomAttributes>
                        <BaseClass>TestSchema:Base</BaseClass>
                        <ECProperty propertyName="Cost" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Index with same name already exists in same table";
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, ImplicitIndexesForRelationships)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships1.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None" >
                        <ECProperty propertyName="AId" typeName="string" />
                        <ECNavigationProperty propertyName="PartnerB" relationshipName="Rel11Backwards" direction="Forward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="AId" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECNavigationProperty propertyName="PartnerA" relationshipName="Rel11" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11Backwards" strength="embedding" strengthDirection="Backward" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelNN" strength="referencing" modifier="Sealed">
                    <Source multiplicity="(1..*)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..*)" polymorphic="True" roleLabel="references">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml")));

            Utf8CP indexName = "ix_ts1_B_fk_ts1_Rel_target";
            ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts1_B", "AId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            indexName = "uix_ts1_B_fk_ts1_Rel11_target";
            ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts1_B", "PartnerAId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            indexName = "uix_ts1_A_fk_ts1_Rel11Backwards_source";
            ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts1_A", "PartnerBId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            indexName = "ix_ts1_RelNN_source";
            ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts1_RelNN", "SourceId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            indexName = "ix_ts1_RelNN_target";
            ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts1_RelNN", "TargetId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            indexName = "uix_ts1_RelNN_sourcetarget";
            ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts1_RelNN", std::vector<Utf8String>{"SourceId", "TargetId"}).ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str());

            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships2.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None" >
                        <ECProperty propertyName="AId" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="AId" typeName="long" />
                        <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml")));

            ASSERT_STRCASEEQ(IndexInfo("ix_ts2_B_fk_ts2_Rel_target", false, "ts2_B", "AId", IndexInfo::WhereClause(true, {"AId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts2_B_fk_ts2_Rel_target").c_str());


            ASSERT_EQ(ExpectedColumns({ExpectedColumn("ts2_b","AId"),
                                      ExpectedColumn("ts2_b","ARelECClassId", Virtual::Yes)}),
                      GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B", "A")));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships3.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='BId' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='BB' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='BBId' typeName='long' />"
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='BB'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));

            ASSERT_STRCASEEQ(IndexInfo("ix_ts3_B_fk_ts3_Rel_target", false, "ts3_B", "AId", IndexInfo::WhereClause(true, {"AId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts3_B_fk_ts3_Rel_target").c_str());

            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships4.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel11' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "        <ECProperty propertyName='BId' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='BB' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='BBId' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='Rel11' modifier='Sealed' >"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));

            ASSERT_STRCASEEQ(IndexInfo("uix_ts4_B_fk_ts4_Rel11_target", true, "ts4_B", "AId", IndexInfo::WhereClause(true, {"AId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("uix_ts4_B_fk_ts4_Rel11_target").c_str());
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships50.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts50" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Code" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                             </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="RelBase" direction="Backward">
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B1" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B1Id" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="RelBase" modifier="Abstract" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A"/>
                    </Source>
                    <Target multiplicity="(1..*)" polymorphic="True" roleLabel="is referenced by">
                      <Class class="B"/>
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelSub1" modifier="Sealed" strength="referencing">
                    <BaseClass>RelBase</BaseClass>
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="is referenced by">
                      <Class class="B1"/>
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml")));

            ASSERT_EQ(3, (int) GetHelper().GetIndexNamesForTable("ts50_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            ASSERT_STRCASEEQ(IndexInfo("ix_ts50_B_fk_ts50_RelBase_target", false, "ts50_B", "AId", IndexInfo::WhereClause(true, {"AId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts50_B_fk_ts50_RelBase_target").c_str());
            ASSERT_STRCASEEQ(IndexInfo("ix_ts50_B_ARelECClassId", false, "ts50_B", "ARelECClassId", IndexInfo::WhereClause(true, {"ARelECClassId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts50_B_ARelECClassId").c_str());

            ASSERT_FALSE(GetHelper().IndexExists("uix_ts50_B_fk_ts50_RelSub1_target"));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships5.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='RelBase' direction='Backward'>"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B1' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='B1Id' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='RelSub1' modifier='Sealed' strength='referencing'>"
                "    <BaseClass>RelBase</BaseClass>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='B1'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));

            ASSERT_EQ(3, (int) GetHelper().GetIndexNamesForTable("ts5_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            ASSERT_STRCASEEQ(IndexInfo("ix_ts5_B_fk_ts5_RelBase_target", false, "ts5_B", "AId", IndexInfo::WhereClause(true, {"AId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts5_B_fk_ts5_RelBase_target").c_str());

            ASSERT_STRCASEEQ(IndexInfo("ix_ts5_B_ARelECClassId", false, "ts5_B", "ARelECClassId", IndexInfo::WhereClause(true, {"ARelECClassId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts5_B_ARelECClassId").c_str());

            ASSERT_FALSE(GetHelper().IndexExists("uix_ts5_B_fk_ts5_RelSub1_target"));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships6.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AInstance' relationshipName='RelBase' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B1' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='B1Id' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='RelSub1' modifier='Sealed' strength='referencing'>"
                "    <BaseClass>RelBase</BaseClass>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='B1'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));

            ASSERT_EQ(3, (int) GetHelper().GetIndexNamesForTable("ts6_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            ASSERT_STRCASEEQ(IndexInfo("ix_ts6_B_AInstanceRelECClassId", false, "ts6_B", "AInstanceRelECClassId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts6_B_AInstanceRelECClassId").c_str());

            ASSERT_STRCASEEQ(IndexInfo("ix_ts6_B_fk_ts6_RelBase_target", false, "ts6_B", "AInstanceId").ToDdl().c_str(),
                             GetHelper().GetIndexDdl("ix_ts6_B_fk_ts6_RelBase_target").c_str());

            ASSERT_FALSE(GetHelper().IndexExists("uix_ts6_B_fk_ts6_RelSub1_target"));
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships7.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts7' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Code' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B1' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='B1Id' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "    <Source cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='RelSub11' modifier='Sealed' strength='referencing'>"
                "    <BaseClass>RelBase</BaseClass>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Source>"
                "    <Target cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B1' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='RelSub1N' modifier='Sealed' strength='referencing'>"
                "    <BaseClass>RelBase</BaseClass>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='B1' />"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B1' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>")));

            ASSERT_EQ(9, (int) GetHelper().GetIndexNamesForTable("ts7_RelBase").size());
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships8.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts8" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="AId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                             </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B1" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B1Code" typeName="long" />
                        <ECNavigationProperty propertyName="A1" relationshipName="RelPoly" direction="Backward" >
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECNavigationProperty propertyName="A2" relationshipName="RelNonPoly" direction="Backward" >
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B11" modifier="None">
                        <BaseClass>B1</BaseClass>
                        <ECProperty propertyName="B11Code" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B2" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B2Code" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="RelNonPoly" modifier="Sealed" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="False" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelPoly" modifier="Sealed" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml")));

            ASSERT_EQ(3, (int) GetHelper().GetIndexNamesForTable("ts8_B").size());

            ECClassId b1ClassId = m_ecdb.Schemas().GetClassId("TestSchema", "B1");
            ECClassId b11ClassId = m_ecdb.Schemas().GetClassId("TestSchema", "B11");

            IndexInfo::WhereClause indexWhereClause;
            indexWhereClause.AppendNotNullFilter({"A2Id"});
            indexWhereClause.AppendClassIdFilter({b1ClassId});
            ASSERT_STRCASEEQ(IndexInfo("uix_ts8_B_fk_ts8_RelNonPoly_target", true, "ts8_B", "A2Id", indexWhereClause).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("uix_ts8_B_fk_ts8_RelNonPoly_target").c_str()) << "RelNonPoly must exclude index on B11 as the constraint is non-polymorphic";

            indexWhereClause.Clear();
            //RelPoly must include index on B11 as the constraint is polymorphic

            indexWhereClause.AppendNotNullFilter({"A1Id"}).AppendClassIdFilter({b1ClassId, b11ClassId});
            ASSERT_STRCASEEQ(IndexInfo("uix_ts8_B_fk_ts8_RelPoly_target", true, "ts8_B", "A1Id", indexWhereClause).ToDdl().c_str(),
                             GetHelper().GetIndexDdl("uix_ts8_B_fk_ts8_RelPoly_target").c_str());
            }

            {
            //Tests that AllowDuplicateRelationships Flag from LinkTableRelationshipMap CA is applied to subclasses
            ASSERT_EQ(SUCCESS, SetupECDb("indexcreationforrelationships9.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts9\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                                                           "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                                                                                           "  <ECEntityClass typeName='A' modifier='None'>"
                                                                                           "    <ECProperty propertyName='Name' typeName='string' />"
                                                                                           "  </ECEntityClass>"
                                                                                           "  <ECEntityClass typeName='B' modifier='None'>"
                                                                                           "    <ECCustomAttributes>"
                                                                                           "        <ClassMap xmlns='ECDbMap.02.00'>"
                                                                                           "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                                                           "        </ClassMap>"
                                                                                           "    </ECCustomAttributes>"
                                                                                           "    <ECProperty propertyName='BName' typeName='string' />"
                                                                                           "  </ECEntityClass>"
                                                                                           "  <ECEntityClass typeName='C' modifier='None'>"
                                                                                           "    <BaseClass>B</BaseClass>"
                                                                                           "    <ECProperty propertyName='CName' typeName='string' />"
                                                                                           "  </ECEntityClass>"
                                                                                           "  <ECRelationshipClass typeName='ARelB' modifier='Abstract' strength='referencing'>"
                                                                                           "    <ECCustomAttributes>"
                                                                                           "        <ClassMap xmlns='ECDbMap.02.00'>"
                                                                                           "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                                                           "        </ClassMap>"
                                                                                           "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'>"
                                                                                           "             <AllowDuplicateRelationships>True</AllowDuplicateRelationships>"
                                                                                           "        </LinkTableRelationshipMap>"
                                                                                           "    </ECCustomAttributes>"
                                                                                           "    <Source cardinality='(0,N)' polymorphic='True'>"
                                                                                           "      <Class class = 'A' />"
                                                                                           "    </Source>"
                                                                                           "    <Target cardinality='(0,N)' polymorphic='True'>"
                                                                                           "      <Class class = 'B' />"
                                                                                           "    </Target>"
                                                                                           "  </ECRelationshipClass>"
                                                                                           "  <ECRelationshipClass typeName='ARelC' modifier='Sealed' strength='referencing'>"
                                                                                           "    <BaseClass>ARelB</BaseClass>"
                                                                                           "    <Source cardinality='(0,1)' polymorphic='True'>"
                                                                                           "      <Class class = 'A' />"
                                                                                           "    </Source>"
                                                                                           "    <Target cardinality='(0,N)' polymorphic='True'>"
                                                                                           "      <Class class = 'C' />"
                                                                                           "    </Target>"
                                                                                           "  </ECRelationshipClass>"
                                                                                           "</ECSchema>")));

            ASSERT_TRUE(GetHelper().TableExists("ts9_ARelB"));
            ASSERT_FALSE(GetHelper().TableExists("ts9_ARelC")) << "ARelC is expected to be persisted in ts9_ARelB as well (SharedTable strategy)";

            //ARelB must not have a unique index on source and target as it as AllowDuplicateRelationship set to true.
            //ARelC must not have the unique index either, as AllowDuplicateRelationship is applied to subclasses
            std::vector<Utf8String> indexNames = GetHelper().GetIndexNamesForTable("ts9_ARelB");
            ASSERT_EQ(3, (int) indexNames.size()) << "Indexes on ts9_ARelB";
            ASSERT_STREQ("ix_ts9_ARelB_ecclassid", indexNames[0].c_str());
            ASSERT_STREQ("ix_ts9_ARelB_source", indexNames[1].c_str());
            ASSERT_STREQ("ix_ts9_ARelB_target", indexNames[2].c_str());
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, ImplicitIndexesForRelationshipsOnSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ImplicitIndexesForRelationshipsOnSharedColumns.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="None">
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Child" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" modifier="None">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>   
        </ECSchema>)xml")));

    Column idCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Child", "Parent.Id"));
    ASSERT_TRUE(idCol.Exists());
    ASSERT_EQ(Virtual::No, idCol.GetVirtual());
    Column relClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Child", "Parent.RelECClassId"));
    ASSERT_TRUE(relClassIdCol.Exists());
    ASSERT_EQ(Virtual::No, relClassIdCol.GetVirtual());

    std::vector<Utf8String> indexes = GetHelper().GetIndexNamesForTable("ts_Child");
    ASSERT_EQ(1, indexes.size()) << "Indexes on ts_Child";
    ASSERT_STRCASEEQ("ix_ts_Child_ecclassid", indexes[0].c_str()) << "Only index on ts_Child";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     1/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, IndexSkippedForIdSpecificationCA)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IdSpecification.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' alias='ecdbmap' />"
        "<ECEntityClass typeName='ClassWithBusinessKey' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <BusinessKeySpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </BusinessKeySpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='ClassWithSyncId' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <SyncIDSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <Property>Name</Property>"
        "            </SyncIDSpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='ClassWithGlobalId' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <GlobalIdSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </GlobalIdSpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithBusinessKey")->GetPropertyCount(false));
    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithGlobalId")->GetPropertyCount(false));
    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithSyncId")->GetPropertyCount(false));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithBusinessKey_BusinessKeySpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for BusinessKeyCA should'nt be created";
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithGlobalId_GlobalIdSpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for GlobalIdCA should'nt be created";
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithSyncId_SyncIDSpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for SyncIdCA should'nt be created";
    }
END_ECDBUNITTESTS_NAMESPACE