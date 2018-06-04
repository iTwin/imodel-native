#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>
#include <BuildingShared/BuildingSharedApi.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/TestHelpers/DgnClientFxTests.h>
#include <Bentley/BeTest.h>


BEGIN_BUILDING_SHARED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Martynas.Saulius   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
    struct MyTestApp : DgnClientFx::DgnClientApp, Dgn::DgnPlatformLib::Host::RepositoryAdmin
    {
    protected:
        mutable Dgn::TestRepositoryManager   m_server;
        Dgn::IRepositoryManagerP _GetRepositoryManager(Dgn::DgnDbR) const override { return &m_server; }

    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Martynas.Saulius   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct SharedRepositoryManagerTest : public BentleyApi::DgnClientFx::TestHelpers::DgnClientFxTests
    {
    public:
        typedef Dgn::IRepositoryManager::Request Request;
        typedef Dgn::IRepositoryManager::Response Response;
        typedef Dgn::IBriefcaseManager::ResponseOptions ResponseOptions;


        DgnClientFx::DgnClientApp* _CreateApp() override { return new MyTestApp(); }

        void ClearRevisions(Dgn::DgnDbR db);

    };

END_BUILDING_SHARED_NAMESPACE