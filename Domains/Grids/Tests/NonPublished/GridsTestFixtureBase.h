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
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <BuildingShared\SharedRepositoryManagerTestBase.h>


#define POINTS_MATCH_TOLERANCE 1.0E-7    //1mm

BEGIN_GRIDS_NAMESPACE

struct GridsTestFixtureBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase
    {
    public:   
        virtual void RegisterDomains() override;
        static void SetUpTestCase();
    };

END_GRIDS_NAMESPACE