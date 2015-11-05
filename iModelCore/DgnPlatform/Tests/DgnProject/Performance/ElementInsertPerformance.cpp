/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementInsertPerformance.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "PerformanceTestFixture.h"

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE


//=======================================================================================
//! Test Fixture for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct PerformanceElementItem : public DgnDbTestFixture
{

public:
    int m_smallCount, m_mediumCount, m_largeCount, m_startCount, m_maxCount, m_increment;
    StopWatch m_stopWatch;

    PerformanceElementItem()
    {
        SetCounters(10000, 100000, 1000000);
        createDatabase(L"small.idgndb", m_smallCount);
        createDatabase(L"medium.idgndb", m_mediumCount);
        createDatabase(L"large.idgndb", m_largeCount);
    };

    void createDatabase(WCharCP dbName, int InstanceCount)
    {
        BeFileName existingName;
        BeTest::GetHost().GetOutputRoot(existingName);
        existingName.AppendToPath(dbName);
        if (!existingName.DoesPathExist())         //if file is there, dont' create it
        {
            BeFileName outFileName;
            ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, L"3dMetricGeneral.idgndb", dbName, __FILE__));
            DbResult result;
            DgnDbPtr db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
            ASSERT_TRUE(db.IsValid());
            ASSERT_TRUE(result == BE_SQLITE_OK);

            BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
            schemaFile.AppendToPath(WString("ECSchemas/" TMTEST_SCHEMA_NAME ".01.00.ecschema.xml", BentleyCharEncoding::Utf8).c_str());

            auto status = DgnPlatformTestDomain::GetDomain().ImportSchema(*db, schemaFile);
            ASSERT_TRUE(DgnDbStatus::Success == status);

            DgnModelId defaultModelId = db->Models().QueryFirstModelId();
            DgnModelPtr defaultModelP = db->Models().GetModel(defaultModelId);
            ASSERT_TRUE(defaultModelP.IsValid());
            defaultModelP->FillModel();

            DgnCategoryId defaultCategoryId = DgnCategory::QueryFirstCategoryId(*db);

            for (int i = 1; i <= InstanceCount; i++)
            {
                TestElementPtr el = TestElement::Create(*db, defaultModelId, defaultCategoryId, DgnElement::Code());
                el->SetTestItemProperty("Test Value");
                DgnElementCPtr el2 = db->Elements().Insert(*el);
                ASSERT_TRUE(el2.IsValid());
            }

            db->CloseDb();
        }
    };
    void SetCounters(int smallCount, int mediumCount, int largeCount)
    {
        m_smallCount = smallCount;
        m_mediumCount = mediumCount;
        m_largeCount = largeCount;
    };

    int InstanceCount()
    {
        Utf8String stmt("SELECT COUNT(" TMTEST_TEST_ITEM_TestItemProperty ") FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
        CachedECSqlStatementPtr selStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
        EXPECT_FALSE(selStmt.IsNull()) << "InstacneCount failed";

        EXPECT_TRUE(selStmt->Step() == BE_SQLITE_ROW);
        return selStmt->GetValueInt(0);

    };

    bvector <uint64_t> GetElementIds(int instanceCount)
    {
        DgnClassId classId = DgnClassId(m_db->Schemas().GetECClassId(TMTEST_SCHEMA_NAME, TMTEST_TEST_ITEM_CLASS_NAME));
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "Select ElementId from dgn_ElementItem WHERE ECClassId = ?"));
        EXPECT_EQ(BE_SQLITE_OK, stmt.BindId(1, classId));
        bvector <uint64_t> elementIds;
        for (int i = 1; i <= instanceCount; i++)
        {
            if (stmt.Step() == BE_SQLITE_ROW)
                elementIds.push_back(stmt.GetValueInt64(0));
            else
                EXPECT_TRUE(false);
        }
        return elementIds;
    }

    void GetDb(WCharCP baseName, WCharCP dbName)
    {
        BeFileName toCopy, copyDb;
        BeTest::GetHost().GetOutputRoot(toCopy);
        toCopy.AppendToPath(baseName);
        if (!toCopy.DoesPathExist())
            ASSERT_TRUE(false) <<toCopy.GetName();
        BeTest::GetHost().GetOutputRoot(copyDb);
        copyDb.AppendToPath(dbName);
        if (copyDb.DoesPathExist()) // delete existing copy
            ASSERT_EQ(BeFileNameStatus::Success, copyDb.BeDeleteFile());
        
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(toCopy, copyDb));

        DbResult result;
        m_db = DgnDb::OpenDgnDb(&result, copyDb, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());
        ASSERT_TRUE(result == BE_SQLITE_OK);
        m_defaultModelId = m_db->Models().QueryFirstModelId();
        m_defaultModelP = m_db->Models().GetModel(m_defaultModelId);
        ASSERT_TRUE(m_defaultModelP.IsValid());
        m_defaultModelP->FillModel();

        m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
    };

    void MeasurePerformanceInsert(WCharCP baseFile, WCharCP testFile, int initialCount, int instanceCount)
    {
        GetDb(baseFile, testFile);
        ASSERT_EQ(initialCount, InstanceCount());

        //First create Elements
        bvector<TestElementPtr> testElements;
        for (int i = 0; i < instanceCount; i++)
        {
            TestElementPtr element = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
            element->SetTestItemProperty("Test Value");
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
        }

        //Now Insert and Measure Time
        DgnDbStatus stat = DgnDbStatus::Success;
        m_stopWatch.Start();
        for (TestElementPtr& element : testElements)
        {
            element->Insert(&stat);
            ASSERT_EQ(DgnDbStatus::Success, stat);
        }
        m_stopWatch.Stop();

        ASSERT_EQ(initialCount + instanceCount, InstanceCount());
        LOGTODB(TEST_DETAILS, m_stopWatch.GetElapsedSeconds(), Utf8PrintfString("Insert. Db Inital Count: %d ", initialCount).c_str(), instanceCount);

    };

    void MeasurePerformanceDelete(WCharCP baseFile, WCharCP testFile, int initialCount, int instanceCount)
    {
        GetDb(baseFile, testFile);
        ASSERT_EQ(initialCount, InstanceCount());
    
        //First get Ids that we need to Delete
        bvector <uint64_t> elementIds = GetElementIds(instanceCount);
        ASSERT_EQ(elementIds.size(), instanceCount);

        //Now Delete them and measure time
        m_stopWatch.Start();
        DgnDbStatus stat;
        for (uint64_t Id : elementIds)
        {
            stat = m_db->Elements().Delete(DgnElementId(Id));
            ASSERT_EQ(DgnDbStatus::Success, stat);
        }
        m_stopWatch.Stop();

        ASSERT_EQ(initialCount - instanceCount, InstanceCount());
        LOGTODB(TEST_DETAILS, m_stopWatch.GetElapsedSeconds(), Utf8PrintfString("Delete. Db Inital Count: %d ", initialCount).c_str(), instanceCount);
    };

    void MeasurePerformanceSelect(WCharCP baseFile, WCharCP testFile, int initialCount, int instanceCount)
    {
        GetDb(baseFile, testFile);
        ASSERT_EQ(initialCount, InstanceCount());

        //First get Ids that we need to Select
        bvector <uint64_t> elementIds = GetElementIds(instanceCount);
        ASSERT_EQ(elementIds.size(), instanceCount);

        //Prepare the select statement
        Utf8String stmt("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
        stmt.append(" WHERE ECInstanceId = ?;");

        CachedECSqlStatementPtr selStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
        ASSERT_FALSE(selStmt.IsNull());

        //Now select and measure time
        m_stopWatch.Start();
        for (uint64_t Id : elementIds)
        {
            ASSERT_EQ(ECSqlStatus::Success, selStmt->BindId(1, ECInstanceId(Id)));
            ASSERT_EQ(BE_SQLITE_ROW, selStmt->Step());
            ASSERT_STREQ("Test Value", selStmt->GetValueText(0));
            ASSERT_EQ(ECSqlStatus::Success, selStmt->Reset());
            ASSERT_EQ(ECSqlStatus::Success, selStmt->ClearBindings());
        }
        m_stopWatch.Stop();

        LOGTODB(TEST_DETAILS, m_stopWatch.GetElapsedSeconds(), Utf8PrintfString("Select. Db Inital Count: %d ", initialCount).c_str(), instanceCount);
    };

    void MeasurePerformanceUpdate(WCharCP baseFile, WCharCP testFile, int initialCount, int instanceCount)
    {
        GetDb(baseFile, testFile);
        ASSERT_EQ(initialCount, InstanceCount());

        //First get some Elements that we need to Update
        bvector <uint64_t> elementIds = GetElementIds(instanceCount);
        ASSERT_EQ(elementIds.size(), instanceCount);
        bvector <TestElementPtr> testElements;
        for (uint64_t Id : elementIds)
        {
            TestElementPtr mod = m_db->Elements().GetForEdit<TestElement>(DgnElementId(Id));
            EXPECT_TRUE(mod.IsValid());
            mod->SetTestItemProperty("Test - New");
            testElements.push_back(mod);
        }

        //Now Update and Measure Time
        DgnDbStatus stat = DgnDbStatus::Success;
        m_stopWatch.Start();
        for (TestElementPtr& element : testElements)
        {
            element->Update(&stat);
            ASSERT_EQ(DgnDbStatus::Success, stat);
        }
        m_stopWatch.Stop();

        //Verify that we have new test value
        Utf8String stmt("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
        stmt.append(" WHERE ECInstanceId = ?;");
        CachedECSqlStatementPtr selStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
        ASSERT_FALSE(selStmt.IsNull());
        for (uint64_t Id : elementIds)
        {
            ECSqlStatus status = selStmt->BindInt64(1, Id);
            ASSERT_EQ(ECSqlStatus::Success, status);
            ASSERT_EQ(BE_SQLITE_ROW, selStmt->Step());
            ASSERT_STREQ("Test - New", selStmt->GetValueText(0));
            ASSERT_EQ(ECSqlStatus::Success, selStmt->Reset());
            ASSERT_EQ(ECSqlStatus::Success, selStmt->ClearBindings());
        }

        LOGTODB(TEST_DETAILS, m_stopWatch.GetElapsedSeconds(), Utf8PrintfString("Update. Db Inital Count: %d ", initialCount).c_str(), instanceCount);

    };
};

//---------------------------------------------------------------------------------**//**
// Test to measure time of Insert in a database with existing Instances
// @bsimethod                                    Majd.Uddin      10/15
// +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, Insert)
{
    //Small
    MeasurePerformanceInsert(L"small.idgndb", L"Insert_Small_200.idgndb", m_smallCount, 200);
    MeasurePerformanceInsert(L"small.idgndb", L"Insert_Small_400.idgndb", m_smallCount, 400);
    MeasurePerformanceInsert(L"small.idgndb", L"Insert_Small_600.idgndb", m_smallCount, 600);
    MeasurePerformanceInsert(L"small.idgndb", L"Insert_Small_800.idgndb", m_smallCount, 800);
    MeasurePerformanceInsert(L"small.idgndb", L"Insert_Small_1000.idgndb", m_smallCount, 1000);
    
    //Medium
    MeasurePerformanceInsert(L"medium.idgndb", L"Insert_Medium_200.idgndb", m_mediumCount, 200);
    MeasurePerformanceInsert(L"medium.idgndb", L"Insert_Medium_400.idgndb", m_mediumCount, 400);
    MeasurePerformanceInsert(L"medium.idgndb", L"Insert_Medium_600.idgndb", m_mediumCount, 600);
    MeasurePerformanceInsert(L"medium.idgndb", L"Insert_Medium_800.idgndb", m_mediumCount, 800);
    MeasurePerformanceInsert(L"medium.idgndb", L"Insert_Medium_1000.idgndb", m_mediumCount, 1000);

    //Large
    MeasurePerformanceInsert(L"large.idgndb", L"Insert_Large_200.idgndb", m_largeCount, 200);
    MeasurePerformanceInsert(L"large.idgndb", L"Insert_Large_400.idgndb", m_largeCount, 400);
    MeasurePerformanceInsert(L"large.idgndb", L"Insert_Large_600.idgndb", m_largeCount, 600);
    MeasurePerformanceInsert(L"large.idgndb", L"Insert_Large_800.idgndb", m_largeCount, 800);
    MeasurePerformanceInsert(L"large.idgndb", L"Insert_Large_1000.idgndb", m_largeCount, 1000);
}
//---------------------------------------------------------------------------------**//**
// Test to measure time of Delete in a database with existing Instances
// @bsimethod                                    Majd.Uddin      10/15
// +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, Delete)
{
    //Small
    MeasurePerformanceDelete(L"small.idgndb", L"Delete_Small_200.idgndb", m_smallCount, 200);
    MeasurePerformanceDelete(L"small.idgndb", L"Delete_Small_400.idgndb", m_smallCount, 400);
    MeasurePerformanceDelete(L"small.idgndb", L"Delete_Small_600.idgndb", m_smallCount, 600);
    MeasurePerformanceDelete(L"small.idgndb", L"Delete_Small_800.idgndb", m_smallCount, 800);
    MeasurePerformanceDelete(L"small.idgndb", L"Delete_Small_1000.idgndb", m_smallCount, 1000);

    //Medium
    MeasurePerformanceDelete(L"medium.idgndb", L"Delete_Medium_200.idgndb", m_mediumCount, 200);
    MeasurePerformanceDelete(L"medium.idgndb", L"Delete_Medium_400.idgndb", m_mediumCount, 400);
    MeasurePerformanceDelete(L"medium.idgndb", L"Delete_Medium_600.idgndb", m_mediumCount, 600);
    MeasurePerformanceDelete(L"medium.idgndb", L"Delete_Medium_800.idgndb", m_mediumCount, 800);
    MeasurePerformanceDelete(L"medium.idgndb", L"Delete_Medium_1000.idgndb", m_mediumCount, 1000);

    //Large
    MeasurePerformanceDelete(L"large.idgndb", L"Delete_Large_200.idgndb", m_largeCount, 200);
    MeasurePerformanceDelete(L"large.idgndb", L"Delete_Large_400.idgndb", m_largeCount, 400);
    MeasurePerformanceDelete(L"large.idgndb", L"Delete_Large_600.idgndb", m_largeCount, 600);
    MeasurePerformanceDelete(L"large.idgndb", L"Delete_Large_800.idgndb", m_largeCount, 800);
    MeasurePerformanceDelete(L"large.idgndb", L"Delete_Large_1000.idgndb", m_largeCount, 1000);
}

//---------------------------------------------------------------------------------**//**
// Test to measure time of Select in a database with existing Instances
// @bsimethod                                    Majd.Uddin      10/15
// +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, Select)
{
    //Small
    MeasurePerformanceSelect(L"small.idgndb", L"Select_Small_200.idgndb", m_smallCount, 200);
    MeasurePerformanceSelect(L"small.idgndb", L"Select_Small_400.idgndb", m_smallCount, 400);
    MeasurePerformanceSelect(L"small.idgndb", L"Select_Small_600.idgndb", m_smallCount, 600);
    MeasurePerformanceSelect(L"small.idgndb", L"Select_Small_800.idgndb", m_smallCount, 800);
    MeasurePerformanceSelect(L"small.idgndb", L"Select_Small_1000.idgndb", m_smallCount, 1000);

    //Medium
    MeasurePerformanceSelect(L"medium.idgndb", L"Select_Medium_200.idgndb", m_mediumCount, 200);
    MeasurePerformanceSelect(L"medium.idgndb", L"Select_Medium_400.idgndb", m_mediumCount, 400);
    MeasurePerformanceSelect(L"medium.idgndb", L"Select_Medium_600.idgndb", m_mediumCount, 600);
    MeasurePerformanceSelect(L"medium.idgndb", L"Select_Medium_800.idgndb", m_mediumCount, 800);
    MeasurePerformanceSelect(L"medium.idgndb", L"Select_Medium_1000.idgndb", m_mediumCount, 1000);

    //Large
    MeasurePerformanceSelect(L"large.idgndb", L"Select_Large_200.idgndb", m_largeCount, 200);
    MeasurePerformanceSelect(L"large.idgndb", L"Select_Large_400.idgndb", m_largeCount, 400);
    MeasurePerformanceSelect(L"large.idgndb", L"Select_Large_600.idgndb", m_largeCount, 600);
    MeasurePerformanceSelect(L"large.idgndb", L"Select_Large_800.idgndb", m_largeCount, 800);
    MeasurePerformanceSelect(L"large.idgndb", L"Select_Large_1000.idgndb", m_largeCount, 1000);
}
//---------------------------------------------------------------------------------**//**
// Test to measure time of Update in a database with existing Instances
// @bsimethod                                    Majd.Uddin      10/15
// +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, Update)
{
    //Small
    MeasurePerformanceUpdate(L"small.idgndb", L"Update_Small_200.idgndb", m_smallCount, 200);
    MeasurePerformanceUpdate(L"small.idgndb", L"Update_Small_400.idgndb", m_smallCount, 400);
    MeasurePerformanceUpdate(L"small.idgndb", L"Update_Small_600.idgndb", m_smallCount, 600);
    MeasurePerformanceUpdate(L"small.idgndb", L"Update_Small_800.idgndb", m_smallCount, 800);
    MeasurePerformanceUpdate(L"small.idgndb", L"Update_Small_1000.idgndb", m_smallCount, 1000);

    //Medium
    MeasurePerformanceUpdate(L"medium.idgndb", L"Update_Medium_200.idgndb", m_mediumCount, 200);
    MeasurePerformanceUpdate(L"medium.idgndb", L"Update_Medium_400.idgndb", m_mediumCount, 400);
    MeasurePerformanceUpdate(L"medium.idgndb", L"Update_Medium_600.idgndb", m_mediumCount, 600);
    MeasurePerformanceUpdate(L"medium.idgndb", L"Update_Medium_800.idgndb", m_mediumCount, 800);
    MeasurePerformanceUpdate(L"medium.idgndb", L"Update_Medium_1000.idgndb", m_mediumCount, 1000);

    //Large
    MeasurePerformanceUpdate(L"large.idgndb", L"Update_Large_200.idgndb", m_largeCount, 200);
    MeasurePerformanceUpdate(L"large.idgndb", L"Update_Large_400.idgndb", m_largeCount, 400);
    MeasurePerformanceUpdate(L"large.idgndb", L"Update_Large_600.idgndb", m_largeCount, 600);
    MeasurePerformanceUpdate(L"large.idgndb", L"Update_Large_800.idgndb", m_largeCount, 800);
    MeasurePerformanceUpdate(L"large.idgndb", L"Update_Large_1000.idgndb", m_largeCount, 1000);
}

//static
const DgnCategoryId PerformanceElementTestFixture::s_catId = DgnCategoryId ((uint64_t)123);
const DgnAuthorityId PerformanceElementTestFixture::s_codeAuthorityId = DgnAuthorityId ((uint64_t)1);
Utf8CP const PerformanceElementTestFixture::s_textVal = "bla bla";
const double PerformanceElementTestFixture::s_doubleVal = -3.1415;
Utf8CP const PerformanceElementTestFixture::s_testSchemaXml =
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
"  <ECClass typeName='Element1' >"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>dgn:PhysicalElement</BaseClass>"
"    <ECProperty propertyName='Prop1_1' typeName='string' />"
"    <ECProperty propertyName='Prop1_2' typeName='long' />"
"    <ECProperty propertyName='Prop1_3' typeName='double' />"
"  </ECClass>"
"  <ECClass typeName='Element2' >"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>Element1</BaseClass>"
"    <ECProperty propertyName='Prop2_1' typeName='string' />"
"    <ECProperty propertyName='Prop2_2' typeName='long' />"
"    <ECProperty propertyName='Prop2_3' typeName='double' />"
"  </ECClass>"
"  <ECClass typeName='Element3' >"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>Element2</BaseClass>"
"    <ECProperty propertyName='Prop3_1' typeName='string' />"
"    <ECProperty propertyName='Prop3_2' typeName='long' />"
"    <ECProperty propertyName='Prop3_3' typeName='double' />"
"  </ECClass>"
"  <ECClass typeName='Element4' >"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>Element3</BaseClass>"
"    <ECProperty propertyName='Prop4_1' typeName='string' />"
"    <ECProperty propertyName='Prop4_2' typeName='long' />"
"    <ECProperty propertyName='Prop4_3' typeName='double' />"
"  </ECClass>"
"  <ECClass typeName='Element4b' >"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>Element3</BaseClass>"
"    <ECProperty propertyName='Prop4b_1' typeName='string' />"
"    <ECProperty propertyName='Prop4b_2' typeName='long' />"
"    <ECProperty propertyName='Prop4b_3' typeName='double' />"
"    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
"  </ECClass>"
"  <ECClass typeName='SimpleElement'>"
"    <ECCustomAttributes>"
"       <ClassHasHandler xmlns=\"dgn.02.00\" />"
"    </ECCustomAttributes>"
"    <BaseClass>dgn:Element</BaseClass>"
"  </ECClass>"
"</ECSchema>";

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceElementTestFixture::ImportTestSchema () const
    {
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext ();
    BeFileName searchDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (searchDir);
    searchDir.AppendToPath (L"ECSchemas").AppendToPath (L"Dgn");
    schemaContext->AddSchemaPath (searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString (schema, s_testSchemaXml, *schemaContext))
        return ERROR;

    if (SUCCESS != m_db->Schemas ().ImportECSchemas (schemaContext->GetCache ()))
        return ERROR;

    m_db->ClearECDbCache ();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PhysicalModelPtr PerformanceElementTestFixture::CreatePhysicalModel () const
    {
    DgnClassId mclassId = DgnClassId (m_db->Schemas ().GetECClassId (DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel (PhysicalModel::CreateParams (*m_db, mclassId, DgnModel::CreateModelCode ("Instances")));
    EXPECT_EQ (DgnDbStatus::Success, targetModel->Insert ());       /* Insert the new model into the DgnDb */
    return targetModel;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementTestFixture::CommitAndLogTiming (StopWatch& timer, Utf8CP scenario, Utf8String testcaseName, Utf8String testName) const
    {
    StopWatch commitTimer (true);
    ASSERT_EQ (BE_SQLITE_OK, m_db->SaveChanges ());
    commitTimer.Stop ();
    LOG.infov ("%s> Inserting %d instances (5 inheritence levels, 3 properties per class) took %.4f seconds. Commit time: %.4f seconds", scenario, s_instanceCount,
               timer.GetElapsedSeconds (),
               commitTimer.GetElapsedSeconds ());
    LOGTODB (testcaseName, testName, timer.GetElapsedSeconds (), scenario, s_instanceCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementTestFixture::SetUpTestDgnDb (WCharCP destFileName, Utf8CP className)
    {
    BeFileName seedFilePath;
    WString seedFileName;
    bool createSeed = false;

    WString wClassName;
    wClassName.AssignUtf8 (className);
    seedFileName.Sprintf (L"sqlVsecsqlPerformance_%ls_seed%d.idgndb", wClassName, DateTime::GetCurrentTimeUtc ().GetDayOfYear ());

    BeTest::GetHost ().GetOutputRoot (seedFilePath);
    seedFilePath.AppendToPath (seedFileName.c_str ());
    if (seedFilePath.DoesPathExist ())
        {
        }
    else
        createSeed = true;

    if (createSeed)
        {
        SetupProject (L"3dMetricGeneral.idgndb", seedFileName.c_str (), BeSQLite::Db::OpenMode::ReadWrite);
        ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext ();
        BeFileName searchDir;
        BeTest::GetHost ().GetDgnPlatformAssetsDirectory (searchDir);
        searchDir.AppendToPath (L"ECSchemas").AppendToPath (L"Dgn");
        schemaContext->AddSchemaPath (searchDir.GetName ());

        ECN::ECSchemaPtr schema = nullptr;
        ASSERT_EQ (ECN::SCHEMA_READ_STATUS_Success, ECN::ECSchema::ReadFromXmlString (schema, s_testSchemaXml, *schemaContext));

        _RegisterDomainAndImportSchema (schema);
        ASSERT_TRUE (m_db->IsDbOpen ());
        _CreateAndInsertElements (className);
        }

    BeFileName dgndbFilePath;
    BeTest::GetHost ().GetOutputRoot (dgndbFilePath);
    dgndbFilePath.AppendToPath (destFileName);

    ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile (seedFilePath, dgndbFilePath, false));

    DbResult status;
    m_db = DgnDb::OpenDgnDb (&status, dgndbFilePath, DgnDb::OpenParams (Db::OpenMode::ReadWrite));
    EXPECT_EQ (DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE (m_db.IsValid ());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNumberedParams.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ (SUCCESS, ImportTestSchema ());

    PhysicalModelPtr model = CreatePhysicalModel ();
    ASSERT_TRUE (model != nullptr);
    DgnModelId modelId = model->GetModelId ();
    ASSERT_TRUE (modelId.IsValid ());

    StopWatch timer (true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement ("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace,"
                                                                              "Prop1_1,Prop1_2,Prop1_3,"
                                                                              "Prop2_1,Prop2_2,Prop2_3,"
                                                                              "Prop3_1,Prop3_2,Prop3_3,"
                                                                              "Prop4_1,Prop4_2,Prop4_3) "
                                                                              "VALUES (?,?,?,?,'',?,?,?,?,?,?,?,?,?,?,?,?)");
        ASSERT_TRUE (insertStmt != nullptr);
        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId (1, modelId);
        stmt.BindId (2, s_catId);
        stmt.BindId (3, s_codeAuthorityId);
        code.Sprintf ("Id-%d", i);
        stmt.BindText (4, code.c_str (), IECSqlBinder::MakeCopy::No);

        stmt.BindText (5, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (6, s_int64Val);
        stmt.BindDouble (7, s_doubleVal);

        stmt.BindText (8, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (9, s_int64Val);
        stmt.BindDouble (10, s_doubleVal);

        stmt.BindText (11, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (12, s_int64Val);
        stmt.BindDouble (13, s_doubleVal);

        stmt.BindText (14, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (15, s_int64Val);
        stmt.BindDouble (16, s_doubleVal);

        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());

        stmt.Reset ();
        stmt.ClearBindings ();
        }

    timer.Stop ();
    CommitAndLogTiming (timer, "Single Insert (numeric parameters)", TEST_DETAILS);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementTestFixture, ElementInsertInDbWithInsertUpdateApproach)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceInsertUpdateApproach.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ (SUCCESS, ImportTestSchema ());

    PhysicalModelPtr model = CreatePhysicalModel ();
    ASSERT_TRUE (model != nullptr);
    DgnModelId modelId = model->GetModelId ();
    ASSERT_TRUE (modelId.IsValid ());

    StopWatch timer (true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement ("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace) VALUES (?,?,?,?,'')");
        ASSERT_TRUE (insertStmt != nullptr);

        std::vector<CachedECSqlStatementPtr> updateStmts;
        CachedECSqlStatementPtr updateStmt = m_db->GetPreparedECSqlStatement ("UPDATE ONLY ts.Element1 SET Prop1_1 = ?, Prop1_2 = ?, Prop1_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE (updateStmt != nullptr);
        updateStmts.push_back (updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement ("UPDATE ONLY ts.Element2 SET Prop2_1 = ?, Prop2_2 = ?, Prop2_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE (updateStmt != nullptr);
        updateStmts.push_back (updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement ("UPDATE ONLY ts.Element3 SET Prop3_1 = ?, Prop3_2 = ?, Prop3_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE (updateStmt != nullptr);
        updateStmts.push_back (updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement ("UPDATE ONLY ts.Element4 SET Prop4_1 = ?, Prop4_2 = ?, Prop4_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE (updateStmt != nullptr);
        updateStmts.push_back (updateStmt);

        insertStmt->BindId (1, modelId);
        insertStmt->BindId (2, s_catId);
        insertStmt->BindId (3, s_codeAuthorityId);
        code.Sprintf ("Id-%d", i);
        insertStmt->BindText (4, code.c_str (), IECSqlBinder::MakeCopy::No);

        ECInstanceKey newKey;
        ASSERT_EQ (BE_SQLITE_DONE, insertStmt->Step (newKey));
        insertStmt->Reset ();
        insertStmt->ClearBindings ();

        for (CachedECSqlStatementPtr& updateStmt : updateStmts)
            {
            updateStmt->BindText (1, s_textVal, IECSqlBinder::MakeCopy::No);
            updateStmt->BindInt64 (2, s_int64Val);
            updateStmt->BindDouble (3, s_doubleVal);
            updateStmt->BindId (4, newKey.GetECInstanceId ());
            ASSERT_EQ (BE_SQLITE_DONE, updateStmt->Step ());
            updateStmt->Reset ();
            updateStmt->ClearBindings ();
            }
        }

    timer.Stop ();
    CommitAndLogTiming (timer, "Insert & Update sub props", TEST_DETAILS);
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproachNamedParameters)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNamedParams.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ (SUCCESS, ImportTestSchema ());

    PhysicalModelPtr model = CreatePhysicalModel ();
    ASSERT_TRUE (model != nullptr);
    DgnModelId modelId = model->GetModelId ();
    ASSERT_TRUE (modelId.IsValid ());

    StopWatch timer (true);
    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement ("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace,"
                                                                              "Prop1_1,Prop1_2,Prop1_3,"
                                                                              "Prop2_1,Prop2_2,Prop2_3,"
                                                                              "Prop3_1,Prop3_2,Prop3_3,"
                                                                              "Prop4_1,Prop4_2,Prop4_3) "
                                                                              "VALUES (:modelid,:catid,:authorityid,:code,'',"
                                                                              ":p11,:p12,:p13,"
                                                                              ":p21,:p22,:p23,"
                                                                              ":p31,:p32,:p33,"
                                                                              ":p41,:p42,:p43)");
        ASSERT_TRUE (insertStmt != nullptr);

        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId (stmt.GetParameterIndex ("modelid"), modelId);
        stmt.BindId (stmt.GetParameterIndex ("catid"), s_catId);
        stmt.BindId (stmt.GetParameterIndex ("authorityid"), s_codeAuthorityId);
        code.Sprintf ("Id-%d", i);
        stmt.BindText (stmt.GetParameterIndex ("code"), code.c_str (), IECSqlBinder::MakeCopy::No);

        stmt.BindText (stmt.GetParameterIndex ("p11"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (stmt.GetParameterIndex ("p12"), s_int64Val);
        stmt.BindDouble (stmt.GetParameterIndex ("p13"), s_doubleVal);

        stmt.BindText (stmt.GetParameterIndex ("p21"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (stmt.GetParameterIndex ("p22"), s_int64Val);
        stmt.BindDouble (stmt.GetParameterIndex ("p23"), s_doubleVal);

        stmt.BindText (stmt.GetParameterIndex ("p31"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (stmt.GetParameterIndex ("p32"), s_int64Val);
        stmt.BindDouble (stmt.GetParameterIndex ("p33"), s_doubleVal);

        stmt.BindText (stmt.GetParameterIndex ("p41"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64 (stmt.GetParameterIndex ("p42"), s_int64Val);
        stmt.BindDouble (stmt.GetParameterIndex ("p43"), s_doubleVal);

        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());

        stmt.Reset ();
        stmt.ClearBindings ();
        }

    timer.Stop ();
    CommitAndLogTiming (timer, "Single Insert (named parameters)", TEST_DETAILS);
    }
