/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/MarkupProject_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void checkProjectAssociation (DgnDbR dgnProject, DgnMarkupProjectR markupProject)
    {
    DgnProjectAssociationData::CheckResults results = markupProject.CheckAssociation (dgnProject);
    ASSERT_TRUE( !results.GuidChanged );
    ASSERT_TRUE( !results.NameChanged );
    ASSERT_TRUE( !results.ContentsChanged );
    ASSERT_TRUE( !results.UnitsChanged );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnMarkupProjectTest, CreateDgnMarkupProject)
    {
    ScopedDgnHost  autoDgnHost;

    Utf8CP     markupProjectBasename = "CreateDgnMarkupProject.markupdb";
    BeFileName dgnProjectFileName;
    BeFileName markupProjectFileName;

    DgnModelId  seedModelId;
    DgnViewId   seedViewId;
    if (true)
        {
        DgnDbTestDgnManager tdmSeed (L"empty2d_english.idgndb", __FILE__, Db::OpenMode::Readonly);
        seedModelId = tdmSeed.GetDgnProjectP()->Models().QueryModelId ("RedlineSeedModel");
        seedViewId  = tdmSeed.GetDgnProjectP()->Views().QueryViewId ("RedlineSeedView");

        DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, Db::OpenMode::Readonly);
        DgnDbP project = tdm.GetDgnProjectP();
        ASSERT_TRUE( project != NULL );

        dgnProjectFileName.SetNameUtf8 (project->GetDbFileName());

        markupProjectFileName = DgnDbTestDgnManager::GetOutputFilePath  (L"CreateDgnMarkupProject");

        CreateDgnMarkupProjectParams cparms (*project);
        cparms.SetOverwriteExisting(true);
        cparms.SetSeedDb (BeFileName(tdmSeed.GetPath()));
        DbResult status;
        DgnMarkupProjectPtr mproject = DgnMarkupProject::CreateDgnDb (&status, markupProjectFileName, cparms);
        ASSERT_TRUE( status == BE_SQLITE_OK );
        ASSERT_TRUE( mproject.get() != NULL );
        Utf8String mpname = mproject->GetDbFileName();
        ASSERT_TRUE( mpname.find (markupProjectBasename) != Utf8String::npos );
        }

    //  Both .dgndb are now closed

    DbResult status;

    DgnDb::OpenParams oparms (Db::OpenMode::ReadWrite);
    DgnMarkupProjectPtr mproject = DgnMarkupProject::OpenDgnDb(&status, markupProjectFileName, oparms);
    ASSERT_TRUE( status == BE_SQLITE_OK);
    ASSERT_TRUE( mproject.get() != NULL );
    Utf8String mpname = mproject->GetDbFileName();
    ASSERT_TRUE( mpname.find (markupProjectBasename) != Utf8String::npos );

    // Reopen DgnDb

    DgnDbPtr dgnProject = DgnDb::OpenDgnDb (&status, dgnProjectFileName, oparms);
    ASSERT_TRUE( status == BE_SQLITE_OK );
    ASSERT_TRUE( dgnProject.get() != NULL );

    checkProjectAssociation (*dgnProject, *mproject);

    // Create a redline model
    RedlineModelP rdlModel = mproject->CreateRedlineModel ("foo", seedModelId);
    ASSERT_TRUE( NULL != rdlModel );

    ASSERT_EQ( rdlModel->GetDgnMarkupProject(), mproject.get() );

#ifdef WIP_REDLINE_ECINSTANCE
    // Work with redline properties
    ECN::ECClassCP rdlClass = rdlModel->GetECClass();
    
    if (true) // Check that the Name property was set (in model creation) as expected and can be accessed as an ECProperty
        {
        ECSqlSelectBuilder selectSql;
        selectSql.Select("Name").From(*rdlClass, false).Where ("ECInstanceId=?");
        ECSqlStatement selectStmt;
        selectStmt.Prepare (*mproject, selectSql.ToString ().c_str ());
        selectStmt.BindId (1, rdlModel->GetECInstanceId());
        ASSERT_TRUE (selectStmt.Step() == BE_SQLITE_ROW);
        ASSERT_STREQ( selectStmt.GetValueText(0), "foo" );
        }

    //TODO: ECSQL does not support updates yet. Once it does, this code needs to be adopted
    if (true) // Set the Description property
        {
        ECSqlUpdateBuilder updateSql;
        updateSql.Update(*rdlClass, false).Where("ECInstanceId=?").AddSet("Description", "'Some Description'");
        ECSqlStatement updateStmt;
        updateStmt.Prepare (*mproject, updateSql.ToString ().c_str ());
        updateStmt.BindId (1, rdlModel->GetECInstanceId());
        ASSERT_TRUE (updateStmt.Step() == BE_SQLITE_DONE);
        mproject->SaveChanges();
        }

    if (true) // Check that the Description property was set as expected
        {
        ECSqlSelectBuilder selectSql;
        selectSql.Select("Description").From(*rdlClass, false).Where ("ECInstanceId=?");
        ECSqlStatement selectStmt;
        selectStmt.Prepare (*mproject, selectSql.ToString ().c_str ());
        selectStmt.BindId (1, rdlModel->GetECInstanceId());
        ASSERT_TRUE (selectStmt.Step() == BE_SQLITE_ROW);
        ASSERT_STREQ( selectStmt.GetValueText(0), "Some Description" );
        }
#endif

    // Create a redline model view
    BSIRect rect;
    rect.Init (0,0, 1024, 768);
    mproject->CreateRedlineModelView (*rdlModel, seedViewId, rect, rect);

    // *** WIP Create association - to do that, we'd need a real ViewInfo
    // rdlModel->SetAssociation (*dgnProject, dgnProjectViewInfo);
    }