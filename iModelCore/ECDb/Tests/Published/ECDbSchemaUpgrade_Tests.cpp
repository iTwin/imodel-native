/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaUpgrade_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbSchemaUpgradeTests : SchemaImportTestFixture
    {
    };

#ifdef NotUsedYet
struct ECSchemaDuplicator :IECSchemaLocater
    {
    private:
        ECSchemaCacheCR m_fromCache;
    private:
        virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override
            {
            if (auto schema = schemaContext.GetCache().GetSchema(key, matchType))
                return schema;

            if (auto schema = m_fromCache.GetSchema(key, matchType))
                {
                Utf8String xml;
                if (SchemaWriteStatus::Success != schema->WriteToXmlString(xml))
                    return nullptr;

                ECSchemaPtr out;
                if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(out, xml.c_str(), schemaContext))
                    return nullptr;

                return out;
                }

            return nullptr;
            }
        ECSchemaDuplicator(ECSchemaCacheCR fromCache)
            :m_fromCache(fromCache)
            {}
    public:
        static ECSchemaCachePtr Duplicate(ECSchemaCacheCR fromCache)
            {
            ECSchemaReadContextPtr ptr = ECSchemaReadContext::CreateContext();
            ECSchemaDuplicator loc(fromCache);
            ptr->AddSchemaLocater(loc);
            bvector<ECSchemaP> schemas;
            fromCache.GetSchemas(schemas);
            for (auto schema : schemas)
                {
                ptr->LocateSchema(const_cast<SchemaKeyR>(schema->GetSchemaKey()), SchemaMatchType::Exact);
                }

            return &ptr->GetCache();
            }
        static ECSchemaCachePtr Duplicate(ECSchemaCR schema)
            {
            ECSchemaCachePtr ptr = ECSchemaCache::Create();
            ptr->AddSchema(const_cast<ECSchemaR>(schema));
            return Duplicate(*ptr);
            }
        static ECSchemaCachePtr Duplicate(ECSchemaReadContextR ctx)
            {
            return Duplicate(ctx.GetCache());
            }
    };
#endif // NotUsedYet

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, UpdateECSchemaAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, UpdateECClassAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema and Class attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, UpdateECPropertyAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class and property attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, UpdateCAProperties)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify CA changes
    IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("PropertyMap");
    ASSERT_TRUE(propertyMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
    ASSERT_FALSE(val.GetBoolean());
    val.Clear();

    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "ColumnName"));
    ASSERT_STREQ("TestProperty1", val.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewEntityClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts'  version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Newly Added Entity Class exists
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP entityClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(entityClass != nullptr);
    ASSERT_TRUE(entityClass->GetDisplayLabel() == "Test Class");
    ASSERT_TRUE(entityClass->GetDescription() == "This is test Class");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewClassModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='NewTestClass' displayLabel='New Test Class' description='This is New test Class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify CA changes
    IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("PropertyMap");
    ASSERT_TRUE(propertyMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
    ASSERT_FALSE(val.GetBoolean());
    val.Clear();

    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "ColumnName"));
    ASSERT_STREQ("TestProperty1", val.GetUtf8CP());

    //verify newly added Entity Class exists
    ECClassCP newTestClass = testSchema->GetClassCP("NewTestClass");
    ASSERT_TRUE(newTestClass != nullptr);
    ASSERT_TRUE(newTestClass->GetDisplayLabel() == "New Test Class");
    ASSERT_TRUE(newTestClass->GetDescription() == "This is New test Class");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify newly added property exists
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is property");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewPropertyModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECProperty propertyName='NewTestProperty' displayLabel='New Test Property' description='this is new property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify CA changes
    IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("PropertyMap");
    ASSERT_TRUE(propertyMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
    ASSERT_FALSE(val.GetBoolean());
    val.Clear();

    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "ColumnName"));
    ASSERT_STREQ("TestProperty1", val.GetUtf8CP());

    //verify newly added Property exists
    ECPropertyCP newTestProperty = testClass->GetPropertyP("NewTestProperty");
    ASSERT_TRUE(newTestProperty->GetDisplayLabel() == "New Test Property");
    ASSERT_TRUE(newTestProperty->GetDescription() == "this is new property");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewCAOnSchema)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='New Display Label' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "        <ECCustomAttributes>"
        "            <SchemaMap xmlns='ECDbMap.01.00'>"
        "                <TablePrefix>myownprefix</TablePrefix>"
        "            </SchemaMap>"
        "        </ECCustomAttributes>"
        "   <ECEntityClass typeName='TestClass' displayLabel='New Display Label' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_FALSE(ecdb.TableExists("myownprefix_TestClass"));
    ASSERT_TRUE(ecdb.TableExists("ts_TestClass"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewCAOnClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "               <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        "               </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema and Class attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    //Verify newly added CA
    IECInstancePtr classMapCA = testClass->GetCustomAttribute("ClassMap");
    ASSERT_TRUE(classMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, classMapCA->GetValue(val, "MapStrategy.Strategy"));
    ASSERT_STREQ("SharedTable", val.GetUtf8CP());
    val.Clear();

    ASSERT_EQ(ECObjectsStatus::Success, classMapCA->GetValue(val, "MapStrategy.Options"));
    ASSERT_STREQ("SharedColumns", val.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaUpgradeTests, AddNewCAOnProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.00'>"
        "                <IsNullable>true</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class and property attributes upgraded successfully
    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify newly added CA on Property
    IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("PropertyMap");
    ASSERT_TRUE(propertyMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
    ASSERT_TRUE(val.GetBoolean());
    }

END_ECDBUNITTESTS_NAMESPACE
