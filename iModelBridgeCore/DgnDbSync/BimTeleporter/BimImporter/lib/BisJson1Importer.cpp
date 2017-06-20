/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/BisJson1Importer.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GenericDomain.h>
#include <Logging/bentleylogging.h>

#include <BimTeleporter/BisJson1Importer.h>
#include "BisJson1ImporterImpl.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

BEGIN_BIM_TELEPORTER_NAMESPACE

struct WindowsKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDirectory; }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDirectory; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   BentleySystems
    //---------------------------------------------------------------------------------------
    WindowsKnownLocationsAdmin()
        {
        // use the standard Windows temporary directory
        wchar_t tempPathW[MAX_PATH];
        ::GetTempPathW(_countof(tempPathW), tempPathW);
        m_tempDirectory.SetName(tempPathW);
        m_tempDirectory.AppendSeparator();

        // the application directory is where the executable is located
        wchar_t moduleFileName[MAX_PATH];
        ::GetModuleFileNameW(NULL, moduleFileName, _countof(moduleFileName));
        BeFileName moduleDirectory(BeFileName::DevAndDir, moduleFileName);
        m_executableDirectory = moduleDirectory;
        m_executableDirectory.AppendSeparator();

        m_assetsDirectory = m_executableDirectory;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct BimImporterHost : DgnPlatformLib::Host
    {
    virtual void                        _SupplyProductName(Utf8StringR name) override { name.assign("BimTeleporter"); }
    virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override { return *new WindowsKnownLocationsAdmin(); };
    virtual L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlangFile.AppendToPath(L"sqlang");
        sqlangFile.AppendToPath(L"BisJson1Importer_en-US.sqlang.db3");

        return L10N::SqlangFiles(sqlangFile);
        }
    };

struct PCQueue
    {
    public:
        folly::ProducerConsumerQueue<Json::Value> m_objectQueue;
        PCQueue() : m_objectQueue(4096) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Importer::BisJson1Importer(const wchar_t* bimPath) : m_outputPath(bimPath)
    {
    m_queue = new PCQueue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Importer::~BisJson1Importer()
    {
    delete m_importer;
    delete m_host;
    delete m_queue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1Importer::CreateBim()
    {
    m_host = new BimImporterHost();
    DgnPlatformLib::Initialize(*m_host, false);

    Utf8String subjectName(m_outputPath.GetFileNameWithoutExtension());

    DbResult dbStatus;
    Dgn::CreateDgnDbParams params;
    params.SetOverwriteExisting(true);
    params.SetRootSubjectName(subjectName.c_str());

    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&dbStatus, m_outputPath, params);

    if (!dgndb.IsValid())
        {
        // Report Error
        return false;
        }
    m_importer = new BisJson1ImporterImpl(dgndb.get());
    if (SUCCESS != m_importer->InitializeSchemas())
        return false;

    if (SUCCESS != m_importer->CreateAndAttachSyncInfo())
        return false;

    if (SUCCESS != m_importer->ImportJson(m_queue->m_objectQueue))
        return false;
    dgndb->CloseDb();
    dgndb->Release();
    return true;
    }

void BisJson1Importer::AddToQueue(const char* entry)
    {
    Json::Value record;
    Json::Reader::Parse(entry, record);
    m_queue->m_objectQueue.write(record);
    }

void BisJson1Importer::SetDone()
    {
    m_importer->SetDone();
    }

END_BIM_TELEPORTER_NAMESPACE