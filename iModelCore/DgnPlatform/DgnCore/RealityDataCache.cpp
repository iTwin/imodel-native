/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_REALITYDATA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::SaveChanges()
    {
    if (!m_hasChanges)
        return;

    m_cache._Cleanup();

    SaveLock lock(m_cache); // wait for readers, block while saving changes
    m_cache.m_db.SaveChanges();
    m_hasChanges = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::Work()
    {
    m_cache.m_workerRunning = true;
    for(;;)
        {
        if (true)
            {
            BeMutexHolder lock(m_cache.m_cv.GetMutex());

            if (!m_cache.IsStopped() && !m_hasChanges)
                m_cache.m_cv.InfiniteWait(lock);

            while (m_saveTime > BeTimePoint::Now() && !m_cache.IsStopped())
                m_cache.m_cv.RelativeWait(lock, 1000);
            }

        if (m_cache.IsStopped())
            break;

        SaveChanges(); // make sure we call this WITHOUT the mutex held
        }

    BeMutexHolder lock(m_cache.m_cv.GetMutex());
    m_cache.m_workerRunning = false;
    m_cache.m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL Cache::Worker::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("TileCacheSave");
    ((Worker*)arg)->Work();
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::Start()
    {
    BeThreadUtilities::StartNewThread(Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* must be called with m_cv.mutex held.
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::ScheduleSave()
    {
    m_worker->m_hasChanges = true;
    m_worker->m_saveTime = BeTimePoint::FromNow(m_saveDelay);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Cache::~Cache()
    {
    m_isStopped = true;
    m_cv.notify_all();

    BeMutexHolder holder(m_cv.GetMutex()); 
    while (m_accessors>0 || m_saveActive || m_workerRunning) 
        {
        m_cv.notify_all();
        m_cv.RelativeWait(holder, 1000);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::OpenAndPrepare(BeFileNameCR fileName)
    {
    DbResult result = m_db.OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));

    if (BE_SQLITE_OK != result)
        {
        Db::CreateParams createParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, false, DefaultTxn::Yes);
        if (BeSQLite::BE_SQLITE_OK != (result = m_db.CreateNewDb(fileName, BeSQLite::BeGuid(true), createParams)))
            {
            BeAssert(false);
            return ERROR;
            }

        if (SUCCESS != _Initialize())
            {
            BeAssert(false);
            return ERROR;
            }
        }        

    if (SUCCESS != _Prepare())
        {
        BeAssert(false);
        return ERROR;
        }
    
    if (BE_SQLITE_OK != (result = m_db.SaveChanges()))
        {
        BeAssert(false);
        return ERROR;
        }
    
    m_worker = new Worker(*this);
    m_worker->Start();
    
    return SUCCESS;
    }
