/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BimFromDgnDb/DgnDb0601ToJson.h>
#include "DgnDb0601ToJsonImpl.h"

BEGIN_BIM_FROM_DGNDB_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDb0601ToJson::DgnDb0601ToJson(wchar_t const* dbPath, const wchar_t* tempPath, const wchar_t* assetsPath)
    {
    m_exporter = new BentleyG0601::Dgn::BimFromDgnDb::DgnDb0601ToJsonImpl(dbPath, tempPath, assetsPath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJson::SetLogger(T_LogGeneralMessage logger)
    {
    m_exporter->SetGeneralLogger(logger);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJson::SetPerformanceLogger(T_LogPerformanceMessage perfLogger)
    {
    m_exporter->SetPerformanceLogger(perfLogger);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDb0601ToJson::ExportDgnDb()
    {
    return m_exporter->ExportDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJson::SetQueueWrite(T_QueueJson queue)
    {
    m_exporter->SetQueueWrite(queue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnDb0601ToJson::~DgnDb0601ToJson()
    {
    }

END_BIM_FROM_DGNDB_NAMESPACE