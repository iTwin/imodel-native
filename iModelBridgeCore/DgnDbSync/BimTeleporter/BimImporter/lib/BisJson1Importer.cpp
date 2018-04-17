/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/BisJson1Importer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
struct PCQueue
    {
    public:
        folly::ProducerConsumerQueue<Json::Value> m_objectQueue;
        PCQueue() : m_objectQueue(65536) {}
    };

struct DgnDbPtrHolder
    {
    public:
        DgnDbPtr dgndb;
        DgnDbPtrHolder(BeFileName outputPath);
        void Finalize();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbPtrHolder::DgnDbPtrHolder(BeFileName outputPath)
    {
    //Utf8String subjectName(m_outputPath.GetFileNameWithoutExtension());
    Utf8String subjectName("TBD");

    DbResult dbStatus;
    Dgn::CreateDgnDbParams params;
    params.SetOverwriteExisting(true);
    params.SetRootSubjectName(subjectName.c_str());

    dgndb = DgnDb::CreateDgnDb(&dbStatus, outputPath, params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Importer::BisJson1Importer(const wchar_t* bimPath) : m_outputPath(bimPath), m_importer(nullptr)
    {
    m_queue = new PCQueue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Importer::~BisJson1Importer()
    {
    //delete m_importer;
    delete m_queue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1Importer::CreateBim()
    {

    m_holder = new DgnDbPtrHolder(m_outputPath);
    if (!m_holder->dgndb.IsValid())
        {
        // Report Error
        return false;
        }
    m_importer = new BisJson1ImporterImpl(m_holder->dgndb.get());
    if (SUCCESS != m_importer->InitializeSchemas())
        return false;

    if (SUCCESS != m_importer->CreateAndAttachSyncInfo())
        return false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1Importer::ImportJson(folly::Future<bool>& exporterFuture)
    {
    if (SUCCESS != m_importer->ImportJson(m_queue->m_objectQueue, exporterFuture))
        return false;
    m_holder->dgndb->CloseDb();
    m_holder->dgndb->Release();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1Importer::AddToQueue(const char* entry)
    {
    Json::Value record;
    Json::Reader::Parse(entry, record);
    while (!m_queue->m_objectQueue.write(record))
        {
        continue;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1Importer::SetDone()
    {
    while (nullptr == m_importer)
        BentleyApi::BeThreadUtilities::BeSleep(1000);

    m_importer->SetDone();
    }

END_BIM_TELEPORTER_NAMESPACE