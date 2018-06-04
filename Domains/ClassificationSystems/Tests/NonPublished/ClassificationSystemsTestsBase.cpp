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
void ClassificationSystemsTestsBase::SetUpTestCase()
    {
    BeFileName seedPath = SharedRepositoryManagerTestBase::GetSeedProjectPath();
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    BeFileName::CreateNewDirectory(outPath);
    seedPath.BeDeleteFile();
    CreateSeedDb(PROJECT_SEED_NAME, &StaticRegisterDomains);
    }
