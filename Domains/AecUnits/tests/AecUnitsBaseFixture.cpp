/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/AecUnitsBaseFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AecUnitsBaseFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools\KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <AecUnits/AecUnitsApi.h>


using namespace BeSQLite;
using namespace Dgn;

AecUnitsTestsHost AecUnitsBaseFixture::m_host;

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
DgnViewLib::Host::NotificationAdmin& AecUnitsTestsHost::_SupplyNotificationAdmin()
    {
    return *new ConverterNotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles AecUnitsTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/AecUnitsTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& AecUnitsTestsHost::_SupplyViewManager()
    {
    return *new ConverterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& AecUnitsTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AecUnitsBaseFixture::SetUp_CreateNewDgnDb()
    {
    //CreateDgnDbParams createProjectParams;
    //createProjectParams.SetRootSubjectName("AecUnitsTests");
   // DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
   // ASSERT_TRUE(db.IsValid());

    // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
  //  db->ChangeBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
   // db->SaveChanges();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AecUnitsBaseFixture::SetUp()
    {
    //BeFileName tmpDir;
    //BeTest::GetHost().GetTempDir(tmpDir);

    //m_seedDgnDbFileName = tmpDir;
    //m_seedDgnDbFileName.AppendToPath(L"testSeed.bim");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AecUnitsBaseFixture::TearDown()
    {}

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called before the first test in this test case.
//-----------------------------------------------------------------------------------------
void AecUnitsBaseFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
 //   BeFileName::EmptyDirectory(GetOutputDir().c_str());
    }

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called after the last test in this test case.
//-----------------------------------------------------------------------------------------
void AecUnitsBaseFixture::TearDownTestCase()
    {
//    BeFileName::EmptyDirectory(GetOutputDir().c_str());
    m_host.Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//BeFileName AecUnitsBaseFixture::GetOutputDir()
//    {
//    BentleyApi::BeFileName filepath;
//    BentleyApi::BeTest::GetHost().GetOutputRoot(filepath);
//    return filepath;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//BeFileName AecUnitsBaseFixture::GetOutputFileName(WCharCP filename)
//    {
//    BentleyApi::BeFileName filepath = GetOutputDir();
//    filepath.AppendToPath(filename);
//    return filepath;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//void AecUnitsBaseFixture::GetWriteableCopyOfTestData(BeFileNameR cpPath, WCharCP testFileName)
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
//void AecUnitsBaseFixture::GetWriteableCopyOfSeed(BeFileNameR cpPath, WCharCP cpName)
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
void AecUnitsTestFixture::SetUp()
    {
    AecUnitsBaseFixture::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AecUnitsTestFixture::TearDown()
    {
    AecUnitsBaseFixture::TearDown();
    }


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Abeesh.Basheer                  05/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr AecUnitsTestFixture::CreateDgnDb()
	{

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::AecUnits::AecUnitsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return nullptr;

    BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);

	tmpDir.CreateNewDirectory(tmpDir.c_str());

	m_workingBimFile = tmpDir;
	m_workingBimFile.AppendToPath(L"AecUnits.bim");

	//This should create a DGN db with building domain.

	CreateDgnDbParams createProjectParams;
	createProjectParams.SetRootSubjectName("DomainTestFile");
	Dgn::DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, m_workingBimFile, createProjectParams);

	return db;
	}


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Abeesh.Basheer                  05/2017
 +---------------+---------------+---------------+---------------+---------------+------*/

DgnDbPtr AecUnitsTestFixture::OpenDgnDb()
	{

	BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);

	m_workingBimFile = tmpDir;
	m_workingBimFile.AppendToPath(L"AecUnits.bim");

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
//void AecUnitsTestFixture::GetWorkingDb(DgnDbPtr& db, BeFileNameR revitFile,  WCharCP rvtFileName)
//    {
//    GetWriteableCopyOfTestData(revitFile, rvtFileName);
//
//    WString testName(BeTest::GetNameOfCurrentTest(), BentleyCharEncoding::Utf8);
//    BeFileName outputFileName = GetOutputFileName(testName.c_str());
//
//    WChar args[] = {L"AecUnits"};
////    BentleyStatus status = m_bridge._Initialize(1, (WCharCP*) &args);
////    ASSERT_EQ(BentleyStatus::SUCCESS, status);
//
//    CreateDgnDbParams createProjectParams;
//    createProjectParams.SetRootSubjectName("DomainTestFile");
//    db = DgnDb::CreateDgnDb(nullptr, outputFileName, createProjectParams);
//    ASSERT_EQ(true, db.IsValid());
//    }


Dgn::DgnCategoryId AecUnitsTestFixture::QueryPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
    {
    Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

    // Create it if is does not exist.

    if (!id.IsValid())
        {
        id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
        }
    return id;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId AecUnitsTestFixture::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    Dgn::SpatialCategory category(db.GetDictionaryModel(), codeValue, Dgn::DgnCategory::Rank::Domain);
    category.Insert(appearance);
    return category.GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::PhysicalElementPtr  AecUnitsTestFixture::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName)
    {

    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();

    // Find the class

    ECN::ECClassCP buildingClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

    if (nullptr == buildingClass)
        return nullptr;

    ECN::ECClassId classId = buildingClass->GetId();

    Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
    if (NULL == elmHandler)
        return nullptr;

    Utf8String localCategoryName = buildingClass->GetDisplayLabel();

    if (nullptr != categoryName)
        localCategoryName = categoryName;

    Dgn::DgnCategoryId categoryId = QueryPhysicalCategoryId(db, localCategoryName.c_str());

    Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

    Dgn::DgnElementPtr element = elmHandler->Create(params);

    Dgn::PhysicalElementPtr buildingElement = dynamic_pointer_cast<Dgn::PhysicalElement>(element);

    auto geomSource = buildingElement->ToGeometrySourceP();

    if (nullptr == geomSource)
        return nullptr;

    geomSource->SetCategoryId(categoryId);

    return buildingElement;

    }


