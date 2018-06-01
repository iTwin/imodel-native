#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <BuildingShared\SharedRepositoryManagerTest.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

struct ClassificationSystemsTestsBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTest
    {
    private:
        static BeFileName GetSeedProjectPath();
    public:
        static void SetUpTestCase();
        virtual void SetUp() override;
        virtual void TearDown() override;
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;
    public:
        BeFileName GetProjectPath() const;
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE