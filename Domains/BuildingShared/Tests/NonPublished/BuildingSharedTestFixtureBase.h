/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>

BEGIN_BUILDING_SHARED_NAMESPACE

struct BuildingSharedTestFixtureBase : BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
    {
    private:
        static BeFileName GetSeedProjectPath ();
    public:
        static void SetUpTestCase ();
        virtual void SetUp () override;
        virtual void TearDown () override;
        DgnClientFx::DgnClientApp* _CreateApp () override { return new DgnClientFx::DgnClientApp (); }
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs () override;
    public:
        BeFileName GetProjectPath () const;
    };

END_BUILDING_SHARED_NAMESPACE