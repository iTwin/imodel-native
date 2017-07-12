/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SchemaUpgradeTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaUpgradeTestFixture : public ECDbTestFixture
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
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::Readonly)));
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
        void AssertSchemaUpdate(Utf8CP schemaXml, BeFileName seedFilePath, std::pair<bool, bool> const& expectedToSucceedList, Utf8CP assertMessage)
            {
            //test 1: unrestricted ECDb
            ECDb ecdb;
            ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, "schemaupdate_unrestricted.ecdb", seedFilePath));

            bool expectedToSucceed = expectedToSucceedList.first;
            Utf8String assertMessageFull("[Unrestricted schema import] ");
            assertMessageFull.append(assertMessage);
            SchemaItem schemaItem(schemaXml);
            if (expectedToSucceed)
                ASSERT_EQ(SUCCESS, TestHelper(ecdb).ImportSchema(schemaItem)) << assertMessageFull.c_str();
            else
                ASSERT_EQ(ERROR, TestHelper(ecdb).ImportSchema(schemaItem)) << assertMessageFull.c_str();

            if (expectedToSucceed)
                m_updatedDbs.push_back((Utf8String) ecdb.GetDbFileName());

            ecdb.CloseDb();

            //test 2: restricted ECDb
            RestrictedSchemaImportECDb restrictedECDb(false, false);
            ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, "schemaupdate_changemergecomtapible.ecdb", seedFilePath));

            expectedToSucceed = expectedToSucceedList.second;
            assertMessageFull.assign("[Changeset-merging compatible schema import] ").append(assertMessage);

            if (expectedToSucceed)
                ASSERT_EQ(SUCCESS, TestHelper(restrictedECDb).ImportSchema(schemaItem)) << assertMessageFull.c_str();
            else
                ASSERT_EQ(ERROR, TestHelper(restrictedECDb).ImportSchema(schemaItem)) << assertMessageFull.c_str();

            if (expectedToSucceed)
                m_updatedDbs.push_back((Utf8String) restrictedECDb.GetDbFileName());

            restrictedECDb.CloseDb();
            }
    };


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

    ECClassCP ecClass = ecdb.Schemas().GetClass(schemaName, className, SchemaLookupMode::AutoDetect);
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
TEST_F(SchemaUpgradeTestFixture, UpdateECSchemaAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Upgrade with some attributes and import schema
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    //Verify Schema attributes upgraded successfully
    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECClassAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    //Verify Schema and Class attributes upgraded successfully
    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
    ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
    ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

    ECClassCP testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
    ASSERT_TRUE(testClass->GetDescription() == "modified test class");

    CloseReOpenECDb();

    //Verify attributes via ECSql using MataSchema
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    //verify class is accessible using new ECSchemaPrefix
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_UpdateEmtptyMixinBaseClass) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
    ASSERT_EQ(2, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionB");
    ASSERT_EQ(2, supportOption->GetBaseClasses().size());
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_UpdateEmtptyMixinBaseClassWithNoneEmptyBaseClass) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "      <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
    ASSERT_EQ(2, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "      <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_AddNewEmptyMixinBaseClasses) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_EQ(1, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
    ASSERT_EQ(3, supportOption->GetBaseClasses().size());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_AddNewNoneEmptyMixinBaseClasses) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "      <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_EQ(1, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_RemoveNoneEmptyMixinBaseClasses) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "       <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
    ASSERT_EQ(3, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateBaseClass_RemoveEmptyMixinBaseClasses) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "       <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <BaseClass>IOptionA</BaseClass>"
        "       <BaseClass>IOptionB</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
    ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
    ASSERT_EQ(3, supportOption->GetBaseClasses().size());
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IOptionB' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Element</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "      <ECProperty propertyName='P2' typeName='string' />"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='Element'>"
        "       <ECProperty propertyName='Code' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='SupportOption' modifier='None' >"
        "       <BaseClass>Element</BaseClass>"
        "       <ECProperty propertyName='P1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, TryRemoveMixinCustomAttribute_Simple) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>TestClass</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'/>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, TryAddMixinCustomAttribute_Simple) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'/>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>TestClass</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, TryRemoveMixinCustomAttribute_Complex) //TFS#917566
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>TestClass</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <BaseClass>ISourceEnd</BaseClass>"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
        "  <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>TestClassA</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "  </ECEntityClass>"
        "   <ECEntityClass typeName='TestClassA' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <BaseClass>ISourceEnd</BaseClass>"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECPropertyAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    //Verify Schema, Class and property attributes upgraded successfully
    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
    ASSERT_STREQ("modified test schema", statement.GetValueText(1));
    ASSERT_STREQ("ts_modified", statement.GetValueText(2));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
    ASSERT_STREQ("modified test class", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is modified property", statement.GetValueText(1));

    //Verify class and Property accessible using new ECSchemaPrefix
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts_modified.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdatingECDbMapCAIsNotSupported)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsNullable>true</IsNullable>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                          03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ClassModifier)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "   <ECEntityClass typeName='Koo' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
    Savepoint sp(m_ecdb, "TestData");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"); //Abstract
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (3, 't3')"); //Abstract
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (4, 't4')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (5, 't5')");

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
        "   <ECEntityClass typeName='Goo' modifier='Abstract' >"
        "       <ECProperty propertyName='L3' typeName='long' />"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' modifier='Sealed' >"
        "       <BaseClass>Goo</BaseClass>"
        "       <ECProperty propertyName='L4' typeName='long' />"
        "       <ECProperty propertyName='S4' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (6, 't6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (7, 't7')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (10, 't10')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (11, 't11')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Voo (L6, S6) VALUES (12, 't12')"); //New class added
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (8, 't8')"); //Class is still abstract
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECClassModifierToAbstract)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
    Savepoint sp(m_ecdb, "TestData");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"); //Abstract
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')");
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");

    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECClassModifierFromAbstract)
    {
    SchemaItem schemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="Foo" modifier="Abstract">
               <ECProperty propertyName="L1" typeName="long" />
               <ECProperty propertyName="S1" typeName="string"/>
           </ECEntityClass>
        </ECSchema>)xml");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_modifyclassmodifiertoabstract.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());

    {
    //Change abstract to sealed
    Utf8CP editedSchemaItem = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="Foo" modifier="Sealed">
               <ECProperty propertyName="L1" typeName="long" />
               <ECProperty propertyName="S1" typeName="string"/>
           </ECEntityClass>
        </ECSchema>)xml";

    AssertSchemaUpdate(editedSchemaItem, filePath, {false, false}, "Change Abstract to Sealed is not supported");
    m_ecdb.AbandonChanges();
    }
    {
    //Change abstract to none
    Utf8CP editedSchemaItem = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="Foo" modifier="None">
               <ECProperty propertyName="L1" typeName="long" />
               <ECProperty propertyName="S1" typeName="string"/>
           </ECEntityClass>
        </ECSchema>)xml";

    AssertSchemaUpdate(editedSchemaItem, filePath, {false, false}, "Change Abstract to None is not supported");
    m_ecdb.AbandonChanges();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteProperty_OwnTable)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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

    AssertSchemaUpdate(deleteECProperty, filePath, {false, false}, "Deleting ECProperty is generally not supported as property is mapped to OwnTable");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                          03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteProperties_TPH)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Koo -> L1, S1");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Foo -> L1, L2, S1, S2");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Goo -> L1, L2, L3, S1, S2, S3");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Boo -> L1, L2, L3, L4, S1, S2, S3, S4");

    //Insert a row for each class
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.SaveChanges());

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Koo -> L1, S1, +D1");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Foo -> L1, L2, S1, -S2, +D1, +D2");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Goo -> L1, L2, L3, S1, -S2, -S3, +D1, +D2, +D3");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Boo -> L1, L2, L3, L4, S1, -S2, -S3, -S4, +D1, +D2, +D3, +D4");

    //see if ECSQL fail for a property which has been deleted
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Ensure new property is null for existing rows
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D1 IS NULL AND D2 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL AND D4 IS NULL");

    //Insert new row with new value
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteProperties_JoinedTable)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Koo -> L1, S1");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Foo -> L1, L2, S1, S2");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Goo -> L1, L2, L3, S1, S2, S3");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Boo -> L1, L2, L3, L4, S1, S2, S3, S4");

    //Insert a row for each class
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    //Make sure ECClass definition is updated correctly
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Koo -> L1, S1, +D1");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Foo -> L1, L2, S1, -S2, +D1, +D2");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Goo -> L1, L2, L3, S1, -S2, -S3, +D1, +D2, +D3");
    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Boo -> L1, L2, L3, L4, S1, -S2, -S3, -S4, +D1, +D2, +D3, +D4");

    //see if ECSQL fail for a property which has been deleted
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')");

    //Ensure new property is null for existing rows
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D2 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D2 IS NULL AND D3 IS NULL");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D2 IS NULL AND D3 IS NULL AND D4 IS NULL");

    //Insert new row with new value
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddDeleteVirtualColumns)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='Abstract' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    //Delete and Add some properties
    Utf8CP editedSchemaItem =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Foo' modifier='Abstract' >"
        "       <ECProperty propertyName='L1' typeName='long' />"
        "       <ECProperty propertyName='D1' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaItem, filePath, {true, true}, "Addition or deletion of virtual column");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Foo -> L1, -S1, +D1");

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteOverriddenProperties)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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

    AssertSchemaUpdate(deleteOverriddenProperty, filePath, {false, false}, "Deletion overridden properties");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateCAProperties)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "            </TestCA>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedCAProperties, filePath, {true, true}, "Modifying CA classes and instances");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is modified property", statement.GetValueText(1));
        statement.Finalize();

        //Verify class and Property accessible using new ECSchemaPrefix
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT TestProperty FROM ts_modified.TestClass");

        //verify CA changes
        testProperty = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
        ASSERT_TRUE(testProperty != nullptr);
        IECInstancePtr propertyMapCA = testProperty->GetCustomAttribute("TestCA");
        ASSERT_TRUE(propertyMapCA != nullptr);
        ECValue val;
        ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
        ASSERT_FALSE(val.GetBoolean());

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewEntityClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    //Upgrade with some attributes and import schema
    Utf8CP addNewEntityClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addNewEntityClass, filePath, {true, true}, "Adding New Entity Class");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //verify tables
        //new class should be added with new namespace prefix
        ASSERT_TRUE(GetHelper().TableExists("ts_modified_TestClass"));

        //Verify Schema attributes upgraded successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);

        //Verify Newly Added Entity Class exists
        ECClassCP entityClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(entityClass != nullptr);
        ASSERT_TRUE(entityClass->GetDisplayLabel() == "Test Class");
        ASSERT_TRUE(entityClass->GetDescription() == "This is test Class");

        //Verify attributes via ECSql using MataSchema
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Test Class", statement.GetValueText(0));
        ASSERT_STREQ("This is test Class", statement.GetValueText(1));

        //Query newly added Entity Class
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewSubClassForBaseWithTPH)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    RestrictedSchemaImportECDb restrictedECDb(false, false);
    ASSERT_EQ(BE_SQLITE_OK, restrictedECDb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    TestHelper restrictedECDbTest(restrictedECDb);
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
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, restrictedECDbTest.ImportSchema(schemaWithNewSubClass)) << "Adding new Class to TPH is expected to succeed";

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
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, restrictedECDbTest.ImportSchema(schemaWithNewSubClassWithNewProperty)) << "Adding new column to TPH is expected to succeed until strict mode is enforced";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewClass_NewProperty_TPH_ShareColumns)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    RestrictedSchemaImportECDb restrictedECDb(false, false);
    ASSERT_EQ(BE_SQLITE_OK, restrictedECDb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, TestHelper(restrictedECDb).ImportSchema(schemaWithNewSubClassWithProperty)) << "Adding new Class with new property to TPH+ShareColumns is expected to fail";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         1/17
