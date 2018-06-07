#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>
#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <Bentley/BeTest.h>
#include "SharedRepositoryManagerTest.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct SharedRepositoryManagerTestBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTest
    {
        protected:
            static BeFileName GetSeedProjectPath();
        public:
            static void CreateSeedDb(WCharCP seedFileName, void (*RegisterDomainFunction)());
            virtual void RegisterDomains() = 0;
            virtual void SetUp() override;
            virtual void TearDown() override;
            BeFileName GetProjectPath() const;
            BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;

    };

END_BUILDING_SHARED_NAMESPACE