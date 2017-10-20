/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridsTestFixtureBase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BUILDING
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_CONFIGURABLEUI



#define PROJECT_SEED_NAME       L"BuildingTests/BuildingTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define EMPTY_DGNDB             L"DgnDb\\empty.bim"

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------+---------------+---------------+---------------+---------------+------
void CreatSeedDb (WCharCP seedFileName)
    {
    DgnDbPtr dgnDbPtr;
    ScopedDgnHost host;
    {
    BeFileName  temporaryDir;

    PhysicalModelPtr siteModel;
    DgnViewId viewId;

    DgnClientUi::InitializeBeJsEngine ();
    BeFileName applicationSchemaDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

    BeTest::GetHost ().GetOutputRoot (temporaryDir);

    BeSQLite::EC::ECDb::Initialize (temporaryDir, &applicationSchemaDir);

    temporaryDir.AppendToPath (L"BuildingTests");
    BeFileName::CreateNewDirectory (temporaryDir);


    ASSERT_TRUE (SUCCESS == DgnDomains::RegisterDomain (ConstraintModel::ConstraintModelDomain::GetDomain (), DgnDomain::Required::Yes)) << "Failed to register ConstraintModel domain";
    ASSERT_TRUE (SUCCESS == DgnDomains::RegisterDomain (Grids::GridsDomain::GetDomain (), DgnDomain::Required::Yes)) << "Failed to register Grids domain";

    dgnDbPtr = DgnDbTestUtils::CreateSeedDb (seedFileName);
    ASSERT_TRUE (dgnDbPtr.IsValid ());

    dgnDbPtr->SetAsBriefcase (BeBriefcaseId (BeBriefcaseId::Standalone ()));
    dgnDbPtr->GeoLocation ().SetProjectExtents (AxisAlignedBox3d (DPoint3d::From (-500.0, -500.0, 0.0), DPoint3d::From (500.0, 500.0, 500.0)));

    dgnDbPtr->SaveChanges ();
    dgnDbPtr->SaveSettings ();
    }
    dgnDbPtr->CloseDb ();
    Dgn::T_HOST.GetScriptAdmin ().TerminateOnThread ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
BeFileName GridsTestFixtureBase::GetSeedProjectPath ()
    {
    BeFileName seedPath;
    BeTest::GetHost ().GetOutputRoot (seedPath);
    seedPath.AppendToPath (PROJECT_SEED_NAME);
    return seedPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
BeFileName GridsTestFixtureBase::GetProjectPath () const
    {
    BeFileName projectPath;
    BeTest::GetHost ().GetOutputRoot (projectPath);
    projectPath.AppendToPath (PROJECT_DEFAULT_NAME);
    return projectPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                05/12
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::SetUpTestCase ()
    {
    BeFileName seedPath = GetSeedProjectPath ();
    BeFileName outPath;
    BeTest::GetHost ().GetOutputRoot (outPath);
    BeFileName::CreateNewDirectory (outPath);
    seedPath.BeDeleteFile ();
    CreatSeedDb (PROJECT_SEED_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                05/12
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::SetUp ()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::SetUp ();
    BeFileName projectPath = GetProjectPath ();
    projectPath.BeDeleteFile ();
    BeFileName::BeCopyFile (GetSeedProjectPath (), projectPath.c_str (), true);

    EXPECT_EQ (BE_SQLITE_OK, DgnClientUi::OpenDgnDb (projectPath, DgnDb::OpenParams (Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
    DgnDbR db = *DgnClientFx::DgnClientApp::App ().Project ();
    ASSERT_TRUE (db.IsBriefcase ());
    ASSERT_TRUE (db.Txns ().IsTracking ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                05/12
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::TearDown ()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
L10N::SqlangFiles GridsTestFixtureBase::_GetApplicationSqlangs ()
    {
    BeFileName documentsRoot;
    BeTest::GetHost ().GetDocumentsRoot (documentsRoot);

    BeFileName sqlangDbPath = BeFileName (documentsRoot).AppendToPath (L"../sqlang/DgnClientFx_pseudo.sqlang.db3");
    return L10N::SqlangFiles (sqlangDbPath);
    }