//+---------------+---------------+---------------+---------------+---------------+------
auto assertInsertECSql = [](ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << " ECSQL: " << ecsql;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << " ECSQL: " << ecsql;
    stmt.Finalize();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         1/17
//+---------------+---------------+---------------+---------------+---------------+------
auto assertSelectSql = [](ECDbCR ecdb, Utf8CP sql, int expectedColumnCount, int expectedRowCount, Utf8CP expectedColumnName)
    {
    Statement stmt;

    //Verify Column count
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, sql)) << " ECSQL: " << sql;
    ASSERT_EQ(expectedColumnCount, stmt.GetColumnCount()) << " ECSQL: " << sql;

    //Verify Row count
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        actualRowCount++;

    ASSERT_EQ(expectedRowCount, actualRowCount) << " ECSQL: " << sql;

    //Verify that the columns generated are same as expected
    Utf8String actualColumnNames;
    for (int i = 0; i < stmt.GetColumnCount(); i++)
        {
        actualColumnNames.append(stmt.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnName, actualColumnNames.c_str());
    stmt.Finalize();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         1/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, VerifyMappingOfPropertiesToOverflowOnJoinedTable)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='C1'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='int'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C2'>"
        "       <BaseClass>C1</BaseClass>"
        "        <ECProperty propertyName='C' typeName='int'/>"
        "        <ECProperty propertyName='D' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());

    //Inserting Instances for C1 and C2
    assertInsertECSql(m_ecdb, "INSERT INTO ts.C2 (A,B,C,D) VALUES (1,'val1',2,33)");
    m_ecdb.SaveChanges();

    assertSelectSql(m_ecdb, "SELECT * FROM ts_C1", 4, 1, "IdECClassIdAB");
    assertSelectSql(m_ecdb, "SELECT * FROM ts_C2", 3, 1, "C1IdECClassIdjs1");
    assertSelectSql(m_ecdb, "SELECT * FROM ts_C2_Overflow", 3, 1, "C1IdECClassIdos1");

    //Verifying the inserted values for classes C1 and C2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT A,B,C,D FROM ts.C2"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    ASSERT_STREQ("val1", stmt.GetValueText(1));
    ASSERT_EQ(2, stmt.GetValueInt(2));
    ASSERT_EQ(33, stmt.GetValueInt(3));
    stmt.Finalize();

    m_ecdb.CloseDb();

    Utf8CP addingEntityClassC3(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='C1'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='int'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C2'>"
        "       <BaseClass>C1</BaseClass>"
        "        <ECProperty propertyName='C' typeName='int'/>"
        "        <ECProperty propertyName='D' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C3'>"
        "       <BaseClass>C1</BaseClass>"
        "        <ECProperty propertyName='E' typeName='double'/>"
        "        <ECProperty propertyName='F' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    m_updatedDbs.clear();
    AssertSchemaUpdate(addingEntityClassC3, filePath, { true, true }, "Adding New Entity Class");
    m_ecdb.CloseDb();

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify that the class is added successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetClassCP("C3") != nullptr);

        m_ecdb.CloseDb();
        }

    Utf8CP addingEntityClassesC31C32(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='C1'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='int'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C2'>"
        "       <BaseClass>C1</BaseClass>"
        "        <ECProperty propertyName='C' typeName='int'/>"
        "        <ECProperty propertyName='D' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C3'>"
        "       <BaseClass>C1</BaseClass>"
        "        <ECProperty propertyName='E' typeName='double'/>"
        "        <ECProperty propertyName='F' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C31'>"
        "       <BaseClass>C3</BaseClass>"
        "        <ECProperty propertyName='G' typeName='double'/>"
        "        <ECProperty propertyName='H' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='C32'>"
        "       <BaseClass>C3</BaseClass>"
        "        <ECProperty propertyName='I' typeName='string'/>"
        "        <ECProperty propertyName='J' typeName='double'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    m_updatedDbs.clear();
    AssertSchemaUpdate(addingEntityClassesC31C32, filePath, { true, true }, "Adding Entity Classes C31 and C32");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify that the classes are added successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetClassCP("C31") != nullptr);
        ASSERT_TRUE(testSchema->GetClassCP("C32") != nullptr);

        //Tables for C1,C2,C3 should exist.
        ASSERT_TRUE(GetHelper().TableExists("ts_C1"));
        ASSERT_TRUE(GetHelper().TableExists("ts_C2"));
        ASSERT_TRUE(GetHelper().TableExists("ts_C3"));
        ASSERT_TRUE(GetHelper().TableExists("ts_C3_Overflow"));

        //C31 and C32 should not exist.
        ASSERT_FALSE(GetHelper().TableExists("ts_C31"));
        ASSERT_FALSE(GetHelper().TableExists("ts_C32"));

        //Verifying that the properties are mapped to the overflow columns
        ASSERT_EQ(Column::Kind::Default, GetHelper().GetPropertyMapColumn(AccessString("ts", "C1", "A")).GetKind());
        ASSERT_EQ(Column::Kind::Default, GetHelper().GetPropertyMapColumn(AccessString("ts", "C1", "B")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C2", "C")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C2", "D")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C3", "E")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C3", "F")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C31", "G")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C31", "H")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C32", "I")).GetKind());
        ASSERT_EQ(Column::Kind::SharedData, GetHelper().GetPropertyMapColumn(AccessString("ts", "C32", "J")).GetKind());

        //Inserting Instances in Classes C31 and C32
        assertInsertECSql(m_ecdb, "INSERT INTO ts.C31 (E,F,G,H) VALUES (10.32,3,11.1,50)");
        assertInsertECSql(m_ecdb, "INSERT INTO ts.C32 (E,F,I,J) VALUES (23.45,6,'val4',44.60)");
        m_ecdb.SaveChanges();

        //Verifying values
        assertSelectSql(m_ecdb, "SELECT * FROM ts_C3", 3, 2, "C1IdECClassIdjs1");
        assertSelectSql(m_ecdb, "SELECT * FROM ts_C3_Overflow", 5, 2, "C1IdECClassIdos1os2os3");

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT G,H FROM ts.C31"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(11.1, stmt.GetValueDouble(0));
        ASSERT_EQ(50, stmt.GetValueInt(1));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,J FROM ts.C32"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("val4", stmt.GetValueText(0));
        ASSERT_EQ(44.60, stmt.GetValueDouble(1));
        stmt.Finalize();
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewClassModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Adding New Entity Class");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description From meta.ECClassDef WHERE Name='NewTestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("New Test Class", statement.GetValueText(0));
        ASSERT_STREQ("This is New test Class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is modified property", statement.GetValueText(1));

        //Query existing and newly added Entity Classes
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts_modified.NewTestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewECDbMapCANotSupported)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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

    AssertSchemaUpdate(addECDbMapCA, filePath, {false, false}, "Adding New ECDbMap CA instance");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                          04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AppendNewCA)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECCustomAttributeClass typeName = 'UserCA1' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA2' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA3' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA4' appliesTo = 'Any' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "           <UserCA1 xmlns = 'TestSchema.01.00.00' />"
        "           <UserCA2 xmlns = 'TestSchema.01.00.00' />"
        "           <UserCA3 xmlns = 'TestSchema.01.00.00' />"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    //Add new CA
    Utf8CP addCAOnClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECCustomAttributeClass typeName = 'UserCA1' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA2' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA3' appliesTo = 'Any' />"
        "   <ECCustomAttributeClass typeName = 'UserCA4' appliesTo = 'Any' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "           <UserCA1 xmlns = 'TestSchema.01.00.00' />"
        "           <UserCA2 xmlns = 'TestSchema.01.00.00' />"
        "           <UserCA3 xmlns = 'TestSchema.01.00.00' />"
        "           <UserCA4 xmlns = 'TestSchema.01.00.00' />"
        "           <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>"
        "               <PropertyName>LastMod</PropertyName>"
        "           </ClassHasCurrentTimeStampProperty>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addCAOnClass, filePath, {true, true}, "Adding new CA instance");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
        ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
        ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

        ECClassCP testClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);
        ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
        ASSERT_TRUE(testClass->GetDescription() == "modified test class");

        //verify tables
        ASSERT_TRUE(GetHelper().TableExists("ts_TestClass"));

        //Verify attributes via ECSql using MataSchema
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();

        //Verify newly added CA
        testClass = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);
        IECInstancePtr bsca = testClass->GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
        ASSERT_TRUE(bsca != nullptr);

        ECValue val;
        ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "PropertyName"));
        ASSERT_STRCASEEQ("LastMod", val.GetUtf8CP());
        m_ecdb.CloseDb();
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewCA)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    //Add new CA
    Utf8CP addCAOnClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>"
        "                <PropertyName>LastMod</PropertyName>"
        "            </ClassHasCurrentTimeStampProperty>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addCAOnClass, filePath, {true, true}, "Adding new CA instance");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
        ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
        ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

        ECClassCP testClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);
        ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
        ASSERT_TRUE(testClass->GetDescription() == "modified test class");

        //verify tables
        ASSERT_TRUE(GetHelper().TableExists("ts_TestClass"));

        //Verify attributes via ECSql using MataSchema
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();

        //Verify newly added CA
        testClass = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);
        IECInstancePtr bsca = testClass->GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
        ASSERT_TRUE(bsca != nullptr);

        ECValue val;
        ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "PropertyName"));
        ASSERT_STRCASEEQ("LastMod", val.GetUtf8CP());
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewECProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(schemaXml, filePath, {true, true}, "Add new ECProperty");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify newly added property exists
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);

        ECClassCP testClass = testSchema->GetClassCP("TestClass");
        ASSERT_TRUE(testClass != nullptr);

        ECPropertyCP testProperty = testClass->GetPropertyP("TestProperty");
        ASSERT_TRUE(testProperty != nullptr);
        ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
        ASSERT_TRUE(testProperty->GetDescription() == "this is property");

        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is property", statement.GetValueText(1));

        //Query newly added Property
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddECPropertyToBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00' />"
        "        </ECCustomAttributes>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "      <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop1' typeName='string' />"
        "        <ECProperty propertyName='Prop2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub(Prop1,Prop2) VALUES ('Instance1 Prop1', 'Instance1 Prop2')"));
    ECInstanceKey row1;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(row1));
    stmt.Finalize();
    m_ecdb.SaveChanges();

    //reopen test file
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    SchemaItem modifiedSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00' />"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='BaseProp1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "      <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop1' typeName='string' />"
        "        <ECProperty propertyName='Prop2' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    //do schema update
    ASSERT_EQ(SUCCESS, ImportSchema(modifiedSchema));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub(BaseProp1,Prop1,Prop2) VALUES ('Instance2 BaseProp1', 'Instance2 Prop1', 'Instance2 Prop2')"));
    ECInstanceKey row2;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(row2));
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BaseProp1, Prop1, Prop2 FROM ts.Sub WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, row1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_TRUE(stmt.IsValueNull(0));
    ASSERT_STREQ("Instance1 Prop1", stmt.GetValueText(1));
    ASSERT_STREQ("Instance1 Prop2", stmt.GetValueText(2));
    stmt.ClearBindings();
    stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, row2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Instance2 BaseProp1", stmt.GetValueText(0));
    ASSERT_STREQ("Instance2 Prop1", stmt.GetValueText(1));
    ASSERT_STREQ("Instance2 Prop2", stmt.GetValueText(2));
    stmt.Finalize();

    //verify that all three props map to different columns
    Statement stmt2;
    ASSERT_EQ(BE_SQLITE_OK, stmt2.Prepare(m_ecdb, 
                "select count(distinct pm.ColumnId) FROM ec_PropertyPath pp JOIN ec_PropertyMap pm "
                "ON pm.PropertyPathId = pp.Id JOIN ec_Property p ON p.Id = pp.RootPropertyId "
                "WHERE p.Name IN ('BaseProp1', 'Prop1', 'Prop2')"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());
    ASSERT_EQ(3, stmt2.GetValueInt(0)) << "The three properties of ECClass Sub must map to 3 different columns";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Add_Delete_ECProperty_ShareColumns)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_EQ(4, GetHelper().GetColumnCount("ts_Parent"));

    ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Parent -> P1, P2");

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P3' typeName='int' />"
        "       <ECProperty propertyName='P4' typeName='int' />"
        "       <ECProperty propertyName='P5' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add Delete Property mapped to shared column");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Parent"));

        ASSERT_PROPERTIES_STRICT(m_ecdb, "TestSchema:Parent -> P1, -P2, +P3, +P4, +P5");

        //Verify insert
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Parent(P1, P3, P4, P5) VALUES(1, 2, 3, 4)");

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewPropertyModifyAllExistingAttributes)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new ECProperty");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is modified property", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='NewTestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("New Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is new property", statement.GetValueText(1));

        //Query existing and newly added Entity Classes
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty, NewTestProperty FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
        statement.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewCAOnProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new CA instance on ECProperty");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify Schema, Class and property attributes upgraded successfully
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts_modified");
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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
        ASSERT_STREQ("modified test schema", statement.GetValueText(1));
        ASSERT_STREQ("ts_modified", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
        ASSERT_STREQ("this is modified property", statement.GetValueText(1));

        //Query Property
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts_modified.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        //verify newly added CA on Property
        testProperty = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
        ASSERT_TRUE(testProperty != nullptr);
        IECInstancePtr testCA = testProperty->GetCustomAttribute("TestCA");
        ASSERT_TRUE(testCA != nullptr);
        ECValue val;
        ASSERT_EQ(ECObjectsStatus::Success, testCA->GetValue(val, "IsUnique"));
        ASSERT_FALSE(val.GetBoolean());
        statement.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECDbMapCA_AddMaxSharedColumnsBeforeOverflow)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, { false, false }, "Adding MaxSharedColumnsBeforeOverflow is supported");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MaxSharedColumnsBeforeOverflowForSubClasses_AddProperty)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_EQ(4, GetHelper().GetColumnCount("ts_Parent"));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new property mapped tp shared column");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ASSERT_EQ(8, GetHelper().GetColumnCount("ts_Parent"));
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MaxSharedColumnsBeforeOverflowWithJoinedTable_AddProperty)
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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "         <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='S1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Sub1"));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
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
    AssertSchemaUpdate(addPropertiesToSharedColumns, filePath, {true, true}, "Add new property mapped to shared columns in a JoinedTable");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));
        ASSERT_EQ(7, GetHelper().GetColumnCount("ts_Sub1"));
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ImportMultipleSchemaVersions_AddNewProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.2.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import edited schema with lower minor version with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Schema upgrade with lower minor version not allowed";

    //Verify newly added property must not exist at this point
    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
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
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem1)) << "import edited schema with higher minor version with some changes";

    //Verify newly added property must exist after third schema import
    testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);

    testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass != nullptr);

    testProperty = testClass->GetPropertyP("TestProperty");
    ASSERT_TRUE(testProperty != nullptr);
    ASSERT_TRUE(testProperty->GetDisplayLabel() == "Test Property");
    ASSERT_TRUE(testProperty->GetDescription() == "this is property");

    CloseReOpenECDb();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Test Property", statement.GetValueText(0));
    ASSERT_STREQ("this is property", statement.GetValueText(1));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateMultipleSchemasInDb)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateStartupCompanyschema.ecdb", SchemaItem::CreateForFile("DSCacheSchema.01.00.ecschema.xml")));
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem::CreateForFile("DSCacheSchema.01.03.ecschema.xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DowngradeSchemaMajorVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import schema with downgraded major version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot Downgrade schema Major Version";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DowngradeSchemaMiddleVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import schema with downgraded middle version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot Downgrade schema middle Version";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DowngradeSchemaMinorVersion)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //import schema with downgraded minor version
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot Downgrade schema Minor Version";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UnsettingSchemaAlias)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Try modifying alias=''
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Schema alias can't be set to empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, InvalidValueForSchemaAlias)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem schemaItem1(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(schemaItem1));

    //Try Upgrading to already existing alias
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "can't upgrade another schema with same alias already exists";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MajorVersionChange_WithoutMajorVersionIncremented)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem MajorVersionChange(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");

    ASSERT_EQ(ERROR, ImportSchema(MajorVersionChange)) << "Major Version change without Major Version incremented is expected to be not supported";

    SchemaItem decrementedMajorVersion(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(decrementedMajorVersion)) << "Schema Update with ECSchema Major Version decremented is expected to be not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_ECDbMapCANotSupported)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    //Delete ECDbMap CA
    Utf8CP deleteECDbMapCA =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";
    m_updatedDbs.clear();
    AssertSchemaUpdate(deleteECDbMapCA, filePath, {false, false}, "Deleting ECDbMap CustomAttribute");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                          05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_ECEntityClass_OwnTable)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(S,D,L) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(S,D,L) VALUES ('test4', 13.3, 2345)");

    ASSERT_TRUE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    //Delete Foo ===================================================================================================
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "       <ECProperty propertyName='S' typeName='string' />"
        "       <ECProperty propertyName='D' typeName='double' />"
        "       <ECProperty propertyName='L' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete class should be successful";
    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT S, D, L FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT S, D, L FROM ts.Goo");

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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "Add class should be successful";

    ASSERT_TRUE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT S, D, L FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT S, D, L FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)");
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
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_TPH)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    ASSERT_EQ(8, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete class should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    //Verify number of columns should not change as we don't delete columns
    ASSERT_EQ(8, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class with ECDbMap CA is expected to be supported
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Deleting Class with ECDbMap CA is expected to be supported";
    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with ECDbMap CA Should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "Adding new derived class should be successful";

    //should exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //following should not exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(8, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_TPH_ShareColumns)
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
        "             <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //following table should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    //Following table should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo"));
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo_Overflow"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "             <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>"))) << "Delete derived class should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);
    m_ecdb.SaveChanges();

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo")) << "After deleting subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class with SharedTable:SharedColumns is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Delete class containing ECDbMap CA should be successful";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

    //Add Goo Again===================================================================================================
    //Add Class with SharedTable:SharedColumns is expected to be supported
    m_ecdb.SaveChanges();
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
        "             <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with ECDbMap CA is expected to be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo")) << "After deleting all classes and readding base class";
    
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    //Adding new derived entity class is expected to be supported
    m_ecdb.SaveChanges();
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
        "             <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "New derived entity class is expected to be supported";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo")) << "After readding subclass";
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo_Overflow")) << "After readding subclass";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_TPH_MaxSharedColumnsBeforeOverflow)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //following table should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    //Following table should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    m_ecdb.SaveChanges();
    SchemaItem deleteFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete derived class should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo")) << "after deleting subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    //Deleting Class is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem deleteGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Delete class containing ECDbMap CA should be successful";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

    //Add Goo Again===================================================================================================
    //Add Class  is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem addGoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with ECDbMap CA (TPH, SharedColumns, MaxSharedColumnsBeforeOverflow) is expected to be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo")) << "after deleting base class and readding it";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    //Adding new derived entity class is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem addFoo(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='Goo' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "New derived entity class is expected to be supported";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo")) << "after readding subclass Foo";
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Goo_Overflow")) << "after readding subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test1', 1.3, 334, 1, 11)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test2', 23.3, 234, 2, 22)");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_JoinedTable)
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Following Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));
    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete a class should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));
    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Delete Derived ECClass is supported";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));

    //Delete Parent ===================================================================================================
    //Deleting Class with CA  JoinedTablePerDirectSubClass is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteParent)) << "Deleting Class with CA  JoinedTablePerDirectSubClass is expected to be supported";

    //Parent should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
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
        "</ECSchema>"))) << "Adding New class containing ECDbMap CA JoinedTablePerDirectSubClass should be successful";

    //Parent should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinecTablePerDirectSubClass on base class is expected to be supported
    m_ecdb.SaveChanges();
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with JoinedTablePerDirectSubClass on Parent is expected to be successful";

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    m_ecdb.SaveChanges();
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "Adding new derived Entity class is supported now";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo"));
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_JoinedTable_ShareColumns)
    {
    //Setup Db ===================================================================================================
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "         <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Following Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Verify number of columns
    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo"));
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo_Overflow"));
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete a class should be successful";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo")) << "after deleting subclass Foo";
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Delete Derived ECClass is supported";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting subclass Goo";

    //Delete Parent ===================================================================================================
    //Deleting Class with CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteParent)) << "Deleting Class with CA  JoinedTablePerDirectSubClass,SharedColumnForSubClasses is expected to be supported";

    //Parent should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>"))) << "Adding New class with ECDbMap CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses should be successful";

    //Parent should exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting and readding base class Parent";

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinedTablePerDirectSubClass,SharedColumnForSubClasses on base class is expected to be supported
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with JoinedTablePerDirectSubClass,SharedColumnForSubClasses on Parent is expected to be successful";

    //Following should exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after readding subclass Goo";
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo")) << "after readding subclass Goo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "Adding new derived Entity class is supported now";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after readding subclass Foo";
    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Goo")) << "after readding subclass Foo";
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo_Overflow")) << "after readding subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Delete_Add_ECEntityClass_JoinedTable_MaxSharedColumnsBeforeOverflow)
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
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Following Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    //Following should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent"));
    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Delete Foo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteFoo)) << "Delete a class should be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting subclass Foo";
    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo")) << "after deleting subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL, FI FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    //Delete Goo ===================================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteGoo)) << "Delete Derived ECClass is supported";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Goo"));

    //Parent should exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting subclass Goo";

    //Delete Parent ===================================================================================================
    //Deleting Class with CA  JoinedTablePerDirectSubclass_MaxSharedColumnsBeforeOverflow is expected to be supported
    m_ecdb.SaveChanges();
    SchemaItem deleteParent(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteParent)) << "Deleting Class with CA  JoinedTablePerDirectSubclass_MaxSharedColumnsBeforeOverflow is expected to be supported";

    //Parent should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "         </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P' typeName='long' />"
        "   </ECEntityClass>"
        "</ECSchema>"))) << "Adding New class containing ECDbMap CA JoinedTablePerDirectSubclass_MaxSharedColumnsBeforeOverflow should be successful";

    //Parent should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after deleting and readding base class Parent";

    //Add Goo Again===================================================================================================
    //Added Derived class with CA JoinedTablePerDirectSubclass_MaxSharedColumnsBeforeOverflow on base class is expected to be supported
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addGoo)) << "Add New Class with JoinedTablePerDirectSubclass_MaxSharedColumnsBeforeOverflow on Parent is expected to be successful";

    //Following should exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after readding subclass Goo";
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Goo")) << "after readding subclass Goo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)");

    //Add Foo Again===============================================================================================
    m_ecdb.SaveChanges();
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
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(addFoo)) << "Adding new derived Entity class is supported";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Foo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Foo"), nullptr);

    //following tables should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Goo"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Goo"), nullptr);

    ASSERT_TRUE(GetHelper().TableExists("ts_Parent"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Parent"), nullptr);

    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Parent")) << "after readding subclass Foo";
    ASSERT_EQ(9, GetHelper().GetColumnCount("ts_Goo")) << "after readding subclass Foo";
    ASSERT_EQ(3, GetHelper().GetColumnCount("ts_Goo_Overflow")) << "after readding subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL, FI, FI1 FROM ts.Foo");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Goo");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test1', 1.3, 334, 1, 11)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Foo(FS,FD,FL,FI,FI1) VALUES ('test2', 23.3, 234, 2, 22)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteSubclassOfRelationshipConstraintConstraint)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
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
        "   <ECEntityClass typeName='C' modifier='None' >"
        "       <ECNavigationProperty propertyName='A' relationshipName='RelClass' direction='Backward' />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='C'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP schemaWithDeletedConstraintClass =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='propA' typeName='int' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='None' >"
        "       <ECNavigationProperty propertyName='A' relationshipName='RelClass' direction='Backward' />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='C'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>";

    AssertSchemaUpdate(schemaWithDeletedConstraintClass, filePath, {true, true}, "Deleting subclass of ECRel ConstraintClass");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteConcreteImplementationOfAbstractConstraintClass)
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
        "       <ECNavigationProperty propertyName='A' relationshipName='RelClass' direction='Backward' />"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));


    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.B(ECInstanceId, propA, propB) VALUES(1, 11, 22)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.D(ECInstanceId, propC, propD) VALUES(2, 33, 44)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.C SET A.Id=1 WHERE ECInstanceId=2");

    //Verify Insertion
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT * FROM ts.RelClass");

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "       <ECNavigationProperty propertyName='A' relationshipName='RelClass' direction='Backward' />"
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
    AssertSchemaUpdate(schemaWithDeletedConstraintClass, filePath, {true, true}, "delete subclass of abstract rel constraint class");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Verify relationship Instance should be deleted along with deletion of constaint class
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT * FROM ts.RelClass");

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteECRelationships)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' modifier='None'>"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "       <ECNavigationProperty propertyName='Foo' relationshipName='EndTableRelationship' direction='Backward' />"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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

    AssertSchemaUpdate(relationshipWithForeignKeyMapping, filePath, {false, false}, "Deleting ECRelationship with ForeignKey Mapping");

    Utf8CP linkTableECRelationship =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' modifier='None'>"
        "       <ECProperty propertyName='S3' typeName='string' />"
        "       <ECNavigationProperty propertyName='Foo' relationshipName='EndTableRelationship' direction='Backward' />"
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

    AssertSchemaUpdate(linkTableECRelationship, filePath, {true, false}, "Deletion of LinkTable mapped relationship");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteECStructClassUnsupported)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='ChangeInfoStruct' modifier='None'>"
        "       <ECProperty propertyName='ChangeStatus' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>"))) << "Deleting ECStructClass is expected to be not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECDbMapCA_DbIndexChanges)
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
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>IDX_Partial_Index</Name>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Properties>"
        "                           <string>AId.Id</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "               </Indexes>"
        "           </DbIndexList>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    ECClassCP b = m_ecdb.Schemas().GetClass("TestSchema", "B");
    ASSERT_NE(b, nullptr);
    IECInstancePtr ca = b->GetCustomAttribute("DbIndexList");
    ASSERT_FALSE(ca.IsNull());

    ECValue indexes, indexName;
    ASSERT_EQ(ca->GetValue(indexes, "Indexes", 0), ECObjectsStatus::Success);
    ASSERT_EQ(indexes.GetStruct()->GetValue(indexName, "Name"), ECObjectsStatus::Success);
    ASSERT_STREQ(indexName.GetUtf8CP(), "IDX_Partial_Index");

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT NULL FROM ec_Index WHERE Name='IDX_Partial_Index'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    stmt.Finalize();

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP schemaWithIndexNameModified =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "       <ECCustomAttributes>"
        "           <DbIndexList xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>IDX_Partial</Name>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Properties>"
        "                           <string>AId.Id</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "               </Indexes>"
        "           </DbIndexList>"
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
    AssertSchemaUpdate(schemaWithIndexNameModified, filePath, {true, true}, "Modifying DbIndex::Name");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECClassCP b = m_ecdb.Schemas().GetClass("TestSchema", "B");
        ASSERT_NE(b, nullptr);
        IECInstancePtr ca = b->GetCustomAttribute("DbIndexList");
        ASSERT_FALSE(ca.IsNull());

        ECValue indexes, indexName;
        ASSERT_EQ(ca->GetValue(indexes, "Indexes", 0), ECObjectsStatus::Success);
        ASSERT_EQ(indexes.GetStruct()->GetValue(indexName, "Name"), ECObjectsStatus::Success);
        ASSERT_STREQ(indexName.GetUtf8CP(), "IDX_Partial");

        //verify entry updated in ec_Index table
        Statement statement;
        ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(m_ecdb, "SELECT NULL FROM ec_Index WHERE Name='IDX_Partial'"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        statement.Finalize();
        m_ecdb.CloseDb();
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
    AssertSchemaUpdate(schemaWithIndexDeleted, filePath, {false, false}, "Deleting DbIndex");
    }



