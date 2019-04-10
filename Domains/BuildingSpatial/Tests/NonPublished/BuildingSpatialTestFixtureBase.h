/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BuildingSpatialTestFixtureBase.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <BuildingShared\SharedRepositoryManagerTestBase.h>
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>

#define POINTS_MATCH_TOLERANCE 1.0E-7    //1mm

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BUILDINGSPATIAL_NAMESPACE

struct BuildingSpatialTestFixtureBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase
    {
    protected:
        SpatialLocationModelPtr m_model;

    public:
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;
        virtual void RegisterDomains() override;
        virtual BeFileName GetGivenProjectPath() override;
        virtual void CopyFile(BeFileName projectPath) override;
        static void SetUpTestCase ();
        static DgnDbR GetDgnDb() { return *DgnClientFx::DgnClientApp::App().Project(); }

        void SetUp() override;
        void TearDown() override;
    };

END_BUILDINGSPATIAL_NAMESPACE
