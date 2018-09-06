/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BuildingSharedTestFixtureBase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "BuildingSharedTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED


#define PROJECT_SEED_NAME       L"BuildingSharedTests/BuildingSharedTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingSharedTests/Default.bim"
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

    temporaryDir.AppendToPath (L"BuildingSharedTests");
    BeFileName::CreateNewDirectory (temporaryDir);

    dgnDbPtr = DgnDbTestUtils::CreateSeedDb (seedFileName);
    ASSERT_TRUE (dgnDbPtr.IsValid ());

    dgnDbPtr->SetAsBriefcase (BeBriefcaseId (BeBriefcaseId::Standalone ()));

    // Import the units and formats schema.
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    static ECN::SchemaKey unitsKey("Units", 1, 0, 0);
    ECN::ECSchemaPtr units = context->LocateSchema(unitsKey, ECN::SchemaMatchType::Latest);
    ASSERT_TRUE(units.IsValid());

    static ECN::SchemaKey formatsKey("Formats", 1, 0, 0);
    ECN::ECSchemaPtr formats = context->LocateSchema(formatsKey, ECN::SchemaMatchType::Latest);
    ASSERT_TRUE(formats.IsValid());

    ECN::ECSchemaCachePtr schemaList = ECN::ECSchemaCache::Create();
    schemaList->AddSchema(*units.get());
    schemaList->AddSchema(*formats.get());
    dgnDbPtr->ImportSchemas(schemaList->GetSchemas());

    dgnDbPtr->GeoLocation ().SetProjectExtents (AxisAlignedBox3d (DPoint3d::From (-500.0, -500.0, 0.0), DPoint3d::From (500.0, 500.0, 500.0)));

    dgnDbPtr->SaveChanges();
    dgnDbPtr->SaveSettings();
    }
    dgnDbPtr->CloseDb ();
    Dgn::T_HOST.GetScriptAdmin ().TerminateOnThread ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
BeFileName BuildingSharedTestFixtureBase::GetSeedProjectPath ()
    {
    BeFileName seedPath;
    BeTest::GetHost ().GetOutputRoot (seedPath);
    seedPath.AppendToPath (PROJECT_SEED_NAME);
    return seedPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
BeFileName BuildingSharedTestFixtureBase::GetProjectPath () const
    {
    BeFileName projectPath;
    BeTest::GetHost ().GetOutputRoot (projectPath);
    projectPath.AppendToPath (PROJECT_DEFAULT_NAME);
    return projectPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                05/12
//---------------------------------------------------------------------------------------
void BuildingSharedTestFixtureBase::SetUpTestCase ()
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
void BuildingSharedTestFixtureBase::SetUp ()
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
void BuildingSharedTestFixtureBase::TearDown ()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/17
//---------------------------------------------------------------------------------------
L10N::SqlangFiles BuildingSharedTestFixtureBase::_GetApplicationSqlangs ()
    {
    BeFileName documentsRoot;
    BeTest::GetHost ().GetDocumentsRoot (documentsRoot);

    BeFileName sqlangDbPath = BeFileName (documentsRoot).AppendToPath (L"../sqlang/DgnClientFx_pseudo.sqlang.db3");
    return L10N::SqlangFiles (sqlangDbPath);
    }