//---------------------------------------------------------------------------------------
// @bsitest                                   Krischan.Eberle                     06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNavigationProperty)
    {
    Utf8CP schemaTemplate = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    %s
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                   <ECEntityClass typeName="B" modifier="None">
                        <ECProperty propertyName="Prop2" typeName="string" />
                        %s
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="owns">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                </ECSchema>)xml";

    Utf8String schemaV1Xml;
    schemaV1Xml.Sprintf(schemaTemplate, "", "");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_addnavprop.ecdb", SchemaItem(schemaV1Xml))) << "Schema import with " << schemaV1Xml.c_str();

    //now add nav prop (logical FK)
    Utf8String schemaV2Xml;
    schemaV2Xml.Sprintf(schemaTemplate, "", R"xml(<ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>)xml");

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a nav prop (w/o ForeignKeyConstraint) is added because it would change the mapping type from link table to logical FK";
    m_ecdb.AbandonChanges();

    //now add nav prop (physical FK)
    schemaV2Xml.Sprintf(schemaTemplate, R"xml(<ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>)xml",
                        R"xml(<ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>)xml");

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a nav prop (with ForeignKeyConstraint) is added because it would change the mapping type from link table to physical FK";
    m_ecdb.AbandonChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteNavigationProperty)
    {
    Utf8CP schemaTemplate = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                   <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop2" typeName="string" />
                        %s
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="owns">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                </ECSchema>)xml";

    Utf8String schemaV1Xml;
    schemaV1Xml.Sprintf(schemaTemplate,"1.0", R"xml(<ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>)xml");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_deletenavprop.ecdb", SchemaItem(schemaV1Xml))) << "Schema import with " << schemaV1Xml.c_str();

    //now delete nav prop (logical FK)
    Utf8String schemaV2Xml;
    schemaV2Xml.Sprintf(schemaTemplate,"2.0", "");

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a nav prop (w/o ForeignKeyConstraint) is deleted because it would change the mapping type logical FK to link table";
    m_ecdb.AbandonChanges();

    //now delete nav prop (physical FK)
    schemaV1Xml.Sprintf(schemaTemplate,"1.0", R"xml(<ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>)xml");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_deletenavprop.ecdb", SchemaItem(schemaV1Xml))) << "Schema import with " << schemaV1Xml.c_str();

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a nav prop (with ForeignKeyConstraint) is deleted because it would change the mapping type from physical FK to link table";
    }

