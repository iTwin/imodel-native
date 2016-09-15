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
    int m_readers = 0;
    int m_writers = 0;
    std::chrono::milliseconds m_saveDelay = std::chrono::seconds(5);
    WorkerPtr m_worker;
    BentleyApi::BeConditionVariable m_cv;
    BeSQLite::Db m_db;

    //! Create the table to hold entries in this Cache
    virtual BentleyStatus _Prepare() const = 0;

    //! Called to free space in the database, if necessary
    virtual BentleyStatus _Cleanup() const = 0;

public:
    friend struct CacheReader;
    friend struct CacheWriter;
    //=======================================================================================
    // On construction block until no writers. Then hold read lock until destruction.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct CacheReader
    {
        CacheR m_cache;
        CacheReader(CacheR cache) : m_cache(cache) {BeMutexHolder holder(cache.m_cv.GetMutex()); while (cache.m_writers>0) cache.m_cv.InfiniteWait(holder); ++cache.m_readers;}
        ~CacheReader() {{BeMutexHolder holder(m_cache.m_cv.GetMutex()); --m_cache.m_readers; m_cache.ScheduleSave(); BeAssert(m_cache.m_readers>=0);} m_cache.m_cv.notify_all();}
    };

    //=======================================================================================
    // On construction block until no readers. Then hold write lock until destruction.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct CacheWriter
    {
        CacheR m_cache;
        CacheWriter(CacheR cache) : m_cache(cache) {BeMutexHolder holder(cache.m_cv.GetMutex()); while (cache.m_readers>0) cache.m_cv.InfiniteWait(holder); ++cache.m_writers;}
        ~CacheWriter() {{BeMutexHolder holder(m_cache.m_cv.GetMutex()); --m_cache.m_writers; BeAssert(m_cache.m_writers>=0);} m_cache.m_cv.notify_all();}
    };

    Cache() = default;
    DGNPLATFORM_EXPORT ~Cache();

    DGNPLATFORM_EXPORT BentleyStatus OpenAndPrepare(BeFileNameCR cacheName);
    DGNPLATFORM_EXPORT void ScheduleSave();
    bool IsStopped() const {return m_isStopped;}
    BeSQLite::DbR GetDb() {return m_db;}
};

END_BENTLEY_REALITYDATA_NAMESPACE

