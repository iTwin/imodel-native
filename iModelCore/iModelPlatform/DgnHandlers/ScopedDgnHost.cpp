/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/BeTest.h>
#include <Bentley/Logging.h>
#include <Bentley/BeDirectoryIterator.h>

#include <DgnPlatform/UnitTests/ScopedDgnHost.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestingConfigurationAdmin : PlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName  m_tmp;
    BeFileName  m_appDir;

    TestingConfigurationAdmin()
        {
        BeTest::GetHost().GetTempDir(m_tmp);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_appDir);
        }

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tmp;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_appDir;}
    };

/*---------------------------------------------------------------------------------**//**
* Here is the real implementation of ScopeDgnHost. Registers itself as a DgnHost in its
* constructor. Supplies key admins that direct DgnPlatform to the files in the
* directories delivered with the unit test framework.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_BENTLEY_DGN_NAMESPACE
struct ScopedDgnHostImpl : PlatformLib::Host
{
    ScopedDgnHostConfig* m_config;
    bool        m_isInitialized;

    ScopedDgnHostImpl(ScopedDgnHostConfig*);
    ~ScopedDgnHostImpl();
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    VisualizationAdmin& _SupplyVisualizationAdmin() override;
    BRepGeometryAdmin& _SupplyBRepGeometryAdmin() override;
};
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHost::ScopedDgnHost(ScopedDgnHostConfig* config)
    {
    m_pimpl = new ScopedDgnHostImpl(config);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHost::~ScopedDgnHost()
    {
    delete m_pimpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHostImpl::ScopedDgnHostImpl(ScopedDgnHostConfig* config) : m_config(config), m_isInitialized(false)
    {
    PlatformLib::StaticInitialize();

    BeAssert((PlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    PlatformLib::Initialize(*this);
    m_isInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHostImpl::~ScopedDgnHostImpl()
    {
    if (m_isInitialized)
        Terminate(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PlatformLib::Host::IKnownLocationsAdmin& ScopedDgnHostImpl::_SupplyIKnownLocationsAdmin() {return *new TestingConfigurationAdmin();}
PlatformLib::Host::VisualizationAdmin& ScopedDgnHostImpl::_SupplyVisualizationAdmin() {
  if (m_config) {
    auto admin = m_config->SupplyVisualizationAdmin();
    if (admin)
      return *admin;
  }

  return PlatformLib::Host::_SupplyVisualizationAdmin();
}

PlatformLib::Host::BRepGeometryAdmin& ScopedDgnHostImpl::_SupplyBRepGeometryAdmin() {
  if (m_config) {
    auto admin = m_config->SupplyBRepGeometryAdmin();
    if (admin)
      return *admin;
  }

  return PlatformLib::Host::_SupplyBRepGeometryAdmin();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TestDataManager::FindTestData(BeFileName& fullFileName, WCharCP fileName, BeFileName const& searchLeafDir)
    {
    BeFileName dir;
    BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(searchLeafDir);

    for (;;)
        {
        fullFileName = dir;
        fullFileName.AppendToPath(fileName);
        if (BeFileName::DoesPathExist(fullFileName))
            return SUCCESS;

        dir.PopDir();
        if (!*dir.GetName())
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TestDataManager::SetAsStandaloneDb(DgnDbPtr& db, DgnDb::OpenMode openMode) {
    if (db->IsWriteableStandalone() || openMode == DgnDb::OpenMode::Readonly && db->IsStandalone())
        return;

    if (openMode == DgnDb::OpenMode::ReadWrite) {
        BeJsDocument val;
        val["txns"] = true;
        db->SaveStandaloneEditFlags(val);
    }

    db->ResetBriefcaseId(BeBriefcaseId(BeBriefcaseId::Standalone()));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TestDataManager::OpenTestFile(bool needTxns)
    {
    DbResult stat;
    DgnDb::OpenParams params(m_openMode);
    m_dgndb = DgnDb::OpenIModelDb(&stat, BeFileName(m_fileName), params);
    if (m_dgndb == NULL)
        {
        if (stat == BE_SQLITE_ERROR_ProfileTooOld)
            {
            NativeLogging::CategoryLogger("BeTest").errorv(L"HORNSWAGGLED! \"%ls\"", m_fileName.c_str());
            BeAssert(false && "HORNSWAGGLED!");
            }
        else
            {
            NativeLogging::CategoryLogger("BeTest").errorv(L"failed to open project fullFileName=\"%ls\" (status=%x)", m_fileName.c_str(), stat);
            BeAssert(false && "failed to open test input project file");
            }
        return ERROR;
        }

    if (needTxns)
        SetAsStandaloneDb(m_dgndb, m_openMode);

    for (ModelIteratorEntryCR entry : m_dgndb->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
        {
        DgnModelPtr dgnModel = m_dgndb->Models().GetModel(entry.GetModelId());
        if (m_model == NULL)
            m_model = dgnModel.get();
        }

    if (NULL == m_model)
        {
        NativeLogging::CategoryLogger("BeTest").errorv(L"failed to load any model from project \"%ls\"", m_fileName.c_str());
        BeAssert(false && "failed to load any model from test input project file");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TestDataManager::CloseTestFile()
    {
    m_model = nullptr;
    m_dgndb = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::TestDataManager(WCharCP fullFileName, Db::OpenMode dbOpenMode, bool needTxns, bool fill)
    {
    m_model     = NULL;
    m_fileName  = fullFileName;
    m_openMode  = dbOpenMode;
    m_fill      = fill;

    OpenTestFile(needTxns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::~TestDataManager()
    {
    m_dgndb = NULL;   // then release/close the project
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP TestDataManager::GetDgnModelP() const {return m_model;}