//---------------------------------------------------------------------------------------
// @bsitest                                   Krischan.Eberle                     06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddForeignKeyConstraint)
    {
    Utf8CP schemaTemplate = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   %s
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                   <ECEntityClass typeName="B" modifier="None">
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                            %s
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None">
                            <Source multiplicity="(0..1)" polymorphic="True" roleLabel="owns">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                </ECSchema>)xml";

    Utf8String schemaV1Xml;
    schemaV1Xml.Sprintf(schemaTemplate, "", "");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_addforeignkeyconstraint.ecdb", SchemaItem(schemaV1Xml.c_str()))) << "Schema import with " << schemaV1Xml.c_str();

    //now add FKConstraint CA
    Utf8String schemaV2Xml;
    schemaV2Xml.Sprintf(schemaTemplate, R"xml(<ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>)xml",
                        R"xml(<ForeignKeyConstraint xmlns="ECDbMap.02.00"/>)xml");

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a ForeignKeyConstraint is added";
    }

//---------------------------------------------------------------------------------------
// @bsitest                                   Krischan.Eberle                     06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddLinkTableRelationshipMap)
    {
    Utf8CP schemaTemplate = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   %s
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Prop1" typeName="string" />
                    </ECEntityClass>
                   <ECEntityClass typeName="B" modifier="None">
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None">
                            <ECCustomAttributes>
                            %s
                            </ECCustomAttributes>
                            <Source multiplicity="%s" polymorphic="True" roleLabel="owns">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                              <Class class="B"/>
                            </Target>
                     </ECRelationshipClass>            
                </ECSchema>)xml";

    for (Utf8CP parentMultiplicity : {"(0..1)", {"(0..*)"}})
        {
        Utf8String schemaV1Xml;
        schemaV1Xml.Sprintf(schemaTemplate, "", "", parentMultiplicity);
        ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_addlinktablerelationshipmap.ecdb", SchemaItem(schemaV1Xml))) << "Schema import with parent multiplicity " << parentMultiplicity << ": " << schemaV1Xml.c_str();

        //now add LinkTableRelationshipMap CA
        Utf8String schemaV2Xml;
        schemaV2Xml.Sprintf(schemaTemplate, R"xml(<ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>)xml",
                            R"xml(<LinkTableRelationshipMap xmlns="ECDbMap.02.00"/>)xml", parentMultiplicity);

        ASSERT_EQ(ERROR, ImportSchema(SchemaItem(schemaV2Xml))) << "Schema update should fail when a LinkTableRelationshipMap CA is added for a rel with parent multiplicity " << parentMultiplicity;
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Add_Class_NavigationProperty_RelationshipClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='PA' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP schemaWithNavProperty=
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(schemaWithNavProperty, filePath, {true, true}, "Adding Classes and Navigation property simultaneously");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECClassCP entityClass = m_ecdb.Schemas().GetClass("TestSchema", "B");
        ASSERT_TRUE(entityClass != nullptr);

        ECClassCP relClass = m_ecdb.Schemas().GetClass("TestSchema", "AHasB");
        ASSERT_TRUE(relClass != nullptr);

        NavigationECPropertyCP navProp = entityClass->GetPropertyP("AId")->GetAsNavigationProperty();
        ASSERT_TRUE(navProp != nullptr);

        m_ecdb.CloseDb();
        }
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ValidateModifingAddingDeletingBaseClassNotSupported)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem schemaWithDeletedBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "   </ECEntityClass >"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDeletedBaseClass)) << "Deleting Base Class not allowed";

    SchemaItem schemaWithModifedBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass0</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithModifedBaseClass)) << "Modifying Base Class not allowed";

    SchemaItem schemaWithNewBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass</BaseClass>"
        "       <BaseClass>TestClass0</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithNewBaseClass)) << "Adding new Base Class not allowed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteExistingECEnumeration)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Deletion of ECEnumeration is not suppported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyExistingECEnumeration)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyIsEntityClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Changing ECClass::IsEntityClass is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyIsStructClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECStructClass typeName='TestClass' />"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Changing ECClass::IsStructClass is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyIsCustomAttributeClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName='TestClass' appliesTo='EntityClass, RelationshipClass' />"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Changing ECClass::IsCustomAttributeClass is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyIsRelationshipClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "   <ECCustomAttributeClass typeName='RelClass' />"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Changing ECClass::IsRelationshipClass is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyRelationship)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    Savepoint sp(m_ecdb, "Schema Import");
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
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDifferentCardinality)) << "changing Relationship Cardinality not allowed";

    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different target Cardinality.
    SchemaItem schemaWithDifferentTargetCardinality(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDifferentTargetCardinality)) << "changing Relationship Cardinality not allowed";
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different source Constraint Class
    SchemaItem differentSourceConstraintClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='None' />"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(differentSourceConstraintClass));
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with different Target Constraint Class
    SchemaItem differentTargetConstraintClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' />"
        "   <ECEntityClass typeName='B' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(differentTargetConstraintClass));
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with NonPolymorphic Source
    SchemaItem nonPolymorphicSource(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='False'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(nonPolymorphicSource));
    sp.Cancel();

    sp.Begin();
    //Try Upgrade with NonPolymorphic Target
    SchemaItem nonPolymorphicTarget(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='False'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(nonPolymorphicTarget));
    sp.Cancel();

    sp.Begin();
    //Try Upgrading schema with different strength.
    SchemaItem schemaWithDifferentStrength(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='referencing' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDifferentStrength)) << "changing relationship Strength not allowed";
    sp.Cancel();

    sp.Begin();
    //Verify Changing strength direction not supported
    SchemaItem schemaWithDifferentStrengthDirection(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='backward' >"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDifferentStrengthDirection)) << "changing relationship Strength Direction not allowed";
    sp.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyRelationshipConstrainsRoleLabel)
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
        "   <ECEntityClass typeName='C' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

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
        "   <ECEntityClass typeName='C' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C Modified' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(modifySourceRoleLabel)) << "Modifying Source constraint class RoleLabel is expected to be successful";

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
        "   <ECEntityClass typeName='C' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has C Modified' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='C belongs to A Modified' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(modifyTargetRoleLabel)) << "Modifying Target Constraint class Role Label is expected to be successful";

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
        "   <ECEntityClass typeName='C' modifier='None' >"
        "     <ECNavigationProperty propertyName='Parent' relationshipName='RelClass' direction='Backward'/>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelClass' modifier='Sealed' strength='embedding' strengthDirection='forward' >"
        "       <Source cardinality='(0,1)' roleLabel='A has B and C' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' roleLabel='B and C belong to A' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(modifyBothRoleLabels)) << "Modifying both source and target class RoleLabels simultaneously is expected to be successful";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECProperties)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(modifiedECPropertyType, filePath, {false, false}, "Modifying ECProperty type name");

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
    AssertSchemaUpdate(modifiedECStructPropertyType, filePath, {false, false}, "Modifying ECStructProperty");

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
    AssertSchemaUpdate(modifiedECStructArrayPropertyType, filePath, {false, false}, "Modifying ECStructArrayProperty");

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
    AssertSchemaUpdate(modifiedPrimitiveArrayType, filePath, {false, false}, "Modifying ECArrayProperty prim type");

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
    AssertSchemaUpdate(modifiedPrimitiveType, filePath, {false, false}, "Modifying PrimitiveType is not supported");

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
    AssertSchemaUpdate(modifiedECPropertyArrayMixOccurs, filePath, {false, false}, "Modifying ECPropertyArray minOccurs");

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
    AssertSchemaUpdate(modifiedECArrayPropertyMaxOccurs, filePath, {false, false}, "Modifying ECPropertyArray maxOccurs");

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
    AssertSchemaUpdate(modifiedExtendedType, filePath, {true, true}, "Modifying extendedTypeName");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyNavigationProperty)
    {
    SchemaItem schemaItem(
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
        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
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
        "        <ECNavigationProperty propertyName='A' relationshipName='A1HasB' direction='Backward' />"
        "        <ECNavigationProperty propertyName='NewA' relationshipName='AHasB' direction='Backward' />"
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
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(modifiedRelNameInNavProperty)) << "Changing relationship Class Name for a Navigation property is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropToReadOnly)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    /*-------------------After 1st Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadOnly
    P2            -> ReadWrite
    */

    //Insert should be successfull
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val1', 'P2_Val1')");

    //readonly property can't be updated
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P1='P1_Val1new'");

    //skipping readonly Property, Update should be successful.
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass Set ReadWriteProp='RW1new', P2='P2_Val1new' WHERE P2='P2_Val1'");

    //Update schema 
    SchemaItem schemaItem2(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='ReadWriteProp' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='P1' typeName='string' readOnly='false' />"// readOnly='false' after update
        "       <ECProperty propertyName='P2' typeName='string' readOnly='true' />"//readOnly='true' after update
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(schemaItem2)) << "Modifying readonly Flag is expected to succeed";

    /*-------------------After 2nd Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadWrite
    P2            -> ReadOnly
    */

    //Verify Insert
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW2', 'P1_Val2', 'P2_Val2')");

    //Verify Update
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P2='P2_Val2new'");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass SET ReadWriteProp='RW2new', P1='P1_Val2new' WHERE P1='P1_Val2'");

    //Verify Select
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P2 FROM ts.TestClass WHERE ReadWriteProp='RW1new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P2_Val1new", statement.GetValueText(0));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW2new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val2new", statement.GetValueText(0));
    statement.Finalize();

    //Verify Delete
    Savepoint sp(m_ecdb, "To Revert Delete Operation");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "DELETE FROM ts.TestClass");

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Finalize();

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
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(schemaItem3)) << "Modifying readonly Flag is expected to succeed";

    /*-------------------After 3rd Schema Import--------------------------
    ReadWriteProp -> ReadWrite
    P1            -> ReadOnly
    P2            -> ReadWrite
    */

    //Insert should be successfull
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.TestClass(ReadWriteProp, P1, P2) VALUES('RW1', 'P1_Val3', 'P2_Val3')");

    //verify update
    //Update Prepare should fail for ReadOnlyProp
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P1='P1_Val3new'");
    //skipping readonly Property Update should be successful.
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.TestClass Set ReadWriteProp='RW3new', P2='P2_Val3new' WHERE P1 = 'P1_Val3'");

    //Verify Select
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P1 FROM ts.TestClass WHERE ReadWriteProp='RW3new'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("P1_Val3", statement.GetValueText(0));
    statement.Finalize();

    //Verify Delete
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "DELETE FROM ts.TestClass");

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.TestClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropToReadOnlyOnClientBriefcase)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    m_ecdb.SetAsBriefcase(BeBriefcaseId(123));

    //Update schema 
    SchemaItem schemaItem2(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='ReadWriteProp' typeName='string' readOnly='false' />"
        "       <ECProperty propertyName='P1' typeName='string' readOnly='false' />"// readOnly='false' after update
        "       <ECProperty propertyName='P2' typeName='string' readOnly='true' />"//readOnly='true' after update
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(schemaItem2)) << "Modifying readonly Flag is expected to succeed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyCustomAttributePropertyValues)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(changeCAPropertyValues, filePath, {true, true}, "Modifying CA instance properties values is supported");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "Select ec_CustomAttribute.[Instance] from ec_Property  INNER JOIN ec_CustomAttribute ON ec_CustomAttribute.[ContainerId] = ec_Property.[Id] Where ec_Property.[Name] = 'TestProperty'"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("<TestCA xmlns=\"TestSchema.01.00\">\n    <BinaryProp>10100011</BinaryProp>\n    <BooleanProp>False</BooleanProp>\n    <DateTimeProp>20160510</DateTimeProp>\n    <DoubleProp>2.0001000000000002</DoubleProp>\n    <IntegerProp>20</IntegerProp>\n    <LongProp>2000000</LongProp>\n    <Point2DProp>4,5.5</Point2DProp>\n    <Point3DProp>35.5,45.5,55.5</Point3DProp>\n    <StringProp>'This is Modified String Property'</StringProp>\n</TestCA>\n", stmt.GetValueText(0));
        stmt.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteECCustomAttributeClass)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty,EntityClass' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP deleteECCustomAttribute =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(deleteECCustomAttribute, filePath, {false, false}, "Deleting a ECCustomAttributeClass");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteCustomAttribute)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECCustomAttributes>"
        "       <DynamicSchema xmlns = 'CoreCustomAttributes.01.00' />"
        "   </ECCustomAttributes>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>"
        "                <PropertyName>LastMod</PropertyName>"
        "            </ClassHasCurrentTimeStampProperty>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "       <ECProperty propertyName='prop' typeName='boolean' >"
        "        <ECCustomAttributes>"
        "            <Localizable xmlns='CoreCustomAttributes.01.00'/>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("deleteca.ecdb", schemaItem));

    //Delete CA from Schema
    SchemaItem deleteCAFromSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>"
        "                <PropertyName>LastMod</PropertyName>"
        "            </ClassHasCurrentTimeStampProperty>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "       <ECProperty propertyName='prop' typeName='boolean' >"
        "        <ECCustomAttributes>"
        "            <Localizable xmlns='CoreCustomAttributes.01.00'/>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteCAFromSchema));
    IECInstancePtr schemaCA = m_ecdb.Schemas().GetSchema("TestSchema")->GetCustomAttribute("DynamicSchema");
    ASSERT_TRUE(schemaCA == nullptr);

    //Delete CA from Class
    SchemaItem deleteCAFromClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "       <ECProperty propertyName='prop' typeName='boolean' >"
        "        <ECCustomAttributes>"
        "            <Localizable xmlns='CoreCustomAttributes.01.00'/>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteCAFromClass));
    IECInstancePtr classCA = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
    ASSERT_TRUE(classCA == nullptr);

    //Delete CA from property
    SchemaItem deleteCAFromProperty(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts_modified' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' />"
        "       <ECProperty propertyName='prop' typeName='boolean' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteCAFromProperty));
    int caCount = 0;
    for (IECInstancePtr ca : m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("prop")->GetCustomAttributes(true))
        {
        caCount++;
        }

    ASSERT_EQ(0, caCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ChangesToExisitingTable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("existingTableUpdate.ecdb"));

    // Create existing table and insert rows
    m_ecdb.ExecuteSql("CREATE TABLE test_Employee(Id INTEGER PRIMARY KEY, Name TEXT);");
    m_ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (101, 'Affan Khan');");
    m_ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (201, 'Muhammad Hassan');");
    m_ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (301, 'Krischan Eberle');");
    m_ecdb.ExecuteSql("INSERT INTO test_Employee (Id, Name) VALUES (401, 'Casey Mullen');");

    // Map ECSchema to exisitng table
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap'/>"
        "   <ECEntityClass typeName='Employee' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>ExistingTable</MapStrategy>"
        "               <TableName>test_Employee</TableName>"
        "           </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='Name' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(schemaItem));

    // Modify and add new existing table and relate it to another one
    m_ecdb.ExecuteSql("CREATE TABLE test_Title(Id INTEGER PRIMARY KEY, Name TEXT);");
    m_ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (1, 'Senior Software Engineer');");
    m_ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (2, 'Software Quality Analyst');");
    m_ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (3, 'Advisory Software Engineer');");
    m_ecdb.ExecuteSql("INSERT INTO test_Title (Id, Name) VALUES (4, 'Distinguished Architect');");
    m_ecdb.ExecuteSql("ALTER TABLE test_Employee ADD COLUMN TitleId INTEGER REFERENCES test_Title(Id);");
    m_ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 1 WHERE Id = 101");
    m_ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 2 WHERE Id = 201");
    m_ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 3 WHERE Id = 301");
    m_ecdb.ExecuteSql("UPDATE test_Employee SET TitleId = 4 WHERE Id = 401");

    // Map ECSchema to exisitng table
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap'/>"
        "   <ECEntityClass typeName='Employee' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>ExistingTable</MapStrategy>"
        "               <TableName>test_Employee</TableName>"
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
        "           </ClassMap>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='Name' typeName='string' >"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "    <ECRelationshipClass typeName='EmployeeHasTitle' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' polymorphic='false' roleLabel='Employee'>"
        "            <Class class='Employee'/>"
        "        </Source>"
        "        <Target multiplicity='(0..1)' polymorphic='false' roleLabel='Title'>"
        "            <Class class='Title'/>"
        "        </Target>"
        "    </ECRelationshipClass>'"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    ECSqlStatement stmt;
    stmt.Prepare(m_ecdb, "SELECT E.Name EmployeeName, T.Name EmployeeTitle FROM ts.Employee E JOIN ts.Title T USING ts.EmployeeHasTitle ORDER BY E.ECInstanceId");

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
TEST_F(SchemaUpgradeTestFixture, DeleteCAInstanceWithoutProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty,EntityClass'>"
        "   </ECCustomAttributeClass>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("deletecainstancewithoutproperty.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(deleteAllCA, filePath, {true, true}, "Deleting CA instance without Properties");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECSchemaCP ecSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ECClassCP testClass = ecSchema->GetClassCP("TestClass");
        ECPropertyP ecProperty = testClass->GetPropertyP("prop");

        IECInstancePtr systemSchemaCA = ecSchema->GetCustomAttribute("SystemSchema");
        ASSERT_TRUE(systemSchemaCA == nullptr);

        IECInstancePtr classCA = testClass->GetCustomAttribute("TestCA");
        ASSERT_TRUE(classCA == nullptr);

        IECInstancePtr propertyCA = ecProperty->GetCustomAttribute("TestCA");
        ASSERT_TRUE(propertyCA == nullptr);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddKoQAndUpdatePropertiesWithKoQ)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='My KindOfQuantity'"
        "                    displayLabel='My KindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN;M' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='MyKindOfQuantity' />"
        "        <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />" 
        "        <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'MyKindOfQuantity'/>"
        "        <ECArrayProperty propertyName='Favorites' typeName='string' extendedTypeName='URL' minOccurs='0' maxOccurs='unbounded' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "AddKoQAndUpdatePropertiesWithKoQ");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        KindOfQuantityCP myKindOfQuantity = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKindOfQuantity");
        ASSERT_TRUE(myKindOfQuantity != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_length = foo->GetPropertyP("Length")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_length != nullptr);
        ASSERT_TRUE(foo_length->GetKindOfQuantity() == myKindOfQuantity);

        PrimitiveECPropertyCP foo_homepage = foo->GetPropertyP("Homepage")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_homepage != nullptr);
        ASSERT_STREQ("URL", foo_homepage->GetExtendedTypeName().c_str());

        PrimitiveArrayECPropertyCP foo_alternativeLengths = foo->GetPropertyP("AlternativeLengths")->GetAsPrimitiveArrayProperty();
        ASSERT_TRUE(foo_alternativeLengths != nullptr);
        ASSERT_TRUE(foo_alternativeLengths->GetKindOfQuantity() == myKindOfQuantity);

        PrimitiveArrayECPropertyCP foo_favorites = foo->GetPropertyP("Favorites")->GetAsPrimitiveArrayProperty();
        ASSERT_TRUE(foo_favorites != nullptr);
        ASSERT_STREQ("URL", foo_favorites->GetExtendedTypeName().c_str());

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyType_PrimitiveToNonStrictEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "changing primitive to NonString Enum is supported");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP nonStrictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
        ASSERT_TRUE(nonStrictEnum != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nonStrictEnum);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyType_PrimitiveToStrictEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "changing primitive to Strict Enum is not supported");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateKindOfQuantity)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='KindOfQuantity1' />"
        "        <ECProperty propertyName='Homepage' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
        "                    displayLabel='KindOfQuantity2' persistenceUnit='M' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Length' typeName='double' kindOfQuantity='KindOfQuantity2' />"
        "    <ECProperty propertyName='Homepage' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Changing the KindOfQuantity of an ECProperty to another KindOfQuantity");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        KindOfQuantityCP KindOfQuantity2 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KindOfQuantity2");
        ASSERT_TRUE(KindOfQuantity2 != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_length = foo->GetPropertyP("Length")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_length != nullptr);
        ASSERT_TRUE(foo_length->GetKindOfQuantity() == KindOfQuantity2);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteKindOfQuantityFromECSchema)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;CM' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='MyKindOfQuantity' />"
        "        <ECProperty propertyName='Homepage' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Length' typeName='double' />"
        "    <ECProperty propertyName='Homepage' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Deleting KindOfQuantity from an ECSchema");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECArrayProperty_KOQToKOQ)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
        "                    displayLabel='KindOfQuantity2' persistenceUnit='M' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity1'/>"
        "        <ECArrayProperty propertyName='Width' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity2'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
        "                    displayLabel='KindOfQuantity2' persistenceUnit='M' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity2'/>"
        "        <ECArrayProperty propertyName='Width' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity1'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Changing of KindOfQuantity of an ECArrayProperty to another KindOfQuantity");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        KindOfQuantityCP KindOfQuantity1 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KindOfQuantity1");
        ASSERT_TRUE(KindOfQuantity1 != nullptr);
        KindOfQuantityCP KindOfQuantity2 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KindOfQuantity2");
        ASSERT_TRUE(KindOfQuantity2 != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        ECPropertyCP foo_length = foo->GetPropertyP("Length");
        ASSERT_TRUE(foo_length != nullptr);
        ECPropertyCP foo_width = foo->GetPropertyP("Width");
        ASSERT_TRUE(foo_width != nullptr);

        ASSERT_TRUE(foo_length->GetKindOfQuantity() == KindOfQuantity2);
        ASSERT_TRUE(foo_width->GetKindOfQuantity() == KindOfQuantity1);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, RemoveKindOfQuantityFromECArrayProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'MyKindOfQuantity'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Verifying KOQ assigned to the property
    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);

    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(foo != nullptr);

    ASSERT_EQ(koq, foo->GetPropertyP("Length")->GetKindOfQuantity());

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Removing KindOfQuantity from an ECArrayProperty");

    //Verifying the property no longer has KOQ
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(m_updatedDbs[0].c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECPropertyCP foo_length = m_ecdb.Schemas().GetClass("TestSchema", "Foo")->GetPropertyP("Length");
    ASSERT_EQ("Length", foo_length->GetName());
    ASSERT_TRUE(foo_length->GetKindOfQuantity() == nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, RemoveKindOfQuantityFromECProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double' kindOfQuantity='MyKindOfQuantity'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //Verifying KOQ assigned to the property
    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);

    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(foo != nullptr);

    ASSERT_EQ(koq, foo->GetPropertyP("Length")->GetKindOfQuantity());

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Removing KindOfQuantity from an ECProperty");

    //Verifying the property no longer has KOQ
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(m_updatedDbs[0].c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECPropertyCP foo_length = m_ecdb.Schemas().GetClass("TestSchema", "Foo")->GetPropertyP("Length");
    ASSERT_EQ("Length", foo_length->GetName());
    ASSERT_TRUE(foo_length->GetKindOfQuantity() == nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, KindOfQuantityAddUpdate)
    {
    SchemaItem schemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
            <KindOfQuantity typeName='K1' description='K1' displayLabel='K1' persistenceUnit='CM' relativeError='.5' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K2' description='K2' displayLabel='K2' persistenceUnit='CM' relativeError='.2' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K3' description='K3' displayLabel='K3' persistenceUnit='CM' relativeError='.1' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K4' description='K4' displayLabel='K4' persistenceUnit='CM' relativeError='.1' presentationUnits='FT;IN' /> 
            <ECEntityClass typeName='Foo' >
                <ECProperty propertyName='L1' typeName='double' kindOfQuantity='K1'/>
                <ECProperty propertyName='L2' typeName='double' kindOfQuantity='K2'/>
                <ECProperty propertyName='L3' typeName='double' kindOfQuantity='K3'/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    if (true)
        {
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
        KindOfQuantityCP k1 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K1");
        ASSERT_TRUE(k1 != nullptr);

        KindOfQuantityCP k2 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K2");
        ASSERT_TRUE(k2 != nullptr);

        KindOfQuantityCP k3 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K3");
        ASSERT_TRUE(k3 != nullptr);

        KindOfQuantityCP k4 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K4");
        ASSERT_TRUE(k4 != nullptr);

        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        ASSERT_TRUE(foo->GetPropertyP("L1") != nullptr);
        ASSERT_TRUE(foo->GetPropertyP("L2") != nullptr);
        ASSERT_TRUE(foo->GetPropertyP("L3") != nullptr);

        ASSERT_EQ(k1, foo->GetPropertyP("L1")->GetKindOfQuantity());
        ASSERT_EQ(k2, foo->GetPropertyP("L2")->GetKindOfQuantity());
        ASSERT_EQ(k3, foo->GetPropertyP("L3")->GetKindOfQuantity());
        }

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
            <KindOfQuantity typeName='K1' description='K1' displayLabel='K1' persistenceUnit='CM' relativeError='.5' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K2' description='K2' displayLabel='K2' persistenceUnit='CM' relativeError='.2' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K3' description='K3' displayLabel='K3' persistenceUnit='CM' relativeError='.1' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K4' description='K4' displayLabel='K4' persistenceUnit='CM' relativeError='.1' presentationUnits='FT;IN' /> 
            <KindOfQuantity typeName='K5' description='K5' displayLabel='K5' persistenceUnit='CM' relativeError='.1' presentationUnits='FT;IN' /> 
            <ECEntityClass typeName='Foo' >
                <ECProperty propertyName='L1' typeName='double' kindOfQuantity='K5'/>
                <ECProperty propertyName='L2' typeName='double' kindOfQuantity='K2'/>
                <ECProperty propertyName='L3' typeName='double' />
            </ECEntityClass>
        </ECSchema>)xml")));

    if (true)
        {
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

        KindOfQuantityCP k1 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K1");
        ASSERT_TRUE(k1 != nullptr);

        KindOfQuantityCP k2 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K2");
        ASSERT_TRUE(k2 != nullptr);

        KindOfQuantityCP k3 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K3");
        ASSERT_TRUE(k3 != nullptr);

        KindOfQuantityCP k4 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K4");
        ASSERT_TRUE(k4 != nullptr);

        KindOfQuantityCP k5 = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "K5");
        ASSERT_TRUE(k5 != nullptr);


        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        ASSERT_TRUE(foo->GetPropertyP("L1") != nullptr);
        ASSERT_TRUE(foo->GetPropertyP("L2") != nullptr);
        ASSERT_TRUE(foo->GetPropertyP("L3") != nullptr);

        ASSERT_EQ(k5, foo->GetPropertyP("L1")->GetKindOfQuantity());
        ASSERT_EQ(k2, foo->GetPropertyP("L2")->GetKindOfQuantity());
        ASSERT_EQ(nullptr, foo->GetPropertyP("L3")->GetKindOfQuantity());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyType_PrimitiveToPrimitive)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "changing primitive to another primitive");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyType_EnumToPrimitive)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Changing Enum to primitive");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nullptr);
        ASSERT_TRUE(foo_type->GetType() == PrimitiveType::PRIMITIVETYPE_Integer);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyType_EnumToEnum)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "changing Enum to Enum");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECEnumerationCP unstrictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "UnStrictEnum");
        ASSERT_TRUE(unstrictEnum != nullptr);

        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);
        ECClassCP goo = m_ecdb.Schemas().GetClass("TestSchema", "Goo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();
        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == unstrictEnum);

        PrimitiveECPropertyCP goo_type = goo->GetPropertyP("Type")->GetAsPrimitiveProperty();
        ASSERT_TRUE(goo_type != nullptr);
        ASSERT_TRUE(goo_type->GetEnumeration() == strictEnum);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyTypeString_EnumToPrimitive)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='string' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = 'val1' displayLabel = 'txt' />"
        "        <ECEnumerator value = 'val2' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='string' isStrict='True'>" // StrictEnum
        "        <ECEnumerator value = 'val1' displayLabel = 'txt' />"
        "        <ECEnumerator value = 'val2' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Changing String Enum to String");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nullptr);
        ASSERT_TRUE(foo_type->GetType() == PrimitiveType::PRIMITIVETYPE_String);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyPropertyTypeString_PrimitiveToUnStrictEnum)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='False'>"
        "        <ECEnumerator value = 'val1' displayLabel = 'txt' />"
        "        <ECEnumerator value = 'val2' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='NonStrictEnum' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "changing String to Unstrict Enum");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP nonStrictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
        ASSERT_TRUE(nonStrictEnum != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == nonStrictEnum);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     2/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyEnumType_IntToString)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator value = '0' displayLabel = 'txt' />"
        "        <ECEnumerator value = '1' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='True'>"
        "        <ECEnumerator value = 'val1' displayLabel = 'txt' />"
        "        <ECEnumerator value = 'val2' displayLabel = 'bat' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='StrictEnum' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, { false, false }, "Changing Enum Type is not supported");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, RemoveExistingEnum)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Type' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Deleting Enum");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECEnumerationCP strictEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StrictEnum");
        ASSERT_TRUE(strictEnum != nullptr);
        ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(foo != nullptr);

        PrimitiveECPropertyCP foo_type = foo->GetPropertyP("Type")->GetAsPrimitiveProperty();

        ASSERT_TRUE(foo_type != nullptr);
        ASSERT_TRUE(foo_type->GetEnumeration() == strictEnum);
         
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewRelationship)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='None' >"
        "    <ECProperty propertyName='AProp' typeName='int' />"
        "    <ECNavigationProperty propertyName='B' relationshipName='RelClass' direction='Backward'/>"
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

    //verify Adding new EndTable relationship
    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new endtable relationship");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ASSERT_EQ(3, GetHelper().GetColumnCount("ts_A"));
        ASSERT_EQ(2, GetHelper().GetColumnCount("ts_B"));
        m_ecdb.CloseDb();
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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new LinkTable relationship");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ASSERT_EQ(true, GetHelper().TableExists("ts_RelClass"));

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewDerivedEndTableRelationship)
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new Derived EndTable relationship");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        ECClassId modelHasGeometricElementsRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ModelHasGeometricElements");
        ASSERT_TRUE(modelHasGeometricElementsRelClassId.IsValid());

        //Insert Test Data
        //Model
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(101, 'Model1')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(102, 'Model2')");

        //GeometricElement
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.GeometricElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement) VALUES(201, 'Code1', 101, %s, 'GeometricElement1')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());
        ecsql.Sprintf("INSERT INTO ts.GeometricElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement) VALUES(202, 'Code2', 101, %s, 'GeometricElement2')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Geometric3dElement
        ecsql.Sprintf("INSERT INTO ts.Geometric3dElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement, Geometry3d) VALUES(301, 'Code3', 102, %s, 'GeometricElement3', 'Geometry3d3')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());
        ecsql.Sprintf("INSERT INTO ts.Geometric3dElement(ECInstanceId, Code, Model.Id, Model.RelECClassId, GeometricElement, Geometry3d) VALUES(302, 'Code4', 102, %s, 'GeometricElement4', 'Geometry3d4')",
                      modelHasGeometricElementsRelClassId.ToString().c_str());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Select statements
        //Verify insertions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.ModelHasElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        //FROM ONLY abstract class is expected to return 0 rows
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ONLY ts.ModelHasElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.ModelHasGeometricElements"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();
        stmt.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddNewDerivedLinkTableRelationship)
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
        "              <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>"
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
        "              <MaxSharedColumnsBeforeOverflow>16</MaxSharedColumnsBeforeOverflow>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

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
        "              <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>"
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
        "              <MaxSharedColumnsBeforeOverflow>16</MaxSharedColumnsBeforeOverflow>"
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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Add new Derived LinkTable relationship");

    //Verify updated schemas
    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //Insert Test Data
        //Model
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(101, 'Model1')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Model(ECInstanceId, Name) VALUES(102, 'Model2')");

        //VolumeElement
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, Model.Id, Name) VALUES(201, 'Code1', 101, 'Volume1')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, Model.Id, Name) VALUES(202, 'Code2', 102, 'Volume2')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.VolumeElement(ECInstanceId, Code, Model.Id, Name) VALUES(203, 'Code3', 102, 'Volume3')");

        //AnnotationElement
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, Model.Id, Font) VALUES(301, 'Code4', 101, 'Font1')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, Model.Id, Font) VALUES(302, 'Code5', 102, 'Font2')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Annotation3dElement(ECInstanceId, Code, Model.Id, Font) VALUES(303, 'Code6', 102, 'Font3')");

        //LinkUrl
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.UrlLink(ECInstanceId, Code, Model.Id, Url) VALUES(401, 'Code7', 101, 'http://www.staufen.de')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.UrlLink(ECInstanceId, Code, Model.Id, Url) VALUES(402, 'Code8', 101, 'http://www.staufen.de')");

        //EmbeddedLink
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.EmbeddedLink(ECInstanceId,Code, Model.Id, Name) VALUES(501, 'Code9', 102,'bliblablub')");
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.EmbeddedLink(ECInstanceId,Code, Model.Id, Name) VALUES(502, 'Code10', 102,'bliblablub')");

        ECClassId urlLInkId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("UrlLink")->GetId();
        ECClassId embeddedLinkId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("EmbeddedLink")->GetId();
        ECClassId annotation3dElementId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("Annotation3dElement")->GetId();

        //InformationElementDrivesInformationElement
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401 , %llu , 501 , %llu )", urlLInkId.GetValue(), embeddedLinkId.GetValue());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401 , %llu , 502 , %llu )", urlLInkId.GetValue(), embeddedLinkId.GetValue());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //UrlLinkDrivesAnnotation3dElement
        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 301 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 302 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        ecsql.Sprintf("INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402 , %llu , 303 , %llu )", urlLInkId.GetValue(), annotation3dElementId.GetValue());
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

        //Verify Insertions
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.ElementDrivesElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(5, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ONLY ts.ElementDrivesElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.InformationElementDrivesInformationElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.UrlLinkDrivesAnnotation3dElement"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(3, stmt.GetValueInt(0)) << stmt.GetECSql();
        stmt.Finalize();

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddMaxSharedColumnsBeforeOverflow)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract'>"
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
        "   <ECEntityClass typeName='Sub1' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_addmaxsharedcolumnsbeforeoverflow.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Adding MaxSharedColumnsBeforeOverflow");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteMaxSharedColumnsBeforeOverflow)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "             <MapStrategy>TablePerHierarchy</MapStrategy>"
        "         </ClassMap>"
        "         <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
        "         </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='GS' typeName='string' />"
        "       <ECProperty propertyName='GD' typeName='double' />"
        "       <ECProperty propertyName='GL' typeName='long' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Sub1' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_deletemaxsharedcolumnsbeforeoverflow.ecdb", schemaItem));
    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='Abstract'>"
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
        "   <ECEntityClass typeName='Sub1' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Deleting MaxSharedColumnsBeforeOverflow");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddingECEnumerationIntegerType)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        "   <ECEnumerator value = '2' displayLabel = 'exe' />"
        "   <ECEnumerator value = '3' displayLabel = 'dll' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    ECEnumerationCP updatedEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
    ASSERT_TRUE(updatedEnum != nullptr);
    ASSERT_STREQ("Test2Display", updatedEnum->GetDisplayLabel().c_str());
    ASSERT_STREQ("Test2Desc", updatedEnum->GetDescription().c_str());
    ASSERT_EQ(false, updatedEnum->GetIsStrict());
    ASSERT_EQ(PRIMITIVETYPE_Integer, updatedEnum->GetType());

    std::function<void(int32_t, Utf8CP)> assertEnumerator = [&](int32_t value, Utf8CP displayLabel)
        {
        ECEnumeratorCP newEnum = updatedEnum->FindEnumerator(value);
        ASSERT_TRUE(newEnum != nullptr);
        ASSERT_STREQ(displayLabel, newEnum->GetDisplayLabel().c_str());
        };

    assertEnumerator(0, "txt");
    assertEnumerator(1, "bat");
    assertEnumerator(2, "exe");
    assertEnumerator(3, "dll");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddingECEnumerationStringType)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = 't0' displayLabel = 'txt' />"
        "   <ECEnumerator value = 't1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>"
        "   <ECEnumerator value = 't0' displayLabel = 'txt' />"
        "   <ECEnumerator value = 't1' displayLabel = 'bat' />"
        "   <ECEnumerator value = 't2' displayLabel = 'exe' />"
        "   <ECEnumerator value = 't3' displayLabel = 'dll' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    ECEnumerationCP updatedEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
    ASSERT_TRUE(updatedEnum != nullptr);
    ASSERT_STREQ("Test2Display", updatedEnum->GetDisplayLabel().c_str());
    ASSERT_STREQ("Test2Desc", updatedEnum->GetDescription().c_str());
    ASSERT_EQ(false, updatedEnum->GetIsStrict());
    ASSERT_EQ(PRIMITIVETYPE_String, updatedEnum->GetType());

    std::function<void(Utf8CP, Utf8CP)> assertEnumerator = [&](Utf8CP value, Utf8CP displayLabel)
        {
        ECEnumeratorCP newEnum = updatedEnum->FindEnumerator(value);
        ASSERT_TRUE(newEnum != nullptr);
        ASSERT_STREQ(displayLabel, newEnum->GetDisplayLabel().c_str());
        };

    assertEnumerator("t0", "txt");
    assertEnumerator("t1", "bat");
    assertEnumerator("t2", "exe");
    assertEnumerator("t3", "dll");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECEnumerationFromStrictToNonStrictAndUpdateEnumerators)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>"
        "   <ECEnumerator value = '10' displayLabel = 'txt1' />"
        "   <ECEnumerator value = '11' displayLabel = 'bat1' />"
        "   <ECEnumerator value = '12' displayLabel = 'exe1' />"
        "   <ECEnumerator value = '13' displayLabel = 'dll1' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    ECEnumerationCP updatedEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
    ASSERT_TRUE(updatedEnum != nullptr);
    ASSERT_STREQ("Test2Display", updatedEnum->GetDisplayLabel().c_str());
    ASSERT_STREQ("Test2Desc", updatedEnum->GetDescription().c_str());
    ASSERT_EQ(false, updatedEnum->GetIsStrict());
    ASSERT_EQ(PRIMITIVETYPE_Integer, updatedEnum->GetType());

    std::function<void(int32_t, Utf8CP)> assertEnumerator = [&](int32_t value, Utf8CP displayLabel)
        {
        ECEnumeratorCP newEnum = updatedEnum->FindEnumerator(value);
        ASSERT_TRUE(newEnum != nullptr);
        ASSERT_STREQ(displayLabel, newEnum->GetDisplayLabel().c_str());
        };

    assertEnumerator(10, "txt1");
    assertEnumerator(11, "bat1");
    assertEnumerator(12, "exe1");
    assertEnumerator(13, "dll1");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECEnumerationFromUnStrictToStrict)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test2Display' description='Test2Desc'>"
        "   <ECEnumerator value = '10' displayLabel = 'txt1' />"
        "   <ECEnumerator value = '11' displayLabel = 'bat1' />"
        "   <ECEnumerator value = '12' displayLabel = 'exe1' />"
        "   <ECEnumerator value = '13' displayLabel = 'dll1' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot change IsStrict from false to true";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECEnumerationStrictEnumAddDeleteEnumerators)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test2Display' description='Test2Desc'>"
        "   <ECEnumerator value = '10' displayLabel = 'txt1' />"
        "   <ECEnumerator value = '11' displayLabel = 'bat1' />"
        "   <ECEnumerator value = '12' displayLabel = 'exe1' />"
        "   <ECEnumerator value = '13' displayLabel = 'dll1' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot change Strict Enum (Only Adding new properties allowed";
    }

