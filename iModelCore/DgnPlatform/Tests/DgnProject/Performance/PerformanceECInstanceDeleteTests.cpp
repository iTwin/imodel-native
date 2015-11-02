/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceECInstanceDeleteTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECInstanceDeleteTestsFixture : public ::testing::Test
    {
    ScopedDgnHost remoteHost;
    void SetUPDgnProj (DgnDbPtr &dgnProj)
        {
        WCharCP testFileName = L"Main.idgndb";
        BeFileName sourceFile = DgnDbTestDgnManager::GetSeedFilePath (testFileName);

        BeFileName dgndbFileName;
        BeTest::GetHost ().GetOutputRoot (dgndbFileName);
        dgndbFileName.AppendToPath (testFileName);

        ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile (sourceFile, dgndbFileName, false));

        DbResult status;
        dgnProj = DgnDb::OpenDgnDb (&status, dgndbFileName, DgnDb::OpenParams (Db::OpenMode::ReadWrite));
        EXPECT_EQ (DbResult::BE_SQLITE_OK, status) << status;
        ASSERT_TRUE (dgnProj != NULL);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesOfDgn_ElementItemUsingDerivedClasses)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj (dgnProj);

    ECN::ECClassCP elementItemClass = dgnProj->Schemas ().GetECClass ("dgn", "ElementItem");
    ASSERT_TRUE (elementItemClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (879, stmt.GetValueInt (0));
    stmt.Finalize ();

    StopWatch timer;
    double ElapsedTime = 0.0;

    for (auto Class : dgnProj->Schemas().GetDerivedECClasses (*elementItemClass))
        {
        Utf8StringCR SchemaPrefix = Class->GetSchema ().GetNamespacePrefix ();
        Utf8String ClassName = Class->GetName ();
        //LOG.infov ("\n Class Name = %s \n", ClassName.c_str ());

        Utf8String stat = "Delete FROM ";
        stat.append (SchemaPrefix);
        stat.append (".");
        stat.append (ClassName);
        timer.Start ();
        ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, stat.c_str ())) << "Prepare failed for " << stat.c_str ();
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
        timer.Stop ();
        ElapsedTime += timer.GetElapsedSeconds ();
        stmt.Finalize ();
        }

    LOGTODB (TEST_DETAILS, ElapsedTime, "ECSql Delete using Derived classes of dgn.ElementItem");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, CascadeDeleteOnDgn_ElementItem)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj (dgnProj);

    ECN::ECClassCP elementItemClass = dgnProj->Schemas ().GetECClass ("dgn", "ElementItem");
    ASSERT_TRUE (elementItemClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (879, stmt.GetValueInt (0));
    stmt.Finalize ();

    Utf8String deleteECSql = "Delete FROM dgn.ElementItem";
    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, deleteECSql.c_str ())) << "Prepare failed for %s" << deleteECSql.c_str ();
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << deleteECSql.c_str ();
    timer.Stop ();
    stmt.Finalize ();

    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), "ECSql Cascade Delete on dgn.ElementItem");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, CascadeDeleteOnDgn_Element)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj (dgnProj);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (2182, stmt.GetValueInt (0));
    stmt.Finalize ();

    Utf8String deleteECSql = "Delete FROM dgn.Element";
    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, deleteECSql.c_str ())) << "Prepare failed for %s" << deleteECSql.c_str ();
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << deleteECSql.c_str ();
    timer.Stop ();
    stmt.Finalize ();

    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), "ECSql Cascade Delete on dgn.Element");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesOfDgn_ElementUsingDerivedClasses)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj (dgnProj);

    ECN::ECClassCP elementItemClass = dgnProj->Schemas ().GetECClass ("dgn", "Element");
    ASSERT_TRUE (elementItemClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (2182, stmt.GetValueInt (0));
    stmt.Finalize ();

    StopWatch timer;
    double deleteTime = 0.0;

    for (auto Class : dgnProj->Schemas ().GetDerivedECClasses (*elementItemClass))
        {
        Utf8StringCR SchemaPrefix = Class->GetSchema ().GetNamespacePrefix ();
        Utf8String ClassName = Class->GetName ();
        //LOG.infov ("\n Class Name = %s \n", ClassName.c_str ());

        Utf8String stat = "Delete FROM ";
        stat.append (SchemaPrefix);
        stat.append (".");
        stat.append (ClassName);
        timer.Start ();
        ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, stat.c_str ())) << "Prepare failed for " << stat.c_str ();
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
        timer.Stop ();
        deleteTime += timer.GetElapsedSeconds ();
        stmt.Finalize ();
        }

    LOGTODB (TEST_DETAILS, deleteTime, "ECSql Delete using Derived classes of dgn.Element");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*dgnProj, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }