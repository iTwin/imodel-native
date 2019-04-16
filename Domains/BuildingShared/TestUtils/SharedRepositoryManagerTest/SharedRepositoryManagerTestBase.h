/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>
#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <Bentley/BeTest.h>

BEGIN_BUILDING_SHARED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Martynas.Saulius   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct MyTestApp : DgnClientFx::DgnClientApp, Dgn::DgnPlatformLib::Host::RepositoryAdmin
    {
    protected:
        mutable Dgn::TestRepositoryManager   m_server;
        Dgn::IRepositoryManagerP _GetRepositoryManager(Dgn::DgnDbR) const override { return &m_server; }

    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Martynas.Saulius   06/18
+---------------+---------------+---------------+---------------+---------------+------*/

struct SharedRepositoryManagerTestBase : public BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
    {
        protected:
            static BeFileName BuildProjectPath(WCharCP seedFileName);
        public:
            static void CreateSeedDb(WCharCP seedFileName, WCharCP appendedPath, void (*RegisterDomainFunction)());
            virtual void RegisterDomains() = 0;
            virtual BeFileName GetGivenProjectPath() = 0;
            virtual void CopyFile(BeFileName projectPath) = 0;
            virtual void SetUp() override;
            virtual void TearDown() override;
            BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;

            typedef Dgn::IRepositoryManager::Request Request;
            typedef Dgn::IRepositoryManager::Response Response;
            typedef Dgn::IBriefcaseManager::ResponseOptions ResponseOptions;

            DgnClientFx::DgnClientApp* _CreateApp() override { return new MyTestApp(); }

            void ClearRevisions(Dgn::DgnDbR db);

    };

END_BUILDING_SHARED_NAMESPACE