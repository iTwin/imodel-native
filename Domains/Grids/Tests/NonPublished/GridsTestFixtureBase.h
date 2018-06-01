/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GridsTestFixtureBase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
//#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <BuildingShared\SharedRepositoryManagerTest.h>


#define POINTS_MATCH_TOLERANCE 1.0E-7    //1mm

BEGIN_GRIDS_NAMESPACE

struct GridsTestFixtureBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTest
    {
    private:
        static BeFileName GetSeedProjectPath ();
    public:
        static void SetUpTestCase ();
        virtual void SetUp () override;
        virtual void TearDown () override;
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs () override;
    public:
        BeFileName GetProjectPath () const;
    };



END_GRIDS_NAMESPACE