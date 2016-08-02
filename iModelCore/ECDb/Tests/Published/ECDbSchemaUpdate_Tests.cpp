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
    std::vector<Utf8String> m_updatedDbs;
    protected:

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Muhammad.Hassan                     06/16
        //+---------------+---------------+---------------+---------------+---------------+------
        void CloseReOpenECDb()
            {
            Utf8CP dbFileName = m_ecdb.GetDbFileName();
            BeFileName dbPath(dbFileName);
            m_ecdb.CloseDb();
            ASSERT_FALSE(m_ecdb.IsDbOpen());
            ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::Readonly)));
            ASSERT_TRUE(m_ecdb.IsDbOpen());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Muhammad.Hassan                     06/16
        //+---------------+---------------+---------------+---------------+---------------+------
        DbResult OpenBesqliteDb(Utf8CP dbPath)
            {
            return m_ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::ReadWrite));
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Muhammad.Hassan                     06/16
        //+---------------+---------------+---------------+---------------+---------------+------
        void AssertSchemaUpdate(bool &asserted, Utf8CP SchemaXml, BeFileName seedFilePath, BeBriefcaseId briefcaseId, bool expectedToSucceed, Utf8CP assertMessage)
            {
            Utf8String dbFileName;
            dbFileName.Sprintf("schemaupdate_briefcaseId_%" PRIu64 ".ecdb", briefcaseId.GetValue());

            ECDb ecdb;
            CloneECDb(ecdb, dbFileName.c_str(), seedFilePath);
            ASSERT_TRUE(ecdb.IsDbOpen());

            if (briefcaseId != ecdb.GetBriefcaseId())
                ASSERT_EQ(BE_SQLITE_OK, ecdb.ChangeBriefcaseId(briefcaseId));

            AssertSchemaImport(asserted, ecdb, SchemaItem(SchemaXml, expectedToSucceed, assertMessage));

            if (expectedToSucceed)
                m_updatedDbs.push_back((Utf8String) ecdb.GetDbFileName());

            ecdb.CloseDb();
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

void AssertECProperties(ECDbCR ecdb, Utf8CP assertExpression, bool strict = true)
    {
    const bool includeBaseProperties = true;
    Utf8String str = assertExpression;
    str.ReplaceAll(" ", "");
    str.Trim();
    size_t n = str.find("->");
    if (n == Utf8String::npos)
        {
        ASSERT_FALSE(true) << "Assert Expression is invalid (" << assertExpression << ")";
        }

    Utf8String classQualifiedName = str.substr(0, n);
    Utf8String expr = str.substr(n + 2);
    n = classQualifiedName.find(":");
    if (n == Utf8String::npos)
        {
        ASSERT_FALSE(true) << "Assert Expression is invalid (ClassName is invalid) (" << assertExpression << ")";
        }

    Utf8String schemaName = classQualifiedName.substr(0, n);
    Utf8String className = classQualifiedName.substr(n + 1);

    ECClassCP ecClass = ecdb.Schemas().GetECClass(schemaName.c_str(), className.c_str(), ResolveSchema::AutoDetect);
    ASSERT_TRUE(ecClass != nullptr) << "Failed to find class " << classQualifiedName;

    bvector<Utf8String> properties;
    BeStringUtilities::Split(expr.c_str(), ",", properties);
    std::set<Utf8String> currentProperties;

    std::set<Utf8String> mustNotExist;
    std::set<Utf8String> mustExist;
    for (ECPropertyCP property : ecClass->GetProperties(includeBaseProperties))
        {
        currentProperties.insert(property->GetName().c_str());
        }

    for (Utf8StringCR property : properties)
        {
        if (property.StartsWith("-"))
            mustNotExist.insert(property.substr(1));
        else if (property.StartsWith("+"))
            mustExist.insert(property.substr(1));
        else
            mustExist.insert(property);
        }

    for (Utf8StringCR property : mustNotExist)
        {
        if (currentProperties.find(property) != currentProperties.end())
            {
            ASSERT_FALSE(true) << "Found ECProperty " << property << " in ECClass " << ecClass->GetName() << " whcih must not exist";
            }
        }

    for (Utf8StringCR property : mustExist)
        {
        if (currentProperties.find(property) == currentProperties.end())
            {
            ASSERT_FALSE(true) << "Failed to find ECProperty " << property << " in ECClass " << ecClass->GetName();
            }
        }

    if (strict)
        {
        if (mustExist.size() > currentProperties.size())
            {
            ASSERT_FALSE(true) << "Number of properties in ECClass " << ecClass->GetName() << " is less than list of properties asserted (" << assertExpression;
            }
        if (mustExist.size() < currentProperties.size())
            {
            ASSERT_FALSE(true) << "Number of properties in ECClass " << ecClass->GetName() << " is greater than list of properties asserted (" << assertExpression;
            }
        }
    }

void ExecuteECSQL(ECDbCR ecdb, Utf8CP ecsql, DbResult stepStatus, ECSqlStatus prepareStatus)
    {}

#define ASSERT_PROPERTIES_STRICT(ECDB_OBJ, EXPRESSION)              AssertECProperties(ECDB_OBJ, EXPRESSION, true);
#define ASSERT_PROPERTIES(ECDB_OBJ, EXPRESSION)                     AssertECProperties(ECDB_OBJ, EXPRESSION, false)
#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

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
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
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
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
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
// @bsimethod                                   Affan Khan                          03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ClassModifier)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='Sealed' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='Abstract' >"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='None' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='S4' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Moo' modifier='Sealed' >"
        "       <BaseClass>Boo</BaseClass>"
        "       <ECProperty propertyName='L5' typeName='long' />"
        "       <ECProperty propertyName='S5' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
    Savepoint sp(GetECDb(), "TestData");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"); //Abstract
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (3, 't3')"); //Abstract
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (4, 't4')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (5, 't5')");

    sp.Cancel();

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Voo' modifier='Sealed' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L6' typeName='long' />"
        "       <ECProperty propertyName='S6' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='Sealed' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='S4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (6, 't6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (7, 't7')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (10, 't10')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (11, 't11')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Voo (L6, S6) VALUES (12, 't12')"); //New class added
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (8, 't8')"); //Class deleted
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateECClassModifierToAbstract)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='Sealed' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
    Savepoint sp(GetECDb(), "TestData");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"); //Abstract
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')");
    sp.Cancel();

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='Abstract' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>", false);

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteProperty_OwnTable)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='Abstract' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Delete some properties
    Utf8CP deleteECProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaUpdate(asserted, deleteECProperty, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Deleting ECProperty is not supported as property is mapped to OwnTable");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteECProperty, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Deleting ECProperty is not supported as property is mapped to OwnTable");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteECProperty, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Deleting ECProperty is not supported as property is mapped to OwnTable");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                          03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteProperties_SharedTable)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='None' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='S4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Koo -> L1, S1");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Foo -> L1, L2, S1, S2");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Goo -> L1, L2, L3, S1, S2, S3");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Boo -> L1, L2, L3, L4, S1, S2, S3, S4");

    //Insert a row for each class
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "       <ECProperty propertyName='D1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='D2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='D3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='None' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='D4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Koo -> L1, S1, +D1");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Foo -> L1, L2, S1, -S2, +D1, +D2");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Goo -> L1, L2, L3, S1, -S2, -S3, +D1, +D2, +D3");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Boo -> L1, L2, L3, L4, S1, -S2, -S3, -S4, +D1, +D2, +D3, +D4");

    //see if ECSQL fail for a property which has been deleted
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Ensure new property is null for existing rows
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D1 IS NULL AND D2 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL AND D4 IS NULL");

    //Insert new row with new value
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteProperties_JoinedTable)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='None' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='S4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Koo -> L1, S1");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Foo -> L1, L2, S1, S2");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Goo -> L1, L2, L3, S1, S2, S3");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Boo -> L1, L2, L3, L4, S1, S2, S3, S4");

    //Insert a row for each class
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "       <ECProperty propertyName='D1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='D2' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='D3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='None' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='D4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Koo -> L1, S1, +D1");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Foo -> L1, L2, S1, -S2, +D1, +D2");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Goo -> L1, L2, L3, S1, -S2, -S3, +D1, +D2, +D3");
    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Boo -> L1, L2, L3, L4, S1, -S2, -S3, -S4, +D1, +D2, +D3, +D4");

    //see if ECSQL fail for a property which has been deleted
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Ensure new property is null for existing rows
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D2 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D2 IS NULL AND D3 IS NULL");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D2 IS NULL AND D3 IS NULL AND D4 IS NULL");

    //Insert new row with new value
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddDeleteVirtualColumns)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='Abstract' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Delete and Add some properties
    Utf8CP editedSchemaItem =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='D1' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaItem, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Addition or Deletion of Virtual column is expected to be supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaItem, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Addition or Deletion of Virtual column is expected to be supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaItem, filePath, BeBriefcaseId(123), true, "ClientBriefcase: Addition or Deletion of Virtual column is expected to be supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Foo -> L1, -S1, +D1");

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteOverriddenProperties)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "       <ECProperty propertyName='L1' typeName='long' />"//Overridden Property
        "       <ECProperty propertyName='S1' typeName='string' />"//Overridden Property
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP deleteOverriddenProperty =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None' >"
        "       <BaseClass>Foo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    bool asserted = false;
    AssertSchemaUpdate(asserted, deleteOverriddenProperty, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Deletion of Overridden properties is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteOverriddenProperty, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Deletion of Overridden properties is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteOverriddenProperty, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Deletion of Overridden properties is not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteProperties_SharedTable_ClientBriefCase)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    GetECDb().ChangeBriefcaseId(BeBriefcaseId(123));

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='D2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteProperties_JoinedTable_ClientBriefCase)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='S2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    GetECDb().ChangeBriefcaseId(BeBriefcaseId(123));

    //Delete some properties
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <BaseClass>Koo</BaseClass>"
        "       <ECProperty propertyName='L2' typeName='long' />"
        "       <ECProperty propertyName='D2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), editedSchemaItem);
    ASSERT_FALSE(asserted);
    GetECDb().Schemas().CreateECClassViewsInDb();
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //import edited schema with some changes.
    Utf8CP editedCAProperties =
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
        "                <IsUnique>true</IsUnique>"
        "                <ColumnName>TestProperty1</ColumnName>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedCAProperties, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Modifying CA Properties is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedCAProperties, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Modifying CA Properties is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedCAProperties, filePath, BeBriefcaseId(123), true, "ClientBriefcase: Modifying CA Properties is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
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
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT TestProperty FROM ts_modified.TestClass");

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

        GetECDb().CloseDb();
        }
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Upgrade with some attributes and import schema
    Utf8CP addNewEntityClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, addNewEntityClass, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Adding New Entity Class is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addNewEntityClass, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Adding New Entity Class is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addNewEntityClass, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Db Tables Modifications not allowed");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //verify tables
        //new class should be added with new namespace prefix
        ASSERT_TRUE(GetECDb().TableExists("ts_modified_TestClass"));

        //Verify Schema attributes upgraded successfully
        ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);

        //Verify Newly Added Entity Class exists
        ECClassCP entityClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(entityClass != nullptr);
        ASSERT_TRUE(entityClass->GetDisplayLabel() == "Test Class");
        ASSERT_TRUE(entityClass->GetDescription() == "This is test Class");

        //Verify attributes via ECSql using MataSchema
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Test Class", statement.GetValueText(0));
        ASSERT_STREQ("This is test Class", statement.GetValueText(1));

        //Query newly added Entity Class
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewSubClassMappedToSharedTable)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    GetECDb().ChangeBriefcaseId(BeBriefcaseId(123));

    SchemaItem schemaWithNewSubClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None' >"
        "       <BaseClass>Parent</BaseClass>"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new Class to SharedTable is expected to succeed");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithNewSubClass);
    ASSERT_FALSE(asserted);

    SchemaItem schemaWithNewSubClassWithNewProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None' >"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='Sub1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Adding new column to SharedTable is expected to fail");

    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithNewSubClassWithNewProperty);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewClass_NewProperty_MappedTo_SharedTable_SharedColumns)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None' >"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='Sub1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    GetECDb().ChangeBriefcaseId(BeBriefcaseId(123));

    SchemaItem schemaWithNewSubClassWithProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None' >"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='Sub1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub2' modifier='None' >"
        "       <BaseClass>Sub1</BaseClass>"
        "       <ECProperty propertyName='Sub2' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new Class with new property to SharedTable_SharedColumns is expected to fail");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithNewSubClassWithProperty);
    ASSERT_FALSE(asserted);
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //import edited schema with some changes.
    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='NewTestClass' displayLabel='New Test Class' description='This is New test Class' modifier='None' />"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Adding New Entity Class is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Adding New Entity Class is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Db Tables Modifications not allowed");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

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

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewECDbMapCANotSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Add New ECDbMapCA on ECClass
    Utf8CP addECDbMapCA =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    bool asserted = false;
    AssertSchemaUpdate(asserted, addECDbMapCA, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Adding New ECDbMapCA is supposed to be not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addECDbMapCA, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Adding New ECDbMapCA is supposed to be not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addECDbMapCA, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Adding New ECDbMapCA is supposed to be not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewCA)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Add new CA
    Utf8CP addCAOnClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <DisplayOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <Hidden>True</Hidden>"
        "            </DisplayOptions>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, addCAOnClass, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Adding New CA is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addCAOnClass, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Adding New CA is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addCAOnClass, filePath, BeBriefcaseId(123), true, "ClientBriefcase: Adding New CA is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        //Verify newly added CA
        testClass = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);
        IECInstancePtr bsca = testClass->GetCustomAttribute("DisplayOptions");
        ASSERT_TRUE(bsca != nullptr);

        ECValue val;
        ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "Hidden"));
        ASSERT_TRUE(val.GetBoolean());

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewECProperty)
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, schemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: add new property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: add new property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: add new property should fail");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify newly added property exists
        ECSchemaCP testSchema = GetECDb().Schemas().GetECSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);

        ECClassCP testClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);

        ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
        ASSERT_TRUE(testProperty != nullptr);
        ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
        ASSERT_TRUE(testProperty->GetDescription() == "this is property");

        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DisplayLabel, Description FROM ec.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is property", statement.GetValueText(1));

        //Query newly added Property
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT TestProperty FROM ts.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Add_Delete_ECProperty_SharedColumns)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 7));
    AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumns");

    ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Parent -> P1, P2");

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P3' typeName='int' />"
        "       <ECProperty propertyName='P4' typeName='int' />"
        "       <ECProperty propertyName='P5' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Add Delete New Property to sharedColumns should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Add Delete New Property to sharedColumns should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "ClientsideBriefcase: Add Delete New Property to sharedColumns should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        testItems.clear();
        testItems.push_back(std::make_pair("ts_Parent", 7));
        AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumns");

        ASSERT_PROPERTIES_STRICT(GetECDb(), "TestSchema:Parent -> P1, -P2, +P3, +P4, +P5");

        //Verify insert
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Parent(P1, P3, P4, P5) VALUES(1, 2, 3, 4)");

        GetECDb().CloseDb();
        }
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //import edited schema with some changes.
    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "       </ECProperty>"
        "       <ECProperty propertyName='NewTestProperty' displayLabel='New Test Property' description='this is new property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: add new property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: add new property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: Db Tables Modifications not allowed");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

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

        GetECDb().CloseDb();
        }
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //import edited schema with some changes.
    Utf8CP editedSchemaXml =
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
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: New CA on Property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: New CA on Property should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: New CA on Property should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

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

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateECDbMapCA_AddMinimumSharedColumnsCount)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Update ECDbMapCA add MinimumSharedColumnCount is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Update ECDbMapCA add MinimumSharedColumnCount is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "ClientsideBriefcase: Db modifications are not supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify number of columns
        std::vector<std::pair<Utf8String, int>> testItems;
        testItems.push_back(std::make_pair("ts_Parent", 8));
        AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumns");

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MinimumSharedColumnsCountForSubClasses_AddProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
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
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: Add new property to sharedColumns in SharedTable should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: Add new property to sharedColumns in SharedTable should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: Add new property to sharedColumns in SharedTable should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify number of columns after upgrade
        testItems.clear();
        testItems.push_back(std::make_pair("ts_Parent", 8));
        AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsForSubClasses");

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MinimumSharedColumnsCountWithJoinedTable_AddProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP addPropertiesToSharedColumns =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, addPropertiesToSharedColumns, filePath, BeBriefcaseId(0), true, "Master Briefcase: Add new property to sharedColumns in a JoinedTable should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addPropertiesToSharedColumns, filePath, BeBriefcaseId(1), true, "standalone Briefcase: Add new property to sharedColumns in a JoinedTable should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, addPropertiesToSharedColumns, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: Add new property to sharedColumns in a JoinedTable should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify number of columns after upgrade
        testItems.clear();
        testItems.push_back(std::make_pair("ts_Parent", 3));
        testItems.push_back(std::make_pair("ts_Sub1", 7));
        AssertColumnCount(GetECDb(), testItems, "MinimumSharedColumnsWithJoinedTable");

        GetECDb().CloseDb();
        }
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
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, MajorVersionChange_WithoutMajorVersionIncremented)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem MajorVersionChange(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Major Version change without Major Version incremented is expected to be not supported");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), MajorVersionChange);
    ASSERT_FALSE(asserted);

    SchemaItem decrementedMajorVersion(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", false, "Schema Update with ECSchema Major Version decremented is expected to be not supported");

    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), decrementedMajorVersion);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_ECDbMapCANotSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //Delete ECDbMap CA
    Utf8CP DeleteECDbMapCA =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";
    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, DeleteECDbMapCA, filePath, BeBriefcaseId(0), false, "Master Briefcase: Deleting ECDbMap CustomAttribute Not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, DeleteECDbMapCA, filePath, BeBriefcaseId(1), false, "standalone Briefcase: Deleting ECDbMap CustomAttribute Not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, DeleteECDbMapCA, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: Deleting ECDbMap CustomAttribute Not supported");
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

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(S,D,L) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(S,D,L) VALUES ('test4', 13.3, 2345)");

    ASSERT_TRUE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Delete Foo ===================================================================================================
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT S, D, L FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT S, D, L FROM ts.Goo");

    //Add Foo Again===============================================================================================
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT S, D, L FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT S, D, L FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)");
    }

