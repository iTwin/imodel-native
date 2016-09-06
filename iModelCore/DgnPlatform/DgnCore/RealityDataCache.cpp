/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeHttp/HttpRequest.h>

#if defined(BENTLEYCONFIG_OS_WINDOWS) /* || defined(__clang__) WIP_ANDROID_CLANG */
#include <folly/futures/Future.h>
#endif

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_REALITYDATA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::SaveChanges()
    {
    if (!m_hasChanges)
        return;

    m_cache._Cleanup();
    m_cache.m_db.SaveChanges();
    m_hasChanges = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::Work()
    {
    for(;;)
        {
        BeMutexHolder lock(m_cache.m_cv.GetMutex());

        if (!m_hasChanges)
            m_cache.m_cv.InfiniteWait(lock);

        while (m_saveTime > std::chrono::steady_clock::now())
            {
            m_cache.m_cv.RelativeWait(lock, 1000);
            }

        if (m_cache.IsStopped())
            return;

        SaveChanges();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL Cache::Worker::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("CacheSave");
    ((Worker*)arg)->Work();
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Worker::Start()
    {
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::ScheduleSave()
    {
    BeMutexHolder lock(m_cv.GetMutex());

    m_worker->m_hasChanges = true;
    m_worker->m_saveTime = std::chrono::steady_clock::now() + m_saveDelay;
    m_cv.notify_one();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Cache::~Cache()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    m_isStopped = true;
    m_cv.notify_one();
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
        if (BeSQLite::BE_SQLITE_OK != (result = m_db.CreateNewDb(fileName, BeSQLite::BeGuid(), createParams)))
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

