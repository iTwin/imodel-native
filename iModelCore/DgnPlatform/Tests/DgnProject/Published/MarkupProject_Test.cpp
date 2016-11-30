/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/MarkupProject_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnMarkupProject.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

struct DgnMarkupProjectTest : DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMarkupProjectTest, CreateDgnMarkupProject)
    {
    SetupSeedProject();
    DgnDomains::RegisterDomain(MarkupDomain::GetDomain());

    Utf8CP     markupProjectBasename = "CreateDgnMarkupProject.markupdb";
    BeFileName markupProjectFileName;

    DgnModelId  seedModelId;
    DgnViewId   seedViewId;
    if (true)
        {
        markupProjectFileName = DgnDbTestDgnManager::GetOutputFilePath(L"CreateDgnMarkupProject");

        CreateDgnMarkupProjectParams cparms (*m_db);
        cparms.SetOverwriteExisting(true);
        cparms.SetRootSubjectName("CreateDgnMarkupProject");
        DbResult status;
        DgnMarkupProjectPtr mproject = DgnMarkupProject::CreateDgnDb (&status, markupProjectFileName, cparms);
        ASSERT_TRUE( status == BE_SQLITE_OK );
        ASSERT_TRUE( mproject.get() != NULL );
        Utf8String mpname = mproject->GetDbFileName();
        ASSERT_TRUE( mpname.find (markupProjectBasename) != Utf8String::npos );
        }

    DbResult status;

    DgnDb::OpenParams oparms (Db::OpenMode::ReadWrite);
    DgnMarkupProjectPtr mproject = DgnMarkupProject::OpenDgnDb(&status, markupProjectFileName, oparms);
    ASSERT_TRUE( status == BE_SQLITE_OK);
    ASSERT_TRUE( mproject.get() != NULL );
    Utf8String mpname = mproject->GetDbFileName();
    ASSERT_TRUE( mpname.find (markupProjectBasename) != Utf8String::npos );

    // Create a redline model
    DgnDbStatus createStatus = DgnDbStatus::Success;
    auto redline = Redline::Create(&createStatus, *mproject->GetRedlineListModel(), Redline::CreateCode(*mproject, "Redline 1"));
    ASSERT_TRUE(redline.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, createStatus);
    ASSERT_TRUE(redline->Insert(&createStatus).IsValid());
    ASSERT_EQ(DgnDbStatus::Success, createStatus);
    RedlineModelPtr rdlModel = RedlineModel::Create(&createStatus, *redline);
    ASSERT_TRUE(rdlModel.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, createStatus);
    createStatus = rdlModel->Insert();
    ASSERT_EQ(DgnDbStatus::Success, createStatus);

    ASSERT_EQ( rdlModel->GetDgnMarkupProject(), mproject.get() );

    // Create a redline model view
    auto viewSize = DVec2d::From(0.5, 0.5); // in meters
    createStatus = DgnDbStatus::Success;
    RedlineViewDefinitionPtr rdlview = RedlineViewDefinition::Create(&createStatus, *rdlModel, viewSize);
    ASSERT_TRUE(rdlview.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, createStatus);
    RedlineViewDefinitionCPtr persistentView = mproject->Elements().Insert(*rdlview, &createStatus);
    ASSERT_TRUE(persistentView.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, createStatus);
    ASSERT_TRUE(persistentView->GetDelta2d().IsEqual(viewSize));
    ASSERT_TRUE(persistentView->GetOrigin2d().IsEqual(DPoint2d::FromZero()));

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == mproject->SaveChanges());


    }
