#pragma once

#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE


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


struct ClassificationSystemsTestsBase : RepositoryManagerTest
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