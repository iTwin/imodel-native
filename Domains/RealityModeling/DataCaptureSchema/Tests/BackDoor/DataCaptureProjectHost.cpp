/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/DataCaptureProjectHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/DataCapture/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>

#include <DgnPlatform\DgnGeoCoord.h>

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
BeFileName m_tempDir;
BeFileName m_assetsDir;

virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDir; }
virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDir; }

TestKnownLocationsAdmin()
    {
    BeTest::GetHost().GetTempDir(m_tempDir);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
    }
};


#if 0  //This is only available in BIM02 - not on DgnDb06Dev
//=======================================================================================
// @bsiclass
//! Refer to http://bsw-wiki.bentley.com/bin/view.pl/Main/DgnDbServerClientAPIExamples
//! This implementation bypasses all the checks for locks and codes
//=======================================================================================
struct RepositoryManager : IRepositoryManager
{
protected:
    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override { return Response(queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire, req.Options(), RepositoryStatus::Success); }
    virtual RepositoryStatus _Demote(DgnLockSet const&, DgnCodeSet const&, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _Relinquish(Resources, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override { return RepositoryStatus::Success; }
};

//=======================================================================================
// @bsiclass                            Alexandre.Gagnon                        04/2016
//=======================================================================================
struct TestRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
    {
    private:
        mutable RepositoryManager m_client;

    protected:
        virtual IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override { return &m_client; }
    };

#endif


//=======================================================================================
// @bsiclass
//=======================================================================================
struct DataCaptureProjectHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    DataCaptureProjectHostImpl();
    ~DataCaptureProjectHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    //virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    //virtual HandlerAdmin& _SupplyHandlerAdmin() override { return *new TestHandlerAdmin; }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("DataCaptureProjectHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    };

END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureProjectHost::DataCaptureProjectHost()
    {
    m_pimpl = new DataCaptureProjectHostImpl;
    CleanOutputDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureProjectHost::~DataCaptureProjectHost()
    {
    delete m_pimpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureProjectHost::CleanOutputDirectory()
    {
    static bool wasOutputDirectoryCleaned = false;
    if (!wasOutputDirectoryCleaned)
        {
        // clean up files from last run
        BeFileName outputDir = GetOutputDirectory();
        BeFileName::EmptyAndRemoveDirectory(outputDir.GetName());
        BeFileName::CreateNewDirectory(outputDir.GetName());
        wasOutputDirectoryCleaned = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BeFileName DataCaptureProjectHost::GetDocumentsDirectory()
    {
    BeFileName documentsDir;
    BeTest::GetHost().GetDocumentsRoot(documentsDir);
    documentsDir.AppendToPath(L"DgnDb/DataCapture");
    return documentsDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName DataCaptureProjectHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"DataCapture");
    return outputDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataCaptureProjectHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataCaptureProjectHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DataCaptureProjectHost::ImportDataCaptureSchema(DgnDbR dgndb)
    {
    BeFileName assetsRootDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDir);

    BeFileName schemaRootDir = assetsRootDir;
    schemaRootDir.AppendToPath(BDCP_SCHEMA_LOCATION);

    BeFileName DataCaptureSchemaFileName = schemaRootDir;
    DataCaptureSchemaFileName.AppendToPath(BDCP_SCHEMA_FILE);

    auto status = DataCaptureDomain::GetDomain().ImportSchema(dgndb, DataCaptureSchemaFileName);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DataCaptureProjectHost::CreateProject(WCharCP baseName)
    {
    CreateDgnDbParams createDgnDbParams;
    createDgnDbParams.SetOverwriteExisting(true);
//     createDgnDbParams.SetRootSubjectName("DataCaptureProject");
//     createDgnDbParams.SetRootSubjectDescription("Created by DataCaptureProjectHost");
    createDgnDbParams.SetProjectName("DataCaptureProject");
    createDgnDbParams.SetProjectDescription("Created by DataCaptureProjectHost");
//     createDgnDbParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    BeFileName fileName = BuildProjectFileName(baseName);
    DgnDbPtr projectPtr = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!projectPtr.IsValid() || DbResult::BE_SQLITE_OK != createStatus)
        return nullptr;

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = ImportDataCaptureSchema(*projectPtr)))
        return nullptr;

    projectPtr->Schemas().CreateClassViewsInDb();

    auto& spatialModelHandlerR = dgn_ModelHandler::Spatial::GetHandler();
    auto spatialModelPtr = spatialModelHandlerR.Create(DgnModel::CreateParams(*projectPtr, projectPtr->Domains().GetClassId(spatialModelHandlerR),
                                                                              DgnModel::CreateModelCode("Test Spatial Model")));

    if (DgnDbStatus::Success != spatialModelPtr->Insert())
        return nullptr;

    return projectPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DataCaptureProjectHost::OpenProject(WCharCP baseName)
    {
    DbResult openStatus;
    BeFileName fileName = BuildProjectFileName(baseName);

    DgnDbPtr projectPtr = DgnDb::OpenDgnDb(&openStatus, fileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    if (!projectPtr.IsValid() || (DbResult::BE_SQLITE_OK != openStatus))
        return nullptr;

    return projectPtr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureProjectHostImpl::DataCaptureProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this, false);
    DgnDomains::RegisterDomain(DataCaptureDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    m_isInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureProjectHostImpl::~DataCaptureProjectHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }



