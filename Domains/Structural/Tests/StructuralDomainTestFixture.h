/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <DgnView/DgnViewLib.h>
#include <DgnView/DgnViewAPI.h>
#include <Bentley/BeFileName.h>

using namespace Dgn;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct StructuralDomainTestsHost : DgnViewLib::Host
    {
    protected:
        virtual void _SupplyProductName(BentleyApi::Utf8StringR name) override { name.assign("StructuralDomainTests"); }
        virtual NotificationAdmin& _SupplyNotificationAdmin() override;
        virtual ViewManager& _SupplyViewManager() override;
        virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
class StructuralDomainTestFixture : public testing::Test
    {
    private:
        static StructuralDomainTestsHost m_host;

    protected:
        BeFileName m_workingBimFile;

    public:
        /// <summary>
        /// Sets up create new DGN database. This function deletes the old seed and recreates them
        /// </summary>
        void SetUp_CreateNewDgnDb();

        /// <summary>
        /// Sets up the test fixture
        /// </summary>
        virtual void SetUp();

        /// <summary>
        /// Tears down.
        /// </summary>
        virtual void TearDown();

        /// <summary>
        /// Sets up DgnDb host.
        /// </summary>
        static void SetUpTestCase();

        /// <summary>
        /// Tears down DgnDb host.
        /// </summary>
        static void TearDownTestCase();

        DgnDbPtr CreateDgnDb();
        DgnDbPtr OpenDgnDb();
    };
