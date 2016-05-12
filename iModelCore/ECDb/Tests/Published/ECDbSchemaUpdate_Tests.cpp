/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaUpdate_Tests.cpp $
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
struct ECSchemaUpdateTests : public SchemaImportTestFixture
    {
    void CloseReOpenECDb()
        {
        Utf8CP dbFileName = m_ecdb.GetDbFileName();
        BeFileName dbPath(dbFileName);
        m_ecdb.CloseDb();
        ASSERT_FALSE(m_ecdb.IsDbOpen());
        ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::Readonly)));
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        }
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
TEST_F(ECSchemaUpdateTests, UpdateECSchemaAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateECClassAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema and Class attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    //verify class is accessible using new ECSchemaPrefix
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateECPropertyAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class and property attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    //Verify class and Property accessible using new ECSchemaPrefix
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdatingECDbMapCAIsNotSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.01'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.01.01'>"
        "                <IsNullable>false</IsNullable>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>", false);
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateCAProperties)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>"
        "       <ECProperty propertyName = 'ColumnName' typeName = 'string' description = 'If not specified, the ECProperty name is used. It must follow EC Identifier specification.' />"
        "       <ECProperty propertyName = 'IsNullable' typeName = 'boolean' description = 'If false, values must not be unset for this property.' />"
        "       <ECProperty propertyName = 'IsUnique' typeName = 'boolean' description = 'Only allow unique values for this property.' />"
        "       <ECProperty propertyName = 'Collation' typeName = 'string' description = 'Specifies how string comparisons should work for this property. Possible values: Binary (default): bit to bit matching. NoCase: The same as binary, except that the 26 upper case characters of ASCII are folded to their lower case equivalents before comparing. Note that it only folds ASCII characters. RTrim: The same as binary, except that trailing space characters are ignored.' />"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>"
        "       <ECProperty propertyName = 'ColumnName' typeName = 'string' description = 'If not specified, the ECProperty name is used. It must follow EC Identifier specification.' />"
        "       <ECProperty propertyName = 'IsNullable' typeName = 'boolean' description = 'If false, values must not be unset for this property.' />"
        "       <ECProperty propertyName = 'IsUnique' typeName = 'boolean' description = 'Only allow unique values for this property.' />"
        "       <ECProperty propertyName = 'Collation' typeName = 'string' description = 'Specifies how string comparisons should work for this property. Possible values: Binary (default): bit to bit matching. NoCase: The same as binary, except that the 26 upper case characters of ASCII are folded to their lower case equivalents before comparing. Note that it only folds ASCII characters. RTrim: The same as binary, except that trailing space characters are ignored.' />"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "                <IsNullable>false</IsNullable>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    //Verify class and Property accessible using new ECSchemaPrefix
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //verify CA changes
    testProperty = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("TestCA");
    ASSERT_TRUE(propertyMapCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
    ASSERT_FALSE(val.GetBoolean());

    val.Clear();
    ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "ColumnName"));
    ASSERT_STREQ("TestProperty1", val.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   AFfan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, SqlSchemaChangeIsNotSupportedOnClientBriefcase)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());
    BeBriefcaseId newClientSideBriefcaseId(123);
    GetECDb().ChangeBriefcaseId(newClientSideBriefcaseId);
    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>", false, "Db Tables Modifications not allowed on Client Briefcase");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewEntityClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    //Verify Newly Added Entity Class exists
    ECClassCP entityClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(entityClass != nullptr);
    ASSERT_TRUE(entityClass->GetDisplayLabel() == "Test Class");
    ASSERT_TRUE(entityClass->GetDescription() == "This is test Class");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Test Class", statement.GetValueText(0));
    ASSERT_STREQ("This is test Class", statement.GetValueText(1));

    //Query newly added Entity Class
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewClassModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='NewTestClass' displayLabel='New Test Class' description='This is New test Class' modifier='None' />"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify newly added Entity Class exists
    ECClassCP newTestClass = testSchema->GetClassCP("NewTestClass");
    ASSERT_TRUE(newTestClass != nullptr);
    ASSERT_TRUE(newTestClass->GetDisplayLabel() == "New Test Class");
    ASSERT_TRUE(newTestClass->GetDescription() == "This is New test Class");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description From ec.ECClassDef WHERE Name='NewTestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("New Test Class", statement.GetValueText(0));
    ASSERT_STREQ("This is New test Class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    //Query existing and newly added Entity Classes
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.NewTestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify newly added property exists
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is property");

    CloseReOpenECDb();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is property", statement.GetValueText(1));

    //Query newly added Property
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewPropertyModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "       </ECProperty>"
        "       <ECProperty propertyName='NewTestProperty' displayLabel='New Test Property' description='this is new property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    //verify newly added Property exists
    ECPropertyCP newTestProperty = testClass->GetPropertyP("NewTestProperty");
    ASSERT_TRUE(newTestProperty != nullptr);
    ASSERT_TRUE(newTestProperty->GetDisplayLabel() == "New Test Property");
    ASSERT_TRUE(newTestProperty->GetDescription() == "this is new property");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='NewTestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("New Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is new property", statement.GetValueText(1));

    //Query existing and newly added Entity Classes
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty, NewTestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewCAOnNewClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' />"
        "   <ECEntityClass typeName='NewTestClass' displayLabel='New Test Class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <DisplayOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <Hidden>True</Hidden>"
        "            </DisplayOptions>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    //verify tables
    ASSERT_TRUE(GetECDb().TableExists("ts_TestClass"));
    //new class should be added with new namespace prefix
    ASSERT_TRUE(GetECDb().TableExists("ts_modified_NewTestClass"));

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.NewTestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Verify newly added CA
    testClass = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("NewTestClass");
    ASSERT_TRUE(testClass != nullptr);
    IECInstancePtr bsca = testClass->GetCustomAttribute("DisplayOptions");
    ASSERT_TRUE(bsca != nullptr);

    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "Hidden"));
    ASSERT_TRUE(val.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddingNewECDbMapCANotSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupgrade.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "               <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        "               </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Adding new ECDbMap Custom Attribute is not supported, schema rejected");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewCAOnProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>"
        "       <ECProperty propertyName = 'IsUnique' typeName = 'boolean' description = 'Only allow unique values for this property.' />"
        "       <ECProperty propertyName = 'Collation' typeName = 'string' description = 'Specifies how string comparisons should work for this property. Possible values: Binary (default): bit to bit matching. NoCase: The same as binary, except that the 26 upper case characters of ASCII are folded to their lower case equivalents before comparing. Note that it only folds ASCII characters. RTrim: The same as binary, except that trailing space characters are ignored.' />"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "                <IsUnique>false</IsUnique>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify Schema, Class and property attributes upgraded successfully
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetNamespacePrefix() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description, NameSpacePrefix FROM ec.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    //Query Property
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //verify newly added CA on Property
    testProperty = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    IECInstancePtr testCA = testProperty->GetCustomAttribute("TestCA");
    ASSERT_TRUE(testCA != nullptr);
    ECValue val;
    ASSERT_EQ(ECObjectsStatus::Success, testCA->GetValue(val, "IsUnique"));
    ASSERT_FALSE(val.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MinimumSharedColumnsCount_AddProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        //    "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 7));
    //AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumns");

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "       <ECProperty propertyName='P3' typeName='int' />"
        "       <ECProperty propertyName='P4' typeName='int' />"
        "       <ECProperty propertyName='P5' typeName='int' />"
        "       <ECProperty propertyName='P6' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    CloseReOpenECDb();

    //Verify number of columns after upgrade
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 8));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumns");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MinimumSharedColumnsCountForSubClasses_AddProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumnsForSubclasses</Options>"
        "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='S1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 8));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsForSubClasses");

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>SharedColumnsForSubclasses</Options>"
        "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='S1' typeName='double' />"
        "        <ECProperty propertyName='S2' typeName='double' />"
        "        <ECProperty propertyName='S3' typeName='double' />"
        "        <ECProperty propertyName='S4' typeName='double' />"
        "        <ECProperty propertyName='S5' typeName='double' />"
        "        <ECProperty propertyName='S6' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    CloseReOpenECDb();

    //Verify number of columns after upgrade
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 10));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsForSubClasses");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MinimumSharedColumnsCountWithJoinedTable_AddProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>JoinedTablePerDirectSubclass,SharedColumnsForSubclasses</Options>"
        "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='S1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Sub1", 7));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsWithJoinedTable");

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <Options>JoinedTablePerDirectSubclass,SharedColumnsForSubclasses</Options>"
        "                   <MinimumSharedColumnCount>5</MinimumSharedColumnCount>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                 </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='S1' typeName='double' />"
        "        <ECProperty propertyName='S2' typeName='double' />"
        "        <ECProperty propertyName='S3' typeName='double' />"
        "        <ECProperty propertyName='S4' typeName='double' />"
        "        <ECProperty propertyName='S5' typeName='double' />"
        "        <ECProperty propertyName='S6' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    CloseReOpenECDb();

    //Verify number of columns after upgrade
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Sub1", 8));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsWithJoinedTable");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ImportMultipleSchemaVersions_AddNewProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.2.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import edited schema with lower minor version with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Schema upgrade with lower minor version not allowed");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);

    //Verify newly added property must not exist at this point
    ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);

    ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty == nullptr);

    //import edited schema with higher minor version with some changes.
    SchemaItem editedSchemaItem1(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.3.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem1);
    ASSERT_FALSE(asserted);

    //Verify newly added property must exist after third schema import
    testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);

    testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is property");

    CloseReOpenECDb();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is property", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateMultipleSchemasInDb)
    {
    ECDbTestFixture::Initialize();
    ECDbR ecdb = SetupECDb("updateStartupCompanyschema.ecdb", BeFileName(L"DSCacheSchema.01.00.ecschema.xml"));

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"DSCacheSchema.01.03.ecschema.xml");
    BentleyStatus schemaStatus = ecdb.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(ERROR, schemaStatus);
    /*
    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"RSComponents.01.00.ecschema.xml");
    schemaStatus = ecdb.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"RSComponents.02.00.ecschema.xml");
    ecSchema->SetVersionMajor(1);
    ecSchema->SetVersionMinor(22);
    schemaStatus = ecdb.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);
    */
    }

