#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <BuildingShared\SharedRepositoryManagerTestBase.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

struct ClassificationSystemsTestsBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase
    {
        virtual void RegisterDomains() override;
        virtual BeFileName GetGivenProjectPath() override;
        virtual void CopyFile(BeFileName projectPath) override;
        static void SetUpTestCase();

        static ClassificationTablePtr CreateAndInsertTable(Dgn::DgnDbR db);
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE