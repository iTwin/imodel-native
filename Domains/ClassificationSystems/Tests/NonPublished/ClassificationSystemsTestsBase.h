#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

struct ClassificationSystemsTestsBase : BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
    {
    private:
        static BeFileName GetSeedProjectPath();
    public:
        static void SetUpTestCase();
        virtual void SetUp() override;
        virtual void TearDown() override;
        DgnClientFx::DgnClientApp* _CreateApp() override { return new DgnClientFx::DgnClientApp(); }
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;
    public:
        BeFileName GetProjectPath() const;
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE