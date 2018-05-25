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
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>

#define POINTS_MATCH_TOLERANCE 1.0E-7    //1mm

BEGIN_GRIDS_NAMESPACE

struct MyTestApp : DgnClientFx::DgnClientApp, Dgn::DgnPlatformLib::Host::RepositoryAdmin 
    {
protected:
    mutable Dgn::TestRepositoryManager   m_server;
    Dgn::IRepositoryManagerP _GetRepositoryManager(Dgn::DgnDbR) const override { return &m_server; }

    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Martynas.Saulius   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryManagerTest : public BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
    {
    public:
        typedef Dgn::IRepositoryManager::Request Request;
        typedef Dgn::IRepositoryManager::Response Response;
        typedef Dgn::IBriefcaseManager::ResponseOptions ResponseOptions;


        DgnClientFx::DgnClientApp* _CreateApp() override { return new MyTestApp(); }

        void ClearRevisions(Dgn::DgnDbR db);

    };



struct GridsTestFixtureBase : RepositoryManagerTest
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