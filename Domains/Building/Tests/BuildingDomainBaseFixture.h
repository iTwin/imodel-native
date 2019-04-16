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
//#include <RevitImodelBridge/RevitImodelBridge.h>
//#include <iModelBridge/iModelBridgeSacAdapter.h>

using namespace Dgn;

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
struct BuildingDomainTestsHost : DgnViewLib::Host
    {
    protected:
        virtual void _SupplyProductName(BentleyApi::Utf8StringR name) override { name.assign("BuildingDomainTests"); }
        virtual ViewManager& _SupplyViewManager() override;
#if defined (NOT_NOW_PARASOLID)
        virtual SolidsKernelAdmin& _SupplySolidsKernelAdmin() override { return *new PSolidKernelAdmin(); }
#endif
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
        virtual NotificationAdmin& _SupplyNotificationAdmin() override;
        virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
    };

/// <summary>
/// Base fixture
/// </summary>
/// <seealso cref="testing::Test" />
class BuildingDomainBaseFixture : public testing::Test
    {
    private:
        static BuildingDomainTestsHost m_host;

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

        /// <summary>
        /// Gets the output dir from the output directory defined in BeTest host.
        /// </summary>
        /// <returns></returns>
    //    static BeFileName GetOutputDir();

        /// <summary>
        /// Gets the name of the output file.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <returns></returns>
     //   BeFileName GetOutputFileName(WCharCP filename);

        /// <summary>
        /// Copies the test data from prouct output to the temp working directory.
        /// </summary>
        /// <param name="cpPath">copied file.</param>
        /// <param name="cpName">Name of the file to copy.</param>
       // void GetWriteableCopyOfTestData(BeFileNameR cpPath, WCharCP name);

        /// <summary>
        /// Copies an empty dgndb file to the temp working directory.
        /// </summary>
        /// <param name="cpPath">copied file.</param>
        /// <param name="cpName">Name of the file to copy.</param>
       // void GetWriteableCopyOfSeed(BeFileNameR cpPath, WCharCP cpName);




    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
/// <summary>
/// The bridge test fixture ensure that a working bridge is initialized
/// </summary>
/// <seealso cref="RevitBridgeBaseFixture" />
class BuildingDomainTestFixture : public BuildingDomainBaseFixture
    {

	private:

   // Revit::RevitBridge m_bridge;
    public:
    /// <summary>
    /// Sets up the bridge host.
    /// </summary>
    virtual void SetUp();

    /// <summary>
    /// Tears down DgnDb host.
    /// </summary>
    virtual void TearDown();


  //  void    GetWorkingDb(DgnDbPtr& db, BeFileNameR revitFile, WCharCP rvtFileName);

	DgnDbPtr CreateDgnDb();
	DgnDbPtr OpenDgnDb();


    };