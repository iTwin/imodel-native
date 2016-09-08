/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RealityDataCache.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <chrono>

#define BEGIN_BENTLEY_REALITYDATA_NAMESPACE  BEGIN_BENTLEY_DGN_NAMESPACE namespace RealityData {
#define END_BENTLEY_REALITYDATA_NAMESPACE    } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_BENTLEY_REALITYDATA using namespace BentleyApi::Dgn::RealityData;

BEGIN_BENTLEY_REALITYDATA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Cache)
DEFINE_REF_COUNTED_PTR(Cache)

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Cache : RefCountedBase
{
    typedef std::chrono::steady_clock::time_point TimePoint;
    struct Worker : RefCountedBase
    {
        Cache& m_cache;
        bool m_hasChanges = false;
        TimePoint m_saveTime;

        Worker(Cache& cache) : m_cache(cache) {}
        Worker(Worker const&) = delete;
        void Work();
        void Start();
        void SaveChanges();
        THREAD_MAIN_DECL Main(void* arg);
    };

protected:
    typedef RefCountedPtr<Worker> WorkerPtr;
    bool m_isStopped=false;
    std::chrono::milliseconds m_saveDelay = std::chrono::seconds(2);
    WorkerPtr m_worker;
    BentleyApi::BeConditionVariable m_cv;
    BeSQLite::Db m_db;

    //! Create the table to hold entries in this Cache
    virtual BentleyStatus _Prepare() const = 0;

    //! Called to free space in the database, if necessary
    virtual BentleyStatus _Cleanup() const = 0;

public:
    Cache() = default;
    DGNPLATFORM_EXPORT ~Cache();

    DGNPLATFORM_EXPORT BentleyStatus OpenAndPrepare(BeFileNameCR cacheName);
    DGNPLATFORM_EXPORT void ScheduleSave();
    bool IsStopped() const {return m_isStopped;}
    BeSQLite::DbR GetDb() {return m_db;}
};

END_BENTLEY_REALITYDATA_NAMESPACE

