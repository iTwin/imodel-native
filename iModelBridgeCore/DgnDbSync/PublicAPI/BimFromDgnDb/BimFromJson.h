/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <BimFromDgnDb/BimFromDgnDb.h>
#include <folly/ProducerConsumerQueue.h>
#include <folly/futures/Future.h>
#include <Bentley/BeFileName.h>

namespace BentleyM0200
    {
    namespace Dgn
        {
        struct DgnDb;
        }
    }


BEGIN_BIM_FROM_DGNDB_NAMESPACE

#ifdef __BIM_FROM_DGNDB_BUILD__
#define BIM_IMPORTER_EXPORT EXPORT_ATTRIBUTE
#endif

struct BimFromJsonImpl;
struct PCQueue;
struct DgnDbPtrHolder;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct BimFromJson
    {
    private:
        BimFromJsonImpl *m_importer;
        BeFileName m_outputPath;
        PCQueue* m_queue;
        DgnDbPtrHolder* m_holder;

    public:
        //! Constructor for the importer.  The calling application must have already been initialized the host.
        //! @param[in] bimPath      Path to the output directory where the bim will be created
        BIM_IMPORTER_EXPORT BimFromJson(const wchar_t* bimPath);
        BIM_IMPORTER_EXPORT ~BimFromJson();

        //! This will create a new bim and initialize it
        BIM_IMPORTER_EXPORT bool CreateBim();

        //! Do the actual import
        BIM_IMPORTER_EXPORT bool ImportJson(folly::Future<bool>& exporterFuture);

        //! Callback to add a new entry to the queue for import
        BIM_IMPORTER_EXPORT void AddToQueue(const char* entry);

        //! Tells the importer that the export has been completed
        BIM_IMPORTER_EXPORT void SetDone();
    };

END_BIM_FROM_DGNDB_NAMESPACE
