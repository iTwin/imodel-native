/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BuildingSpatialTestFixtureBase.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <BuildingSpatial/Domain/BuildingSpatialDomain.h>
#include <SpatialComposition/Domain/SpatialCompositionDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "BuildingSpatialTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDINGSPATIAL

#define PROJECT_SEED_NAME       L"BuildingSpatialTests/BuildingSpatialTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingSpatialTests/Default.bim"
#define PROJECT_TEMP_FOLDER     L"BuildingSpatialTests"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Joana.Smitaite                  02/2019
//---------------------------------------------------------------------------------------
void StaticRegisterDomains()
    {
    DgnDomains::RegisterDomain(Dgn::FunctionalDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(SPATIALCOMPOSITION_NAMESPACE_NAME::SpatialCompositionDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(BuildingSpatialDomain::GetDomain(), DgnDomain::Required::Yes);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Joana.Smitaite                  02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialTestFixtureBase::RegisterDomains()
    {
    StaticRegisterDomains();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Joana.Smitaite                  02/2019
//---------------------------------------------------------------------------------------
BeFileName BuildingSpatialTestFixtureBase::GetGivenProjectPath() {
    return SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_DEFAULT_NAME);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Joana.Smitaite                  02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialTestFixtureBase::CopyFile(BeFileName projectPath) {
    BeFileName::BeCopyFile(SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_SEED_NAME), projectPath.c_str(), true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Joana.Smitaite                  02/2019
//---------------------------------------------------------------------------------------
void BuildingSpatialTestFixtureBase::SetUpTestCase ()
    {
    BeFileName seedPath = SharedRepositoryManagerTestBase::BuildProjectPath (PROJECT_SEED_NAME);
    BeFileName outPath;
    BeTest::GetHost ().GetOutputRoot (outPath);
    BeFileName::CreateNewDirectory (outPath);
    seedPath.BeDeleteFile ();
    CreateSeedDb(PROJECT_SEED_NAME, PROJECT_TEMP_FOLDER, &StaticRegisterDomains);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                  02/2019
//---------------+---------------+---------------+---------------+---------------+------
L10N::SqlangFiles BuildingSpatialTestFixtureBase::_GetApplicationSqlangs()
    {
    BeFileName documentsRoot;
    BeTest::GetHost().GetDocumentsRoot(documentsRoot);

    BeFileName sqlangDbPath = BeFileName(documentsRoot).AppendToPath(L"../sqlang/BuildingSpatial_en.sqlang.db3");
    return L10N::SqlangFiles(sqlangDbPath);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                  02/2019
//---------------+---------------+---------------+---------------+---------------+------
void BuildingSpatialTestFixtureBase::SetUp()
    {
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase::SetUp();
    DgnDbR db = *DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create(*rootSubject, "SiteSpatialPartition");
    db.Elements().Insert<SpatialLocationPartition>(*partition);
    m_model = SpatialLocationModel::CreateAndInsert(*partition);
    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                  02/2019
//---------------+---------------+---------------+---------------+---------------+------
void BuildingSpatialTestFixtureBase::TearDown()
    {
    m_model = nullptr;
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase::TearDown();
    }
