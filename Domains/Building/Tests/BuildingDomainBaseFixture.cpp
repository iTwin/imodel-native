/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#ifdef NO_DGNVIEW_IMODEL02

#include "BuildingDomainBaseFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools\KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <BuildingDomain\BuildingDomainApi.h>


using namespace BeSQLite;
using namespace Dgn;

BuildingDomainTestsHost BuildingDomainBaseFixture::m_host;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ConverterViewManager : ViewManager
{
protected:
    virtual Display::SystemContext* _GetSystemContext() override {return nullptr;}
    virtual bool _DoesHostHaveFocus() override {return true;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConverterNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
{
    virtual BentleyApi::StatusInt _OutputMessage(NotifyMessageDetails const& msg) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGE: %s %s\n", msg.GetBriefMsg().c_str(), msg.GetDetailedMsg().c_str());
        return BentleyApi::SUCCESS;
        }

    virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType t, BentleyApi::Utf8CP msg, NotificationManager::MessageBoxIconType iconType) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGEBOX: %s\n", msg);
        printf("<<NOTIFICATION MessageBox: %s >>\n", msg);
        return NotificationManager::MESSAGEBOX_VALUE_Ok;
        }

    virtual void _OutputPrompt(BentleyApi::Utf8CP msg) override
        { // Log this as an error because we cannot prompt while running a unit test!
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->errorv("PROMPT (IGNORED): %s\n", msg);
        }

    virtual bool _GetLogSQLiteErrors() override
        {
        return BentleyApi::NativeLogging::LoggingManager::GetLogger("BeSQLite")->isSeverityEnabled(BentleyApi::NativeLogging::LOG_INFO);
        }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::NotificationAdmin& BuildingDomainTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles BuildingDomainTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/BuildingDomainTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& BuildingDomainTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& BuildingDomainTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingDomainBaseFixture::SetUp_CreateNewDgnDb()
    {
    //CreateDgnDbParams createProjectParams;
    //createProjectParams.SetRootSubjectName("BuildingDomainTests");
   // DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
   // ASSERT_TRUE(db.IsValid());

    // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
  //  db->ChangeBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
   // db->SaveChanges();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingDomainBaseFixture::SetUp()
    {
    //BeFileName tmpDir;
    //BeTest::GetHost().GetTempDir(tmpDir);

    //m_seedDgnDbFileName = tmpDir;
    //m_seedDgnDbFileName.AppendToPath(L"testSeed.bim");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingDomainBaseFixture::TearDown()
    {}

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called before the first test in this test case.
//-----------------------------------------------------------------------------------------
void BuildingDomainBaseFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
 //   BeFileName::EmptyDirectory(GetOutputDir().c_str());
    }

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called after the last test in this test case.
//-----------------------------------------------------------------------------------------
void BuildingDomainBaseFixture::TearDownTestCase()
    {
//    BeFileName::EmptyDirectory(GetOutputDir().c_str());
    m_host.Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//BeFileName BuildingDomainBaseFixture::GetOutputDir()
//    {
//    BentleyApi::BeFileName filepath;
//    BentleyApi::BeTest::GetHost().GetOutputRoot(filepath);
//    return filepath;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//BeFileName BuildingDomainBaseFixture::GetOutputFileName(WCharCP filename)
//    {
//    BentleyApi::BeFileName filepath = GetOutputDir();
//    filepath.AppendToPath(filename);
//    return filepath;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//void BuildingDomainBaseFixture::GetWriteableCopyOfTestData(BeFileNameR cpPath, WCharCP testFileName)
//    {
//    BeFileName srcFileName;
//    BentleyApi::BeTest::GetHost().GetDocumentsRoot(srcFileName);
//    srcFileName.AppendToPath(testFileName);
//
//    cpPath = GetOutputFileName(testFileName);
//
//    BeFileName dirName = cpPath.GetDirectoryName();
//    BeFileName::CreateNewDirectory(cpPath.GetDirectoryName().c_str());
//    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(srcFileName, cpPath));
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//void BuildingDomainBaseFixture::GetWriteableCopyOfSeed(BeFileNameR cpPath, WCharCP cpName)
//    {
//    BeFileName seedDbName = m_seedDgnDbFileName;
//    cpPath = BeFileName(seedDbName.GetDirectoryName());
//    cpPath.AppendToPath(cpName);
//    BeFileName::CreateNewDirectory(cpPath.GetDirectoryName().c_str());
//    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedDbName, cpPath));
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingDomainTestFixture::SetUp()
    {
    BuildingDomainBaseFixture::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingDomainTestFixture::TearDown()
    {
    BuildingDomainBaseFixture::TearDown();
    }


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Abeesh.Basheer                  05/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr BuildingDomainTestFixture::CreateDgnDb()
	{

	BuildingDomain::BuildingDomainUtilities::RegisterDomainHandlers();

	BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);

	tmpDir.CreateNewDirectory(tmpDir.c_str());

	m_workingBimFile = tmpDir;
	m_workingBimFile.AppendToPath(L"BuildingDomain.bim");

	//This should create a DGN db with building domain.

	CreateDgnDbParams createProjectParams;
	createProjectParams.SetRootSubjectName("DomainTestFile");
	Dgn::DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_workingBimFile, createProjectParams);

	return db;
	}


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Abeesh.Basheer                  05/2017
 +---------------+---------------+---------------+---------------+---------------+------*/

DgnDbPtr BuildingDomainTestFixture::OpenDgnDb()
	{
	BuildingDomain::BuildingDomainUtilities::RegisterDomainHandlers();

	BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);

	m_workingBimFile = tmpDir;
	m_workingBimFile.AppendToPath(L"BuildingDomain.bim");

	// Initialize parameters needed to create a DgnDb
	Dgn::DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);

	// Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
	BeSQLite::DbResult openStatus;


	Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&openStatus, m_workingBimFile, openParams);

	return db;
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//void BuildingDomainTestFixture::GetWorkingDb(DgnDbPtr& db, BeFileNameR revitFile,  WCharCP rvtFileName)
//    {
//    GetWriteableCopyOfTestData(revitFile, rvtFileName);
//
//    WString testName(BeTest::GetNameOfCurrentTest(), BentleyCharEncoding::Utf8);
//    BeFileName outputFileName = GetOutputFileName(testName.c_str());
//
//    WChar args[] = {L"BuildingDomain"};
////    BentleyStatus status = m_bridge._Initialize(1, (WCharCP*) &args);
////    ASSERT_EQ(BentleyStatus::SUCCESS, status);
//
//    CreateDgnDbParams createProjectParams;
//    createProjectParams.SetRootSubjectName("DomainTestFile");
//    db = DgnDb::CreateDgnDb(nullptr, outputFileName, createProjectParams);
//    ASSERT_EQ(true, db.IsValid());
//    }

#endif // NO_DGNVIEW_IMODEL02
