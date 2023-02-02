/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECInstanceDeleteTestsFixture : public ::testing::Test
    {
    ScopedDgnHost remoteHost;
    void SetUPDgnProj(DgnDbPtr &dgnProj)
        {
        WCharCP testFileName = L"Main.ibim";
        BeFileName sourceFile = DgnDbTestDgnManager::GetSeedFilePath(testFileName);

        BeFileName dgndbFileName;
        BeTest::GetHost().GetOutputRoot(dgndbFileName);
        dgndbFileName.AppendToPath(testFileName );

        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(sourceFile, dgndbFileName, false));

        DbResult status;
        dgnProj = DgnDb::OpenIModelDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_EQ(DbResult::BE_SQLITE_OK, status) << status;
        ASSERT_TRUE(dgnProj != NULL);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
#if defined (WIP_DGNITEM)
TEST_F(PerformanceECInstanceDeleteTestsFixture, DeleteInstancesOfDgn_ElementItemUsingDerivedClasses)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj(dgnProj);

    ECN::ECClassCP elementItemClass = dgnProj->Schemas().GetClass("dgn", "ElementItem");
    ASSERT_TRUE(elementItemClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(879, stmt.GetValueInt(0));
    stmt.Finalize();

    StopWatch timer;
    double ElapsedTime = 0.0;

    for (auto Class : dgnProj->Schemas().GetDerivedECClasses(*elementItemClass))
        {
        Utf8StringCR SchemaPrefix = Class->GetSchema().GetNamespacePrefix();
        Utf8String ClassName = Class->GetName();
        //LOG.infov ("\n Class Name = %s \n", ClassName.c_str ());

        Utf8String stat = "Delete FROM  ";
        stat.append(SchemaPrefix);
        stat.append(".");
        stat.append(ClassName);
        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, stat.c_str())) << "Prepare failed for " << stat.c_str();
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step()) << "Step failed for " << stat.c_str();
        timer.Stop();
        ElapsedTime += timer.GetElapsedSeconds();
        stmt.Finalize();
        }

    LOGTODB(TEST_DETAILS, ElapsedTime, -1, "ECSql Delete using Derived classes of dgn.ElementItem");

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECInstanceDeleteTestsFixture, CascadeDeleteOnDgn_ElementItem)
    {
    DgnDbPtr dgnProj = nullptr;
    SetUPDgnProj(dgnProj);

    ECN::ECClassCP elementItemClass = dgnProj->Schemas().GetClass("dgn", "ElementItem");
    ASSERT_TRUE(elementItemClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(879, stmt.GetValueInt(0));
    stmt.Finalize();

    Utf8String deleteECSql = "Delete FROM dgn.ElementItem";
    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, deleteECSql.c_str())) << "Prepare failed for %s" << deleteECSql.c_str();
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step()) << "Step failed for " << deleteECSql.c_str();
    timer.Stop();
    stmt.Finalize();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), -1, "ECSql Cascade Delete on dgn.ElementItem");

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgnProj, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ(stmt.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }
#endif