/*********************************************************************Example Scenario******************************************************************************************

-Parent(JoinedTable)   ->    Parent(JoinedTable)     ->      Parent(JoinedTable) -> *DeletedParent* -> Parent(JoinedTable)   ->   Parent(JoinedTable)   ->    Parent(JoinedTable)
---|-----------------------------|--------------------------------------------------------------------------------------------------|-----------------------------|--------------
--Goo---------------------------Goo------------------------------------------------------------------------------------------------Goo---------------------------Goo-------------
---|--------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------
--Foo------------------------------------------------------------------------------------------------------------------------------------------------------------Foo-------------

********************************************************************************************************************************************************************************/

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_SharedTable)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Goo", 8));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_AppliedToSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
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

    //Verify number of columns should not change as we don't delete columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 8));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_AppliedToSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class with ECDbMap CA is expected to be supported
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Deleting Class with ECDbMap CA is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);
    GetECDb().SaveChanges();
    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Add Goo Again===================================================================================================
    //Adding new class with ECDbMapCA applied on it is expected to be supported
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with ECDbMap CA Should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify Number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 5));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_AppliedToSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new derived class should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //should exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //following should not exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 8));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_AppliedToSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_SharedTable_SharedColumns)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //following table should exist.
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Following table should not exist
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete derived class should be successfull");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class with SharedTable:SharedColumns is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Delete class containing ECDbMap CA should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Add Goo Again===================================================================================================
    //Add Class with SharedTable:SharedColumns is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with ECDbMap CA is expected to be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify Column count
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 5));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    //Adding new derived entity class is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "New derived entity class is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //Table should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //Table should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify column count
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_SharedTable_SharedColumns_MinimumSharedColumn)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //following table should exist.
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Following table should not exist
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns_minimumSharedColumn");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete derived class should be successful");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns_MinimumSharedColumn");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class with SharedTable_SharedColumns_minimumSharedColumn is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Delete class containing ECDbMap CA should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Add Goo Again===================================================================================================
    //Add Class with SharedTable_SharedColumns_minimumSharedColumn is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with ECDbMap CA (SharedTable_SharedColumns_minimumSharedColumn) is expected to be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify Column count
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns_minimumSharedColumn");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    //Adding new derived entity class is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "       <ECProperty propertyName='FI1' typeName='int' />"//Extra column to verify that sharedcolumn count should be incremented
        "   </ECEntityClass>"
        "</ECSchema>", true, "New derived entity class is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //Table should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //Table should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //Verify column count
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Goo", 10));
    AssertColumnCount(GetECDb(), testItems, "SharedTable_SharedColumns_minimumSharedColumn");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test1', 1.3, 334, 1, 11)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test2', 23.3, 234, 2, 22)");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_JoinedTable)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Following Table should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete a class should be successful");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify Number of columns
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete Derived ECClass is supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    //Delete Parent ===================================================================================================
    //Deleting Class with CA  JoinedTablePerDirectSubClass is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Deleting Class with CA  JoinedTablePerDirectSubClass is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteParent);
    ASSERT_FALSE(asserted);

    //Parent should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem AddParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding New class containing ECDbMap CA JoinedTablePerDirectSubClass should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), AddParent);
    ASSERT_FALSE(asserted);

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinecTablePerDirectSubClass on base class is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='6.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with JoinedTablePerDirectSubClass on Parent is expected to be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //verify number of columns
    testItems.push_back(std::make_pair("ts_Goo", 5));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    GetECDb().SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='7.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new derived Entity class is supported now");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //Table should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_JoinedTable_SharedColumns)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Following Table should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete a class should be successful");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify Number of columns
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete Derived ECClass is supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    //Delete Parent ===================================================================================================
    //Deleting Class with CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Deleting Class with CA  JoinedTablePerDirectSubClass,SharedColumnForSubClasses is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteParent);
    ASSERT_FALSE(asserted);

    //Parent should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem AddParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding New class with ECDbMap CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), AddParent);
    ASSERT_FALSE(asserted);

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses on base class is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='6.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with JoinedTablePerDirectSubClass,SharedColumnForSubClasses on Parent is expected to be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //verify number of columns
    testItems.push_back(std::make_pair("ts_Goo", 5));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    GetECDb().SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='7.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new derived Entity class is supported now");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //Table should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubClass,SharedColumnForSubClasses");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, Delete_Add_ECEntityClass_MappedTo_JoinedTable_MinimumSharedColumnCount)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    //Following Table should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Verify number of columns
    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete a class should be successful");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteFoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_EQ(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify Number of columns
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Delete Derived ECClass is supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteGoo);
    ASSERT_FALSE(asserted);

    //Following should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    //Delete Parent ===================================================================================================
    //Deleting Class with CA  JoinedTablePerDirectSubclass_MinimumSharedColumnCount is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>", true, "Deleting Class with CA  JoinedTablePerDirectSubclass_MinimumSharedColumnCount is expected to be supported");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteParent);
    ASSERT_FALSE(asserted);

    //Parent should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    GetECDb().SaveChanges();
    SchemaItem AddParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding New class containing ECDbMap CA JoinedTablePerDirectSubclass_MinimumSharedColumnCount should be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), AddParent);
    ASSERT_FALSE(asserted);

    //Parent should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //Verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinedTablePerDirectSubclass_MinimumSharedColumnCount on base class is expected to be supported
    GetECDb().SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='6.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>", true, "Add New Class with JoinedTablePerDirectSubclass_MinimumSharedColumnCount on Parent is expected to be successful");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addGoo);
    ASSERT_FALSE(asserted);

    //Following should exist
    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    //verify number of columns
    testItems.push_back(std::make_pair("ts_Goo", 9));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    GetECDb().SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='7.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>7</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "       <ECProperty propertyName='FI1' typeName='int' />"//Extra column to verify that sharedcolumn count should be incremented
        "   </ECEntityClass>"
        "</ECSchema>", true, "Adding new derived Entity class is supported now");
    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), addFoo);
    ASSERT_FALSE(asserted);

    //Table should not exist
    ASSERT_FALSE(GetECDb().TableExists("ts_Foo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetECDb().TableExists("ts_Goo"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetECDb().TableExists("ts_Parent"));
    ASSERT_NE(GetECDb().Schemas().GetECClass("TestSchema", "Parent"), nullptr);

    //verify number of columns
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Goo", 10));
    AssertColumnCount(GetECDb(), testItems, "JoinedTablePerDirectSubclass_MinimumSharedColumnCount");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL, FI, FI1 FROM ts.Foo");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test1', 1.3, 334, 1, 11)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test2', 23.3, 234, 2, 22)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteEntityClassPartOfRelationshipConstraint)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP schemaWithDeletedConstraintClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    bool asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(0), true, "MasterBriefcase: ConstraintClass can be deleted unless it's the last relationship constraint");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: ConstraintClass can be deleted unless it's the last relationship constraint");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(123), true, "ClientsideBriefcase: ConstraintClass can be deleted unless it's the last relationship constraint");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteConcreteImplementationOfAbstractConstraintClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propC' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='D' modifier='None' >"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='propD' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    ECClassCP classB = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("B");
    ECClassCP classD = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("D");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.B(ECInstanceId, propA, propB) VALUES(1, 11, 22)");
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.D(ECInstanceId, propC, propD) VALUES(2, 33, 44)");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ts.RelClass(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(101, 1, %llu, 2, %llu)", classB->GetId().GetValue(), classD->GetId().GetValue());
    //Insert Relationship instance
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

    //Verify Insertion
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT * FROM ts.RelClass");

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP schemaWithDeletedConstraintClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propC' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";
    bool asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Only Direct relationship constraint classes can't be deleted");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Only Direct relationship constraint classes can't be deleted");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithDeletedConstraintClass, filePath, BeBriefcaseId(123), true, "ClientsideBriefcase: Only Direct relationship constraint classes can't be deleted");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify relationship Instance should be deleted along with deletion of constaint class
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT * FROM ts.RelClass");

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteECRelationships)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' modifier='None'>"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='EndTableRelationship' modifier='Sealed' strength='Embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='Foo' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Roo' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='LinkTableRelationship' modifier='Sealed' strength='referencing' strengthDirection='forward' >"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Foo' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Roo' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP relationshipWithForeignKeyMapping =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' modifier='None'>"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='LinkTableRelationship' modifier='Sealed' strength='referencing' strengthDirection='forward' >"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Foo' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Roo' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    bool asserted = false;
    AssertSchemaUpdate(asserted, relationshipWithForeignKeyMapping, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Deleting ECRelationship with ForeignKey Mapping is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, relationshipWithForeignKeyMapping, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Deleting ECRelationship with ForeignKey Mapping is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, relationshipWithForeignKeyMapping, filePath, BeBriefcaseId(123), false, "ClientsideBriefcase: Deleting ECRelationship with ForeignKey Mapping is supported");
    ASSERT_FALSE(asserted);

    Utf8CP linkTableECRelationship =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' modifier='None'>"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='EndTableRelationship' modifier='Sealed' strength='Embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='Foo' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='Roo' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    asserted = false;
    AssertSchemaUpdate(asserted, linkTableECRelationship, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Deletion of LinkTable mapped relationship is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, linkTableECRelationship, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Deletion of LinkTable mapped relationship is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, linkTableECRelationship, filePath, BeBriefcaseId(123), false, "ClientsideBriefcase: Sql Db changes are not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteECStructClassUnSupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem deleteStructECClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>", false, "Deleting ECStructClass is expected to be not supported");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), deleteStructECClass);
    ASSERT_FALSE(asserted);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, UpdateECDbMapCA_DbIndexChanges)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>IDX_Partial_Index</Name>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Properties>"
        "                           <string>AId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "               </Indexes>"
        "           </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    ECClassCP b = GetECDb().Schemas().GetECClass("TestSchema", "B");
    ASSERT_NE(b, nullptr);
    IECInstancePtr ca = b->GetCustomAttribute("ClassMap");
    ASSERT_FALSE(ca.IsNull());

    ECValue indexes, indexName;
    ASSERT_EQ(ca->GetValue(indexes, "Indexes", 0), ECObjectsStatus::Success);
    ASSERT_EQ(indexes.GetStruct()->GetValue(indexName, "Name"), ECObjectsStatus::Success);
    ASSERT_STREQ(indexName.GetUtf8CP(), "IDX_Partial_Index");

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), "SELECT NULL FROM ec_Index WHERE Name='IDX_Partial_Index'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    stmt.Finalize();

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP schemaWithIndexNameModified =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>IDX_Partial</Name>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Properties>"
        "                           <string>AId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "               </Indexes>"
        "           </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexNameModified, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Updating ECDbMapCA, modifying index Name is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexNameModified, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Updating ECDbMapCA, modifying index Name is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexNameModified, filePath, BeBriefcaseId(123), true, "ClientsideBriefcase: Updating ECDbMapCA, modifying index Name is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECClassCP b = GetECDb().Schemas().GetECClass("TestSchema", "B");
        ASSERT_NE(b, nullptr);
        IECInstancePtr ca = b->GetCustomAttribute("ClassMap");
        ASSERT_FALSE(ca.IsNull());

        ECValue indexes, indexName;
        ASSERT_EQ(ca->GetValue(indexes, "Indexes", 0), ECObjectsStatus::Success);
        ASSERT_EQ(indexes.GetStruct()->GetValue(indexName, "Name"), ECObjectsStatus::Success);
        ASSERT_STREQ(indexName.GetUtf8CP(), "IDX_Partial");

        //verify entry updated in ec_Index table
        Statement statement;
        ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(GetECDb(), "SELECT NULL FROM ec_Index WHERE Name='IDX_Partial'"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        statement.Finalize();
        GetECDb().CloseDb();
        }

    Utf8CP schemaWithIndexDeleted =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "           </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexDeleted, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Updating ECDbMapCA, deleting DbIndexes is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexDeleted, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Updating ECDbMapCA, deleting DbIndexes is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, schemaWithIndexDeleted, filePath, BeBriefcaseId(123), false, "ClientsideBriefcase: Updating ECDbMapCA, deleting DbIndexes is not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteNavigationProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Embedding' modifier='Sealed'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem schemaWithDeletedKeyProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Embedding' modifier='Sealed'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", false, "can't delete a property used as keyProperty.");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), schemaWithDeletedKeyProperty);
    ASSERT_FALSE(asserted);
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
// @bsimethod                                   Affan Khan                     04/16
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='referencing' strengthDirection='forward' >"
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
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='backward' >"
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
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyRelationshipConstrainsRoleLabel)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem modifySourceRoleLabel(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C Modified' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", true, "Modifying Source constraint class RoleLabel is expected to be successfull");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifySourceRoleLabel);
    ASSERT_FALSE(asserted);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem modifyTargetRoleLabel(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C Modified' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A Modified' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", true, "Modifying Target Constraint class Role Label is expected to be successfull");

    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifyTargetRoleLabel);
    ASSERT_FALSE(asserted);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem modifyBothRoleLabels(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "       <BaseClass>A</BaseClass>"
        "       <ECProperty propertyName='propB' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has B and C' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='B and C belong to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", true, "Modifying both source and target class RoleLabels simultaneously is expected to be successfull");

    asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifyBothRoleLabels);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyECProperties)
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP modifiedECPropertyType =
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
        "</ECSchema>";
    bool asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyType, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECProperty typeName is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyType, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECProperty typeName is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyType, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECProperty typeName is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedECStructPropertyType =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructPropertyType, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECStructProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructPropertyType, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECStructProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructPropertyType, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECStructProperty is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedECStructArrayPropertyType =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructArrayPropertyType, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECStructArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructArrayPropertyType, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECStructArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECStructArrayPropertyType, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECStructArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedPrimitiveArrayType =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveArrayType, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveArrayType, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveArrayType, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECArrayProperty is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedPrimitiveType =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveType, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying IsPrimitiveType is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveType, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying IsPrimitiveType is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedPrimitiveType, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying IsPrimitiveType is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedECPropertyArrayMixOccurs =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyArrayMixOccurs, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECPropertyArray minOccurs is not Supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyArrayMixOccurs, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECPropertyArray minOccurs is not Supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECPropertyArrayMixOccurs, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECPropertyArray minOccurs is not Supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedECArrayPropertyMaxOccurs =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECArrayPropertyMaxOccurs, filePath, BeBriefcaseId(0), false, "MasterBriefcase: Modifying ECArrayProperty maxOccuers is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECArrayPropertyMaxOccurs, filePath, BeBriefcaseId(1), false, "StandaloneBriefcase: Modifying ECArrayProperty maxOccuers is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedECArrayPropertyMaxOccurs, filePath, BeBriefcaseId(123), false, "ClientBriefcase: Modifying ECArrayProperty maxOccuers is not supported");
    ASSERT_FALSE(asserted);

    Utf8CP modifiedExtendedType =
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
        "</ECSchema>";
    asserted = false;
    AssertSchemaUpdate(asserted, modifiedExtendedType, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Modifying extendedTypeName is expected to be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedExtendedType, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Modifying extendedTypeName is expected to be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, modifiedExtendedType, filePath, BeBriefcaseId(123), true, "ClientBriefcase: Modifying extendedTypeName is expected to be successful");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyNavigationProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='Embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    SchemaItem modifiedRelNameInNavProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='A1'>"
        "        <ECProperty propertyName='PA1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='PB' typeName='int' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='A1HasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='A1HasB' modifier='Sealed' strength='embedding'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A1' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", false, "Changing relationship Class Name for a Navigation property is not supported");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), modifiedRelNameInNavProperty);
    ASSERT_FALSE(asserted);
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

    //Insert should be successfull
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val1', 'P2_Val1')");

    //readonly property can't be updated
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P1='P1_Val1new'");

    //skipping readonly Property, Update should be successful.
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P2='P2_Val1new' WHERE P2='P2_Val1'");

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
    ASSERT_FALSE(asserted);

    /*-------------------After 2nd Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadWrite
    P2            -> ReadOnly
    */

    //Verify Insert
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW2', 'P1_Val2', 'P2_Val2')");

    //Verify Update
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P2='P2_Val2new'");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P1='P1_Val2new' WHERE P1='P1_Val2'");

    //Verify Select
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P2 FROM ts.TestClass WHERE ReadWriteProp='RW1new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P2_Val1new", statement.GetValueText(0));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW2new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val2new", statement.GetValueText(0));

    //Verify Delete
    Savepoint sp(GetECDb(), "To Revert Delete Operation");

    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "DELETE FROM ts.TestClass");

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

    //Insert should be successfull
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val3', 'P2_Val3')");

    //verify update
    //Update Prepare should fail for ReadOnlyProp
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P1='P1_Val3new'");
    //skipping readonly Property Update should be successful.
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P2='P2_Val3new' WHERE P1 = 'P1_Val3'");

    //Verify Select
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW3new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val3", statement.GetValueText(0));

    //Verify Delete
    ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "DELETE FROM ts.TestClass");

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT COUNT(*) FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropToReadOnlyOnClientBriefcase)
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

    GetECDb().ChangeBriefcaseId(BeBriefcaseId(123));

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
    ASSERT_FALSE(asserted);
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

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP changeCAPropertyValues =
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
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, changeCAPropertyValues, filePath, BeBriefcaseId(0), true, "Master Briefcase: Modifying CA Properties values is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, changeCAPropertyValues, filePath, BeBriefcaseId(1), true, "standalone Briefcase: Modifying CA Properties values is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, changeCAPropertyValues, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: Modifying CA Properties values is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), "Select ec_CustomAttribute.[Instance] from ec_Property  INNER JOIN ec_CustomAttribute ON ec_CustomAttribute.[ContainerId] = ec_Property.[Id] Where ec_Property.[Name] = 'TestProperty'"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("<TestCA xmlns=\"TestSchema.01.00\">\n    <BinaryProp>10100011</BinaryProp>\n    <BooleanProp>False</BooleanProp>\n    <DateTimeProp>20160510</DateTimeProp>\n    <DoubleProp>2.0001000000000002</DoubleProp>\n    <IntegerProp>20</IntegerProp>\n    <LongProp>2000000</LongProp>\n    <Point2DProp>4,5.5</Point2DProp>\n    <Point3DProp>35.5,45.5,55.5</Point3DProp>\n    <StringProp>'This is Modified String Property'</StringProp>\n</TestCA>\n", stmt.GetValueText(0));
        stmt.Finalize();

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteECCustomAttributeClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty,EntityClass' />"
        "</ECSchema>");
    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP deleteECCustomAttribute =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, deleteECCustomAttribute, filePath, BeBriefcaseId(0), false, "Master Briefcase: Deleting a ECCustomAttributeClass is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteECCustomAttribute, filePath, BeBriefcaseId(1), false, "standalone Briefcase: Deleting a ECCustomAttributeClass is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteECCustomAttribute, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: Deleting a ECCustomAttributeClass is not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteCustomAttribute)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECCustomAttributes>"
        "       <TreeColorCustomAttributes xmlns = 'Bentley_Standard_CustomAttributes.01.13' >"
        "           <NodeBackColor>Black</NodeBackColor>"
        "       </TreeColorCustomAttributes>"
        "   </ECCustomAttributes>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <SearchOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <SearchPolymorphically>true</SearchPolymorphically>"
        "            </SearchOptions>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "        <ECCustomAttributes>"
        "            <DisplayOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <Hidden>True</Hidden>"
        "            </DisplayOptions>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    ECDb ecdb;
    AssertSchemaImport(ecdb, asserted, schemaItem, "deleteca.ecdb");
    ASSERT_FALSE(asserted);

    //Delete CA from Schema
    ecdb.SaveChanges();
    SchemaItem deleteCAFromSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <SearchOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <SearchPolymorphically>true</SearchPolymorphically>"
        "            </SearchOptions>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "        <ECCustomAttributes>"
        "            <DisplayOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <Hidden>True</Hidden>"
        "            </DisplayOptions>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, deleteCAFromSchema);
    ASSERT_FALSE(asserted);
    IECInstancePtr schemaCA = ecdb.Schemas().GetECSchema("TestSchema")->GetCustomAttribute("TreeColorCustomAttributes");
    ASSERT_TRUE(schemaCA == nullptr);

    //Delete CA from Class
    ecdb.SaveChanges();
    SchemaItem deleteCAFromClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "        <ECCustomAttributes>"
        "            <DisplayOptions xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "                <Hidden>True</Hidden>"
        "            </DisplayOptions>"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, deleteCAFromClass);
    ASSERT_FALSE(asserted);
    IECInstancePtr classCA = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("TestClass")->GetCustomAttribute("TreeColorCustomAttributes");
    ASSERT_TRUE(classCA == nullptr);

    //Delete CA from property
    ecdb.SaveChanges();
    SchemaItem deleteCAFromProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, deleteCAFromProperty);
    ASSERT_FALSE(asserted);
    IECInstancePtr propertyCA = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("prop")->GetCustomAttribute("TreeColorCustomAttributes");
    ASSERT_TRUE(propertyCA == nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ChangesToExisitingTable)
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, WString("existingTableUpdate.ecdb", BentleyCharEncoding::Utf8).c_str()));
    // Create existing table and insert rows
    ecdb.ExecuteSql("CREATE TABLE test_Employee(Id INTEGER PRIMARY KEY, Name TEXT);");
    ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (101, 'Affan Khan');");
    ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (201, 'Muhammad Hassan');");
    ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (301, 'Krischan Eberle');");
    ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (401, 'Casey Mullen');");

    // Map ECSchema to exisitng table
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap'/>"
        "   <ECEntityClass typeName='Employee' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>ExistingTable</MapStrategy>"
        "               <TableName>test_Employee</TableName>"
        "               <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
        "           </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='Name' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, schemaItem);
    ASSERT_FALSE(asserted);

    // Modify and add new existing table and relate it to another one
    ecdb.ExecuteSql("CREATE TABLE test_Title(Id INTEGER PRIMARY KEY, Name TEXT);");
    ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (1, 'Senior Software Engineer');");
    ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (2, 'Software Quality Analyst');");
    ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (3, 'Advisory Software Engineer');");
    ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (4, 'Distinguished Architect');");
    ecdb.ExecuteSql("ALTER TABLE test_Employee ADD COLUMN TitleId INTEGER REFERENCES test_Title(Id);");
    ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 1 WHERE Id = 101");
    ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 2 WHERE Id = 201");
    ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 3 WHERE Id = 301");
    ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 4 WHERE Id = 401");

    // Map ECSchema to exisitng table
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap'/>"
        "   <ECEntityClass typeName='Employee' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>ExistingTable</MapStrategy>"
        "               <TableName>test_Employee</TableName>"
        "               <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
        "           </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='Name' typeName='string' >"
        "       </ECProperty>"
        "       <ECNavigationProperty propertyName='TitleId' relationshipName='EmployeeHasTitle' direction='forward'/>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Title' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>ExistingTable</MapStrategy>"
        "               <TableName>test_Title</TableName>"
        "               <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
        "           </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='Name' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='EmployeeHasTitle' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
        "        <Source cardinality='(0,N)' polymorphic='false'>"
        "            <Class class='Employee'/>"
        "        </Source>"
        "        <Target cardinality='(0,1)' polymorphic='false'>"
        "            <Class class='Title'/>"
        "        </Target>"
        "    </ECRelationshipClass>'"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, editedSchemaItem);
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    stmt.Prepare(ecdb, "SELECT E.Name EmployeeName, T.Name EmployeeTitle FROM ts.Employee E JOIN ts.Title T USING ts.EmployeeHasTitle ORDER BY E.ECInstanceId");

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Affan Khan", stmt.GetValueText(0));
    ASSERT_STREQ("Senior Software Engineer", stmt.GetValueText(1));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Muhammad Hassan", stmt.GetValueText(0));
    ASSERT_STREQ("Software Quality Analyst", stmt.GetValueText(1));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Krischan Eberle", stmt.GetValueText(0));
    ASSERT_STREQ("Advisory Software Engineer", stmt.GetValueText(1));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Casey Mullen", stmt.GetValueText(0));
    ASSERT_STREQ("Distinguished Architect", stmt.GetValueText(1));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, DeleteCAInstanceWithoutProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.13' prefix = 'bsca' />"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty,EntityClass'>"
        "   </ECCustomAttributeClass>"
        "   <ECCustomAttributes>"
        "       <SystemSchema xmlns = 'Bentley_Standard_CustomAttributes.01.13' >"
        "       </SystemSchema>"
        "   </ECCustomAttributes>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='prop' typeName='boolean' >"
        "        <ECCustomAttributes>"
        "            <TestCA xmlns='TestSchema.01.00'>"
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("deletecainstancewithoutproperty.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP deleteAllCA =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty,EntityClass'>"
        "   </ECCustomAttributeClass>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, deleteAllCA, filePath, BeBriefcaseId(0), true, "Master Briefcase: Deleting CA without Properties are expected to be supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteAllCA, filePath, BeBriefcaseId(1), true, "standalone Briefcase: Deleting CA without Properties are expected to be supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, deleteAllCA, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: Deleting CA without Properties are expected to be supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECSchemaCP ecSchema = GetECDb().Schemas().GetECSchema("TestSchema");
        ECClassCP testClass = ecSchema->GetClassCP("TestClass");
        ECPropertyP ecProperty = testClass->GetPropertyP("prop");

        IECInstancePtr systemSchemaCA = ecSchema->GetCustomAttribute("SystemSchema");
        ASSERT_TRUE(systemSchemaCA == nullptr);

        IECInstancePtr classCA = testClass->GetCustomAttribute("TestCA");
        ASSERT_TRUE(classCA == nullptr);

        IECInstancePtr propertyCA = ecProperty->GetCustomAttribute("TestCA");
        ASSERT_TRUE(propertyCA == nullptr);

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddKoQAndUpdatePropertiesWithKoQ)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Length' typeName='double' />"
        "    <ECProperty propertyName='Homepage' typeName='string' />"
        "    <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' />"
        "    <ECArrayProperty propertyName='Favorites' typeName='string'  minOccurs='0' maxOccurs='unbounded' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='My KindOfQuantity'"
        "                    displayLabel='My KindOfQuantity' persistenceUnit='CENTIMETRE' precision='10'"
        "                    defaultPresentationUnit='FOOT' alternativePresentationUnits='INCH;YARD' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='MyKindOfQuantity' />" // kindOfQuantity='s1:MyKindOfQuantity'
        "        <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />" // extendedTypeName='URL'
        "        <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'MyKindOfQuantity'/>" // kindOfQuantity='s1:MyKindOfQuantity'
        "        <ECArrayProperty propertyName='Favorites' typeName='string' extendedTypeName='URL' minOccurs='0' maxOccurs='unbounded' />" // extendedTypeName='URL'
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: AddKoQAndUpdatePropertiesWithKoQ is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: AddKoQAndUpdatePropertiesWithKoQ is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: AddKoQAndUpdatePropertiesWithKoQ is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        KindOfQuantityCP myKindOfQuantity = GetECDb().Schemas().GetKindOfQuantity("TestSchema", "MyKindOfQuantity");
        ASSERT_TRUE(myKindOfQuantity != nullptr);
        ECClassCP foo = GetECDb().Schemas().GetECClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_length = foo->GetPropertyP("Length")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_length != nullptr);
        ASSERT_TRUE(foo_length->GetKindOfQuantity() == myKindOfQuantity);

        PrimitiveECPropertyCP foo_homepage = foo->GetPropertyP("Homepage")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_homepage != nullptr);
        ASSERT_TRUE(foo_homepage->GetExtendedTypeName() == "URL");

        ArrayECPropertyCP foo_alternativeLengths = foo->GetPropertyP("AlternativeLengths")->GetAsArrayProperty();
        ASSERT_TRUE(foo_alternativeLengths != nullptr);
        ASSERT_TRUE(foo_alternativeLengths->GetKindOfQuantity() == myKindOfQuantity);

        ArrayECPropertyCP foo_favorites = foo->GetPropertyP("Favorites")->GetAsArrayProperty();
        ASSERT_TRUE(foo_favorites != nullptr);
        ASSERT_TRUE(foo_favorites->GetExtendedTypeName() == "URL");

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropertyType_PrimitiveToNonStrictEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='NonStrictEnum' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: changing primitive to NonString Enum is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: changing primitive to NonString Enum is supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: changing primitive to NonString Enum is supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP nonStrictEnum = GetECDb().Schemas().GetECEnumeration("TestSchema", "NonStrictEnum");
        ASSERT_TRUE(nonStrictEnum != nullptr);
        ECClassCP foo = GetECDb().Schemas().GetECClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nonStrictEnum);

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropertyType_PrimitiveToStrictEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), false, "Master Briefcase: changing primitive to Strict Enum is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), false, "standalone Briefcase: changing primitive to Strict Enum is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: changing primitive to Strict Enum is not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropertyType_PrimitiveToPrimitive)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), false, "Master Briefcase: changing primitive to another primitive is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), false, "standalone Briefcase: changing primitive to another primitive is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: changing primitive to another primitive is not supported");
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropertyType_EnumToPrimitive)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: changing Enum to primitive should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: changing Enum to primitive should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: changing Enum to primitive should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = GetECDb().Schemas().GetECEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECClassCP foo = GetECDb().Schemas().GetECClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nullptr);
        ASSERT_TRUE(foo_type->GetType() == PrimitiveType::PRIMITIVETYPE_Integer);

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, ModifyPropertyType_EnumToEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEnumeration typeName='UnStrictEnum' backingTypeName='int' isStrict='False'>" // NonStrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Goo' >"
        "    <ECProperty propertyName='Type' typeName='UnStrictEnum' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEnumeration typeName='UnStrictEnum' backingTypeName='int' isStrict='False'>" // NonStrictEnum
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='UnStrictEnum' />"
        "    </ECEntityClass>"
        "  <ECEntityClass typeName='Goo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "  </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: changing Enum to Enum should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: changing Enum to Enum should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: changing Enum to Enum should be successful");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = GetECDb().Schemas().GetECEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECEnumerationCP unstrictEnum = GetECDb().Schemas().GetECEnumeration("TestSchema", "UnStrictEnum");
        ASSERT_TRUE(unstrictEnum != nullptr);

        ECClassCP foo = GetECDb().Schemas().GetECClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);
        ECClassCP goo = GetECDb().Schemas().GetECClass("TestSchema", "Goo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == unstrictEnum);

        PrimitiveECPropertyCP goo_type = goo->GetPropertyP("Type")->GetAsPrimitiveProperty();
        ASSERT_TRUE(goo_type != nullptr);
        ASSERT_TRUE(goo_type->GetEnumeration() == strictEnum);

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, RemoveExistingEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />" // NonStrictEnum
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), false, "Master Briefcase: Deleting Enum is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), false, "standalone Briefcase: Deleting Enum is not supported");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: Deleting Enum is not supported");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = GetECDb().Schemas().GetECEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECClassCP foo = GetECDb().Schemas().GetECClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == strictEnum);

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewRelationship)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "    <ECProperty propertyName='AProp' typeName='int' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "    <ECProperty propertyName='BProp' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "    <ECProperty propertyName='AProp' typeName='int' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "    <ECProperty propertyName='BProp' typeName='int' />"
        "  </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    //verify Adding new EndTable relationship for different briefcaseIds.
    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: add new endtable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: add new endtable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: add new endtable relationship should fail");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        std::vector<std::pair<Utf8String, int>> testItems;
        testItems.push_back(std::make_pair("ts_A", 3));
        testItems.push_back(std::make_pair("ts_B", 2));
        AssertColumnCount(GetECDb(), testItems, "");

        GetECDb().CloseDb();
        }

    editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "    <ECProperty propertyName='AProp' typeName='int' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "    <ECProperty propertyName='BProp' typeName='int' />"
        "  </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='referencing' strengthDirection='forward' >"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    //verify Adding new linkTable relationship for different briefcaseIds.
    m_updatedDbs.clear();
    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: add new LinkTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: add new LinkTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), false, "clientside BriefcaseId: add new LinkTable relationship should fail");
    ASSERT_FALSE(asserted);

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ASSERT_EQ(true, GetECDb().TableExists("ts_RelClass"));

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewDerivedEndTableRelationship)
    {
    SchemaItem schemaItem(
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' modifier='Abstract' >"
        "    <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometricElement' modifier='None'>"
        "    <BaseClass>Element</BaseClass>"
        "    <ECProperty propertyName='GeometricElement' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Geometric3dElement' modifier='None'>"
        "    <BaseClass>GeometricElement</BaseClass>"
        "    <ECProperty propertyName='Geometry3d' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' modifier='Abstract' >"
        "    <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometricElement' modifier='None'>"
        "    <BaseClass>Element</BaseClass>"
        "    <ECProperty propertyName='GeometricElement' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Geometric3dElement' modifier='None'>"
        "    <BaseClass>GeometricElement</BaseClass>"
        "    <ECProperty propertyName='Geometry3d' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='ModelHasGeometricElements' strength='embedding' modifier='Sealed'>"
        "   <BaseClass>ModelHasElements</BaseClass>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='GeometricElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    //verify Adding new derived endtable relationship for different briefcaseIds.
    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "MasterBriefcase: Add new Derived EndTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "StandaloneBriefcase: Add new Derived EndTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "ClientsideBriefcase: Add new Derived EndTable relationship should succeed as it doens't change db schema");
    ASSERT_FALSE(asserted);

    ECClassId modelHasGeometricElementsRelClassId = GetECDb().Schemas().GetECClassId("TestSchema", "ModelHasGeometricElements");
    ASSERT_TRUE(modelHasGeometricElementsRelClassId.IsValid());

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        GetECDb().Schemas().CreateECClassViewsInDb();
        //Insert Test Data
        //Model
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(101, 'Model1')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(102, 'Model2')");

        //GeometricElement
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.GeometricElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement) VALUES(201, 'Code1', 101, %s, 'GeometricElement1')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());
        ecsql.Sprintf("INSERT INTO ts.GeometricElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement) VALUES(202, 'Code2', 101, %s, 'GeometricElement2')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Geometric3dElement
        ecsql.Sprintf("INSERT INTO ts.Geometric3dElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement, Geometry3d) VALUES(301, 'Code3', 102, %s, 'GeometricElement3', 'Geometry3d3')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());
        ecsql.Sprintf("INSERT INTO ts.Geometric3dElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement, Geometry3d) VALUES(302, 'Code4', 102, %s, 'GeometricElement4', 'Geometry3d4')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Select statements
        //Verify insertions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.ModelHasElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(8, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ONLY ts.ModelHasElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.ModelHasGeometricElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();

        GetECDb().CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSchemaUpdateTests, AddNewDerivedLinkTableRelationship)
    {
    SchemaItem schemaItem(
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' modifier='Abstract' >"
        "    <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
        "     <BaseClass>Element</BaseClass>"
        "     <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "     </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='LinkElement' modifier='Abstract'>"
        "     <BaseClass>InformationElement</BaseClass>"
        "     <ECCustomAttributes>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>8</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "     </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UrlLink' modifier='Sealed'>"
        "     <BaseClass>LinkElement</BaseClass>"
        "    <ECProperty propertyName='Url' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='EmbeddedLink' modifier='Sealed'>"
        "     <BaseClass>LinkElement</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometricElement' modifier='Abstract'>"
        "     <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "     </ECCustomAttributes>"
        "    <BaseClass>Element</BaseClass>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Geometric3dElement' modifier='Abstract'>"
        "     <ECCustomAttributes>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>16</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "     </ECCustomAttributes>"
        "    <BaseClass>GeometricElement</BaseClass>"
        "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='VolumeElement'>"
        "    <BaseClass>Geometric3dElement</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Annotation3dElement'>"
        "    <BaseClass>Geometric3dElement</BaseClass>"
        "    <ECProperty propertyName='Font' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Sealed' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='ElementDrivesElement' strength='referencing' modifier='Abstract'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='InformationElementDrivesInformationElement' strength='referencing' modifier='Sealed'>"
        "   <BaseClass>ElementDrivesElement</BaseClass>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='InformationElement' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='InformationElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    SetupECDb("schemaupdate.ecdb", schemaItem);
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    BeFileName filePath(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    Utf8CP editedSchemaXml =
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' modifier='Abstract' >"
        "    <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
        "     <BaseClass>Element</BaseClass>"
        "     <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "     </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='LinkElement' modifier='Abstract'>"
        "     <BaseClass>InformationElement</BaseClass>"
        "     <ECCustomAttributes>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>8</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "     </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UrlLink' modifier='Sealed'>"
        "     <BaseClass>LinkElement</BaseClass>"
        "    <ECProperty propertyName='Url' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='EmbeddedLink' modifier='Sealed'>"
        "     <BaseClass>LinkElement</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometricElement' modifier='Abstract'>"
        "     <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "     </ECCustomAttributes>"
        "    <BaseClass>Element</BaseClass>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Geometric3dElement' modifier='Abstract'>"
        "     <ECCustomAttributes>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>16</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "     </ECCustomAttributes>"
        "    <BaseClass>GeometricElement</BaseClass>"
        "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='VolumeElement'>"
        "    <BaseClass>Geometric3dElement</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Annotation3dElement'>"
        "    <BaseClass>Geometric3dElement</BaseClass>"
        "    <ECProperty propertyName='Font' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Sealed' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='ElementDrivesElement' strength='referencing' modifier='Abstract'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='InformationElementDrivesInformationElement' strength='referencing' modifier='Sealed'>"
        "   <BaseClass>ElementDrivesElement</BaseClass>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='InformationElement' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='InformationElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='UrlLinkDrivesAnnotation3dElement' strength='referencing' modifier='Sealed'>"
        "   <BaseClass>ElementDrivesElement</BaseClass>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='UrlLink' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='Annotation3dElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    //verify Adding new derived LinkTable relationship for different briefcaseIds.
    m_updatedDbs.clear();
    bool asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(0), true, "Master Briefcase: add new Derived LinkTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(1), true, "standalone Briefcase: add new Derived LinkTable relationship should be successful");
    ASSERT_FALSE(asserted);

    asserted = false;
    AssertSchemaUpdate(asserted, editedSchemaXml, filePath, BeBriefcaseId(123), true, "clientside BriefcaseId: add new Derived LinkTable relationship should be successful as it do not modify Db");
    ASSERT_FALSE(asserted);

    //Verify updated schemas
    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Insert Test Data
        //Model
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(101, 'Model1')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(102, 'Model2')");

        //VolumeElement
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, ModelId, Name) VALUES(201, 'Code1', 101, 'Volume1')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, ModelId, Name) VALUES(202, 'Code2', 102, 'Volume2')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, ModelId, Name) VALUES(203, 'Code3', 102, 'Volume3')");

        //AnnotationElement
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, ModelId, Font) VALUES(301, 'Code4', 101, 'Font1')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, ModelId, Font) VALUES(302, 'Code5', 102, 'Font2')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, ModelId, Font) VALUES(303, 'Code6', 102, 'Font3')");

        //LinkUrl
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.UrlLink(ECInstanceId, Code, ModelId, Url) VALUES(401, 'Code7', 101, 'http://www.staufen.de')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.UrlLink(ECInstanceId, Code, ModelId, Url) VALUES(402, 'Code8', 101, 'http://www.staufen.de')");

        //EmbeddedLink
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.EmbeddedLink(ECInstanceId,Code, ModelId, Name) VALUES(501, 'Code9', 102,'bliblablub')");
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.EmbeddedLink(ECInstanceId,Code, ModelId, Name) VALUES(502, 'Code10', 102,'bliblablub')");

        ECClassId urlLInkId = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("UrlLink")->GetId();
        ECClassId embeddedLinkId = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("EmbeddedLink")->GetId();
        ECClassId annotation3dElementId = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("Annotation3dElement")->GetId();

        //InformationElementDrivesInformationElement
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401 , %llu , 501 , %llu )", urlLInkId.GetValue(), embeddedLinkId.GetValue());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401 , %llu , 502 , %llu )", urlLInkId.GetValue(), embeddedLinkId.GetValue());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //UrlLinkDrivesAnnotation3dElement
        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 301 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 302 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 303 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(GetECDb(), ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Verify Insertions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.ElementDrivesElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(5, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ONLY ts.ElementDrivesElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.InformationElementDrivesInformationElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.UrlLinkDrivesAnnotation3dElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(3, stmt.GetValueInt(0)) << stmt.GetECSql();

        GetECDb().CloseDb();
        }
    }
END_ECDBUNITTESTS_NAMESPACE
