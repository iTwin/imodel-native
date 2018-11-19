#include "ProfilesDomainTestFixture.h"
#include <DgnPlatform\DgnPlatformLib.h>
#include <DgnPlatform\DesktopTools\KnownDesktopLocationsAdmin.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Profiles/ProfilesApi.h>

using namespace BeSQLite;
using namespace Dgn;

ProfilesDomainTestsHost ProfilesDomainTestsFixture::s_host;
BeFileName ProfilesDomainTestsFixture::s_baseDbPath;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras          11/17
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
* @bsiclass                                     Arturas.Mizaras          11/17
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
void ProfilesDomainTestsHost::_SupplyProductName (BentleyApi::Utf8StringR name)
    {
    name.assign ("ProfilesDomainTests");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::NotificationAdmin& ProfilesDomainTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& ProfilesDomainTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles ProfilesDomainTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile (GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath (L"sqlang/ProfilesTests_en-US.sqlang.db3");
    BeAssert (sqlangFile.DoesPathExist());

    return L10N::SqlangFiles (sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& ProfilesDomainTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesDomainTestsFixture::ProfilesDomainTestsFixture()
    : m_dbPtr (nullptr), m_modelPtr (nullptr)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbR ProfilesDomainTestsFixture::GetDb()
    {
    BeAssert (m_dbPtr.IsValid());
    return *m_dbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelR ProfilesDomainTestsFixture::GetModel()
    {
    BeAssert (m_modelPtr.IsValid());
    return *m_modelPtr;
    }

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
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getWorkingDbPath (BeFileName const& baseDbPath)
    {
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    return BeFileName (directory + name + L"_Working." + extension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbPtr createDgnDb (BeFileName const& bimFilename)
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting (true);
    createProjectParams.SetRootSubjectName ("ProfilesTest");
    createProjectParams.SetRootSubjectDescription ("Tests for Profiles domain handlers");

    DbResult status = BeSQLite::DbResult::BE_SQLITE_ERROR;
    DgnDbPtr db = DgnDb::CreateDgnDb (&status, bimFilename, createProjectParams);
    BeAssert (status == BE_SQLITE_OK);

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called before each test. Copy base (empty) db file and open it as
* a working db for the test.
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesDomainTestsFixture::SetUp()
    {
    BeFileName workingDbPath = getWorkingDbPath (s_baseDbPath);
    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (s_baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    DgnDb::OpenParams openParams (BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    BeSQLite::DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid());

    SubjectCPtr rootSubjectPtr = m_dbPtr->Elements().GetRootSubject();
    DefinitionPartitionPtr partitionPtr = DefinitionPartition::Create (*rootSubjectPtr, "TestPartition");
    m_dbPtr->BriefcaseManager().AcquireForElementInsert (*partitionPtr);

    DgnDbStatus status;
    m_dbPtr->Elements().Insert<DefinitionPartition> (*partitionPtr, &status);
    BeAssert (status == DgnDbStatus::Success);

    m_modelPtr = DefinitionModel::Create (*partitionPtr);
    status = m_modelPtr->Insert();
    BeAssert (status == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called after each test.
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesDomainTestsFixture::TearDown()
    {
    m_dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called only once for the the test case. Called before first test.
* Create a base db file that will be used as a starting point for each test. 
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesDomainTestsFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize (s_host, true); // this initializes the DgnDb libraries

    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (Profiles::ProfilesDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    BeAssert (BentleyStatus::SUCCESS == registrationStatus);

    DgnDbPtr dbPtr = createDgnDb (getOutputDirectory().AppendToPath (L"ProfilesTests"));
    BeAssert (dbPtr.IsValid());

    // Get fully qualified path (including db extension)
    s_baseDbPath = dbPtr->GetFileName();
    dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras          11/17
* @description Method called only once for the the test case. Called after last test. 
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesDomainTestsFixture::TearDownTestCase()
    {
    s_host.Terminate (false);
    }
