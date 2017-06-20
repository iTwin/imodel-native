/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimExporter0601/lib/BisJson1Exporter0601.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BimTeleporter/BisJson1Exporter0601.h>
#include "BisJson1ExporterImpl0601.h"

BEGIN_BIM_TELEPORTER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Exporter0601::BisJson1Exporter0601(wchar_t const* dbPath)
    {
    m_exporter = new BentleyG0601::Dgn::BimTeleporter::BisJson1ExporterImpl(dbPath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1Exporter0601::SetLogger(T_LogGeneralMessage logger)
    {
    m_exporter->SetGeneralLogger(logger);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1Exporter0601::SetPerformanceLogger(T_LogPerformanceMessage perfLogger)
    {
    m_exporter->SetPerformanceLogger(perfLogger);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1Exporter0601::ExportDgnDb()
    {
    return m_exporter->ExportDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1Exporter0601::SetQueueWrite(T_QueueJson queue)
    {
    m_exporter->SetQueueWrite(queue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1Exporter0601::~BisJson1Exporter0601()
    {
    delete m_exporter;
    }


END_BIM_TELEPORTER_NAMESPACE