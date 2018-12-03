/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/TestHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestHost.h"
#include <DgnPlatform\DesktopTools\KnownDesktopLocationsAdmin.h>
#include <Bentley\BeTest.h>
#include <Profiles/ProfilesApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_PROFILES

BeFileName ProfilesTestHost::s_baseDbPath;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConverterNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
    {
    virtual BentleyApi::StatusInt _OutputMessage (NotifyMessageDetails const& msg) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"NOTIFICATION-ADMIN")->warningv ("MESSAGE: %s %s\n", msg.GetBriefMsg().c_str(), msg.GetDetailedMsg().c_str());
        return BentleyApi::SUCCESS;
        }

    virtual NotificationManager::MessageBoxValue _OpenMessageBox (NotificationManager::MessageBoxType t, BentleyApi::Utf8CP msg, NotificationManager::MessageBoxIconType iconType) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"NOTIFICATION-ADMIN")->warningv ("MESSAGEBOX: %s\n", msg);
        printf ("<<NOTIFICATION MessageBox: %s >>\n", msg);
        return NotificationManager::MESSAGEBOX_VALUE_Ok;
        }

    virtual void _OutputPrompt (BentleyApi::Utf8CP msg) override
        { // Log this as an error because we cannot prompt while running a unit test!
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"NOTIFICATION-ADMIN")->errorv ("PROMPT (IGNORED): %s\n", msg);
        }

    virtual bool _GetLogSQLiteErrors() override
        {
        return BentleyApi::NativeLogging::LoggingManager::GetLogger ("BeSQLite")->isSeverityEnabled (BentleyApi::NativeLogging::LOG_INFO);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConverterViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override {return nullptr;}
        virtual bool _DoesHostHaveFocus() override {return true;}
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getOutputDirectory()
    {
    BeFileName outputDirectory;
    BeTest::GetHost().GetTempDir (outputDirectory);
    outputDirectory.AppendToPath (L"ProfilesTests");

    if (!BeFileName::DoesPathExist (outputDirectory.c_str()))
        BeFileName::CreateNewDirectory (outputDirectory.c_str());

    return outputDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbPtr createDgnDb (BeFileName const& bimFilename)
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting (true);
    createProjectParams.SetRootSubjectName ("ProfilesTests");
    createProjectParams.SetRootSubjectDescription ("Tests for Profiles domain");

    DbResult status = BeSQLite::DbResult::BE_SQLITE_ERROR;
    DgnDbPtr db = DgnDb::CreateDgnDb (&status, bimFilename, createProjectParams);
    BeAssert (status == BE_SQLITE_OK);

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestHost::ProfilesTestHost()
    {
    DgnViewLib::Initialize (*this, true);

    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    BeAssert (BentleyStatus::SUCCESS == registrationStatus);

    DgnDbPtr dbPtr = createDgnDb (getOutputDirectory().AppendToPath (L"ProfilesTests"));
    BeAssert (dbPtr.IsValid());

    // Get fully qualified path (including db extension)
    s_baseDbPath = dbPtr->GetFileName();
    dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestHost::~ProfilesTestHost()
    {
    Terminate (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestHost& ProfilesTestHost::Instance()
    {
    static ProfilesTestHost testHost;
    return testHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesTestHost::_SupplyProductName (BentleyApi::Utf8StringR name)
    {
    name.assign ("ProfilesDomainTests");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::NotificationAdmin& ProfilesTestHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& ProfilesTestHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles ProfilesTestHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile (GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath (L"sqlang/ProfilesTests_en-US.sqlang.db3");
    BeAssert (sqlangFile.DoesPathExist());

    return L10N::SqlangFiles (sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& ProfilesTestHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }
