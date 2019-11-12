/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "FormsDomainTestFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>

#include "FormsDomain\FormsDomainUtilities.h"

using namespace BeSQLite;
using namespace Dgn;

USING_NAMESPACE_BENTLEY_FORMS

FormsDomainTestsHost FormsDomainTestsFixture::m_host;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras          11/17
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
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::NotificationAdmin& FormsDomainTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConverterViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override {return nullptr;}
        virtual bool _DoesHostHaveFocus() override {return true;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& FormsDomainTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles FormsDomainTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/FormsDomainTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& FormsDomainTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FormsDomainTestsFixture::SetUp_CreateNewDgnDb()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FormsDomainTestsFixture::SetUp()
    {
    FormsDomainUtilities::RegisterDomainHandlers();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FormsDomainTestsFixture::TearDown()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called only once for the the test case. Called before first test. 
+---------------+---------------+---------------+---------------+---------------+------*/
void FormsDomainTestsFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called only once for the the test case. Called after last test. 
+---------------+---------------+---------------+---------------+---------------+------*/
void FormsDomainTestsFixture::TearDownTestCase()
    {
    m_host.Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr FormsDomainTestsFixture::CreateDgnDb()
    {
    FormsDomainUtilities::RegisterDomainHandlers();

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    tmpDir.CreateNewDirectory(tmpDir.c_str());

    m_workingBimFile = tmpDir;
    m_workingBimFile.AppendToPath(L"FormsDomain.bim");

    //This should create a DGN db with building domain.
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("DomainTestFile");
    Dgn::DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_workingBimFile, createProjectParams);

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr FormsDomainTestsFixture::OpenDgnDb()
    {
    FormsDomainUtilities::RegisterDomainHandlers();

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    m_workingBimFile = tmpDir;
    m_workingBimFile.AppendToPath(L"FormsDomain.bim");

    // Initialize parameters needed to create a DgnDb
    Dgn::DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes,
        SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    BeSQLite::DbResult openStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&openStatus, m_workingBimFile, openParams);

    return db;
    }
