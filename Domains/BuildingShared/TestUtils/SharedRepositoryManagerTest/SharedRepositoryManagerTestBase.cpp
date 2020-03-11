/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "SharedRepositoryManagerTestBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED

#define PROJECT_SEED_NAME       L"BuildingTests/BuildingTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define EMPTY_DGNDB             L"DgnDb\\empty.bim"

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void SharedRepositoryManagerTestBase::CreateSeedDb(WCharCP seedFileName, WCharCP appendedPath, void(*RegisterDomainFunction)())
    {
    DgnDbPtr dgnDbPtr;
    ScopedDgnHost host;
    {
    BeFileName  temporaryDir;
    PhysicalModelPtr siteModel;

    DgnViewId viewId;

    DgnClientUi::InitializeBeJsEngine();
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

    BeTest::GetHost().GetOutputRoot(temporaryDir);

    BeSQLite::EC::ECDb::Initialize(temporaryDir, &applicationSchemaDir);

    temporaryDir.AppendToPath(appendedPath);
    BeFileName::CreateNewDirectory(temporaryDir);
    
    (*RegisterDomainFunction)();

    dgnDbPtr = DgnDbTestUtils::CreateSeedDb(seedFileName);
    ASSERT_TRUE(dgnDbPtr.IsValid());

    dgnDbPtr->SetAsBriefcase(BeBriefcaseId(2));
    dgnDbPtr->GeoLocation().SetProjectExtents(AxisAlignedBox3d(DPoint3d::From(-500.0, -500.0, 0.0), DPoint3d::From(500.0, 500.0, 500.0)));

    dgnDbPtr->SaveChanges();
    dgnDbPtr->SaveSettings();
    }
    dgnDbPtr->CloseDb();
    Dgn::T_HOST.GetScriptAdmin().TerminateOnThread();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
BeFileName SharedRepositoryManagerTestBase::BuildProjectPath(WCharCP seedFileName)
    {
    BeFileName seedPath;
    BeTest::GetHost().GetOutputRoot(seedPath);
    seedPath.AppendToPath(seedFileName);
    return seedPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void SharedRepositoryManagerTestBase::SetUp()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::SetUp();
    BeFileName projectPath = GetGivenProjectPath();
    projectPath.BeDeleteFile();
    CopyFile(projectPath);
    //BeFileName::BeCopyFile(GetSeedProjectPath(), projectPath.c_str(), true);

    RegisterDomains();

    EXPECT_EQ(BE_SQLITE_OK, DgnClientUi::OpenDgnDb(projectPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    ASSERT_TRUE(db.IsBriefcase());
    ASSERT_TRUE(db.Txns().IsTracking());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void SharedRepositoryManagerTestBase::TearDown()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
L10N::SqlangFiles SharedRepositoryManagerTestBase::_GetApplicationSqlangs()
    {
    BeFileName documentsRoot;
    BeTest::GetHost().GetDocumentsRoot(documentsRoot);

    BeFileName sqlangDbPath = BeFileName(documentsRoot).AppendToPath(L"../sqlang/DgnClientFx_pseudo.sqlang.db3");
    return L10N::SqlangFiles(sqlangDbPath);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                05/18
//---------------------------------------------------------------------------------------
void SharedRepositoryManagerTestBase::ClearRevisions(DgnDbR db)
    {
    // Ensure the seed file doesn't contain any changes pending for a revision
    if (!db.IsLegacyMaster())
        {
        DgnRevisionPtr rev = db.Revisions().StartCreateRevision();
        if (rev.IsValid())
            {
            db.Revisions().FinishCreateRevision();
            db.SaveChanges();
            }
        }
    }