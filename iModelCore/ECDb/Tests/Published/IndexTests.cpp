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
        "    <ECEntityClass typeName='Element' modifier='None'>"
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
        "    <ECEntityClass typeName='Element' modifier='None'>"
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
        "    <ECEntityClass typeName='Element' modifier='None'>"
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
        "    <ECEntityClass typeName='Element' modifier='None'>"
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
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
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
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
        "    <ECEntityClass typeName='Element' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
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
        "    <ECEntityClass typeName='DgnElement' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
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
        "    <ECEntityClass typeName='Element' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </ClassMap>"
        "            </ShareColumns>"
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
        "    <ECEntityClass typeName='A' modifier='None' >"
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
        "    <ECEntityClass typeName='B' modifier='None' >"
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
        "    <ECEntityClass typeName='B' modifier='None' >"
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

    indexName = "ix_sub1_prop";
    ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts2_Base", "Sub1_Prop").ToDdl().c_str(),
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
TEST_F(IndexTests, UserDefinedIndexesOnNonTph)
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
                               <Name>uix_root</Name>
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
        </ECSchema>)xml"))) << "unique index on abstract base class with multiple subclasses";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>uix_root</Name>
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
        </ECSchema>)xml"))) << "unique index on abstract base class with multiple subclasses";

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
                               <Name>uix_sub</Name>
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
        </ECSchema>)xml"))) << "unique index on abstract base class with multiple subclasses";

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
                               <Name>uix_sub</Name>
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
        </ECSchema>)xml"))) << "unique index on abstract base class with multiple subclasses";

            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb",SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>uix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "unique index on abstract class, no subclasses";

            ASSERT_FALSE(GetHelper().TableExists("ts_Root")) << "unique index on abstract class, no subclasses";
            ASSERT_FALSE(GetHelper().IndexExists("uix_root")) << "unique index on abstract class, no subclasses";
            }

            //multi-session import

            {
            ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>uix_root</Name>
                               <Properties>
                                  <string>RootProp</string>
                               </Properties>
                           </DbIndex>
                         </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "unique index on abstract class, no subclasses";

            ASSERT_FALSE(GetHelper().TableExists("ts_Root")) << "unique index on abstract class, no subclasses";
            ASSERT_FALSE(GetHelper().IndexExists("uix_root")) << "unique index on abstract class, no subclasses";

            ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
            <ECEntityClass typeName="Sub">
               <BaseClass>ts:Root</BaseClass>
               <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Adding single subclass to abstract base class";

            ASSERT_TRUE(GetHelper().TableExists("ts2_Sub")) << "unique index on abstract class with single subclasses";
            Utf8CP indexName = "uix_root_ts2_Sub";
            ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts2_Sub", "RootProp").ToDdl().c_str(), GetHelper().GetIndexDdl(indexName).c_str()) << "unique index on abstract class with single subclasses";

            ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema3" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
            <ECEntityClass typeName="Sub2">
               <BaseClass>ts:Root</BaseClass>
               <ECProperty propertyName="SubProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Adding second subclass to abstract base class";
            }


    {
    ASSERT_EQ(SUCCESS, SetupECDb("UserDefinedIndexesOnNonTph.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECProperty propertyName="RootProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub" modifier="None">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                         <Indexes>
                           <DbIndex>
                               <IsUnique>True</IsUnique>
                               <Name>uix_sub</Name>
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
        </ECSchema>)xml"))) << "Unique index on base class which doesn't have subclasses yet";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />    
            <ECEntityClass typeName="SubSub">
               <BaseClass>ts:Sub</BaseClass>
               <ECProperty propertyName="SubSubProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "unique index would now span two tables";
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
            ASSERT_STRCASEEQ(IndexInfo(expectedIndexName, false, "ts_Base", "ps3").ToDdl().c_str(), GetHelper().GetIndexDdl(expectedIndexName).c_str());
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
TEST_F(IndexTests, UserDefinedOnSystemProperties)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_Foo_ECInstanceId</Name>"
        "                       <Properties>"
        "                          <string>ECInstanceId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on ECInstanceId";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_Foo_ECClassId</Name>"
        "                       <Properties>"
        "                          <string>ECClassId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on ECClassId";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedOnRelationships)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "        <ECNavigationProperty propertyName='Foo' relationshipName='FooHasGoo' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_SourceECInstanceId</Name>"
        "                       <Properties>"
        "                          <string>SourceECInstanceId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..1)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on fk relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_ECInstanceId</Name>"
        "                       <Properties>"
        "                          <string>ECInstanceId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on ECInstanceId for link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_ECClassId</Name>"
        "                       <Properties>"
        "                          <string>ECClassId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on ECClassId for link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_SourceECInstanceId</Name>"
        "                       <Properties>"
        "                          <string>SourceECInstanceId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on SourceECInstanceId for link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_TargetECInstanceId</Name>"
        "                       <Properties>"
        "                          <string>TargetECInstanceId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on TargetECInstanceId for link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_SourceECClassId</Name>"
        "                       <Properties>"
        "                          <string>SourceECClassId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on SourceECClassId for link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_FooHasGoo_TargetECClassId</Name>"
        "                       <Properties>"
        "                          <string>TargetECClassId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on TargetECClassId for link table relationship";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("indexonrelationships.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECProperty propertyName='Code' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "        <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_FooHasGoo_Order</Name>"
        "                       <Properties>"
        "                          <string>Order</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..*)' polymorphic='true' roleLabel='has'>"
        "      <Class class='Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='is related to'>"
        "      <Class class='Goo' />"
        "    </Target>"
        "    <ECProperty propertyName='Order' typeName='int'/>"
        "  </ECRelationshipClass>"
        "</ECSchema>")));

    Utf8CP indexName = "uix_FooHasGoo_Order";
    ASSERT_STRCASEEQ(IndexInfo(indexName, true, "ts_FooHasGoo", "Order").ToDdl().c_str(), GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, RelECClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IndexGenerationOnClassId.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassB' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Statement sqlstmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassA')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassA_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassB')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassB_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin                         03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IndexTests, UserDefinedIndexWithWhereIsNotNull)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmapindextest.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                                     "   <ECSchemaReference name='ECDbMap' version='02.00' prefix ='ecdbmap' />"
                                                                     "   <ECEntityClass typeName = 'IndexClass' >"
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
                                                                     "<ECEntityClass typeName='IndexClass2' >"
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