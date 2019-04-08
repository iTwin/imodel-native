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

#define PROJECT_SEED_NAME       L"BuildingTests/ClassificationTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define PROJECT_TEMP_FOLDER     L"BuildingTests"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void StaticRegisterDomains() {
    DgnDomains::RegisterDomain(ClassificationSystemsDomain::GetDomain(), DgnDomain::Required::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void ClassificationSystemsTestsBase::RegisterDomains()
    {
    StaticRegisterDomains();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
BeFileName ClassificationSystemsTestsBase::GetGivenProjectPath()
    {
    return SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_DEFAULT_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void ClassificationSystemsTestsBase::CopyFile(BeFileName projectPath)
    {
    BeFileName::BeCopyFile(SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_SEED_NAME), projectPath.c_str(), true);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                06/18
//---------------------------------------------------------------------------------------
void ClassificationSystemsTestsBase::SetUpTestCase()
    {
    BeFileName seedPath = SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_DEFAULT_NAME);
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    BeFileName::CreateNewDirectory(outPath);
    seedPath.BeDeleteFile();
    CreateSeedDb(PROJECT_SEED_NAME, PROJECT_TEMP_FOLDER, &StaticRegisterDomains);
    }