//*******************Schema Upgrade Invalid Cases*******************//
//******************************************************************//

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DowngradeSchemaMajorVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import schema with downgraded major version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Cannot Downgrade schema Major Version");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DowngradeSchemaMiddleVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import schema with downgraded middle version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Cannot Downgrade schema middle Version");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DowngradeSchemaMinorVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //import schema with downgraded minor version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Cannot Downgrade schema Minor Version");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, SetSchemaNamePrefixNull)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Try modifying nameSpacePrefix=''
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Schema nameSparePrefix can't be set to NULL");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, InvalidValueForNameSpacePrefix)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem schemaItem1(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaItem1);
    ASSERT_FALSE(asserted);

    //Try Upgrading to already existing namespaceprefix
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "can't upgrade Another schema with same namespaceprefix already exists");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                          05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_ECEntityClass_MappedTo_OwnTable)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    auto assertAndExecuteECSQL = [] (ECDbCR ecdb, Utf8CP ecsql, ECSqlStatus prepareStatus = ECSqlStatus::Success, DbResult stepStatus = BE_SQLITE_DONE)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(stmt.Prepare(ecdb, ecsql), prepareStatus);
        if (stmt.IsPrepared())
            {
            ASSERT_EQ(stmt.Step(), stepStatus);
            }
        };

    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)" , ECSqlStatus::Success, BE_SQLITE_DONE);
    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)", ECSqlStatus::Success, BE_SQLITE_DONE);
    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Goo(S,D,L) VALUES ('test3', 44.32, 3344)", ECSqlStatus::Success, BE_SQLITE_DONE);
    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Goo(S,D,L) VALUES ('test4', 13.3, 2345)", ECSqlStatus::Success, BE_SQLITE_DONE);

    ASSERT_TRUE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Delete Foo ===================================================================================================
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete class should be successfull");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);
    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    assertAndExecuteECSQL(GetECDb(), "SELECT S, D, L FROM ts.Foo", ECSqlStatus::InvalidECSql);
    assertAndExecuteECSQL(GetECDb(), "SELECT S, D, L FROM ts.Goo", ECSqlStatus::Success, BE_SQLITE_ROW);

    //Add Foo Again===============================================================================================
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete class should be successfull");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    assertAndExecuteECSQL(GetECDb(), "SELECT S, D, L FROM ts.Foo", ECSqlStatus::Success, BE_SQLITE_DONE);
    assertAndExecuteECSQL(GetECDb(), "SELECT S, D, L FROM ts.Goo", ECSqlStatus::Success, BE_SQLITE_ROW);

    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)", ECSqlStatus::Success, BE_SQLITE_DONE);
    assertAndExecuteECSQL(GetECDb(), "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)", ECSqlStatus::Success, BE_SQLITE_DONE);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ValidateModifingAddingDeletingBaseClassNotSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    Savepoint sp(GetECDb(), "schemaImport");
    SchemaItem schemaWithDeletedBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "   </ECEntityClass >"
        "</ECSchema>", false, "Deleting Base Class not allowed");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDeletedBaseClass);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem schemaWithModifedBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass0</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>", false, "Modifying Base Class not allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithModifedBaseClass);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem schemaWithNewBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass</BaseClass>"
        "       <BaseClass>TestClass0</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>", false, "Adding new Base Class not allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithNewBaseClass);
    ASSERT_FALSE(asserted);
    sp.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteExistingECEnumeration)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Deletion of ECEnumeration is not suppported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyExistingECEnumeration)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        " </ECEnumeration>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>", false, "Modifying ECEnumeration is not suppported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewECEnumeration)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        " </ECEnumeration>"
        "</ECSchema>", false, "Adding new ECEnumeration is not suppported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpgradeECClassModifier)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Try Updating Class modifier
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='Abstract' />"
        "</ECSchema>", false, "Updating class Modifier is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyIsEntityClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='TestClass' modifier='None' />"
        "</ECSchema>", false, "Changing ECClass::IsEntityClass is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyIsStructClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='TestClass' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>", false, "Changing ECClass::IsStructClass is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyIsCustomAttributeClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName='TestClass' appliesTo='EntityClass, RelationshipClass' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>", false, "Changing ECClass::IsCustomAttributeClass is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyIsRelationshipClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "   <ECCustomAttributeClass typeName='RelClass' />"
        "</ECSchema>", false, "Changing ECClass::IsRelationshipClass is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyRelationship)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    Savepoint sp(GetECDb(), "Schema Import");
    //Try Upgrade with different source Cardinality.
    SchemaItem schemaWithDifferentCardinality(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDifferentCardinality);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different target Cardinality.
    SchemaItem schemaWithDifferentTargetCardinality(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDifferentTargetCardinality);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different source Constraint Class
    SchemaItem differentSourceConstraintClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "   <ECEntityClass typeName='B' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), differentSourceConstraintClass);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different Target Constraint Class
    SchemaItem differentTargetConstraintClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "   <ECEntityClass typeName='B' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), differentTargetConstraintClass);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with NonPolymorphic Source
    SchemaItem nonPolymorphicSource(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='False'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), nonPolymorphicSource);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with NonPolymorphic Target
    SchemaItem nonPolymorphicTarget(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='False'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing Relationship Cardinality now allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), nonPolymorphicTarget);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Try Upgrading schema with different strength.
    SchemaItem schemaWithDifferentStrength(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='referencing' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing relationship Strength not allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDifferentStrength);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    //Verify Changing strength direction not supported
    SchemaItem schemaWithDifferentStrengthDirection(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='None' strength='embedding' strengthDirection='backward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "changing relationship Strength Direction not allowed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDifferentStrengthDirection);
    ASSERT_FALSE(asserted);
    sp.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyProperties)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    Savepoint sp(GetECDb(), "SchemaUpgrade");
    SchemaItem modifiedECPropertyType(
        //SchemaItem with modified ECProperty type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='int' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECSchema Property type is not supported");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedECPropertyType);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem modifiedECStructPropertyType(
        //SchemaItem with modified ECStructProperty type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructArrayProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECStructProperty is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedECStructPropertyType);
    ASSERT_FALSE(asserted);
    sp.Cancel();


    sp.Begin();
    SchemaItem modifiedECStructArrayPropertyType(
        //SchemaItem with modified ECStructArrayProperty type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECStructArrayProperty is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedECStructArrayPropertyType);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem modifiedPrimitiveArrayType(
        //SchemaItem with modified IsPrimitiveArray Type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='PrimitiveArrayProperty' typeName='string' minOccurs='0' maxOccurs='5' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECArrayProperty is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedPrimitiveArrayType);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem modifiedPrimitiveType(
        //SchemaItem with modified IsPrimitive type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECArrayProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying IsPrimitiveType is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedPrimitiveType);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem modifiedECPropertyArrayMixOccurs(
        //SchemaItem with Modified ECPropertyArray MinOccurs
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='1' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECPropertyArray minOccurs is not Supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedECPropertyArrayMixOccurs);
    ASSERT_FALSE(asserted);
    sp.Cancel();


    sp.Begin();
    SchemaItem modifiedECArrayPropertyMaxOccurs(
        //SchemaItem with Modified ECArrayProperty MaxOccurs
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='10' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='10' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ECArrayProperty maxOccuers is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedECArrayPropertyMaxOccurs);
    ASSERT_FALSE(asserted);
    sp.Cancel();


    sp.Begin();
    SchemaItem modifiedExtendedType(
        //SchemaItem with Modifed Extended Type
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='false' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='email' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Modifying ExtendedTypeName is not supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedExtendedType);
    ASSERT_FALSE(asserted);
    sp.Cancel();

    sp.Begin();
    SchemaItem modifiedReadonlyFlag(
        //SchemaItem with Modified readonly flag
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='PrimitiveProperty' typeName='string' readOnly='true' />"
        "       <ECArrayProperty propertyName='PrimitiveArrayProperty' minOccurs='0' maxOccurs='5' typeName='string' />"
        "       <ECStructProperty propertyName='structProp' typeName='ChangeInfoStruct' readOnly='false' />"
        "       <ECStructArrayProperty propertyName='StructArrayProp' typeName='ChangeInfoStruct' minOccurs='0' maxOccurs='5' readOnly='false' />"
        "       <ECProperty propertyName='ExtendedProperty' typeName='string' extendedTypeName='URL' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Modifying ReadOnly flag is supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedReadonlyFlag);
    ASSERT_FALSE(asserted);
    sp.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropToReadOnly)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='ReadWriteProp' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='P1' typeName='string' readOnly='true' />"
        "       <ECProperty propertyName='P2' typeName='string' readOnly='false' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    /*-------------------After 1st Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadOnly
    P2            -> ReadWrite
    */

    ECSqlStatement statement;
    //Insert should be successfull
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val1', 'P2_Val1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    ASSERT_NE(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P1='P1_Val1new'"));

    statement.Finalize();
    //skipping readonly Property, Update should be successful.
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P2='P2_Val1new' WHERE P2='P2_Val1'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Update schema 
    SchemaItem schemaItem2(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='ReadWriteProp' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='P1' typeName='string' readOnly='false' />"// readOnly='false' after update
        "       <ECProperty propertyName='P2' typeName='string' readOnly='true' />"//readOnly='true' after update
        "   </ECEntityClass>"
        "</ECSchema>", true, "Modifying readonly Flag is expected to succeed");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaItem2);
    EXPECT_FALSE(asserted);

    /*-------------------After 2nd Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadWrite
    P2            -> ReadOnly
    */

    //Verify Insert
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW2', 'P1_Val2', 'P2_Val2')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Verify Update
    statement.Finalize();
    ASSERT_NE(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P2='P2_Val2new'"));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P1='P1_Val2new' WHERE P1='P1_Val2'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Verify Select
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P2 FROM ts.TestClass WHERE ReadWriteProp='RW1new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P2_Val1new", statement.GetValueText(0));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW2new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val2new", statement.GetValueText(0));

    //Verify Delete
    Savepoint sp(GetECDb(), "To Revert Delete Operation");
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "DELETE FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT COUNT(*) FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    sp.Cancel();

    //Update schema 
    SchemaItem schemaItem3(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='ReadWriteProp' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='P1' typeName='string' readOnly='true' />"// readOnly='true' after update
        "       <ECProperty propertyName='P2' typeName='string' readOnly='false' />"//readOnly='false' after update
        "   </ECEntityClass>"
        "</ECSchema>", true, "Modifying readonly Flag is expected to succeed");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaItem3);
    EXPECT_FALSE(asserted);

    /*-------------------After 3rd Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadOnly
    P2            -> ReadWrite
    */
    statement.Finalize();
    //Insert should be successfull
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val3', 'P2_Val3')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Verify Update
    statement.Finalize();
    //Update Prepare should fail for ReadOnlyProp
    ASSERT_NE(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P1='P1_Val3new''"));

    statement.Finalize();
    //skipping readonly Property Update should be successful.
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P2='P2_Val3new' WHERE P1 = 'P1_Val3'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    //Verify Select
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW3new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val3", statement.GetValueText(0));

    statement.Finalize();
    //Verify Delete
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "DELETE FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT COUNT(*) FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyCustomAttributePropertyValues)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>"
        "       <ECProperty propertyName = 'BinaryProp' typeName = 'Binary' />"
        "       <ECProperty propertyName = 'BooleanProp' typeName = 'boolean' />"
        "       <ECProperty propertyName = 'DateTimeProp' typeName = 'DateTime' />"
        "       <ECProperty propertyName = 'DoubleProp' typeName = 'Double' />"
        "       <ECProperty propertyName = 'IntegerProp' typeName = 'int' />"
        "       <ECProperty propertyName = 'LongProp' typeName = 'long' />"
        "       <ECProperty propertyName = 'Point2DProp' typeName = 'Point2D' />"
        "       <ECProperty propertyName = 'Point3DProp' typeName = 'Point3D' />"
        "       <ECProperty propertyName = 'StringProp' typeName = 'string' />"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "                <BinaryProp>10100101</BinaryProp>"
        "                <BooleanProp>true</BooleanProp>"
        "                <DateTimeProp>20160509</DateTimeProp>"
        "                <DoubleProp>1.0001</DoubleProp>"
        "                <IntegerProp>10</IntegerProp>"
        "                <LongProp>1000000</LongProp>"
        "                <Point2DProp>3.0,4.5</Point2DProp>"
        "                <Point3DProp>30.5,40.5,50.5</Point3DProp>"
        "                <StringProp>'This is String Property'</StringProp>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>"
        "       <ECProperty propertyName = 'BinaryProp' typeName = 'Binary' />"
        "       <ECProperty propertyName = 'BooleanProp' typeName = 'boolean' />"
        "       <ECProperty propertyName = 'DateTimeProp' typeName = 'DateTime' />"
        "       <ECProperty propertyName = 'DoubleProp' typeName = 'Double' />"
        "       <ECProperty propertyName = 'IntegerProp' typeName = 'int' />"
        "       <ECProperty propertyName = 'LongProp' typeName = 'long' />"
        "       <ECProperty propertyName = 'Point2DProp' typeName = 'Point2D' />"
        "       <ECProperty propertyName = 'Point3DProp' typeName = 'Point3D' />"
        "       <ECProperty propertyName = 'StringProp' typeName = 'string' />"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "                <BinaryProp>10100011</BinaryProp>"
        "                <BooleanProp>false</BooleanProp>"
        "                <DateTimeProp>20160510</DateTimeProp>"
        "                <DoubleProp>2.0001</DoubleProp>"
        "                <IntegerProp>20</IntegerProp>"
        "                <LongProp>2000000</LongProp>"
        "                <Point2DProp>4.0,5.5</Point2DProp>"
        "                <Point3DProp>35.5,45.5,55.5</Point3DProp>"
        "                <StringProp>'This is Modified String Property'</StringProp>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }
END_ECDBUNITTESTS_NAMESPACE
