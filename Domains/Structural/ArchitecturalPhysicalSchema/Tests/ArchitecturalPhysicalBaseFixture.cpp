/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/Tests/ArchitecturalPhysicalBaseFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalBaseFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>

using namespace BeSQLite;
using namespace Dgn;

ArchitecturalPhysicalTestsHost ArchitecturalPhysicalBaseFixture::m_host;

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
DgnViewLib::Host::NotificationAdmin& ArchitecturalPhysicalTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles ArchitecturalPhysicalTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/ArchitecturalPhysicalTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& ArchitecturalPhysicalTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& ArchitecturalPhysicalTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new WindowsKnownLocationsAdmin();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalBaseFixture::SetUp_CreateNewDgnDb()
    {
    //CreateDgnDbParams createProjectParams;
    //createProjectParams.SetRootSubjectName("ArchitecturalPhysicalTests");
   // DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
   // ASSERT_TRUE(db.IsValid());

    // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
  //  db->ChangeBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
   // db->SaveChanges();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalBaseFixture::SetUp()
    {
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    m_seedDgnDbFileName = tmpDir;
    m_seedDgnDbFileName.AppendToPath(L"testSeed.bim");

    /*static bool s_isSeedCreated;
    if (!s_isSeedCreated)
        {
        BeFileName::CreateNewDirectory(tmpDir.c_str());
        SetUp_CreateNewDgnDb();
        s_isSeedCreated = true;
        }*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalBaseFixture::TearDown()
    {}

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called before the first test in this test case.
//-----------------------------------------------------------------------------------------
void ArchitecturalPhysicalBaseFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
    BeFileName::EmptyDirectory(GetOutputDir().c_str());
    }

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called after the last test in this test case.
//-----------------------------------------------------------------------------------------
void ArchitecturalPhysicalBaseFixture::TearDownTestCase()
    {
    BeFileName::EmptyDirectory(GetOutputDir().c_str());
    m_host.Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ArchitecturalPhysicalBaseFixture::GetOutputDir()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(filepath);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ArchitecturalPhysicalBaseFixture::GetOutputFileName(WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalBaseFixture::GetWriteableCopyOfTestData(BeFileNameR cpPath, WCharCP testFileName)
    {
    BeFileName srcFileName;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(srcFileName);
    srcFileName.AppendToPath(testFileName);

    cpPath = GetOutputFileName(testFileName);

    BeFileName dirName = cpPath.GetDirectoryName();
    BeFileName::CreateNewDirectory(cpPath.GetDirectoryName().c_str());
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(srcFileName, cpPath));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalBaseFixture::GetWriteableCopyOfSeed(BeFileNameR cpPath, WCharCP cpName)
    {
    BeFileName seedDbName = m_seedDgnDbFileName;
    cpPath = BeFileName(seedDbName.GetDirectoryName());
    cpPath.AppendToPath(cpName);
    BeFileName::CreateNewDirectory(cpPath.GetDirectoryName().c_str());
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedDbName, cpPath));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalTestFixture::SetUp()
    {
    ArchitecturalPhysicalBaseFixture::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalTestFixture::TearDown()
    {
    ArchitecturalPhysicalBaseFixture::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalTestFixture::GetWorkingDb(DgnDbPtr& db, BeFileNameR revitFile,  WCharCP rvtFileName)
    {
    GetWriteableCopyOfTestData(revitFile, rvtFileName);

    WString testName(BeTest::GetNameOfCurrentTest(), BentleyCharEncoding::Utf8);
    BeFileName outputFileName = GetOutputFileName(testName.c_str());

    WChar args[] = {L"ArchitecturalPhysical"};
//    BentleyStatus status = m_bridge._Initialize(1, (WCharCP*) &args);
//    ASSERT_EQ(BentleyStatus::SUCCESS, status);

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("DomainTestFile");
    db = DgnDb::CreateDgnDb(nullptr, outputFileName, createProjectParams);
    ASSERT_EQ(true, db.IsValid());
    }