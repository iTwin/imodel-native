#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_CLASSIFICATIONSYSTEMS
USING_NAMESPACE_BENTLEY_DGNCLIENTFX



#define PROJECT_SEED_NAME       L"BuildingTests/BuildingTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define EMPTY_DGNDB             L"DgnDb\\empty.bim"

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void CreatSeedDb(WCharCP seedFileName)
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

    temporaryDir.AppendToPath(L"BuildingTests");
    BeFileName::CreateNewDirectory(temporaryDir);

    DgnDomains::RegisterDomain(ClassificationSystemsDomain::GetDomain(), DgnDomain::Required::Yes);

    dgnDbPtr = DgnDbTestUtils::CreateSeedDb(seedFileName);
    ASSERT_TRUE(dgnDbPtr.IsValid());

    dgnDbPtr->SetAsBriefcase(BeBriefcaseId(BeBriefcaseId::Standalone()));
    dgnDbPtr->GeoLocation().SetProjectExtents(AxisAlignedBox3d(DPoint3d::From(-500.0, -500.0, 0.0), DPoint3d::From(500.0, 500.0, 500.0)));

    dgnDbPtr->SaveChanges();
    dgnDbPtr->SaveSettings();
    }
    dgnDbPtr->CloseDb();
    Dgn::T_HOST.GetScriptAdmin().TerminateOnThread();
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ClassificationSystemsTestsBase::SetUpTestCase()
    {
    BeFileName seedPath = GetSeedProjectPath();
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    BeFileName::CreateNewDirectory(outPath);
    seedPath.BeDeleteFile();
    CreatSeedDb(PROJECT_SEED_NAME);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ClassificationSystemsTestsBase::TearDown()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::TearDown();
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ClassificationSystemsTestsBase::SetUp()
    {
    BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests::SetUp();
    BeFileName projectPath = GetProjectPath();
    projectPath.BeDeleteFile();
    BeFileName::BeCopyFile(GetSeedProjectPath(), projectPath.c_str(), true);
    DgnDomains::RegisterDomain(ClassificationSystemsDomain::GetDomain(), DgnDomain::Required::Yes);

    EXPECT_EQ(BE_SQLITE_OK, DgnClientUi::OpenDgnDb(projectPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    ASSERT_TRUE(db.IsBriefcase());
    ASSERT_TRUE(db.Txns().IsTracking());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
BeFileName ClassificationSystemsTestsBase::GetSeedProjectPath()
    {
    BeFileName seedPath;
    BeTest::GetHost().GetOutputRoot(seedPath);
    seedPath.AppendToPath(PROJECT_SEED_NAME);
    return seedPath;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
BeFileName ClassificationSystemsTestsBase::GetProjectPath() const
    {
    BeFileName projectPath;
    BeTest::GetHost().GetOutputRoot(projectPath);
    projectPath.AppendToPath(PROJECT_DEFAULT_NAME);
    return projectPath;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
L10N::SqlangFiles ClassificationSystemsTestsBase::_GetApplicationSqlangs()
    {
    BeFileName documentsRoot;
    BeTest::GetHost().GetDocumentsRoot(documentsRoot);

    BeFileName sqlangDbPath = BeFileName(documentsRoot).AppendToPath(L"../sqlang/DgnClientFx_pseudo.sqlang.db3");
    return L10N::SqlangFiles(sqlangDbPath);
    }