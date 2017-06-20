/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/BisJson1Importer.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <BimTeleporter/BimTeleporter.h>
#include <folly/ProducerConsumerQueue.h>
#include <Bentley/BeFileName.h>

namespace BentleyB0200
    {
    namespace Dgn
        {
        struct DgnDb;
        }
    }


BEGIN_BIM_TELEPORTER_NAMESPACE

#ifdef __BIM_TELEPORTER_BUILD__
#define BIM_IMPORTER_EXPORT __declspec(dllexport)
#else
#define BIM_IMPORTER_EXPORT __declspec(dllimport)
#endif

struct BisJson1ImporterImpl;
struct BimImporterHost;
struct PCQueue;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct BisJson1Importer
    {
    private:
        BisJson1ImporterImpl *m_importer;
        BimImporterHost* m_host;
        BeFileName m_outputPath;
        PCQueue* m_queue;

    public:
        BIM_IMPORTER_EXPORT BisJson1Importer(const wchar_t* bimPath);
        BIM_IMPORTER_EXPORT ~BisJson1Importer();
        BIM_IMPORTER_EXPORT bool CreateBim();
        BIM_IMPORTER_EXPORT void AddToQueue(const char* entry);
        BIM_IMPORTER_EXPORT void SetDone();
    };

END_BIM_TELEPORTER_NAMESPACE