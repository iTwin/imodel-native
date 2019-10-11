/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <Logging/bentleylogging.h>
#include <Raster/RasterApi.h>
#include <PointCloud/PointCloudApi.h>
#include <ThreeMx/ThreeMxApi.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>

#include <BimFromDgnDb/BimFromJson.h>
#include "BimFromJsonImpl.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_RASTER
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

BEGIN_BIM_FROM_DGNDB_NAMESPACE
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
BimFromJson::BimFromJson(const wchar_t* bimPath) : m_outputPath(bimPath), m_importer(nullptr)
    {
    m_queue = new PCQueue();

    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(Raster::RasterDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    //HINSTANCE handle = ::LoadLibrary("d:\\bsw\\bim\\0200dev\\output\\DgnClient\\Debug\\Winx64\\build\\ConstructionSchema\\ConstructionSchemaB02.dll");
    //if (nullptr != handle)
    //    {
    //    GetDomainFn getDomainFn = (GetDomainFn) GetProcAddress(handle, "?GetDomain@ConstructionPlanningDomain@ConstructionPlanning@BentleyB0200@@SAAEAU123@XZ");
    //    if (nullptr != getDomainFn)
    //        {
    //        DgnDomainP domain = getDomainFn();
    //        if (nullptr != domain)
    //            DgnDomains::RegisterDomain(*domain, DgnDomain::Required::No, DgnDomain::Readonly::No);
    //        }
    //    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BimFromJson::~BimFromJson()
    {
    // This is a hack - When a schema that references the ECv3CustomAttributes schema is imported, the copy being held by this helper receives an ECSchemaId.  Since that is a static
    // helper, if the converter attempts to import that schema into a different ecdb, it will fail.
    ECN::ConversionCustomAttributeHelper::Reset();

    delete m_queue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool BimFromJson::CreateBim()
    {

    m_holder = new DgnDbPtrHolder(m_outputPath);
    if (!m_holder->dgndb.IsValid())
        {
        // Report Error
        return false;
        }
    m_importer = new BimFromJsonImpl(m_holder->dgndb.get());
    if (SUCCESS != m_importer->InitializeSchemas())
        return false;

    if (SUCCESS != m_importer->CreateAndAttachSyncInfo())
        return false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool BimFromJson::ImportJson(folly::Future<bool>& exporterFuture)
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
void BimFromJson::AddToQueue(const char* entry)
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
void BimFromJson::SetDone()
    {
    while (nullptr == m_importer)
        BentleyApi::BeThreadUtilities::BeSleep(1000);

    m_importer->SetDone();
    }

END_BIM_FROM_DGNDB_NAMESPACE