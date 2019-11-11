/*--------------------------------------------------------------------------------------+
|
|     $Source$
|
|  $Copyright$
|
+--------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalTestFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalApi.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomainUtilities.h>


using namespace BeSQLite;
using namespace Dgn;
BridgeStructuralPhysicalTestsHost BridgeStructuralPhysicalTestFixture::m_host;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
DgnViewLib::Host::NotificationAdmin& BridgeStructuralPhysicalTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
struct ConverterViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override {return nullptr;}
        virtual bool _DoesHostHaveFocus() override {return true;}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
ViewManager& BridgeStructuralPhysicalTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
L10N::SqlangFiles BridgeStructuralPhysicalTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/BridgeStructuralPhysicalTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
DgnViewLib::Host::IKnownLocationsAdmin& BridgeStructuralPhysicalTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
void BridgeStructuralPhysicalTestFixture::SetUp_CreateNewDgnDb()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
void BridgeStructuralPhysicalTestFixture::SetUp()
    {
    //!!registration order is important!!
	// Domains must be registered according to their heirarchy
	DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes);
	DgnDomains::RegisterDomain(RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes);
	//Profiles::ProfilesDomainUtilities::RegisterDomainHandlers();
	Forms::FormsDomainUtilities::RegisterDomainHandlers();
	StructuralDomainUtilities::RegisterDomainHandlers();
	BridgeStructuralPhysicalDomainUtilities::RegisterDomainHandlers();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
void BridgeStructuralPhysicalTestFixture::TearDown()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
// @description Method called only once for the the test case. Called before first test. 
//---------------------------------------------------------------------------------------
void BridgeStructuralPhysicalTestFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
// @description Method called only once for the the test case. Called before last test. 
//---------------------------------------------------------------------------------------
void BridgeStructuralPhysicalTestFixture::TearDownTestCase()
    {
    m_host.Terminate(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
// @description Method called only once for the the test case. Called before first test. 
//---------------------------------------------------------------------------------------
DgnDbPtr BridgeStructuralPhysicalTestFixture::CreateDgnDb(WCharCP fileName)
    {
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    tmpDir.CreateNewDirectory(tmpDir.c_str());

    m_workingBimFile = tmpDir;
    m_workingBimFile.AppendToPath(fileName);

    //This should create a DGN db with building domain.
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("DomainTestFile");
    Dgn::DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_workingBimFile, createProjectParams);

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
// @description Method called only once for the the test case. Called before first test. 
//---------------------------------------------------------------------------------------
DgnDbPtr BridgeStructuralPhysicalTestFixture::OpenDgnDb(WCharCP fileName)
	{
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    m_workingBimFile = tmpDir;
    m_workingBimFile.AppendToPath(fileName);

    // Initialize parameters needed to create a DgnDb
    Dgn::DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes,
        Dgn::SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    BeSQLite::DbResult openStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&openStatus, m_workingBimFile, openParams);

    return db;
    }


