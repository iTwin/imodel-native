#pragma once
#include <Bentley\BeTest.h>
#include <Bentley\BeAssert.h>
#include <Bentley\BeFileName.h>
#include <DgnView\DgnViewLib.h>
#include <DgnView\DgnViewAPI.h>


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras                 11/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesDomainTestsHost : Dgn::DgnViewLib::Host
    {
    protected:
        virtual void _SupplyProductName(BentleyApi::Utf8StringR name) override;
        virtual NotificationAdmin& _SupplyNotificationAdmin() override;
        virtual Dgn::ViewManager& _SupplyViewManager() override;
        virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras                 11/17
+---------------+---------------+---------------+---------------+---------------+------*/
class ProfilesDomainTestsFixture : public testing::Test
    {
    private:
        static ProfilesDomainTestsHost s_host;
        static BeFileName s_baseDbPath;
        Dgn::DgnDbPtr m_dbPtr;
        Dgn::DgnModelPtr m_modelPtr;

    public:
        ProfilesDomainTestsFixture();

        Dgn::DgnDbR GetDb();
        Dgn::DgnModelR GetModel();

        /// <summary>
        /// Sets up the test fixture
        /// </summary>
        virtual void SetUp() override;

        /// <summary>
        /// Tears down.
        /// </summary>
        virtual void TearDown() override;

        /// <summary>
        /// Sets up DgnDb host.
        /// </summary>
        static void SetUpTestCase();

        /// <summary>
        /// Tears down DgnDb host.
        /// </summary>
        static void TearDownTestCase();
    };
