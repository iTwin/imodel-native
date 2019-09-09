/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <BimFromDgnDb/BimFromDgnDb.h>

namespace BentleyG0601
    {
    namespace Dgn
        {
        namespace BimFromDgnDb
            {
            struct DgnDb0601ToJsonImpl;
            }
        struct DgnProgressMeter;
        }
    }

#ifdef __BIM_FROM_DGNDB_BUILD__
#define BIM_EXPORTER_EXPORT EXPORT_ATTRIBUTE
#endif

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct DgnDb0601ToJson
    {
    private:
        BentleyG0601::Dgn::BimFromDgnDb::DgnDb0601ToJsonImpl *m_exporter;

    public:
        //! Constructor for the exporter.  The calling application must have already initialized the host.
        //! @param[in] dbPath      Path to the 1.6 dgndb/imodel
        //! @param[in] tempPath     Path returned by T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory
        //! @param[in] assetsPath   Path returned by T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory
        BIM_EXPORTER_EXPORT DgnDb0601ToJson(wchar_t const* dbPath, const wchar_t* tempPath, const wchar_t* assetsPath);
        BIM_EXPORTER_EXPORT ~DgnDb0601ToJson();

        //! Sets the progress meter for the exporter
        BIM_EXPORTER_EXPORT void SetProgressMeter(BentleyG0601::Dgn::DgnProgressMeter* meter);

        //! Sets the logging callback for the exporter
        BIM_EXPORTER_EXPORT void SetLogger(T_LogGeneralMessage logger);

        //! Sets the performance logging callback for the exporter
        BIM_EXPORTER_EXPORT void SetPerformanceLogger(T_LogPerformanceMessage perfLogger);

        //! Sets the callback for writing a new entry to the queue
        BIM_EXPORTER_EXPORT void SetQueueWrite(T_QueueJson);

        //! Workhorse of the exporter.  This exports the entire dgndb.  Each entity is written individually to the queue for the importer to pick up asynchronously
        BIM_EXPORTER_EXPORT bool ExportDgnDb();
    };

END_BIM_FROM_DGNDB_NAMESPACE
