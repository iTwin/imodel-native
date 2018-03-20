/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/BisJson1Exporter0601.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        //! Constructor for the exporter.  The calling application must have already initialized the host.
        //! @param[in] dbPath      Path to the 1.6 dgndb/imodel
        //! @param[in] tempPath     Path returned by T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory
        //! @param[in] assetsPath   Path returned by T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory
        BIM_EXPORTER_EXPORT BisJson1Exporter0601(wchar_t const* dbPath, const wchar_t* tempPath, const wchar_t* assetsPath);
        BIM_EXPORTER_EXPORT ~BisJson1Exporter0601();

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

END_BIM_TELEPORTER_NAMESPACE
