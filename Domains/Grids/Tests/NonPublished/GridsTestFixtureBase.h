/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridsTestFixtureBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <TestsBackdoor/TestApplication.h>

#define POINTS_MATCH_TOLERANCE 1.0E-7    //1mm

BEGIN_GRIDS_NAMESPACE

struct GridsTestFixtureBase : BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
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

END_GRIDS_NAMESPACE