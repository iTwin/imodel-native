/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    struct Worker : RefCountedBase
    {
        Cache& m_cache;
        bool m_hasChanges = false;
        BeTimePoint m_saveTime;

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
    bool m_saveActive = false;
    bool m_workerRunning = false;
    int m_accessors = 0;
    BeDuration m_saveDelay = BeDuration::Seconds(5);
    WorkerPtr m_worker;
    BentleyApi::BeConditionVariable m_cv;
    BeSQLite::Db m_db;

    uint32_t _GetExcessiveRefCountThreshold() const override { return 100000; } // seen data set containing > 5000 models, each has a Tile::Tree that wants a ptr to cache

    //! Perform any additional initialization in a newly-created cache Db.
    virtual BentleyStatus _Initialize() const { return SUCCESS; }

    //! Create or update the tables to hold entries in this Cache. The cache Db may already contain data in its tables.
    virtual BentleyStatus _Prepare() const = 0;

    //! Called to free space in the database, if necessary
    virtual BentleyStatus _Cleanup() const = 0;

public:
    friend struct AccessLock;
    friend struct SaveLock;

    //=======================================================================================
    // On construction block until no save active. Then hold access lock until destruction. 
    // Note that there can be more than one simultaneous accessors.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct AccessLock
    {
        CacheR m_cache;
        AccessLock(CacheR cache) : m_cache(cache) {BeMutexHolder holder(cache.m_cv.GetMutex()); while (cache.m_saveActive) cache.m_cv.InfiniteWait(holder); ++cache.m_accessors;}
        ~AccessLock() {{BeMutexHolder holder(m_cache.m_cv.GetMutex()); --m_cache.m_accessors; m_cache.ScheduleSave(); BeAssert(m_cache.m_accessors>=0);} m_cache.m_cv.notify_all();}
    };

    //=======================================================================================
    // On construction block until no accessors. Then hold save lock until destruction.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct SaveLock
    {
        CacheR m_cache;
        SaveLock(CacheR cache) : m_cache(cache) {BeMutexHolder holder(cache.m_cv.GetMutex()); while (cache.m_accessors>0) cache.m_cv.InfiniteWait(holder); BeAssert(!cache.m_saveActive); cache.m_saveActive=true;}
        ~SaveLock() {{BeMutexHolder holder(m_cache.m_cv.GetMutex()); BeAssert(m_cache.m_saveActive); m_cache.m_saveActive=false; } m_cache.m_cv.notify_all();}
    };

    Cache() = default;
    DGNPLATFORM_EXPORT ~Cache();

    DGNPLATFORM_EXPORT BentleyStatus OpenAndPrepare(BeFileNameCR cacheName);
    DGNPLATFORM_EXPORT void ScheduleSave();
    bool IsStopped() const {return m_isStopped;}
    BeSQLite::DbR GetDb() {return m_db;}
};

END_BENTLEY_REALITYDATA_NAMESPACE