TEST_F(SchemaUpgradeTestFixture, PropertyCategoryAddUpdateDelete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("getpropertycategories.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                        <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                                        <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                                        <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                                        <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                                        <ECEntityClass typeName="Foo" >
                                            <ECProperty propertyName="P1" typeName="double" category="C1" />
                                            <ECProperty propertyName="P2" typeName="double" category="C2" />
                                            <ECProperty propertyName="P3" typeName="double" category="C3" />
                                        </ECEntityClass>
                                    </ECSchema>)xml")));


    if (true)
        {
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

        PropertyCategoryCP c1 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C1");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C1", c1->GetName().c_str());
        ASSERT_EQ(1, (int) c1->GetPriority());

        PropertyCategoryCP c2 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C2");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C2", c2->GetName().c_str());
        ASSERT_EQ(2, (int) c2->GetPriority());

        PropertyCategoryCP c3 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C3");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C3", c3->GetName().c_str());
        ASSERT_EQ(3, (int) c3->GetPriority());

        PropertyCategoryCP c5 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C5");
        ASSERT_TRUE(c5 != nullptr);
        ASSERT_STREQ("C5", c5->GetName().c_str());
        ASSERT_EQ(5, (int) c5->GetPriority());

        ECSchemaCP schema1 = m_ecdb.Schemas().GetSchema("Schema1", false);
        ASSERT_TRUE(schema1 != nullptr);

        ECClassCP fooClass = m_ecdb.Schemas().GetClass("Schema1", "Foo");
        ASSERT_TRUE(fooClass != nullptr);
        ASSERT_EQ(4, schema1->GetPropertyCategoryCount());

        ECPropertyCP p1 = fooClass->GetPropertyP("P1");
        ASSERT_TRUE(p1 != nullptr);
        ASSERT_TRUE(p1->GetCategory() != nullptr);
        ASSERT_STREQ("C1", p1->GetCategory()->GetName().c_str());

        ECPropertyCP p2 = fooClass->GetPropertyP("P2");
        ASSERT_TRUE(p2 != nullptr);
        ASSERT_TRUE(p2->GetCategory() != nullptr);
        ASSERT_STREQ("C2", p2->GetCategory()->GetName().c_str());

        ECPropertyCP p3 = fooClass->GetPropertyP("P3");
        ASSERT_TRUE(p3 != nullptr);
        ASSERT_TRUE(p3->GetCategory() != nullptr);
        ASSERT_STREQ("C3", p3->GetCategory()->GetName().c_str());
        }

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                        <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                                        <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                                        <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                                        <PropertyCategory typeName="C4" description="C4" displayLabel="C4" priority="4" />
                                        <ECEntityClass typeName="Foo" >
                                            <ECProperty propertyName="P1" typeName="double" category="C4" />
                                            <ECProperty propertyName="P2" typeName="double" />
                                            <ECProperty propertyName="P3" typeName="double" category="C3" />
                                            <ECProperty propertyName="P4" typeName="double" category="C1" />
                                        </ECEntityClass>
                                    </ECSchema>)xml")));
        
    if (true)
        {
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

        PropertyCategoryCP c1 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C1");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C1", c1->GetName().c_str());
        ASSERT_EQ(1, (int) c1->GetPriority());

        PropertyCategoryCP c2 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C2");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C2", c2->GetName().c_str());
        ASSERT_EQ(2, (int) c2->GetPriority());

        PropertyCategoryCP c3 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C3");
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_STREQ("C3", c3->GetName().c_str());
        ASSERT_EQ(3, (int) c3->GetPriority());

        PropertyCategoryCP c4 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C4");
        ASSERT_TRUE(c4 != nullptr);
        ASSERT_STREQ("C4", c4->GetName().c_str());
        ASSERT_EQ(4, (int) c4->GetPriority());

        PropertyCategoryCP c5 = m_ecdb.Schemas().GetPropertyCategory("Schema1", "C5");
        ASSERT_TRUE(c5 == nullptr);

        ECSchemaCP schema1 = m_ecdb.Schemas().GetSchema("Schema1", false);
        ASSERT_TRUE(schema1 != nullptr);

        ECClassCP fooClass = m_ecdb.Schemas().GetClass("Schema1", "Foo");
        ASSERT_TRUE(fooClass != nullptr);
        ASSERT_EQ(4, schema1->GetPropertyCategoryCount());

        ECPropertyCP p1 = fooClass->GetPropertyP("P1");
        ASSERT_TRUE(p1 != nullptr);
        ASSERT_TRUE(p1->GetCategory() != nullptr);
        ASSERT_STREQ("C4", p1->GetCategory()->GetName().c_str());

        ECPropertyCP p2 = fooClass->GetPropertyP("P2");
        ASSERT_TRUE(p2 != nullptr);
        ASSERT_TRUE(p2->GetCategory() == nullptr);

        ECPropertyCP p3 = fooClass->GetPropertyP("P3");
        ASSERT_TRUE(p3 != nullptr);
        ASSERT_TRUE(p3->GetCategory() != nullptr);
        ASSERT_STREQ("C3", p3->GetCategory()->GetName().c_str());;

        ECPropertyCP p4 = fooClass->GetPropertyP("P4");
        ASSERT_TRUE(p4 != nullptr);
        ASSERT_TRUE(p4->GetCategory() != nullptr);
        ASSERT_STREQ("C1", p4->GetCategory()->GetName().c_str());
        }

    }
END_ECDBUNITTESTS_NAMESPACE
