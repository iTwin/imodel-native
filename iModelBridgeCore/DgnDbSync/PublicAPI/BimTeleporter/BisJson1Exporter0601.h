/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/BisJson1Exporter0601.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <BimTeleporter/BimTeleporter.h>

namespace BentleyG0601
    {
    namespace Dgn
        {
        namespace BimTeleporter
            {
            struct BisJson1ExporterImpl;
            }
        struct DgnProgressMeter;
        }
    }

#ifdef __BIM_TELEPORTER_BUILD__
#define BIM_EXPORTER_EXPORT __declspec(dllexport)
#else
#define BIM_EXPORTER_EXPORT __declspec(dllimport)
#endif

BEGIN_BIM_TELEPORTER_NAMESPACE

struct BisJson1Exporter0601
    {
    private:
        BentleyG0601::Dgn::BimTeleporter::BisJson1ExporterImpl *m_exporter;

    public:
        BIM_EXPORTER_EXPORT BisJson1Exporter0601(wchar_t const* dbPath);
        BIM_EXPORTER_EXPORT ~BisJson1Exporter0601();
        BIM_EXPORTER_EXPORT void SetProgressMeter(BentleyG0601::Dgn::DgnProgressMeter* meter);
        BIM_EXPORTER_EXPORT void SetLogger(T_LogGeneralMessage logger);
        BIM_EXPORTER_EXPORT void SetPerformanceLogger(T_LogPerformanceMessage perfLogger);
        BIM_EXPORTER_EXPORT void SetQueueWrite(T_QueueJson);

        BIM_EXPORTER_EXPORT bool ExportDgnDb();
    };

END_BIM_TELEPORTER_NAMESPACE
