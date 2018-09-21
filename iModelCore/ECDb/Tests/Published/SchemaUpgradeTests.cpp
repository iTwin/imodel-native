/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SchemaUpgradeTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>

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
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
            ASSERT_TRUE(m_ecdb.IsDbOpen());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Muhammad.Hassan                     06/16
        //+---------------+---------------+---------------+---------------+---------------+------
        DbResult OpenBesqliteDb(Utf8CP dbPath) { return m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)); }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Muhammad.Hassan                     06/16
        //+---------------+---------------+---------------+---------------+---------------+------
        void AssertSchemaUpdate(Utf8CP schemaXml, BeFileName seedFilePath, std::pair<bool, bool> const& expectedToSucceedList, Utf8CP assertMessage)
            {
            //test 1: unrestricted ECDb
            ECDb ecdb;
            ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, "schemaupgrade_unrestricted.ecdb", seedFilePath));

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
            ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, "schemaupgrade_disallowmajorschemachange.ecdb", seedFilePath));

            expectedToSucceed = expectedToSucceedList.second;
            assertMessageFull.assign("[schema import with disallowed major schema changes] ").append(assertMessage);

            if (expectedToSucceed)
                ASSERT_EQ(SUCCESS, TestHelper(ecdb).ImportSchema(schemaItem, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << assertMessageFull.c_str();
            else
                ASSERT_EQ(ERROR, TestHelper(ecdb).ImportSchema(schemaItem, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << assertMessageFull.c_str();

            if (expectedToSucceed)
                m_updatedDbs.push_back((Utf8String) ecdb.GetDbFileName());

            ecdb.CloseDb();
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
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateECSchemaAttributes.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>")));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                           "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                           "</ECSchema>"))) << "Modifying display label and description is expected to be supported";

    ASSERT_EQ(JsonValue(R"json([{"DisplayLabel":"New Test Schema", "Description":"This is a New Test Schema", "Alias":"ts"}])json"), GetHelper().ExecuteSelectECSql("SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'")) << "After modifying display label and description";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                           "<ECSchema schemaName='TestSchema' alias='ts2' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                           "</ECSchema>"))) << "Modifying alias is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifySchemaVersion)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaUpgrade_ModifySchemaVersion.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='10.11.9' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing minor version when write version was incremented is supported";

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='11.10.9' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing write version when read version was incremented is supported";

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='12.10.8' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing minor version when read version was incremented is supported";

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='13.1.7' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing minor and write version when read version was incremented is supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='12.1.7' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing read version is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='13.0.7' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing write version is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='13.1.6' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Decreasing minor version is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     02/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ECVersions)
    {
    auto verifySchemaVersion = [] (ECDbCR ecdb, Utf8CP schemaName, uint32_t expectedOriginalXmlVersionMajor, uint32_t expectedOriginalXmlVersionMinor)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT OriginalECXmlVersionMajor, OriginalECXmlVersionMinor FROM meta.ECSchemaDef WHERE Name=?"));
        stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(expectedOriginalXmlVersionMajor, (uint32_t) stmt.GetValueInt(0));
        ASSERT_EQ(expectedOriginalXmlVersionMinor, (uint32_t) stmt.GetValueInt(1));
        };

    ASSERT_EQ(SUCCESS, SetupECDb("SchemaOriginalECXmlVersion.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>")));
    verifySchemaVersion(m_ecdb, "TestSchema", 3, 0);

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>")));
    verifySchemaVersion(m_ecdb, "TestSchema", 3, 1);

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "</ECSchema>")));
    verifySchemaVersion(m_ecdb, "TestSchema", 3, 2);

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "</ECSchema>"))) << "Downgrade of ECXml version is not supported";
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECClassAttributes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateECClassAttributes.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                             "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                            "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='Test Class' modifier='None' />"
                                                                             "</ECSchema>")));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                           "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                           "   <ECEntityClass typeName='TestClass' displayLabel='My Test Class' description='My Test Class' modifier='None' />"
                                                           "</ECSchema>"))) << "Modifying ECClass display label and description is expected to be supported";

    ASSERT_EQ(JsonValue(R"json([{"DisplayLabel":"My Test Class", "Description":"My Test Class"}])json"), GetHelper().ExecuteSelectECSql("SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'")) << "After modifying display label and description";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddingUpdatingAndDeletingMinMax)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='P1' typeName='long'   minimumValue  = '1'     maximumValue   = '200'    />"
        "       <ECProperty propertyName='P2' typeName='double' minimumValue  = '1.22'  maximumValue   = '100.22' />"
        "       <ECProperty propertyName='P3' typeName='string' minimumLength = '1'     maximumLength  = '1000'   />"
        "       <ECProperty propertyName='P4' typeName='long'   maximumValue  = '1200'                            />"
        "       <ECProperty propertyName='P5' typeName='double' maximumValue  = '1200.12'                         />"
        "       <ECProperty propertyName='P6' typeName='string' maximumLength = '1000'                            />"
        "   </ECEntityClass>"
        "</ECSchema>");


    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_minMax.ecdb", schemaItem));
    {
    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_NE(nullptr, foo);

    ECValue minV, maxV;
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P1")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P1")->GetMaximumValue(maxV));
    ASSERT_EQ(1, minV.GetLong());
    ASSERT_EQ(200, maxV.GetLong());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P2")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P2")->GetMaximumValue(maxV));
    ASSERT_EQ(1.22, minV.GetDouble());
    ASSERT_EQ(100.22, maxV.GetDouble());

    ASSERT_EQ(1, foo->GetPropertyP("P3")->GetMinimumLength());
    ASSERT_EQ(1000, foo->GetPropertyP("P3")->GetMaximumLength());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P4")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMaximumValue(maxV));
    ASSERT_TRUE(minV.IsNull());
    ASSERT_EQ(1200, maxV.GetLong());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P5")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMaximumValue(maxV));
    ASSERT_TRUE(minV.IsNull());
    ASSERT_EQ(1200.12, maxV.GetDouble());

    ASSERT_EQ(0, foo->GetPropertyP("P6")->GetMinimumLength());
    ASSERT_EQ(1000, foo->GetPropertyP("P6")->GetMaximumLength());
    }

    ReopenECDb();
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='P1' typeName='long'   />"
        "       <ECProperty propertyName='P2' typeName='double' />"
        "       <ECProperty propertyName='P3' typeName='string' />"
        "       <ECProperty propertyName='P4' typeName='long'   minimumValue  = '12'   maximumValue   = '2200'    />"
        "       <ECProperty propertyName='P5' typeName='double' minimumValue  = '1.33' maximumValue   = '2200.12' />"
        "       <ECProperty propertyName='P6' typeName='string' minimumLength = '11'    maximumLength  = '9000'    />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    ReopenECDb();
    {
    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_NE(nullptr, foo);

    ECValue minV, maxV;
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P1")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P1")->GetMaximumValue(maxV));
    ASSERT_TRUE(minV.IsNull());
    ASSERT_TRUE(maxV.IsNull());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P2")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P2")->GetMaximumValue(maxV));
    ASSERT_TRUE(minV.IsNull());
    ASSERT_TRUE(maxV.IsNull());

    ASSERT_EQ(0, foo->GetPropertyP("P3")->GetMinimumLength());
    ASSERT_EQ(0, foo->GetPropertyP("P3")->GetMaximumLength());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMaximumValue(maxV));
    ASSERT_EQ(12, minV.GetLong());
    ASSERT_EQ(2200, maxV.GetLong());

    minV = maxV = ECValue();
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMinimumValue(minV));
    ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMaximumValue(maxV));
    ASSERT_EQ(1.33, minV.GetDouble());
    ASSERT_EQ(2200.12, maxV.GetDouble());

    ASSERT_EQ(11, foo->GetPropertyP("P6")->GetMinimumLength());
    ASSERT_EQ(9000, foo->GetPropertyP("P6")->GetMaximumLength());
    }

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddingUpdatingAndDeletingPriority) 
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='P1' typeName='string' priority='1010' />"
        "       <ECProperty propertyName='P2' typeName='string' priority='1020' />"
        "       <ECProperty propertyName='P3' typeName='string' priority='1030' />"
        "       <ECProperty propertyName='P4' typeName='string' />"
        "       <ECProperty propertyName='P5' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_priority.ecdb", schemaItem));
    {
    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_NE(nullptr, foo);
    ASSERT_EQ(1010, foo->GetPropertyP("P1")->GetPriority());
    ASSERT_EQ(1020, foo->GetPropertyP("P2")->GetPriority());
    ASSERT_EQ(1030, foo->GetPropertyP("P3")->GetPriority());
    ASSERT_EQ(0, foo->GetPropertyP("P4")->GetPriority());
    ASSERT_EQ(0, foo->GetPropertyP("P5")->GetPriority());
    }
    ReopenECDb();
    //import edited schema with some changes.
    SchemaItem editedSchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='P1' typeName='string' priority='2010' />"
        "       <ECProperty propertyName='P2' typeName='string' priority='2020' />"
        "       <ECProperty propertyName='P3' typeName='string'                 />"
        "       <ECProperty propertyName='P4' typeName='string' priority='1040' />"
        "       <ECProperty propertyName='P5' typeName='string' priority='1050' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    {
    ECClassCP foo = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_NE(nullptr, foo);
    ASSERT_EQ(2010, foo->GetPropertyP("P1")->GetPriority());
    ASSERT_EQ(2020, foo->GetPropertyP("P2")->GetPriority());
    ASSERT_EQ(0, foo->GetPropertyP("P3")->GetPriority());
    ASSERT_EQ(1040, foo->GetPropertyP("P4")->GetPriority());
    ASSERT_EQ(1050, foo->GetPropertyP("P5")->GetPriority());
    }
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
        //"       <ECProperty propertyName='P2' typeName='string' />"
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
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    {
    ECClassCP supportOption = m_ecdb.Schemas().GetClass("TestSchema", "SupportOption");
    ASSERT_NE(supportOption, nullptr);
    ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
    ASSERT_EQ(1, supportOption->GetBaseClasses().size());
    }
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
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateECClassAttributes.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                            "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                            "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
                                                                            "       <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='Test Property' typeName='string' />"
                                                                            "   </ECEntityClass>"
                                                                            "</ECSchema>")));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                           "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                           "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >"
                                                           "       <ECProperty propertyName='TestProperty' displayLabel='My Test Property' description='My Test Property' typeName='string' />"
                                                           "   </ECEntityClass>"
                                                           "</ECSchema>"))) << "Modifying ECProperty display label and description is expected to be supported";

    ASSERT_EQ(JsonValue(R"json([{"DisplayLabel":"My Test Property", "Description":"My Test Property"}])json"), GetHelper().ExecuteSelectECSql("SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'")) << "After modifying display label and description";
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
// @bsimethod                                   Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UnsealingClasses)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("UnsealingClasses.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECProperty propertyName="Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "index on sealed class";

            EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Foo" modifier="None">
                    <ECProperty propertyName="Prop" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub" modifier="None">
                    <BaseClass>Foo</BaseClass>
                    <ECProperty propertyName="Prop" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml"))) << "Class modifier changed from Sealed to None";
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("UnsealingClasses.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="None">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Sealed">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "sealed sub class (TPH)";

            EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="None">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="None">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"))) << "Unsealing subclass (TPH)";
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
    AssertSchemaUpdate(editedSchemaItem, filePath, {true, false}, "Addition or deletion of virtual column");

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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

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

        //Verify class and Property accessible
        ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT TestProperty FROM ts.TestClass");

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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addNewEntityClass, filePath, {true, true}, "Adding New Entity Class");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));

        //verify tables
        //new class should be added with new namespace prefix
        ASSERT_TRUE(GetHelper().TableExists("ts_TestClass"));

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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts.TestClass"));
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

    RestrictedSchemaImportECDb restrictedECDb(false);
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

    RestrictedSchemaImportECDb restrictedECDb(false);
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts.TestClass"));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts.NewTestClass"));
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
        "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
        "       <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addCAOnClass, filePath, {true, true}, "Adding new CA instance");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts.TestClass"));
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
        "<ECSchema schemaName='TestSchema' alias='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />"
        "   <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>"
        "                <PropertyName>LastMod</PropertyName>"
        "            </ClassHasCurrentTimeStampProperty>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(addCAOnClass, filePath, {true, true}, "Adding new CA instance");

    for (Utf8StringCR dbPath : m_updatedDbs)
        {
        ASSERT_EQ(BE_SQLITE_OK, OpenBesqliteDb(dbPath.c_str()));
        ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(testSchema != nullptr);
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
        ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
        ASSERT_STREQ("modified test class", statement.GetValueText(1));

        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ts.TestClass"));
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
TEST_F(SchemaUpgradeTestFixture, AddPropertyToBaseClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddPropertyToBaseClass.ecdb", SchemaItem(
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
// @bsimethod                                   Krischan.Eberle                09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_SharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_SharedColumns.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Base" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row1, sub2Row1;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Prop1,Prop2) VALUES (10,'1-1', '1-2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row1, "INSERT INTO ts.Sub2(Code,PropA,PropB) VALUES (20,'1-A', 1)"));
    m_ecdb.SaveChanges();

    //Add property to Sub1
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Base" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row2, "INSERT INTO ts.Sub1(Code,Prop1,Prop2,Prop3) VALUES (11,'2-1', '2-2', 2)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":10,"Prop1":"1-1","Prop2":"1-2"},{"Code":11,"Prop1":"2-1","Prop2":"2-2","Prop3":2}])json"), GetHelper().ExecuteSelectECSql("SELECT Code, Prop1, Prop2, Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":20,"PropA":"1-A","PropB":1}])json"), GetHelper().ExecuteSelectECSql("SELECT Code, PropA, PropB FROM ts.Sub2"));

    //Add property to BAse
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Base" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    ECInstanceKey sub1Row3,sub2Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row3, "INSERT INTO ts.Sub1(Code,Name,Prop1,Prop2,Prop3) VALUES (12,'Object 12', '3-1', '3-2', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row2, "INSERT INTO ts.Sub2(Code,Name,PropA,PropB) VALUES (21,'Object 21', '2-A', 2)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":10,"Prop1":"1-1","Prop2":"1-2"},{"Code":11,"Prop1":"2-1","Prop2":"2-2","Prop3":2},{"Code":12, "Name":"Object 12","Prop1":"3-1","Prop2":"3-2","Prop3":3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code, Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":20,"PropA":"1-A","PropB":1},{"Code":21,"Name":"Object 21","PropA":"2-A","PropB":2}])json"), GetHelper().ExecuteSelectECSql("SELECT Code, Name, PropA, PropB FROM ts.Sub2"));

    // Expected Mapping:
    //          ts_Base  
    //          ps1    ps2     ps3     ps4     ps5
    //Sub1      Code   Prop1   Prop2   Prop3   Name
    //Sub2      Code   PropA   PropB           Name

    ASSERT_EQ(ExpectedColumn("ts_Base", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Base", "Code")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps5"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Base", "Name")));

    ASSERT_EQ(ExpectedColumn("ts_Base","ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Code")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps5"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop1")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop2")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps4"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop3")));

    ASSERT_EQ(ExpectedColumn("ts_Base", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Code")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps5"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_Base", "ps3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropB")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row1, sub2Row1;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row1, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
    m_ecdb.SaveChanges();

    //Add property to Sub1
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));

    //Add property to SubBase
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
                <ECProperty propertyName="Kind" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    ECInstanceKey sub1Row3, sub2Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row3, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row2, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
            {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
            {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));

    // Expected Mapping:
    //          ts_Element  ts_Geometric2dElement               
    //          Code        Origin_X  Origin_Y  js1     js2     js3     js3     js4
    //Sub1      Code        Origin.X  Origin.Y  Name    Prop1   Prop2   Prop3   Kind
    //Sub2      Code        Origin.X  Origin.Y  Name    PropA   PropB           Kind

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Code")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Geometric2dElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}), 
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Geometric2dElement", "Origin")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "PhysicalElement", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Name")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub1", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js5"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Kind")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop1")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop2")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js4"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop3")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub2", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js5"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Kind")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropB")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AllAddedPropsToOverflow)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AllAddedPropsToOverflow.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row1, sub2Row1;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row1, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
    m_ecdb.SaveChanges();

    //Add property to Sub1
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));

    //Add property to SubBase
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
                <ECProperty propertyName="Kind" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    ECInstanceKey sub1Row3, sub2Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row3, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row2, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
            {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
            {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));

    // Expected Mapping:
    //          ts_Element  ts_Geometric2dElement               ts_Geometric2dElement_Overflow
    //          Code        Origin_X  Origin_Y  js1     js2     os1     os2     os3
    //Sub1      Code        Origin.X  Origin.Y  Name    Prop1   Prop2   Prop3   Kind
    //Sub2      Code        Origin.X  Origin.Y  Name    PropA   PropB           Kind

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Code")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Geometric2dElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Geometric2dElement", "Origin")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "PhysicalElement", "Origin")));

    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Name"))) << "subclass of sharedcolumns holder";

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub1", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Kind"))) << "mapped to overflow table as it is the property added at last";
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop1")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop2")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop3")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub2", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Kind")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropB")));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AddedBasePropToOverflow)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AddedBasePropToOverflow.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row1, sub2Row1;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row1, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
    m_ecdb.SaveChanges();

    //Add property to Sub1
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey sub1Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row1, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));

    //Add property to SubBase
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
           <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <BaseClass>GeometricElement</BaseClass>
                <ECProperty propertyName="Origin" typeName="Point2d" />
           </ECEntityClass>
           <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                <BaseClass>Geometric2dElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
           </ECEntityClass>
           <ECEntityClass typeName="SubBase" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
                <ECProperty propertyName="Kind" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub1" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="string" />
                <ECProperty propertyName="Prop3" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub2" modifier="None" >
              <BaseClass>SubBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string" />
                <ECProperty propertyName="PropB" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    ECInstanceKey sub1Row3, sub2Row2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Row3, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub2Row2, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
            {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
            {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
    ASSERT_EQ(JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"), GetHelper().ExecuteSelectECSql("SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));

    // Expected Mapping:
    //          ts_Element  ts_Geometric2dElement                               ts_Geometric2dElement_Overflow
    //          Code        Origin_X  Origin_Y  js1     js2     js3     js4     os1
    //Sub1      Code        Origin.X  Origin.Y  Name    Prop1   Prop2   Prop3   Kind
    //Sub2      Code        Origin.X  Origin.Y  Name    PropA   PropB           Kind

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Code")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Geometric2dElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Geometric2dElement", "Origin")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "PhysicalElement", "Origin")));

    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "PhysicalElement", "Name"))) << "subclass of sharedcolumns holder";

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub1", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Kind"))) << "mapped to overflow table as it is the property added at last";
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop1")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop2")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js4"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub1", "Prop3")));

    ASSERT_EQ(ExpectedColumn("ts_Element", "Code"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Code")));
    ASSERT_EQ(std::vector<ExpectedColumn>({ExpectedColumn("ts_Geometric2dElement", "Origin_X"), ExpectedColumn("ts_Geometric2dElement", "Origin_Y")}),
              GetHelper().GetPropertyMapColumns(AccessString("ts", "Sub2", "Origin")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Name")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement_Overflow", "os1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "Kind")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_Geometric2dElement", "js3"), GetHelper().GetPropertyMapColumn(AccessString("ts", "Sub2", "PropB")));
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
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, false}, "Add Delete Property mapped to shared column");

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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty, NewTestProperty FROM ts.TestClass"));
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        ASSERT_TRUE(testSchema->GetAlias() == "ts");
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
        ASSERT_STREQ("ts", statement.GetValueText(2));

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
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT TestProperty FROM ts.TestClass"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("updateStartupCompanyschema.ecdb", SchemaItem::CreateForFile("DSCacheSchema.01.00.00.ecschema.xml")));
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem::CreateForFile("DSCacheSchema.01.00.03.ecschema.xml")));
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
        "   <ECEntityClass typeName='Base' modifier='None'>"
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
        "   <ECEntityClass typeName='Sub' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    //following table should exist.
    ASSERT_TRUE(GetHelper().TableExists("ts_Base"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Base"), nullptr);

    //Following table should not exist
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Sub"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Sub"));

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Base"));
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Base_Overflow"));

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test4', 13.3, 2345)");
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    //Delete Foo ===================================================================================================
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='None'>"
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
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Sub"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Sub"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Base"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Base"), nullptr);

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Base")) << "After deleting subclass Foo";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::InvalidECSql, BE_SQLITE_ERROR, "SELECT FS, FD, FL FROM ts.Sub");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Base");

    //Delete Base ===================================================================================================
    //test that the index definitions in ec_Index are cleaned up when a table is deleted
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ec_Index i JOIN ec_Table t ON i.TableId=t.Id WHERE t.Name LIKE 'ts_Base' COLLATE NOCASE"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetSql();
    stmt.Reset();

    //Deleting Class with ShareColumns is expected to be supported
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "</ECSchema>"))) << "Delete class containing ECDbMap CA should be successful";

    //Following should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Base"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetSql();
    stmt.Finalize();

    //Add Base Again===================================================================================================
    //Add Class with SharedTable:SharedColumns is expected to be supported
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='None'>"
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
        "</ECSchema>"))) << "Add New Class with ECDbMap CA is expected to be successful";

    //Following should not exist
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Sub"), nullptr);
    ASSERT_FALSE(GetHelper().TableExists("ts_Sub"));

    //Following should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Base"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Base"), nullptr);

    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Base")) << "After deleting all classes and readding base class";
    
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test3', 44.32, 3344)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test4', 13.3, 2345)");
    m_ecdb.SaveChanges();

    //Add Sub Again===============================================================================================
    //Adding new derived entity class is expected to be supported
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Base' modifier='None'>"
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
        "   <ECEntityClass typeName='Sub' modifier='None'>"
        "       <BaseClass>Base</BaseClass>"
        "       <ECProperty propertyName='FS' typeName='string' />"
        "       <ECProperty propertyName='FD' typeName='double' />"
        "       <ECProperty propertyName='FL' typeName='long' />"
        "       <ECProperty propertyName='FI' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>"))) << "New derived entity class is expected to be supported";

    //Table should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Sub"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Sub"), nullptr);

    //Table should exist
    ASSERT_TRUE(GetHelper().TableExists("ts_Base"));
    ASSERT_NE(m_ecdb.Schemas().GetClass("TestSchema", "Base"), nullptr);

    ASSERT_EQ(6, GetHelper().GetColumnCount("ts_Base")) << "After readding subclass";
    ASSERT_EQ(5, GetHelper().GetColumnCount("ts_Base_Overflow")) << "After readding subclass";

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "SELECT FS, FD, FL FROM ts.Sub");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_ROW, "SELECT GS, GD, GL FROM ts.Base");

    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)");
    m_ecdb.SaveChanges();
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
        "<ECSchema schemaName='TestSchema' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
        "<ECSchema schemaName='TestSchema' alias='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
        "<ECSchema schemaName='TestSchema' alias='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' alias = 'ecdbmap' />"
        "</ECSchema>");
    ASSERT_EQ(SUCCESS, ImportSchema(deleteParent)) << "Deleting Class with CA  JoinedTablePerDirectSubClass,SharedColumnForSubClasses is expected to be supported";

    //Parent should not exist
    ASSERT_FALSE(GetHelper().TableExists("ts_Parent"));

    //Add Parent ===================================================================================================
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
        "<ECSchema schemaName='TestSchema' alias='ts' version='6.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
        "<ECSchema schemaName='TestSchema' alias='ts' version='7.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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

    AssertSchemaUpdate(schemaWithDeletedConstraintClass, filePath, {true, false}, "Deleting subclass of ECRel ConstraintClass");
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
    AssertSchemaUpdate(schemaWithDeletedConstraintClass, filePath, {true, false}, "delete subclass of abstract rel constraint class");

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
        "    <ECEntityClass typeName='B' modifier='Sealed'>"
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
        "    <ECEntityClass typeName='B' modifier='Sealed'>"
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
    AssertSchemaUpdate(schemaWithIndexNameModified, filePath, {false, false}, "Modifying DbIndex::Name");

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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "   </ECEntityClass >"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithDeletedBaseClass)) << "Deleting Base Class not allowed";

    SchemaItem schemaWithModifedBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='TestClass0' modifier='None' />"
        "   <ECEntityClass typeName='TestClass' modifier='None' />"
        "   <ECEntityClass typeName='Sub' modifier='None' >"
        "       <BaseClass>TestClass0</BaseClass>"
        "   </ECEntityClass >"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(schemaWithModifedBaseClass)) << "Modifying Base Class not allowed";

    SchemaItem schemaWithNewBaseClass(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
    ASSERT_EQ(SUCCESS, SetupECDb("DeleteExistingECEnumeration.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>")));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>"))) << "Deletion of ECEnumeration is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECEnumeratorsOfPreEC32Enum)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyECEnumeratorsOfPreEC32Enum.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "EC3 Enum";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' />"
        "   <ECEnumerator value = '1' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Deleting enumerator display label";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Deleting enumerator";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='TxtFile' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='LogFile' value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Changing enumerator names when the old schema originates from pre EC3.2 is allowed";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Importing a pre EC3.2 schema as the old schemas's enumerator names were already changed away from the default name";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECEnumeratorsOfEC32Enum)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyECEnumeratorsOfEC32Enum.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Txt' value = '0' displayLabel = 'txt' description='TXT Files'/>"
        "   <ECEnumerator name='Log' value = '1' displayLabel = 'log' description='LOG Files'/>"
        " </ECEnumeration>"
        "</ECSchema>"))) << "EC3.2 Enum";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Txt' value = '0' description='TXT Files'/>"
        "   <ECEnumerator name='Log' value = '1' description='LOG Files'/>"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Deleting enumerator display label";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Txt' value = '0' displayLabel = 'txt'/>"
        "   <ECEnumerator name='Log' value = '1' displayLabel = 'log'/>"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Deleting enumerator description";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Txt' value = '0' displayLabel = 'txt' description='TXT Files'/>"
        "   <ECEnumerator name='Log2' value = '1' displayLabel = 'log' description='LOG Files'/>"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Modifying enumerator name is not supported";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Log' value = '1' displayLabel = 'log' description='LOG Files' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Deleting enumerator";


    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='Txt' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='Log' value = '1' displayLabel = 'log' />"
        "   <ECEnumerator name='Csv' value = '2' displayLabel = 'csv' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Adding enumerator";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'log' />"
        "   <ECEnumerator value = '2' displayLabel = 'csv' />"
        "   <ECEnumerator value = '3' displayLabel = 'cpp' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Adding enumerator with EC3.2 schema should fail because existing enumerator names will not match";

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  02/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyECEnumeratorNames)
    {
    //starting with pre EC3.2 schema
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyECEnumeratorNames.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "EC3 Enum";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='TxtFile' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='LogFile' value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Changing enumerator names when the old schema originates from pre EC3.2 is allowed";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='TxtFile1' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='LogFile' value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Changing the names again after it was upgraded to EC3.2 is not allowed";

    //starting with EC3.2 schema
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyECEnumeratorNames.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='TxtFile' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='LogFile' value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "EC3.2 Enum";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        " <ECEnumeration typeName='MyEnum' backingTypeName='int' isStrict='True'>"
        "   <ECEnumerator name='TxtFile1' value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator name='LogFile' value = '1' displayLabel = 'log' />"
        " </ECEnumeration>"
        "</ECSchema>"))) << "Changing the names in an EC3.2 schema is not supported";
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
// @bsimethod                                  Krischan.Eberle                   02/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ExtendedTypeName)
    {
    auto assertExtendedType = [] (ECDbCR ecdb, Utf8CP propName, Utf8CP expectedExtendedTypeName)
        {
        ECClassCP fooClass = ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(fooClass != nullptr);
        ECPropertyCP prop = fooClass->GetPropertyP(propName);
        ASSERT_TRUE(prop != nullptr);
        if (expectedExtendedTypeName == nullptr)
            ASSERT_FALSE(prop->HasExtendedType()) << propName;
        else
            {
            if (prop->GetIsPrimitive())
                ASSERT_STREQ(expectedExtendedTypeName, prop->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << propName;
            else if (prop->GetIsPrimitiveArray())
                ASSERT_STREQ(expectedExtendedTypeName, prop->GetAsPrimitiveArrayProperty()->GetExtendedTypeName().c_str()) << propName;
            else
                FAIL() << propName << " Expected extended type name: " << expectedExtendedTypeName;
            }
        };

    ASSERT_EQ(SUCCESS, SetupECDb("ModifyExtendedTypeName.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo" >
                   <ECProperty propertyName="noext" typeName="string" />
                   <ECProperty propertyName="ext" typeName="string" extendedTypeName="url" />
                   <ECArrayProperty propertyName="noext_array" typeName="string"/>
                   <ECArrayProperty propertyName="ext_array" typeName="string" extendedTypeName="email"/>
                </ECEntityClass>
              </ECSchema>)xml")));

    assertExtendedType(m_ecdb, "noext", nullptr);
    assertExtendedType(m_ecdb, "ext", "url");
    assertExtendedType(m_ecdb, "noext_array", nullptr);
    assertExtendedType(m_ecdb, "ext_array", "email");

    //add extended type name
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo" >
                   <ECProperty propertyName="noext" typeName="string" extendedTypeName="json" />
                   <ECProperty propertyName="ext" typeName="string" extendedTypeName="url" />
                   <ECArrayProperty propertyName="noext_array" typeName="string" extendedTypeName="xml"/>
                   <ECArrayProperty propertyName="ext_array" typeName="string" extendedTypeName="email"/>
                </ECEntityClass>
              </ECSchema>)xml")));

    assertExtendedType(m_ecdb, "noext", "json");
    assertExtendedType(m_ecdb, "ext", "url");
    assertExtendedType(m_ecdb, "noext_array", "xml");
    assertExtendedType(m_ecdb, "ext_array", "email");

    //modify extended type name
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo" >
                   <ECProperty propertyName="noext" typeName="string" extendedTypeName="json" />
                   <ECProperty propertyName="ext" typeName="string" extendedTypeName="http" />
                   <ECArrayProperty propertyName="noext_array" typeName="string" extendedTypeName="xml"/>
                   <ECArrayProperty propertyName="ext_array" typeName="string" extendedTypeName="mail"/>
                </ECEntityClass>
              </ECSchema>)xml")));

    assertExtendedType(m_ecdb, "noext", "json");
    assertExtendedType(m_ecdb, "ext", "http");
    assertExtendedType(m_ecdb, "noext_array", "xml");
    assertExtendedType(m_ecdb, "ext_array", "mail");

    //remove extended type name
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo" >
                   <ECProperty propertyName="noext" typeName="string" />
                   <ECProperty propertyName="ext" typeName="string" />
                   <ECArrayProperty propertyName="noext_array" typeName="string"/>
                   <ECArrayProperty propertyName="ext_array" typeName="string"/>
                </ECEntityClass>
              </ECSchema>)xml")));

    assertExtendedType(m_ecdb, "noext", nullptr);
    assertExtendedType(m_ecdb, "ext", nullptr);
    assertExtendedType(m_ecdb, "noext_array", nullptr);
    assertExtendedType(m_ecdb, "ext_array", nullptr);
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
// @bsimethod                                   Affan Khan                     05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteECCustomAttributeClass_Complex)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='T' nameSpacePrefix='T' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributeClass typeName = 'C01' appliesTo = 'Schema'>                      <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C02' appliesTo = 'EntityClass'>                 <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C03' appliesTo = 'CustomAttributeClass'>        <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C04' appliesTo = 'StructClass'>                 <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C05' appliesTo = 'RelationshipClass'>           <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C06' appliesTo = 'AnyClass'>                    <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C07' appliesTo = 'StructProperty'>              <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C08' appliesTo = 'ArrayProperty'>               <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C09' appliesTo = 'StructArrayProperty'>         <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C10' appliesTo = 'NavigationProperty'>          <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C11' appliesTo = 'AnyProperty'>                 <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C12' appliesTo = 'SourceRelationshipConstraint'><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C13' appliesTo = 'TargetRelationshipConstraint'><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C14' appliesTo = 'AnyRelationshipConstraint'>   <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C15' appliesTo = 'Any'>                         <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate_customAttributes.ecdb", schemaItem));
    std::map<Utf8String, ECClassId> caClasses;
    for (ECClassCP ecClass : m_ecdb.Schemas().GetSchema("T")->GetClasses())
        if (ecClass->IsCustomAttributeClass())
            caClasses[ecClass->GetName()] = ecClass->GetId();

    ReopenECDb();
    //<ECSchemaReference name="T" version="01.00.00" alias="T" />
    //<ECCustomAttributes><C01 xmlns='T.01.00'><S>Value</S></C01></ECCustomAttributes>
    SchemaItem s1(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='T' nameSpacePrefix='T' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECCustomAttributes><C01 xmlns='T.01.00'><S>test1_c01</S></C01></ECCustomAttributes>"
        "   <ECCustomAttributeClass typeName = 'C01' appliesTo = 'Schema'>                      <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c01</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C02' appliesTo = 'EntityClass'>                 <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c02</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C03' appliesTo = 'CustomAttributeClass'>                                                                                            <ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C04' appliesTo = 'StructClass'>                 <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c04</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C05' appliesTo = 'RelationshipClass'>           <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c05</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C06' appliesTo = 'AnyClass'>                    <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c06</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C07' appliesTo = 'StructProperty'>              <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c07</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C08' appliesTo = 'ArrayProperty'>               <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c08</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C09' appliesTo = 'StructArrayProperty'>         <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c09</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C10' appliesTo = 'NavigationProperty'>          <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c10</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C11' appliesTo = 'AnyProperty'>                 <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c11</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C12' appliesTo = 'SourceRelationshipConstraint'><ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c12</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C13' appliesTo = 'TargetRelationshipConstraint'><ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c13</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C14' appliesTo = 'AnyRelationshipConstraint'>   <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c14</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "   <ECCustomAttributeClass typeName = 'C15' appliesTo = 'Any'>                         <ECCustomAttributes><C03 xmlns='T.01.00'><S>test1_c15</S></C03></ECCustomAttributes><ECProperty propertyName = 'S' typeName = 'string' /></ECCustomAttributeClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, ImportSchema(s1));
    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT  NULL FROM ec_Class WHERE Id = ?");
    for (auto const& kp : caClasses)
        {
        stmt->ClearBindings();
        stmt->Reset();
        stmt->BindId(1, kp.second);
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        }
    }

    SchemaItem s2(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='T1' nameSpacePrefix='T1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='T' version='01.00.00' prefix='T' />"
        "   <ECCustomAttributes><C01 xmlns='T.01.00'><S>test1_t1</S></C01></ECCustomAttributes>"
        "   <ECEntityClass typeName='TestClass' modifier='None' >"
        "        <ECCustomAttributes>"
        "           <C02 xmlns = 'T.01.00'><S>entity_Class</S></C02>"
        "           <C06 xmlns = 'T.01.00'><S>any_class</S></C06>"
        "           <C15 xmlns = 'T.01.00'><S>any</S></C15>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='prop' typeName='boolean' >"
        "        <ECCustomAttributes>"
        "           <Localizable xmlns='CoreCustomAttributes.01.00'/>"
        "           <C11 xmlns = 'T.01.00'><S>AnyProperty</S></C11>"
        "           <C11 xmlns = 'T.01.00'><S>AnyProperty</S></C11>"
        "        </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, ImportSchema(s2));
    {
    int customAttributeInstances = 0;
    auto stmt = m_ecdb.GetCachedStatement("SELECT  NULL FROM ec_CustomAttribute WHERE ClassId = ?");
    for (auto const& kp : caClasses)
        {
        stmt->ClearBindings();
        stmt->Reset();
        stmt->BindId(1, kp.second);
        while (stmt->Step() == BE_SQLITE_ROW)
            customAttributeInstances++;
        }
    ASSERT_EQ(20, customAttributeInstances);
    }

    ReopenECDb();
    SchemaItem s3(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='T' nameSpacePrefix='T' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>");
    
    ASSERT_EQ(SUCCESS, ImportSchema(s3));
    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT  NULL FROM ec_Class WHERE Id = ?");
    for (auto const& kp : caClasses)
        {
        stmt->ClearBindings();
        stmt->Reset();
        stmt->BindId(1, kp.second);
        ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());
        }

    stmt = m_ecdb.GetCachedStatement("SELECT  NULL FROM ec_ClassMap WHERE ClassId = ?");
    for (auto const& kp : caClasses)
        {
        stmt->ClearBindings();
        stmt->Reset();
        stmt->BindId(1, kp.second);
        ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());
        }

    int customAttributeInstances = 0;
    stmt = m_ecdb.GetCachedStatement("SELECT  NULL FROM ec_CustomAttribute WHERE ClassId = ?");
    for (auto const& kp : caClasses)
        {
        stmt->ClearBindings();
        stmt->Reset();
        stmt->BindId(1, kp.second);
        while (stmt->Step() == BE_SQLITE_ROW)
            customAttributeInstances++;
        }
    ASSERT_EQ(0, customAttributeInstances);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DeleteECCustomAttributeClass_Simple)
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
    AssertSchemaUpdate(deleteECCustomAttribute, filePath, {true, false}, "Deleting a ECCustomAttributeClass");
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
        "       <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>"
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
        "       <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>"
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
        "   <ECEntityClass typeName='Employee' modifier='Sealed' >"
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
        "   <ECEntityClass typeName='Employee' modifier='Sealed' >"
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
        "   <ECEntityClass typeName='Title' modifier='Sealed' >"
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
    ASSERT_EQ(SUCCESS, SetupECDb("AddKoQAndUpdatePropertiesWithKoQ.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Length' typeName='double' />"
        "    <ECProperty propertyName='Homepage' typeName='string' />"
        "    <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' />"
        "    <ECArrayProperty propertyName='Favorites' typeName='string'  minOccurs='0' maxOccurs='unbounded' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

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
TEST_F(SchemaUpgradeTestFixture, ReplaceKindOfQuantityWithSamePersistenceUnit)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ReplaceKindOfQuantityWithSamePersistenceUnit.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='KindOfQuantity1' />"
        "        <ECProperty propertyName='Homepage' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
        "                    displayLabel='KindOfQuantity2' persistenceUnit='CM' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "  <ECEntityClass typeName='Foo' >"
        "    <ECProperty propertyName='Length' typeName='double' kindOfQuantity='KindOfQuantity2' />"
        "    <ECProperty propertyName='Homepage' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Changing the KindOfQuantity of an ECProperty to another KindOfQuantity with same persistence unit");

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
// @bsimethod                                   Krischan.Eberle                 04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ReplaceKindOfQuantityWithDifferentPersistenceUnit)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ReplaceKindOfQuantityWithDifferentPersistenceUnit.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                 "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
                                                                 "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
                                                                 "                    presentationUnits='FT;IN' />"
                                                                 "    <ECEntityClass typeName='Foo' >"
                                                                 "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='KindOfQuantity1' />"
                                                                 "        <ECProperty propertyName='Homepage' typeName='string' />"
                                                                 "    </ECEntityClass>"
                                                                 "</ECSchema>")));

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
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Changing the KindOfQuantity of an ECProperty to another KindOfQuantity with different persistence unit");
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
        "                    displayLabel='KindOfQuantity2' persistenceUnit='CM' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity3' persistenceUnit='M' relativeError='.1'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity1'/>"
        "        <ECArrayProperty propertyName='Width' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity2'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();


    m_updatedDbs.clear();
    AssertSchemaUpdate("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
        "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
        "                    displayLabel='KindOfQuantity2' persistenceUnit='CM' relativeError='.2'"
        "                    presentationUnits='FT;IN' />"
        "    <KindOfQuantity typeName='KindOfQuantity3' persistenceUnit='M' relativeError='.1'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity2'/>"
        "        <ECArrayProperty propertyName='Width' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity1'/>"
        "    </ECEntityClass>"
                       "</ECSchema>", filePath, 
                       {true, true}, "Changing of KindOfQuantity of an ECArrayProperty to another KindOfQuantity with same persistence unit");

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

    AssertSchemaUpdate("<?xml version='1.0' encoding='utf-8'?>"
                       "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                       "    <KindOfQuantity typeName='KindOfQuantity1' description='KindOfQuantity1'"
                       "                    displayLabel='KindOfQuantity1' persistenceUnit='CM' relativeError='.5'"
                       "                    presentationUnits='FT;IN' />"
                       "    <KindOfQuantity typeName='KindOfQuantity2' description='KindOfQuantity2'"
                       "                    displayLabel='KindOfQuantity2' persistenceUnit='CM' relativeError='.2'"
                       "                    presentationUnits='FT;IN' />"
                       "    <KindOfQuantity typeName='KindOfQuantity3' persistenceUnit='M' relativeError='.1'"
                       "                    presentationUnits='FT;IN' />"
                       "    <ECEntityClass typeName='Foo' >"
                       "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity2'/>"
                       "        <ECArrayProperty propertyName='Width' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'KindOfQuantity3'/>"
                       "    </ECEntityClass>"
                       "</ECSchema>", filePath,
                       {false, false}, "Changing of KindOfQuantity of an ECArrayProperty to another KindOfQuantity with different persistence unit");
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECArrayProperty propertyName='Length' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Removing KindOfQuantity from an ECArrayProperty is not supported");
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
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='MyKindOfQuantity'"
        "                    displayLabel='MyKindOfQuantity' persistenceUnit='CM' relativeError='.5'"
        "                    presentationUnits='FT;IN' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {false, false}, "Removing KindOfQuantity from an ECProperty is not supported");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, KindOfQuantity)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_KindOfQuantity.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" description="My KOQ 1" displayLabel="KOQ 1" persistenceUnit="CM" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" displayLabel="KOQ 2" persistenceUnit="M" presentationUnits="FT;IN" relativeError="2" />
                                    <KindOfQuantity typeName="K3" description="My KOQ 3" displayLabel="KOQ 3" persistenceUnit="KG" presentationUnits="G" relativeError="3" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="KOQ 4" persistenceUnit="G" presentationUnits="MG" relativeError="4" />
                            </ECSchema>)xml")));

    auto assertKoq = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Utf8CP persUnit, Utf8CP presFormats, double relError)
        {
        KindOfQuantityCP koq = schema.GetKindOfQuantityCP(name);
        ASSERT_TRUE(koq != nullptr) << name;
        EXPECT_STRCASEEQ(name, koq->GetName().c_str()) << name;
        if (Utf8String::IsNullOrEmpty(displayLabel))
            EXPECT_FALSE(koq->GetIsDisplayLabelDefined()) << name;
        else
            EXPECT_STRCASEEQ(displayLabel, koq->GetDisplayLabel().c_str()) << name;

        if (Utf8String::IsNullOrEmpty(description))
            description = "";

        EXPECT_STRCASEEQ(description, koq->GetDescription().c_str()) << name;

        EXPECT_DOUBLE_EQ(relError, koq->GetRelativeError()) << name;
        EXPECT_STRCASEEQ(persUnit, koq->GetPersistenceUnit()->GetQualifiedName(koq->GetSchema()).c_str()) << name;
        Utf8String actualPresentationFormats;
        bool isFirstFormat = true;
        for (ECN::NamedFormatCR format : koq->GetPresentationFormats())
            {
            if (!isFirstFormat)
                actualPresentationFormats.append(";");

            actualPresentationFormats.append(format.GetQualifiedFormatString(koq->GetSchema()));
            isFirstFormat = false;
            }

        EXPECT_STRCASEEQ(presFormats, actualPresentationFormats.c_str()) << name;
        };

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertKoq(*schema, "K1", "KOQ 1", "My KOQ 1", "u:CM", "", 1);
    assertKoq(*schema, "K2", "KOQ 2", "My KOQ 2", "u:M", "f:DefaultReal[u:FT];f:DefaultReal[u:IN]", 2);
    assertKoq(*schema, "K3", "KOQ 3", "My KOQ 3", "u:KG", "f:DefaultReal[u:G]", 3);
    assertKoq(*schema, "K4", "KOQ 4", "My KOQ 4", "u:G", "f:DefaultReal[u:MG]", 4);
    }

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" description="My KOQ 1" displayLabel="KOQ 1" persistenceUnit="CM" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" displayLabel="KOQ 2" persistenceUnit="M" presentationUnits="FT;IN" relativeError="2" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="KOQ 4" persistenceUnit="G" presentationUnits="MG" relativeError="4" />
                                    </ECSchema>)xml"))) << "Deleting a KOQ is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" description="My KOQ 1" displayLabel="KOQ 1" persistenceUnit="CM" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" displayLabel="KOQ 2" persistenceUnit="CM" presentationUnits="FT;IN"  relativeError="3"/>
                                    <KindOfQuantity typeName="K3" description="My KOQ 3" displayLabel="KOQ 3" persistenceUnit="KG" presentationUnits="G" relativeError="3" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="KOQ 4" persistenceUnit="G" presentationUnits="MG" relativeError="4" />
                                    </ECSchema>)xml"))) << "Modifying the persistence unit is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" description="My KOQ 1" displayLabel="KOQ 1" persistenceUnit="CM" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" displayLabel="KOQ 2" persistenceUnit="M" presentationUnits="FT;IN"/>
                                    <KindOfQuantity typeName="K3" description="My KOQ 3" displayLabel="KOQ 3" persistenceUnit="KG" presentationUnits="G" relativeError="3" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="KOQ 4" persistenceUnit="G" presentationUnits="MG" relativeError="4" />
                                    </ECSchema>)xml"))) << "Removing the relative error is not supported";

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" description="My KOQ 1" displayLabel="KOQ 1" persistenceUnit="CM" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" displayLabel="KOQ 2" persistenceUnit="M" presentationUnits="FT;IN" relativeError="2" />
                                    <KindOfQuantity typeName="K3" description="My KOQ 3" displayLabel="KOQ 3" persistenceUnit="KG" presentationUnits="G" relativeError="3" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="KOQ 4" persistenceUnit="G" presentationUnits="MG" relativeError="4" />
                                    <KindOfQuantity typeName="K5" description="My KOQ 5" displayLabel="KOQ 5" persistenceUnit="M" presentationUnits="M(Meters4u);IN(Inches4u);FT(fi8);FT(feet4u)" relativeError="5" />
                                    <KindOfQuantity typeName="K6" description="My KOQ 6" displayLabel="KOQ 6" persistenceUnit="M" presentationUnits="M(Meters4u);IN(Inches4u);FT(fi8);FT(feet4u)" relativeError="6" />
                                    </ECSchema>)xml"))) << "Adding a KOQ is supported";

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertKoq(*schema, "K1", "KOQ 1", "My KOQ 1", "u:CM", "", 1);
    assertKoq(*schema, "K2", "KOQ 2", "My KOQ 2", "u:M", "f:DefaultReal[u:FT];f:DefaultReal[u:IN]", 2);
    assertKoq(*schema, "K3", "KOQ 3", "My KOQ 3", "u:KG", "f:DefaultReal[u:G]", 3);
    assertKoq(*schema, "K4", "KOQ 4", "My KOQ 4", "u:G", "f:DefaultReal[u:MG]", 4);
    assertKoq(*schema, "K5", "KOQ 5", "My KOQ 5", "u:M", "f:DefaultRealUNS(4)[u:M|m];f:DefaultRealUNS(4)[u:IN|&quot;];f:AmerFI;f:DefaultRealUNS(4)[u:FT|']", 5);
    assertKoq(*schema, "K6", "KOQ 6", "My KOQ 6", "u:M", "f:DefaultRealUNS(4)[u:M|m];f:DefaultRealUNS(4)[u:IN|&quot;];f:AmerFI;f:DefaultRealUNS(4)[u:FT|']", 6);
    }

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                    <KindOfQuantity typeName="K1" displayLabel="KOQ 1" persistenceUnit="CM" presentationUnits="IN;FT" relativeError="1" />
                                    <KindOfQuantity typeName="K2" description="My KOQ 2" persistenceUnit="M" presentationUnits="IN;FT" relativeError="2" />
                                    <KindOfQuantity typeName="K3" description="My Nice KOQ 3" displayLabel="KOQ 3" persistenceUnit="KG" presentationUnits="G" relativeError="3" />
                                    <KindOfQuantity typeName="K4" description="My KOQ 4" displayLabel="Nice KOQ 4" persistenceUnit="G" presentationUnits="KG;MG" relativeError="40" />
                                    <KindOfQuantity typeName="K5" description="My KOQ 5" displayLabel="KOQ 5" persistenceUnit="M" presentationUnits="M(Meters4u);FT(fi8);FT(feet4u)" relativeError="5" />
                                    <KindOfQuantity typeName="K6" description="My KOQ 6" displayLabel="KOQ 6" persistenceUnit="M" relativeError="6" />
                                    </ECSchema>)xml"))) << "Modifying a KOQ is supported";

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertKoq(*schema, "K1", "KOQ 1", nullptr, "u:CM", "f:DefaultReal[u:IN];f:DefaultReal[u:FT]", 1);
    //changing the order of pres units is supported
    assertKoq(*schema, "K2", nullptr, "My KOQ 2", "u:M", "f:DefaultReal[u:IN];f:DefaultReal[u:FT]", 2);
    assertKoq(*schema, "K3", "KOQ 3", "My Nice KOQ 3", "u:KG", "f:DefaultReal[u:G]", 3);
    //changing the order of pres units and adding a pres unit is supported
    assertKoq(*schema, "K4", "Nice KOQ 4", "My KOQ 4", "u:G", "f:DefaultReal[u:KG];f:DefaultReal[u:MG]", 40);
    assertKoq(*schema, "K5", "KOQ 5", "My KOQ 5", "u:M", "f:DefaultRealUNS(4)[u:M|m];f:AmerFI;f:DefaultRealUNS(4)[u:FT|']", 5);
    assertKoq(*schema, "K6", "KOQ 6", "My KOQ 6", "u:M", "", 6);
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
// @bsimethod                                  Krischan.Eberle                 01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, AddEnumAndEnumProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AddEnumAndEnumProperty.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Name" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="False">
           <ECEnumerator value="0" displayLabel="On" />
           <ECEnumerator value="1" displayLabel="Off" />
         </ECEnumeration>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Name" typeName="string"/>
            <ECProperty propertyName="Status" typeName="MyEnum"/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECClassCP fooClass = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(fooClass != nullptr);
    ECPropertyCP enumProp = fooClass->GetPropertyP("Status");
    ASSERT_TRUE(enumProp != nullptr && enumProp->GetIsPrimitive());
    ECEnumerationCP actualEnum = enumProp->GetAsPrimitiveProperty()->GetEnumeration();
    ASSERT_TRUE(actualEnum != nullptr);
    ASSERT_STREQ("MyEnum", actualEnum->GetName().c_str());
    ASSERT_EQ(actualEnum, m_ecdb.Schemas().GetEnumeration("TestSchema", "MyEnum"));
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
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Modifying enumerator values is not supported";
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
        " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>"
        "   <ECEnumerator value = '0' displayLabel = 'txt' />"
        "   <ECEnumerator value = '1' displayLabel = 'bat' />"
        " </ECEnumeration>"
        "</ECSchema>");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Cannot change IsStrict from false to true";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ChangeECEnumeratorValue)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangeECEnumeratorValue.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="Turn On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, ModifyEnumeratorNameInPre32ECSchema)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyEnumeratorNameInPre32ECSchema.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator value="0" />
                    <ECEnumerator value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator value="On" />
                    <ECEnumerator value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="An" value="On" />
                    <ECEnumerator name="Aus" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml"))) << "When coming from 3.1 schema, an enumerator name change is valid";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="TurnOn" value="0" />
                    <ECEnumerator name="TurnOff" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml"))) << "Once the name was changed after the 3.1 conversion, it cannot be changed anymore";

    //now start with EC3.2 enum which should never allow to rename an enumerator
    ASSERT_EQ(SUCCESS, SetupECDb("ModifyEnumeratorNameInPre32ECSchema.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="IntEnum0" value="0" />
                    <ECEnumerator name="IntEnum1" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml"))) << "Even if the enumerator name is the default EC3.2 conversion name, the change is not valid";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="TurnOn" value="0" />
                    <ECEnumerator name="TurnOff" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml"))) << "Once the name was changed away from the EC3.2 conversion default name, it cannot be changed anymore";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateECEnumerationAddDeleteEnumerators)
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
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
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
                                        <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                                        <ECEntityClass typeName="Foo" >
                                            <ECProperty propertyName="P1" typeName="double" category="C4" />
                                            <ECProperty propertyName="P2" typeName="double" />
                                            <ECProperty propertyName="P3" typeName="double" category="C3" />
                                            <ECProperty propertyName="P4" typeName="double" category="C1" />
                                        </ECEntityClass>
                                    </ECSchema>)xml")));
        
    
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
        ASSERT_TRUE(c5 != nullptr);
        ASSERT_STREQ("C5", c5->GetName().c_str());
        ASSERT_EQ(5, (int) c5->GetPriority());

        ECSchemaCP schema1 = m_ecdb.Schemas().GetSchema("Schema1", false);
        ASSERT_TRUE(schema1 != nullptr);

        ECClassCP fooClass = m_ecdb.Schemas().GetClass("Schema1", "Foo");
        ASSERT_TRUE(fooClass != nullptr);
        ASSERT_EQ(5, schema1->GetPropertyCategoryCount());

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

    //Delete a Category
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="3.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, PropertyCategory)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_PropertyCategory.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                        <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                                        <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                                        <PropertyCategory typeName="C3" description="My Category 3" displayLabel="Category 3" priority="3" />
                                        <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                                </ECSchema>)xml")));

    auto assertCategory = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, uint32_t priority)
        {
        PropertyCategoryCP cat = schema.GetPropertyCategoryCP(name);
        ASSERT_TRUE(cat != nullptr) << name;
        EXPECT_STREQ(name, cat->GetName().c_str()) << name;
        EXPECT_EQ(priority, cat->GetPriority()) << name;
        if (Utf8String::IsNullOrEmpty(displayLabel))
            EXPECT_FALSE(cat->GetIsDisplayLabelDefined()) << name;
        else
            EXPECT_STREQ(displayLabel, cat->GetDisplayLabel().c_str()) << name;

        if (Utf8String::IsNullOrEmpty(description))
            description = "";

        EXPECT_STREQ(description, cat->GetDescription().c_str()) << name;
        };

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertCategory(*schema, "C1", "Category 1", "My Category 1", 1);
    assertCategory(*schema, "C2", "Category 2", "My Category 2", 2);
    assertCategory(*schema, "C3", "Category 3", "My Category 3", 3);
    assertCategory(*schema, "C4", "Category 4", "My Category 4", 4);
    }

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                                        <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                                        <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                                    </ECSchema>)xml"))) << "Deleting a category is not supported";

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                                        <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                                        <PropertyCategory typeName="C3" description="My Category 3" displayLabel="Category 3" priority="3" />
                                        <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                                        <PropertyCategory typeName="C5" description="My Category 5" displayLabel="Category 5" priority="5" />
                                        <PropertyCategory typeName="C6" description="My Category 6" displayLabel="Category 6" priority="6" />
                                    </ECSchema>)xml"))) << "Adding a category is supported";

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertCategory(*schema, "C1", "Category 1", "My Category 1", 1);
    assertCategory(*schema, "C2", "Category 2", "My Category 2", 2);
    assertCategory(*schema, "C3", "Category 3", "My Category 3", 3);
    assertCategory(*schema, "C4", "Category 4", "My Category 4", 4);
    assertCategory(*schema, "C5", "Category 5", "My Category 5", 5);
    assertCategory(*schema, "C6", "Category 6", "My Category 6", 6);
    }

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <PropertyCategory typeName="C1" displayLabel="Category 1" priority="1" />
                                        <PropertyCategory typeName="C2" description="My Category 2" priority="2" />
                                        <PropertyCategory typeName="C3" description="My nice Category 3" displayLabel="Category 3" priority="3" />
                                        <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Nice Category 4" priority="4" />
                                        <PropertyCategory typeName="C5" description="My Category 5" displayLabel="Category 5" priority="50" />
                                        <PropertyCategory typeName="C6" description="My Category 6" displayLabel="Category 6" />
                                    </ECSchema>)xml"))) << "Modifying a category is supported";

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertCategory(*schema, "C1", "Category 1", nullptr, 1);
    assertCategory(*schema, "C2", nullptr, "My Category 2", 2);
    assertCategory(*schema, "C3", "Category 3", "My nice Category 3", 3);
    assertCategory(*schema, "C4", "Nice Category 4", "My Category 4", 4);
    assertCategory(*schema, "C5", "Category 5", "My Category 5", 50);
    //deleting the priority from the schema amounts to setting it to 0
    assertCategory(*schema, "C6", "Category 6", "My Category 6", 0);
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UnitSystems)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_unitsystems.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <UnitSystem typeName="UNUSEDU" displayLabel="Unused" description="Unused" />
                                    </ECSchema>)xml")));

    auto assertUnitSystem = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description)
        {
        UnitSystemCP system = schema.GetUnitSystemCP(name);
        ASSERT_TRUE(system != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, system->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, system->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, system->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertUnitSystem(*schema, "METRIC", "Metric", "Metric Units of measure");
    assertUnitSystem(*schema, "IMPERIAL", "Imperial", "Units of measure from the British Empire");
    assertUnitSystem(*schema, "UNUSEDU", "Unused", "Unused");
    }

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                    </ECSchema>)xml"))) << "Deleting a unit system is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <UnitSystem typeName="UNUSEDUS" displayLabel="Unused" description="Unused" />
                                    </ECSchema>)xml"))) << "Renaming a UnitSystem is not supported";

    {
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="ImperialSystem" description="Units of measure from the British Empire." />
                                        <UnitSystem typeName="UNUSEDU" displayLabel="Unused" description="Unused" />
                                        <UnitSystem typeName="MyLocalOne" displayLabel="My Local one"  />
                                    </ECSchema>)xml"))) << "Adding a unit system, modifying display label and description of unit system, removing display label and description of unit system";
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertUnitSystem(*schema, "METRIC", "METRIC", "");
    assertUnitSystem(*schema, "IMPERIAL", "ImperialSystem", "Units of measure from the British Empire.");
    assertUnitSystem(*schema, "UNUSEDU", "Unused", "Unused");
    assertUnitSystem(*schema, "MyLocalOne", "My Local one", "");
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Phenomena)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_phenomena.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                    </ECSchema>)xml")));

    auto assertPhenomenon = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Utf8CP definition)
        {
        PhenomenonCP phen = schema.GetPhenomenonCP(name);
        ASSERT_TRUE(phen != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, phen->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(definition, phen->GetDefinition().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, phen->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, phen->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };


    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertPhenomenon(*schema, "AREA", "Area", "", "LENGTH*LENGTH");
    assertPhenomenon(*schema, "UNUSEDP", "Unused", "", "LENGTH*LENGTH");
    }

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                    </ECSchema>)xml"))) << "Deleting a phenomenon is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDPHEN" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                    </ECSchema>)xml"))) << "Renaming a Phenomenon is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                    </ECSchema>)xml"))) << "Modifying Phenomenon.Definition is not supported";

    {
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.4" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <Phenomenon typeName="AREA" displayLabel="Areal" description="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="VOLUME" displayLabel="Volume" definition="LENGTH*LENGTH*LENGTH" />
                                    </ECSchema>)xml"))) << "Adding a phenomenon, modifying display label and description of phenomenon";
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertPhenomenon(*schema, "AREA", "Areal", "Area", "LENGTH*LENGTH");
    assertPhenomenon(*schema, "UNUSEDP", "Unused", "", "LENGTH*LENGTH");
    assertPhenomenon(*schema, "VOLUME", "Volume", "", "LENGTH*LENGTH*LENGTH");
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                02/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Units)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_units.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml")));

    auto assertUnit = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Utf8CP definition, double numerator, double denominator, double offset, Utf8CP phenomenon, Utf8CP unitSystem)
        {
        ECUnitCP unit = schema.GetUnitCP(name);
        ASSERT_TRUE(unit != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, unit->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(definition, unit->GetDefinition().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, unit->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, unit->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(numerator, unit->GetNumerator()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(denominator, unit->GetDenominator()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(offset, unit->GetOffset()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(phenomenon, unit->GetPhenomenon()->GetFullName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(unitSystem, unit->GetUnitSystem()->GetFullName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };

    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertUnit(*schema, "SquareM", "Square Meter", "", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
    assertUnit(*schema, "SquareFt", "Square Feet", "", "Ft*Ft", 10.0, 1.0, 0.4, "Schema1:AREA", "Schema1:IMPERIAL");

    }

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Deleting a unit is not supported";





    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareMeter" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Renaming a Unit is not supported";


    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M*" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.Definition is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.4" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.5" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.Numerator is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.5" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" denominator="2.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.Denominator is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.6" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" offset="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.Offset is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.7" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="IMPERIAL" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.UnitSystem is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.8" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                                        <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="UNUSEDP" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                    </ECSchema>)xml"))) << "Modifying Unit.Phenomenon is not supported";

    {
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                    <ECSchema schemaName="Schema1" alias="s1" version="1.0.9" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                        <UnitSystem typeName="METRIC" />
                                        <UnitSystem typeName="IMPERIAL" displayLabel="ImperialSystem" description="Units of measure from the British Empire." />
                                        <Phenomenon typeName="AREA" displayLabel="Areal" description="Area" definition="LENGTH*LENGTH" />
                                        <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                                        <Unit typeName="SquareM" displayLabel="Square Metre" description="Square Metre" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                        <Unit typeName="SquareFt" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                                        <Unit typeName="MyUnit" displayLabel="My Unit" description="my nice unit" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                                    </ECSchema>)xml"))) << "Adding a unit, modifying display label and description of unit, removing display label of unit";
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertUnit(*schema, "SquareM", "Square Metre", "Square Metre", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
    assertUnit(*schema, "SquareFt", "SquareFt", "", "Ft*Ft", 10.0, 1.0, 0.4, "Schema1:AREA", "Schema1:IMPERIAL");
    assertUnit(*schema, "MyUnit", "My Unit", "my nice unit", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, Formats)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_formats.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                    </Format>
                                </ECSchema>)xml")));

    auto assertFormat = [] (ECDbCR ecdb, Utf8CP name, Utf8CP displayLabel, Utf8CP description, JsonValue const& numericSpec, JsonValue const& compSpec)
        {
        ECFormatCP format = ecdb.Schemas().GetFormat("Schema", name);
        ASSERT_TRUE(format != nullptr) << "Schema." << name;

        Utf8String assertMessage(format->GetSchema().GetFullSchemaName());
        assertMessage.append(".").append(format->GetName());

        ASSERT_STREQ(name, format->GetName().c_str()) << assertMessage;
        if (Utf8String::IsNullOrEmpty(displayLabel))
            ASSERT_FALSE(format->GetIsDisplayLabelDefined()) << assertMessage;
        else
            ASSERT_STREQ(displayLabel, format->GetInvariantDisplayLabel().c_str()) << assertMessage;

        if (Utf8String::IsNullOrEmpty(description))
            ASSERT_FALSE(format->GetIsDescriptionDefined()) << assertMessage;
        else
            ASSERT_STREQ(description, format->GetInvariantDescription().c_str()) << assertMessage;

        if (numericSpec.m_value.isNull())
            ASSERT_FALSE(format->HasNumeric()) << assertMessage;
        else
            {
            ASSERT_TRUE(format->HasNumeric()) << assertMessage;
            Json::Value jval;
            ASSERT_TRUE(format->GetNumericSpec()->ToJson(jval, false)) << assertMessage;
            ASSERT_EQ(numericSpec, JsonValue(jval)) << assertMessage;
            }

        if (compSpec.m_value.isNull())
            ASSERT_FALSE(format->HasComposite()) << assertMessage;
        else
            {
            Json::Value jval;
            ASSERT_TRUE(format->GetCompositeSpec()->ToJson(jval)) << assertMessage;
            ASSERT_TRUE(format->HasComposite()) << assertMessage;
            ASSERT_EQ(compSpec, JsonValue(jval)) << assertMessage;
            }
        };

    assertFormat(m_ecdb, "MyFormat", "My Format", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue());


    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Format typeName="MyFormat" displayLabel="My nice Format" description="Real nice format" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways" formatTraits="KeepSingleZero"
                                            precision="5" decimalSeparator="," thousandSeparator="." uomSeparator="#">
                                    </Format>
                                </ECSchema>)xml"))) << "Modify DisplayLabel, Description, NumericSpec";

    assertFormat(m_ecdb, "MyFormat", "My nice Format", "Real nice format",
                 JsonValue(R"json({"roundFactor":1.3, "type": "Scientific", "scientificType":"ZeroNormalized", "showSignOption": "SignAlways", "formatTraits": ["keepSingleZero"], "precision": 5, "decimalSeparator": ",", "thousandSeparator": ".", "uomSeparator": "#"})json"),
                 JsonValue());


    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Format typeName="MyFormat" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways"
                                            precision="5">
                                    </Format>
                                </ECSchema>)xml"))) << "remove optional attributes from num spec";

    assertFormat(m_ecdb, "MyFormat", "", "",
                 JsonValue(R"json({"roundFactor":1.3, "type": "Scientific", "scientificType":"ZeroNormalized", "showSignOption": "SignAlways", "precision": 5})json"),
                 JsonValue());

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Format typeName="MyFormat" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways"
                                            precision="5">
                                        <Composite>
                                            <Unit label="mm">u:MM</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml"))) << "Adding composite is not supported";

    // now start with format that already has a composite

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_formats.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="2.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                        <Composite>
                                            <Unit label="m">MyMeter</Unit>
                                            <Unit label="mm">u:MM</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml")));

    assertFormat(m_ecdb, "MyFormat", "My Format", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue(R"json({"spacer":" ", "includeZero":true, "units": [{"name":"MyMeter", "label":"m"}, {"name":"MM", "label":"mm"}]})json"));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                        <Composite spacer="=" includeZero="False">
                                            <Unit label="meterle">MyMeter</Unit>
                                            <Unit label="millimeterle">u:MM</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml"))) << "Modify CompSpec except for units";

    assertFormat(m_ecdb, "MyFormat", "My Format", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue(R"json({"spacer":"=", "includeZero":false, "units": [{"name":"MyMeter", "label":"meterle"}, {"name":"MM", "label":"millimeterle"}]})json"));

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                        <Composite spacer="=" includeZero="False">
                                            <Unit label="meterle">u:M</Unit>
                                            <Unit label="millimeterle">u:MM</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml"))) << "Modifying Composite Unit is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                        <Composite spacer="=" includeZero="False">
                                            <Unit label="meterle">MyMeter</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml"))) << "Deleting a Composite Unit is not supported";

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                        <Composite spacer="=" includeZero="False">
                                            <Unit label="meterle">MyMeter</Unit>
                                            <Unit label="millimeterle">u:MM</Unit>
                                            <Unit label="kilometerle">u:KM</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml"))) << "Adding a Composite Unit is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MultiSessionSchemaImport_TPC)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multisession_si.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECEntityClass typeName='TestClassA' >
            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
        <ECEntityClass typeName='TestClassB' >
            <BaseClass>ts1:TestClassA</BaseClass>
            <ECProperty propertyName='L2' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));


    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
        <ECEntityClass typeName='TestClassC' >
            <BaseClass>ts2:TestClassB</BaseClass>
            <ECProperty propertyName='L3' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));


    const ECClassCP classTestClassA = m_ecdb.Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_ecdb.Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_ecdb.Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    //L1=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                             classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L2=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L3=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }
    }

// -------------------------------------------------------------------------------------- -
// @bsimethod                                   Affan Khan                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MultiSessionSchemaImport_TPH_Joined_OnDerivedClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multisession_si.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECEntityClass typeName='TestClassA' >

            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
        <ECEntityClass typeName='TestClassB' >
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
             <JoinedTablePerDirectSubclass xmlns = "ECDbMap.02.00" />
            </ECCustomAttributes>
            <BaseClass>ts1:TestClassA</BaseClass>
            <ECProperty propertyName='L2' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));


    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
        <ECEntityClass typeName='TestClassC' >
            <BaseClass>ts2:TestClassB</BaseClass>
            <ECProperty propertyName='L3' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));
    ReopenECDb();

    const ECClassCP classTestClassA = m_ecdb.Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_ecdb.Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_ecdb.Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    //L1=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                             classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L2=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L3=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, MultiSessionSchemaImport_TPH_OnDerivedClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multisession_si.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECEntityClass typeName='TestClassA' >

            <ECProperty propertyName='L1' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
        <ECEntityClass typeName='TestClassB' >
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <BaseClass>ts1:TestClassA</BaseClass>
            <ECProperty propertyName='L2' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));


    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
        <ECEntityClass typeName='TestClassC' >
            <BaseClass>ts2:TestClassB</BaseClass>
            <ECProperty propertyName='L3' typeName='double'/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));


    const ECClassCP classTestClassA = m_ecdb.Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_ecdb.Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_ecdb.Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    //L1=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                             classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L2=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                             classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }

    //L3=====================================================
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }
    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
        }

    
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                             classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, UpdateClass_AddStructProperty)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECStructClass typeName='ST' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S2' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S3' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S4' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S5' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>TablePerHierarchy</MapStrategy>"
        "           </ClassMap>"
        "           <ShareColumns xmlns='ECDbMap.02.00'>"
        "               <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "               <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
        "           </ShareColumns>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "       <ECProperty propertyName='P3' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("schemaupdate.ecdb", schemaItem));

    BeFileName filePath(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP editedSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECStructClass typeName='ST' modifier='None'>"
        "       <ECProperty propertyName='S1' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S2' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S3' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S4' typeName='int' readOnly='false' />"
        "       <ECProperty propertyName='S5' typeName='int' readOnly='false' />"
        "   </ECStructClass>"
        "   <ECEntityClass typeName='Foo' modifier='None' >"
        "       <ECCustomAttributes>"
        "           <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>TablePerHierarchy</MapStrategy>"
        "           </ClassMap>"
        "           <ShareColumns xmlns='ECDbMap.02.00'>"
        "               <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "               <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
        "           </ShareColumns>"
        "       </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "       <ECProperty propertyName='P2' typeName='int' />"
        "       <ECProperty propertyName='P3' typeName='int' />"
        "       <ECStructProperty propertyName='S' typeName='ST'/>"
        "   </ECEntityClass>"
        "</ECSchema>";

    m_updatedDbs.clear();
    AssertSchemaUpdate(editedSchemaXml, filePath, {true, true}, "Adding StructProperty is supported");
    OpenECDb(filePath);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, DisallowMajorSchemaUpgrade)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemaupgrade_DisallowMajorSchemaUpgrade.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml")));

    //Note: for each test schema we test it with the minor version being incremented and the major version being incremented

    auto assertImport = [this] (Utf8CP schemaTemplate, Utf8CP newSchemaVersion, SchemaManager::SchemaImportOptions options)
        {
        Utf8String schemaXml;
        schemaXml.Sprintf(schemaTemplate, newSchemaVersion);
        BentleyStatus actualStat = GetHelper().ImportSchema(SchemaItem(schemaXml), options);
        EXPECT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
        return actualStat;
        };

    //Deleting a property
    Utf8CP newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Deleting a property on a shared column (must fail because it requires the major schema version to be incremented)";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Deleting a property on a shared column (must fail because it requires the major schema version to be incremented)";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Deleting a property on a shared column";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Deleting a property on a shared column";

    //Deleting a class
    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Deleting a class (must fail because it requires the major schema version to be incremented)";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Deleting a class (must fail because it requires the major schema version to be incremented)";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Deleting a class";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Deleting a class";

    //adding IsNullable constraint
    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsNullable>False</IsNullable>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "IsNullable on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "IsNullable on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on existing property is never supported because adding ECDbMap CA is not allowed";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                    <ECProperty propertyName="NewProp" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsNullable>False</IsNullable>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "IsNullable on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "IsNullable on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on new property in existing class is never supported because adding ECDbMap CA is not allowed";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub2" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsNullable>False</IsNullable>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "IsNullable on property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "IsNullable on property on new subclass works because ECDb ignores it because the class is not the root of the table.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on property on new subclass works because ECDb ignores it because the class is not the root of the table.";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema2" alias="ts2" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="TestSchema" version="01.00" alias="ts"/>
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Sub2" >
                                    <BaseClass>ts:Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsNullable>False</IsNullable>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::None)) << "IsNullable on property on new subclass in new schema";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsNullable on property on new subclass in new schema";

    //adding IsUnique constraint

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsUnique>True</IsUnique>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "IsUnique on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsUnique on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "IsUnique on existing property is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsUnique on existing property is never supported because adding ECDbMap CA is not allowed";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                    <ECProperty propertyName="NewProp" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsUnique>True</IsUnique>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "IsUnique on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsUnique on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "IsUnique on new property in existing class is never supported because adding ECDbMap CA is not allowed";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "IsUnique on new property in existing class is never supported because adding ECDbMap CA is not allowed";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub2" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsUnique>True</IsUnique>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Adding unique index on shared column is ignored";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Adding unique index on shared column is ignored";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Adding unique index on shared column is ignored";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Adding unique index on shared column is ignored";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema2" alias="ts2" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="TestSchema" version="01.00" alias="ts"/>
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Sub2" >
                                    <BaseClass>ts:Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="string" >
                                        <ECCustomAttributes>
                                           <PropertyMap xmlns="ECDbMap.02.00">
                                             <IsUnique>True</IsUnique>
                                           </PropertyMap>
                                        </ECCustomAttributes>
                                    </ECProperty>
                                </ECEntityClass>
                            </ECSchema>)xml";

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::None)) << "Adding unique index on shared column is ignored";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Adding unique index on shared column is ignored";
    }

    //adding unique index

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Parent_Code</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>Code</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Unique index on existing property must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on existing property must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on existing property must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on existing property must fail because adding a ECDbMap CA on existing class is not allowed.";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Parent_NewProp</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>NewProp</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                    <ECProperty propertyName="NewProp" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Unique index on new property in existing class must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on new property in existing class must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on new property in existing class must fail because adding a ECDbMap CA on existing class is not allowed.";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on new property in existing class must fail because adding a ECDbMap CA on existing class is not allowed.";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub2" >
                                   <ECCustomAttributes>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Sub2_SubProp2</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>SubProp2</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="int" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Unique index on new property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on new property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on new property on new subclass";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on new property on new subclass";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema2" alias="ts2" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="TestSchema" version="01.00" alias="ts"/>
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Sub2" >
                                   <ECCustomAttributes>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Sub2_SubProp2</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>SubProp2</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <BaseClass>ts:Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="int" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on new property on new subclass in new schema";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on new property on new subclass in new schema";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub2" >
                                   <ECCustomAttributes>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Sub2_Code</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>Code</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="int" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Unique index on inherited property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on inherited property on new subclass";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on inherited property on new subclass";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on inherited property on new subclass";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema2" alias="ts2" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="TestSchema" version="01.00" alias="ts"/>
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Sub2" >
                                   <ECCustomAttributes>
                                       <DbIndexList xmlns="ECDbMap.02.00">
                                            <Indexes>
                                                <DbIndex>
                                                    <Name>uix_Sub2_Code</Name>
                                                    <IsUnique>True</IsUnique>
                                                    <Properties>
                                                        <string>Code</string>
                                                    </Properties>
                                                </DbIndex>
                                            </Indexes>
                                       </DbIndexList>
                                    </ECCustomAttributes>
                                    <BaseClass>ts:Parent</BaseClass>
                                    <ECProperty propertyName="SubProp2" typeName="int" />
                                </ECEntityClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::None)) << "Unique index on inherited property on new subclass in new schema";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Unique index on inherited property on new subclass in new schema";

    //adding physical FK

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Child" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="backward">
                                        <ECCustomAttributes>
                                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                                         </ECCustomAttributes>
                                    </ECNavigationProperty>
                                </ECEntityClass>
                                <ECRelationshipClass typeName="ParentHasChildren" modifier="Sealed" strength="embedding" strengthDirection="forward" >
                                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                                        <Class class="Parent" />
                                    </Source>
                                    <Target multiplicity="(0..1)" polymorphic="True"  roleLabel="is contained by">
                                        <Class class="Child" />
                                    </Target>
                                 </ECRelationshipClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Physical FK on new class";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Physical FK on new class";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Physical FK on new class";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Physical FK on new class";

    newSchema = R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                                <ECEntityClass typeName="Parent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                       <ShareColumns xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                    <ECNavigationProperty propertyName="Sibling" relationshipName="GrandparentHasParent" direction="backward">
                                        <ECCustomAttributes>
                                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                                         </ECCustomAttributes>
                                    </ECNavigationProperty>
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Grandparent" >
                                    <ECCustomAttributes>
                                       <ClassMap xmlns="ECDbMap.02.00">
                                           <MapStrategy>TablePerHierarchy</MapStrategy>
                                       </ClassMap>
                                    </ECCustomAttributes>
                                    <ECProperty propertyName="Name" typeName="string" />
                                </ECEntityClass>
                                <ECRelationshipClass typeName="GrandparentHasParent" modifier="Sealed" strength="embedding" strengthDirection="forward" >
                                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                                        <Class class="Grandparent" />
                                    </Source>
                                    <Target multiplicity="(0..1)" polymorphic="True"  roleLabel="is contained by">
                                        <Class class="Parent" />
                                    </Target>
                                 </ECRelationshipClass>
                            </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::None)) << "Physical FK on new nav prop in existing class";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "1.1", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Physical FK on new nav prop in existing class";
    EXPECT_EQ(SUCCESS, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::None)) << "Physical FK on new nav prop in existing class";
    EXPECT_EQ(ERROR, assertImport(newSchema, "2.0", SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "Physical FK on new nav prop in existing class";

    }
   

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpgradeTestFixture, SchemaDiff)
    {
    ECSchemaReadContextPtr ctx1 = ECSchemaReadContext::CreateContext();
    ECSchemaReadContextPtr ctx2 = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema1, schema2;
    ASSERT_EQ(ECN::SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                <ECEntityClass typeName="Parent" >
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECProperty propertyName="Val" typeName="int" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Sub" >
                                    <BaseClass>Parent</BaseClass>
                                    <ECProperty propertyName="SubProp" typeName="string" />
                                </ECEntityClass>
                            </ECSchema>)xml", *ctx1));

    // some changes, including strange ones which are not supported by ECDb. But the pure schema diff functionality should
    // cope with them.
    // * add new entity class
    // * change property from primitive to primitive array type
    // * change class from entity to relationship class
    ASSERT_EQ(ECN::SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                <ECEntityClass typeName="Parent" >
                                    <ECProperty propertyName="Name" typeName="string" />
                                    <ECProperty propertyName="Code" typeName="int"/>
                                    <ECArrayProperty propertyName="Val" typeName="string" extendedTypeName="JSON" />
                                </ECEntityClass>
                                <ECEntityClass typeName="Foo" >
                                    <ECProperty propertyName="FooProp" typeName="string" />
                                </ECEntityClass>
                                <ECRelationshipClass typeName="Sub" modifier="Sealed" strength="referencing">
                                   <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                                      <Class class="Parent" />
                                    </Source>
                                    <Target multiplicity="(0..*)" polymorphic="True"  roleLabel="is contained by">
                                        <Class class="Foo" />
                                    </Target>
                                </ECRelationshipClass>
                            </ECSchema>)xml", *ctx2));

    SchemaComparer comparer;
    SchemaDiff diff;
    ASSERT_EQ(SUCCESS, comparer.Compare(diff, {schema1.get()}, {schema2.get()}));
    ASSERT_EQ(1, diff.Changes().Count());
    SchemaChange& schemaChange = diff.Changes()[0];
    Utf8String s;
    schemaChange.WriteToString(s);
    LOG.trace(s.c_str());

    ASSERT_TRUE(schemaChange.IsChanged());
    ASSERT_EQ(ECChange::OpCode::Modified, schemaChange.GetOpCode());
    EXPECT_FALSE(schemaChange.Alias().IsChanged());
    EXPECT_FALSE(schemaChange.CustomAttributes().IsChanged());
    EXPECT_FALSE(schemaChange.Description().IsChanged());
    EXPECT_FALSE(schemaChange.DisplayLabel().IsChanged());
    EXPECT_FALSE(schemaChange.ECVersion().IsChanged());
    EXPECT_FALSE(schemaChange.Enumerations().IsChanged());
    EXPECT_FALSE(schemaChange.Formats().IsChanged());
    EXPECT_FALSE(schemaChange.KindOfQuantities().IsChanged());
    EXPECT_FALSE(schemaChange.Name().IsChanged());
    EXPECT_FALSE(schemaChange.OriginalECXmlVersionMajor().IsChanged());
    EXPECT_FALSE(schemaChange.OriginalECXmlVersionMinor().IsChanged());
    EXPECT_FALSE(schemaChange.Phenomena().IsChanged());
    EXPECT_FALSE(schemaChange.PropertyCategories().IsChanged());
    EXPECT_FALSE(schemaChange.References().IsChanged());
    EXPECT_FALSE(schemaChange.VersionMinor().IsChanged());
    EXPECT_FALSE(schemaChange.VersionRead().IsChanged());
    EXPECT_FALSE(schemaChange.VersionWrite().IsChanged());

    ClassChanges& classChanges = schemaChange.Classes();
    ASSERT_TRUE(classChanges.IsChanged());
    ASSERT_EQ(ECChange::OpCode::Modified, classChanges.GetOpCode());
    ASSERT_EQ(3, classChanges.Count());
    for (size_t i = 0; i < classChanges.Count(); i++)
        {
        ClassChange& classChange = classChanges[i];
        //In class Parent, property Val was changed
        if (BeStringUtilities::StricmpAscii(classChange.GetChangeName(), "Parent") == 0)
            {
            PropertyChanges& propChanges = classChange.Properties();
            ASSERT_TRUE(propChanges.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChanges.GetOpCode());
            ASSERT_EQ(1, propChanges.Count());
            PropertyChange& propChange = propChanges[0];
            ASSERT_TRUE(propChange.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.GetOpCode());
            EXPECT_FALSE(propChange.Name().IsChanged());
            ASSERT_STREQ("Val", propChange.GetChangeName());

            ASSERT_TRUE(propChange.IsPrimitive().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.IsPrimitive().GetOpCode());
            ASSERT_TRUE(propChange.IsPrimitive().GetOld().Value());
            ASSERT_FALSE(propChange.IsPrimitive().GetNew().Value());

            ASSERT_TRUE(propChange.IsPrimitiveArray().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.IsPrimitiveArray().GetOpCode());
            ASSERT_FALSE(propChange.IsPrimitiveArray().GetOld().Value());
            ASSERT_TRUE(propChange.IsPrimitiveArray().GetNew().Value());

            ASSERT_TRUE(propChange.TypeName().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.TypeName().GetOpCode());
            ASSERT_STRCASEEQ("int", propChange.TypeName().GetOld().Value().c_str());
            ASSERT_STRCASEEQ("string", propChange.TypeName().GetNew().Value().c_str());

            ASSERT_TRUE(propChange.ExtendedTypeName().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.ExtendedTypeName().GetOpCode());
            ASSERT_TRUE(propChange.ExtendedTypeName().GetOld().IsNull());
            ASSERT_STREQ("JSON", propChange.ExtendedTypeName().GetNew().Value().c_str());

            ASSERT_TRUE(propChange.ArrayMinOccurs().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.ArrayMinOccurs().GetOpCode());
            ASSERT_TRUE(propChange.ArrayMinOccurs().GetOld().IsNull());
            ASSERT_EQ(0, propChange.ArrayMinOccurs().GetNew().Value());

            ASSERT_TRUE(propChange.ArrayMaxOccurs().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChange.ArrayMaxOccurs().GetOpCode());
            ASSERT_TRUE(propChange.ArrayMaxOccurs().GetOld().IsNull());
            ASSERT_EQ(std::numeric_limits<uint32_t>::max(), propChange.ArrayMaxOccurs().GetNew().Value());

            continue;
            }

        //Class Foo was added
        if (BeStringUtilities::StricmpAscii(classChange.GetChangeName(), "Foo") == 0)
            {
            ASSERT_EQ(ECChange::OpCode::New, classChange.GetOpCode());
            continue;
            }

        //Class Sub was changed from entity to relationship class
        if (BeStringUtilities::StricmpAscii(classChange.GetChangeName(), "Sub") == 0)
            {
            ASSERT_EQ(ECChange::OpCode::Modified, classChange.GetOpCode());

            ASSERT_TRUE(classChange.ClassType().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, classChange.ClassType().GetOpCode());
            ASSERT_EQ(ECClassType::Entity, classChange.ClassType().GetOld().Value());
            ASSERT_EQ(ECClassType::Relationship, classChange.ClassType().GetNew().Value());

            PropertyChanges& propChanges = classChange.Properties();
            ASSERT_TRUE(propChanges.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, propChanges.GetOpCode());
            ASSERT_EQ(1, propChanges.Count());
            PropertyChange& propChange = propChanges[0];
            ASSERT_STREQ("SubProp", propChange.GetChangeName());
            ASSERT_TRUE(propChange.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Deleted, propChange.GetOpCode());

            BaseClassChanges& baseClassChanges = classChange.BaseClasses();
            ASSERT_TRUE(baseClassChanges.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, baseClassChanges.GetOpCode());
            ASSERT_EQ(1, baseClassChanges.Count());
            StringChange& baseClassChange = baseClassChanges[0];
            ASSERT_TRUE(baseClassChange.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Deleted, baseClassChange.GetOpCode());
            ASSERT_STREQ("TestSchema:Parent", baseClassChange.GetOld().Value().c_str());

            ASSERT_TRUE(classChange.Strength().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, classChange.Strength().GetOpCode());
            ASSERT_TRUE(classChange.Strength().GetOld().IsNull());
            ASSERT_EQ(StrengthType::Referencing, classChange.Strength().GetNew().Value());

            ASSERT_TRUE(classChange.StrengthDirection().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, classChange.StrengthDirection().GetOpCode());
            ASSERT_TRUE(classChange.StrengthDirection().GetOld().IsNull());
            ASSERT_EQ(ECRelatedInstanceDirection::Forward, classChange.StrengthDirection().GetNew().Value());

            
            RelationshipConstraintChange& sourceChange = classChange.Source();
            ASSERT_TRUE(sourceChange.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, sourceChange.GetOpCode());

            ASSERT_TRUE(sourceChange.IsPolymorphic().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, sourceChange.IsPolymorphic().GetOpCode());
            ASSERT_TRUE(sourceChange.IsPolymorphic().GetOld().IsNull());
            ASSERT_EQ(true, sourceChange.IsPolymorphic().GetNew().Value());

            ASSERT_TRUE(sourceChange.Multiplicity().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, sourceChange.Multiplicity().GetOpCode());
            ASSERT_TRUE(sourceChange.Multiplicity().GetOld().IsNull());
            ASSERT_STREQ("(0..1)", sourceChange.Multiplicity().GetNew().Value().c_str());

            ASSERT_TRUE(sourceChange.RoleLabel().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, sourceChange.RoleLabel().GetOpCode());
            ASSERT_TRUE(sourceChange.RoleLabel().GetOld().IsNull());
            ASSERT_STREQ("has", sourceChange.RoleLabel().GetNew().Value().c_str());

            ASSERT_TRUE(sourceChange.ConstraintClasses().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, sourceChange.ConstraintClasses().GetOpCode());
            ASSERT_EQ(1, sourceChange.ConstraintClasses().Count());
            //a constraint class was added
            ASSERT_TRUE(sourceChange.ConstraintClasses()[0].IsChanged());
            ASSERT_EQ(ECChange::OpCode::New, sourceChange.ConstraintClasses()[0].GetOpCode());
            ASSERT_STREQ("TestSchema:Parent", sourceChange.ConstraintClasses()[0].GetNew().Value().c_str());

            RelationshipConstraintChange& targetChange = classChange.Target();
            ASSERT_TRUE(targetChange.IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, targetChange.GetOpCode());

            ASSERT_TRUE(targetChange.IsPolymorphic().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, targetChange.IsPolymorphic().GetOpCode());
            ASSERT_TRUE(targetChange.IsPolymorphic().GetOld().IsNull());
            ASSERT_EQ(true, targetChange.IsPolymorphic().GetNew().Value());

            ASSERT_TRUE(targetChange.Multiplicity().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, targetChange.Multiplicity().GetOpCode());
            ASSERT_TRUE(targetChange.Multiplicity().GetOld().IsNull());
            ASSERT_STREQ("(0..*)", targetChange.Multiplicity().GetNew().Value().c_str());

            ASSERT_TRUE(targetChange.RoleLabel().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, targetChange.RoleLabel().GetOpCode());
            ASSERT_TRUE(targetChange.RoleLabel().GetOld().IsNull());
            ASSERT_STREQ("is contained by", targetChange.RoleLabel().GetNew().Value().c_str());

            ASSERT_TRUE(targetChange.ConstraintClasses().IsChanged());
            ASSERT_EQ(ECChange::OpCode::Modified, targetChange.ConstraintClasses().GetOpCode());
            ASSERT_EQ(1, targetChange.ConstraintClasses().Count());
            //a constraint class was added
            ASSERT_TRUE(targetChange.ConstraintClasses()[0].IsChanged());
            ASSERT_EQ(ECChange::OpCode::New, targetChange.ConstraintClasses()[0].GetOpCode());
            ASSERT_STREQ("TestSchema:Foo", targetChange.ConstraintClasses()[0].GetNew().Value().c_str());
            }
        }
    }

END_ECDBUNITTESTS_NAMESPACE
