/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridsTestFixtureBase.cpp $
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
#include <Grids/GridsApi.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED

#define PROJECT_SEED_NAME       L"BuildingTests/GridsTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define PROJECT_TEMP_FOLDER     L"BuildingTests"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void StaticRegisterDomains() {
    DgnDomains::RegisterDomain(Grids::GridsDomain::GetDomain(), DgnDomain::Required::Yes);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::RegisterDomains() 
    {
    StaticRegisterDomains();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
BeFileName GridsTestFixtureBase::GetGivenProjectPath()
    {
    return SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_DEFAULT_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::CopyFile(BeFileName projectPath)
    {
    BeFileName::BeCopyFile(SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_SEED_NAME), projectPath.c_str(), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void GridsTestFixtureBase::SetUpTestCase()
    {
    BeFileName seedPath = SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_SEED_NAME);
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    BeFileName::CreateNewDirectory(outPath);
    seedPath.BeDeleteFile();
    CreateSeedDb(PROJECT_SEED_NAME, PROJECT_TEMP_FOLDER, &StaticRegisterDomains);
    }
