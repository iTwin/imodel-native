/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/HttpHandler.h>

#ifndef BENTLEY_WINRT
    #include <curl/curl.h>
    #ifdef GetCurrentTime
        #undef GetCurrentTime
    #endif
#endif

DPILOG_DEFINE(RealityDataCache)
#define RDCLOG(sev,...) {if (RealityDataCache_getLogger().isSeverityEnabled(sev)) {RealityDataCache_getLogger().messagev(sev, __VA_ARGS__);}}

USING_NAMESPACE_BENTLEY_DGN

/*======================================================================================+
|   Debug timer                                                 Grigas.Petraitis
+======================================================================================*/
//#define DEBUG_THREADPOOL 1
//#define DEBUG_TILEDRASTER_CLEANUP 1
#if defined DEBUG_THREADPOOL || defined DEBUG_TILEDRASTER_CLEANUP
    struct DebugTimer
        {
        Utf8String  m_msg;
        uint64_t    m_timeBefore;
        DebugTimer(Utf8CP msg) : m_msg(msg), m_timeBefore(BeTimeUtilities::GetCurrentTimeAsUnixMillis()) {}
        ~DebugTimer() {BeDebugLog(Utf8PrintfString("%s took %llu ms", m_msg.c_str(),(BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_timeBefore)).c_str());}
        };
 #endif
/*======================================================================================+
|   RealityDataQueue
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::PushFront(T element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = Item::Create(element, NULL, m_first.get());
    if (m_first.IsNull())
        {
        BeAssert(m_last.IsNull());
        m_first = m_last = item;
        }
    else
        {
        m_first->prev = item;
        m_first = item;
        }
    m_count++;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::PushBack(T element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = Item::Create(element, m_last.get(), NULL);
    if (m_last.IsNull())
        {
        BeAssert(m_first.IsNull());
        m_first = m_last = item;
        }
    else
        {
        m_last->next = item;
        m_last = item;
        }
    m_count++;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront(T* element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_first.IsNull())
        return false;

    if (nullptr != element)
        *element = m_first->data;

    if (m_first.Equals(m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_first->next->prev = NULL;
        m_first = m_first->next;
        }
    m_count--;
    m_cv.notify_all();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack(T* element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_last.IsNull())
        return false;

    if (nullptr != element)
        *element = m_last->data;

    if (m_first.Equals(m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_last->prev->next = NULL;
        m_last = m_last->prev;
        }
    m_count--;
    m_cv.notify_all();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop(T* element) {return SchedulingMethod::FIFO == m_schedulingMethod ? PopFront(element) : PopBack(element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop(T& element) {return Pop(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop() {return Pop(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack(T& element) {return PopBack(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack() {return PopBack(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront(T& element) {return PopFront(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront() {return PopFront(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::Erase(Iterator const& iterator)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (!iterator.IsValid() || m_first.IsNull())
        return;

    Item* erase = iterator.m_curr;

    if (erase != m_first.get())
        erase->prev->next = erase->next;
    else
        m_first = erase->next;

    if (erase != m_last.get())
        erase->next->prev = erase->prev;
    else
        m_last = erase->prev;

    m_count--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::Clear()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = m_first;
    while (item.IsValid())
        {
        auto temp = item;
        item = item->next;
        temp->next = NULL;
        temp->prev = NULL;
        }
    m_first = NULL;
    m_last = NULL;
    m_count = 0;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::IsEmpty() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_first.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> unsigned ThreadSafeQueue<T>::Size() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> typename ThreadSafeQueue<T>::Iterator ThreadSafeQueue<T>::begin() const {return Iterator(m_first.get());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> typename ThreadSafeQueue<T>::Iterator ThreadSafeQueue<T>::end() const {return Iterator(nullptr);}

// explicitly implement for testing purposes, 
// note: must be done AFTER all template functions are defined
BEGIN_BENTLEY_DGN_NAMESPACE
template struct ThreadSafeQueue<int>;
END_BENTLEY_DGN_NAMESPACE

/*======================================================================================+
|   IRealityData
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP IRealityDataBase::GetId() const {return _GetId();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool IRealityDataBase::IsExpired() const {return _IsExpired();}

/*======================================================================================+
|   IRealityDataStorageBase
+======================================================================================*/
void IRealityDataStorageBase::Terminate() {_Terminate();}
IRealityDataStorageBase::~IRealityDataStorageBase() {Terminate();}

/*======================================================================================+
|   BeSQLiteRealityDataStorage
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStoragePtr BeSQLiteRealityDataStorage::Create(BeFileName const& filename, uint32_t idleTime, uint64_t cacheSize)
    {
    return new BeSQLiteRealityDataStorage(filename, idleTime, cacheSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStorage::BeSQLiteRealityDataStorage(BeFileName const& filename, uint32_t idleTime, uint64_t cacheSize)
    : m_filename(filename), m_database(*new BeSQLite::Db()), m_initialized(false), m_hasChanges(false), m_idleTime(idleTime), m_cacheSize(cacheSize)
    {
    m_threadPool = BeSQLiteStorageThreadPool::Create(*this);
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct ThreadPoolQueueNotEmptyPredicate : IConditionVariablePredicate
    {
     BeSQLiteRealityDataStorage::BeSQLiteStorageThreadPool& m_pool;
    ThreadPoolQueueNotEmptyPredicate( BeSQLiteRealityDataStorage::BeSQLiteStorageThreadPool& pool) : m_pool(pool) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override 
        {
        BeMutexHolder lock(cv.GetMutex());
        return !m_pool.GetQueue().IsEmpty() || m_pool.IsTerminating();
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct SelectDataWorkHasResultPredicate : IConditionVariablePredicate
    {
    bool const& m_hasResult;
    SelectDataWorkHasResultPredicate(bool const& hasResult) : m_hasResult(hasResult) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override 
        {
        BeMutexHolder lock(cv.GetMutex());
        return m_hasResult;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::CleanAndSaveChangesWork::_DoWork()
    {
    auto& queue = m_storage->m_threadPool->GetQueue();
    ThreadPoolQueueNotEmptyPredicate predicate(*m_storage->m_threadPool);
    if (queue.GetConditionVariable().WaitOnCondition(&predicate, m_idleTime))
        return;

    BeMutexHolder lock(m_storage->m_saveChangesCS);
    if (!m_storage->m_hasChanges)
        return;

    for (auto const& handler : m_storage->m_cleanupHandlers)
        m_storage->wt_Prepare(*handler);

    m_storage->wt_Cleanup();
    m_storage->wt_SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::PersistDataWork::_DoWork()
    {
    m_storage->wt_Persist(*m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::SelectDataWork::_DoWork()
    {
    BeMutexHolder lock(m_resultCV.GetMutex());
    m_result = m_storage->wt_Select(*m_data, m_id.c_str(), *m_options, *m_responseReceiver);
    m_hasResult = true;
    m_resultCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::SelectDataWork::GetResult() const
    {
    SelectDataWorkHasResultPredicate predicate(m_hasResult);
    auto result = m_resultCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    BeAssert(result);
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::_Terminate()
    {
    m_threadPool->Terminate();
    m_threadPool->WaitUntilAllThreadsIdle();

    for (auto& cleanupHandler : m_cleanupHandlers)
        cleanupHandler->Release();

    BeMutexHolder lock(m_databaseCS);
    if (m_initialized)
        {
        m_database.CloseDb();
        delete &m_database;
        }
    m_initialized = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Prepare(DatabasePrepareAndCleanupHandler const& prepareHandler)
    {
    BeSQLite::DbResult result;
    m_databaseCS.Enter();
    if (!m_initialized)
        {
        if (BeSQLite::BE_SQLITE_OK != (result = m_database.OpenBeSQLiteDb(m_filename.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes))))
            {
            Db::CreateParams createParams(Db::PageSize::PAGESIZE_32K, Db::Encoding::Utf8, false, DefaultTxn::Yes);
            if (BeSQLite::BE_SQLITE_OK != (result = m_database.CreateNewDb(m_filename.c_str(), BeSQLite::BeGuid(), createParams)))
                {
                m_databaseCS.Leave();
                RDCLOG(LOG_ERROR, "%s: %s", m_filename.c_str(), BeSQLite::Db::InterpretDbResult(result));
                BeAssert(false);
                return;
                }
            }

        // Stop all current transactions with savepoint <commit>. Enable auto VACUUM for database. Resume operations with <begin>.
        BeSQLite::Savepoint* savepoint = m_database.GetSavepoint(0);
        if (NULL != savepoint)
            savepoint->Commit();

        m_database.TryExecuteSql("PRAGMA auto_vacuum = FULL");
        m_database.TryExecuteSql("VACUUM");

        if (NULL != savepoint)
            savepoint->Begin();
        
        m_initialized = true;
        }
    m_databaseCS.Leave();

    if (!prepareHandler._IsPrepared())
        {
        BeMutexHolder lock(m_databaseCS);

        BentleyStatus preparationStatus = prepareHandler._PrepareDatabase(m_database);
        BeAssert(SUCCESS == preparationStatus);
        BeAssert(m_cleanupHandlers.end() == m_cleanupHandlers.find(&prepareHandler) || typeid(prepareHandler).hash_code() == typeid(**m_cleanupHandlers.find(&prepareHandler)).hash_code());
        if (m_cleanupHandlers.end() == m_cleanupHandlers.find(&prepareHandler))
            {
            prepareHandler.AddRef();
            m_cleanupHandlers.insert(&prepareHandler);
            }
    
        if (BeSQLite::BE_SQLITE_OK != (result = m_database.SaveChanges()))
            {
            RDCLOG(LOG_ERROR, "%s: %s", m_filename.c_str(), BeSQLite::Db::InterpretDbResult(result));
            BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Mantas.Ragauskas               01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Cleanup()
    {
    uint64_t realCacheSize;
    auto realFileSizeResult = BeFileName::GetFileSize(realCacheSize, WString(m_filename.c_str(), true).c_str());
    BeAssert(BeFileNameStatus::Success == realFileSizeResult);
   
    if ((0 != m_cacheSize) && (realCacheSize > m_cacheSize))
        {
        // percentage of cache to delete
        double pct = 100.0 * (realCacheSize - m_cacheSize) / realCacheSize;
        if (pct > 100)
            pct = 100;

        BeMutexHolder lock(m_databaseCS);
        for (auto const& cleanupHandler : m_cleanupHandlers)
            {
            BentleyStatus result = cleanupHandler->_CleanupDatabase(m_database, pct);
            BeAssert(SUCCESS == result);
            }
        }                                 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_SaveChanges()
    {
    BeMutexHolder lock(m_databaseCS);
    auto result = m_database.SaveChanges();
    BeAssert(BeSQLite::BE_SQLITE_OK == result);
    m_hasChanges = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::wt_Select(Data& data, Utf8CP id, SelectOptions const& options, IRealityDataStorageResponseReceiver& responseReceiver)
    {
    wt_Prepare(*data._GetDatabasePrepareAndCleanupHandler());

    RealityDataStorageResult result;
    if (SUCCESS != data._InitFrom(m_database, m_databaseCS, id, options))
        {
        responseReceiver._OnResponseReceived(*RealityDataStorageResponse::Create(RealityDataStorageResult::NotFound, id, data), options, !options.ForceSynchronousRequest());
        result = RealityDataStorageResult::NotFound;
        }
    else
        {
        responseReceiver._OnResponseReceived(*RealityDataStorageResponse::Create(RealityDataStorageResult::Success, id, data), options, !options.ForceSynchronousRequest());
        result = RealityDataStorageResult::Success;
        }

    if (true)
        {
        BeMutexHolder lock(m_activeRequestsCS);
        BeAssert(m_activeRequests.end() != m_activeRequests.find(id));
        m_activeRequests.erase(m_activeRequests.find(id));
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::Select(Data& data, Utf8CP id, SelectOptions const& options, IRealityDataStorageResponseReceiver& responseReceiver)
    {
    if (true)
        {
        BeMutexHolder lock(m_activeRequestsCS);
        if (m_activeRequests.end() != m_activeRequests.find(id))
            return RealityDataStorageResult::Pending;
        m_activeRequests.insert(id);
        }

    auto work = SelectDataWork::Create(*this, id, data, options, responseReceiver);
    m_threadPool->QueueWork(*work);

    if (options.ForceSynchronousRequest())
        return work->GetResult();
    else
        return RealityDataStorageResult::Queued;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Persist(Data const& data)
    {
    wt_Prepare(*data._GetDatabasePrepareAndCleanupHandler());

    BentleyStatus result = data._Persist(m_database, m_databaseCS);
    BeAssert(SUCCESS == result);
    m_hasChanges = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::Persist(Data const& data)
    {
    m_threadPool->QueueWork(*PersistDataWork::Create(*this, data));
    return RealityDataStorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::PersistHandler::_Persist() const {return m_storage.Persist(*m_data);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataBase const* BeSQLiteRealityDataStorage::PersistHandler::_GetData() const {return m_data.get();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeSQLiteRealityDataStorage::BeSQLiteStorageThreadPool::_AssignWork(RealityDataWorkerThread& thread)
    {
    if (RealityDataThreadPool::_AssignWork(thread))
        return true;

    if (m_storage->m_hasChanges && !IsTerminating())
        {
        thread.DoWork(*CleanAndSaveChangesWork::Create(*m_storage, m_storage->m_idleTime));
        return true;
        }

    return false;
    }

/*======================================================================================+
|   InMemoryRealityDataStorage
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryRealityDataStoragePtr InMemoryRealityDataStorage::Create() {return new InMemoryRealityDataStorage();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryRealityDataStorage::~InMemoryRealityDataStorage()
    {
    for (auto pair : m_map)
        pair.second->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult InMemoryRealityDataStorage::Select(Data& data, Utf8CP id, SelectOptions const& options, IRealityDataStorageResponseReceiver& responseReceiver)
    {
    BeMutexHolder lock(m_cs);
    auto iter = m_map.find(id);
    if (m_map.end() != iter)
        {
        data._InitFrom(id, *iter->second, options);
        if (options.GetRemoveAfterSelect())
            {
            iter->second->Release();
            m_map.erase(iter);
            }
        return RealityDataStorageResult::Success;
        }

    return RealityDataStorageResult::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult InMemoryRealityDataStorage::Persist(Data const& data)
    {
    BeMutexHolder lock(m_cs);
    BeAssert(m_map.end() == m_map.find(data.GetId()));
    data.AddRef();
    m_map[data.GetId()] = &data;
    return RealityDataStorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult InMemoryRealityDataStorage::PersistHandler::_Persist() const {return m_storage.Persist(*m_data);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataBase const* InMemoryRealityDataStorage::PersistHandler::_GetData() const {return m_data.get();}

/*======================================================================================+
|   IRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult IRealityDataSourceBase::Request(Data& data, bool& handled, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    return _Request(data, handled, id, options, responseReceiver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void IRealityDataSourceBase::Terminate() {_Terminate();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataSourceBase::~IRealityDataSourceBase() {Terminate();}

/*======================================================================================+
|   AsyncRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
void AsyncRealityDataSource<Derived>::_Terminate()
    {
    m_terminateRequested = true;
    m_threadPool->Terminate();
    m_threadPool->WaitUntilAllThreadsIdle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AsyncRealityDataSourceRequest::GetId() const {return _GetId();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncRealityDataSourceRequest::RequestOptions const& AsyncRealityDataSourceRequest::GetRequestOptions() const {return _GetRequestOptions();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<RealityDataSourceResponse> AsyncRealityDataSourceRequest::Handle(BeMutex& cs) const
    {
    RefCountedPtr<RealityDataSourceResponse> response = _Handle();
    BeMutexHolder lock(cs);
    m_response = response;
    return m_response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncRealityDataSourceRequest::ShouldCancelRequest() const {return nullptr != m_cancellationToken && m_cancellationToken->_ShouldCancel();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
void AsyncRealityDataSource<Derived>::SetIgnoreRequests(uint32_t ignoreTime)
    {
    m_ignoreRequestsUntil = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + ignoreTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
bool AsyncRealityDataSource<Derived>::ShouldIgnoreRequests() const
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_ignoreRequestsUntil;
    }

//===================================================================================
// @bsiclass                                        Grigas.Petraitis        04/2015
//===================================================================================
struct AsyncRealityDataSourceRequest::SynchronousRequestPredicate : IConditionVariablePredicate
{
private:
    AsyncRealityDataSourceRequest const& m_request;
public:
    SynchronousRequestPredicate(AsyncRealityDataSourceRequest const& request) : m_request(request) {}
    RealityDataSourceResult GetResult() const {return m_request.m_response->GetResult();}
    virtual bool _TestCondition (BeConditionVariable &cv) override {BeMutexHolder lock(cv.GetMutex()); return m_request.m_response.IsValid();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
RealityDataSourceResult AsyncRealityDataSource<Derived>::QueueRequest(AsyncRealityDataSourceRequest& request, bool& handled, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    handled = true;
    if (!request.GetRequestOptions().RetryNotFoundRequests())
        {
        BeMutexHolder lock(m_requestsCS);
        if (m_notFoundRequests.end() != m_notFoundRequests.find(request.GetId()))
            return RealityDataSourceResult::Error_NotFound;
        }

    if (ShouldIgnoreRequests())
        return RealityDataSourceResult::Ignored;

    if (!request.GetRequestOptions().ForceSynchronousRequest())
        {
        BeMutexHolder lock(m_requestsCS);
        auto requestIter = m_activeRequests.find(request.GetId());
        if (requestIter != m_activeRequests.end())
            return RealityDataSourceResult::Pending;

        m_activeRequests.insert(request.GetId());
        }
    
    RefCountedPtr<RequestHandler> handler = RequestHandler::Create(*this, request, responseReceiver);
    request.SetCancellationToken(*handler);
    m_threadPool->QueueWork(*handler);

    if (!request.GetRequestOptions().ForceSynchronousRequest())
        {
        handled = false;
        return RealityDataSourceResult::Queued;
        }

    AsyncRealityDataSourceRequest::SynchronousRequestPredicate predicate(request);
    m_synchronizationCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    return predicate.GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived> 
void AsyncRealityDataSource<Derived>::RequestHandler::_DoWork()
    {
    RefCountedPtr<RealityDataSourceResponse> response = m_request->Handle(m_source->m_synchronizationCV.GetMutex());
    switch(response->GetResult())
        {
        case RealityDataSourceResult::Error_CouldNotResolveHost:
        case RealityDataSourceResult::Error_NoConnection:
            m_source->SetIgnoreRequests(10 * 1000);
            break;
        case RealityDataSourceResult::Error_GatewayTimeout:
            m_source->SetIgnoreRequests(3 * 1000);
            break;
        case RealityDataSourceResult::Error_NotFound:
            {
            if (!m_request->GetRequestOptions().RetryNotFoundRequests())
                {
                BeMutexHolder lock(m_source->m_requestsCS);
                m_source->m_notFoundRequests.insert(m_request->GetId());
                }
            }
        }

    if (!m_source->m_terminateRequested)
        SendResponse(*response, m_request->GetRequestOptions());

    if (!m_request->GetRequestOptions().ForceSynchronousRequest())
        {
        BeMutexHolder lock(m_source->m_requestsCS);
        auto requestIter = m_source->m_activeRequests.find(m_request->GetId());
        BeAssert(requestIter != m_source->m_activeRequests.end());
        m_source->m_activeRequests.erase(requestIter);
        }

    m_source->m_synchronizationCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived> 
bool AsyncRealityDataSource<Derived>::RequestHandler::_ShouldCancel() const
    {
    return m_source->m_terminateRequested;
    }

/*======================================================================================+
|   FileRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
FileRealityDataSourcePtr FileRealityDataSource::Create(int numThreads) {return new FileRealityDataSource(numThreads);}

//===================================================================================
// @bsiclass                                        Grigas.Petraitis        03/2015
//===================================================================================
struct FileRealityDataSourceRequest : AsyncRealityDataSourceRequest
{
private:
    RefCountedPtr<FileRealityDataSource::Data>  m_data;
    Utf8String m_filename;
    RefCountedPtr<FileRealityDataSource::RequestOptions const> m_requestOptions;
    
    FileRealityDataSourceRequest(FileRealityDataSource::Data& data, Utf8CP url, FileRealityDataSource::RequestOptions const& requestOptions)
        : m_data(&data), m_filename(url), m_requestOptions(&requestOptions)
        {}

protected:
    virtual Utf8CP _GetId() const {return m_filename.c_str();}
    virtual AsyncRealityDataSourceRequest::RequestOptions const& _GetRequestOptions() const {return *m_requestOptions;}
    virtual RefCountedPtr<RealityDataSourceResponse> _Handle() const override
        {
        if (!BeFileName::DoesPathExist(BeFileName(m_filename).c_str()))
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_NotFound, m_filename.c_str(), *m_data);

        // open the file
        FILE* file = fopen (m_filename.c_str (), "rb");
        if (NULL == file)
            {
            BeAssert (false);
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_filename.c_str(), *m_data);
            }
    
        // get file size
        fseek (file, 0, SEEK_END);
        long size = ftell (file);
        rewind (file);

        // read file content
        ByteStream data;
        data.ReserveMemory(size);
        size_t bytesRead = fread (data.GetDataR(), sizeof (Utf8Char), size, file);
        fclose(file);
        if (bytesRead != size)
            {
            BeAssert (false);
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_filename.c_str(), *m_data);
            }

        if (ShouldCancelRequest())
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Cancelled, m_filename.c_str(), *m_data);
        
        if (SUCCESS != m_data->_InitFrom(m_filename.c_str(), data, *m_requestOptions))
            {
            BeAssert (false);
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_filename.c_str(), *m_data);
            }

        return RealityDataSourceResponse::Create(RealityDataSourceResult::Success, m_filename.c_str(), *m_data);
        }

public:
    static RefCountedPtr<FileRealityDataSourceRequest> Create(FileRealityDataSource::Data& data, Utf8CP url, FileRealityDataSource::RequestOptions const& requestOptions)
        {
        return new FileRealityDataSourceRequest(data, url, requestOptions);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult FileRealityDataSource::Request(Data& data, bool& handled, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    return QueueRequest(*FileRealityDataSourceRequest::Create(data, id, options), handled, responseReceiver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult FileRealityDataSource::RequestHandler::_Request() const {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, *m_responseReceiver);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataBase const* FileRealityDataSource::RequestHandler::_GetData() const {return m_data.get();}

/*======================================================================================+
|   HttpRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime GetExpirationDateFromCacheControlStr(Utf8CP str)
    {
    int maxage = 0, smaxage = 0;
    int start = 0;
    int i = 0;
    while (true)
        {
        if (str[i] == ',' || str[i] == '\0')
            {
            // means we passed an option
            Utf8String option(str + start, i - start);
            option.Trim();

            if (option.Equals("private") || option.Equals("no-cache") || option.Equals("no-store"))
                return DateTime::GetCurrentTimeUtc(); 
            
            if (option.length() >= 7 && 0 == strncmp("max-age", option.c_str(), 7))
                {
                Utf8CP value = option.c_str() + 8;
                sscanf(value, "%d", &maxage);
                }

            if (option.length() >= 8 && 0 == strncmp("s-maxage", option.c_str(), 8))
                {
                Utf8CP value = option.c_str() + 9;
                sscanf(value, "%d", &smaxage);
                }

            if (str[i] == '\0')
                break;

            start = i + 1;
            }
        i++;
        }

    int64_t currentTime;
    DateTime::GetCurrentTime().ToUnixMilliseconds(currentTime);

    DateTime expirationDate;
    DateTime::FromUnixMilliseconds(expirationDate, currentTime + 1000 * (0 != smaxage ? smaxage : maxage));
    return expirationDate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetAnsiCFormattedDateTime(DateTime const& dateTime)
    {
    Utf8CP wkday = "";
    switch(dateTime.GetDayOfWeek())
        {
        case DateTime::DayOfWeek::Sunday:   wkday = "Sun"; break;
        case DateTime::DayOfWeek::Monday:   wkday = "Mon"; break;
        case DateTime::DayOfWeek::Tuesday:  wkday = "Tue"; break;
        case DateTime::DayOfWeek::Wednesday:wkday = "Wed"; break;
        case DateTime::DayOfWeek::Thursday: wkday = "Thu"; break;
        case DateTime::DayOfWeek::Friday:   wkday = "Fri"; break;
        case DateTime::DayOfWeek::Saturday: wkday = "Sat"; break;
        }

    Utf8CP month = "";
    switch(dateTime.GetMonth())
        {
        case 1:  month = "Jan"; break;
        case 2:  month = "Feb"; break;
        case 3:  month = "Mar"; break;
        case 4:  month = "Apr"; break;
        case 5:  month = "May"; break;
        case 6:  month = "Jun"; break;
        case 7:  month = "Jul"; break;
        case 8:  month = "Aug"; break;
        case 9:  month = "Sep"; break;
        case 10: month = "Oct"; break;
        case 11: month = "Nov"; break;
        case 12: month = "Dec"; break;
        }

    return Utf8PrintfString("%s %s %02d %02d:%02d:%02d %d", wkday, month, dateTime.GetDay(), dateTime.GetHour(), dateTime.GetMinute(), dateTime.GetSecond(), dateTime.GetYear());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime const& HttpRealityDataSource::Data::GetExpirationDate() const {return m_expirationDate;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRealityDataSource::Data::GetEntityTag() const {return m_entityTag.c_str();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRealityDataSource::Data::SetExpirationDate(DateTime const& date) {m_expirationDate = date;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRealityDataSource::Data::SetEntityTag(Utf8CP eTag) {m_entityTag = eTag;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRealityDataSource::Data::ParseExpirationDateAndETag(bmap<Utf8String, Utf8String> const& header)
    {
    auto cacheControlIter = header.find("Cache-Control");
    SetExpirationDate((cacheControlIter != header.end()) ? GetExpirationDateFromCacheControlStr(cacheControlIter->second.c_str()) : DateTime::GetCurrentTimeUtc());

    auto etagIter = header.find("ETag");
    if (header.end() != etagIter)
        SetEntityTag(etagIter->second.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRealityDataSourcePtr HttpRealityDataSource::Create(int numThreads) {return new HttpRealityDataSource(numThreads);}

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct HttpRealityDataSourceRequest : AsyncRealityDataSourceRequest, IHttpRequestCancellationToken
{
private:
    RefCountedPtr<HttpRealityDataSource::Data> m_data;
    Utf8String                   m_url;
    RefCountedPtr<HttpRealityDataSource::RequestOptions const> m_requestOptions;
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    HttpRealityDataSourceRequest(HttpRealityDataSource::Data& data, Utf8CP url, HttpRealityDataSource::RequestOptions const& requestOptions)
        : m_data(&data), m_url(url), m_requestOptions(&requestOptions)
        {}

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual Utf8CP _GetId() const override {return m_url.c_str();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               04/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual AsyncRealityDataSourceRequest::RequestOptions const& _GetRequestOptions() const {return *m_requestOptions;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               05/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual bool _ShouldCancelHttpRequest() const override {return ShouldCancelRequest();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual RefCountedPtr<RealityDataSourceResponse> _Handle() const override 
        {
        bmap<Utf8String, Utf8String> header;
        if (m_data->GetExpirationDate().IsValid())
            {
            BeAssert(DateTime::Kind::Utc == m_data->GetExpirationDate().GetInfo().GetKind());
            header["If-Modified-Since"] = GetAnsiCFormattedDateTime(m_data->GetExpirationDate());
            }
        if (!Utf8String::IsNullOrEmpty(m_data->GetEntityTag()))
            header["If-None-Match"] = m_data->GetEntityTag();

        HttpResponsePtr response;
        HttpRequestStatus requestStatus = HttpHandler::Instance().Request(response, HttpRequest(m_url.c_str(), header), this);
        switch (requestStatus)
            {
            case HttpRequestStatus::NoConnection:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_NoConnection, m_url.c_str(), *m_data);
            case HttpRequestStatus::CouldNotResolveHost:
            case HttpRequestStatus::CouldNotResolveProxy:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_CouldNotResolveHost, m_url.c_str(), *m_data);
            case HttpRequestStatus::UnknownError:
                BeAssert(false && "All HTTP errors should be handled");
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_url.c_str(), *m_data);
            }

        if (requestStatus == HttpRequestStatus::Aborted || ShouldCancelRequest())
            return RealityDataSourceResponse::Create(RealityDataSourceResult::Cancelled, m_url.c_str(), *m_data);

        BeAssert(response.IsValid());
        switch (response->GetStatus())
            {
            case HttpResponseStatus::Success:
                {
                m_data->ParseExpirationDateAndETag(header);
                if (SUCCESS != m_data->_InitFrom(m_url.c_str(), response->GetHeader(), response->GetBody(), *m_requestOptions))
                    {
                    RDCLOG(LOG_INFO, "[%lld] response 200 but unable to initialize reality data",(uint64_t)BeThreadUtilities::GetCurrentThreadId());
                    return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_url.c_str(), *m_data);
                    }
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Success, m_url.c_str(), *m_data);
                }

            case HttpResponseStatus::NotModified:
                {
                m_data->ParseExpirationDateAndETag(header);
                return RealityDataSourceResponse::Create(RealityDataSourceResult::NotModified, m_url.c_str(), *m_data);
                }

            case HttpResponseStatus::NotFound:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_NotFound, m_url.c_str(), *m_data);

            case HttpResponseStatus::Forbidden:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_AccessDenied, m_url.c_str(), *m_data);

            case HttpResponseStatus::GatewayTimeout:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_GatewayTimeout, m_url.c_str(), *m_data);

            default:
                return RealityDataSourceResponse::Create(RealityDataSourceResult::Error_Unknown, m_url.c_str(), *m_data);
            }
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    static RefCountedPtr<HttpRealityDataSourceRequest> Create(HttpRealityDataSource::Data& data, Utf8CP url, HttpRealityDataSource::RequestOptions const& requestOptions)
        {
        return new HttpRealityDataSourceRequest(data, url, requestOptions);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult HttpRealityDataSource::Request(Data& data, bool& handled, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    return QueueRequest(*HttpRealityDataSourceRequest::Create(data, id, options), handled, responseReceiver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult HttpRealityDataSource::RequestHandler::_Request() const {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, *m_responseReceiver);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataBase const* HttpRealityDataSource::RequestHandler::_GetData() const {return m_data.get();}

/*======================================================================================+
|   CombinedRealityDataSource
+======================================================================================*/
void CombinedRealityDataSourceBase::_Terminate()
    {
    for (auto& source : m_sources)
        source.second->_Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CombinedRealityDataSourceBase::GetIdFormat(int numSources)
    {
    Utf8String format("Combined:");
    for (int i = 0; i < numSources - 1; i++)
        format.append("%s,");
    format.append("%s");
    return format;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CombinedRealityDataSourceBase::RegisterSource(IRealityDataSourceBase& source) {m_sources[source._GetSourceId()] = &source;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResult CombinedRealityDataSourceBase::Request(Data& data, bool& handled, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& receiver)
    {
    IRealityDataSourceBase& source = data._PickDataSource(id, m_sources);
    return source.Request(data, handled, id, options, receiver);
    }

/*======================================================================================+
|   RealityDataCache
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCachePtr RealityDataCache::Create(int arrivalsQueueSize) {return new RealityDataCache(arrivalsQueueSize);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCache::~RealityDataCache()
    {
    for (auto& source : m_sources)
        source.second->_Terminate();

    for (auto& storage : m_storages)
        storage.second->_Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataStorageBase* RealityDataCache::GetStorage(Utf8CP id) const
    {
    auto iter = m_storages.find(id);
    if (m_storages.end() == iter)
        return nullptr;
    return iter->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::RegisterStorage(IRealityDataStorageBase& storage)
    {
    BeAssert(nullptr == GetStorage(storage._GetStorageId().c_str()));
    m_storages[storage._GetStorageId()] = &storage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataSourceBase* RealityDataCache::GetSource(Utf8CP id) const
    {
    auto iter = m_sources.find(id);
    if (m_sources.end() == iter)
        return nullptr;
    return iter->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::RegisterSource(IRealityDataSourceBase& source)
    {
    BeAssert(nullptr == GetSource(source._GetSourceId().c_str()));
    m_sources[source._GetSourceId()] = &source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::QueuePersistHandler(Utf8CP id, IRealityDataStoragePersistHandler& handler)
    {
    BeMutexHolder lock(m_persistHandlersCS);
    m_persistHandlers[id].insert(&handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<IRealityDataStoragePersistHandler> RealityDataCache::DequeuePersistHandler(Utf8CP id, IRealityDataBase const& data)
    {
    BeMutexHolder lock(m_persistHandlersCS);
    auto setIter = m_persistHandlers.find(id);
    if (m_persistHandlers.end() == setIter)
        return nullptr;

    auto& set = setIter->second;
    for (auto handlerIter = set.begin(); handlerIter != set.end(); handlerIter++)
        {
        RefCountedPtr<IRealityDataStoragePersistHandler> handler = *handlerIter;
        if (handler->_GetData() == &data)
            {
            set.erase(handlerIter);
            if (set.empty())
                m_persistHandlers.erase(setIter);
            return handler;
            }        
        }
    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::QueueRequestHandler(Utf8CP id, IRealityDataSourceRequestHandler& handler)
    {
    BeMutexHolder lock(m_requestHandlersCS);
    BeAssert(m_requestHandlers.end() == m_requestHandlers.find(id) || m_requestHandlers[id].end() == m_requestHandlers[id].find(&handler));
    m_requestHandlers[id].insert(&handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<IRealityDataSourceRequestHandler> RealityDataCache::DequeueRequestHandler(Utf8CP id, IRealityDataBase const& data)
    {
    BeMutexHolder lock(m_requestHandlersCS);
    auto setIter = m_requestHandlers.find(id);
    if (m_requestHandlers.end() == setIter)
        return nullptr;

    auto& set = setIter->second;
    for (auto handlerIter = set.begin(); handlerIter != set.end(); handlerIter++)
        {
        RefCountedPtr<IRealityDataSourceRequestHandler> handler = *handlerIter;
        if (handler->_GetData() == &data)
            {
            set.erase(handlerIter);
            if (set.empty())
                m_requestHandlers.erase(setIter);
            return handler;
            }        
        }
    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::_OnResponseReceived(RealityDataSourceResponse const& response, IRealityDataSourceBase::RequestOptions const& options)
    {
    RefCountedPtr<IRealityDataStoragePersistHandler> persistHandler = DequeuePersistHandler(response.GetId(), response.GetData());
    switch(response.GetResult())
        {
        case RealityDataSourceResult::Success:
        case RealityDataSourceResult::NotModified:
            {
            BeAssert(nullptr != dynamic_cast<RealityDataCacheOptions const*>(&options));
            RealityDataCacheOptions const& cacheOptions = dynamic_cast<RealityDataCacheOptions const&>(options);

            if (cacheOptions.SaveInArrivals())
                AddToArrivals(response.GetData());
            
            if (cacheOptions.UseStorage() && persistHandler.IsValid())
                {
                BeAssert(&response.GetData() == persistHandler->_GetData());
                RealityDataStorageResult result = persistHandler->_Persist();
                BeAssert(RealityDataStorageResult::Success == result);
                }
            break;
            }

        case RealityDataSourceResult::Error_GatewayTimeout:
        case RealityDataSourceResult::Error_CouldNotResolveHost:
        case RealityDataSourceResult::Error_NoConnection:
        case RealityDataSourceResult::Error_NotFound:
        case RealityDataSourceResult::Error_AccessDenied:
        case RealityDataSourceResult::Error_Unknown:
        case RealityDataSourceResult::Cancelled:
            break;

        default:
            BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::_OnResponseReceived(RealityDataStorageResponse const& response, IRealityDataStorageBase::SelectOptions const& options, bool isAsync)
    {
    BeAssert(nullptr != dynamic_cast<RealityDataCacheOptions const*>(&options));
    RealityDataCacheOptions const& cacheOptions = dynamic_cast<RealityDataCacheOptions const&>(options);
    RealityDataCacheResult result = HandleStorageResponse(response, cacheOptions);

    if (isAsync)
        return;

    BeMutexHolder lock(m_resultsCS);
    BeAssert(m_results.end() == m_results.find(&response.GetData()));
    m_results[&response.GetData()] = result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult RealityDataCache::HandleStorageResponse(RealityDataStorageResponse const& response, RealityDataCacheOptions const& options)
    {    
    switch (response.GetResult())
        {
        case RealityDataStorageResult::Success:
            {
            if (options.SaveInArrivals())
                AddToArrivals(response.GetData());
            if (response.GetData().IsExpired() && options.RequestFromSource())
                {
                RefCountedPtr<IRealityDataSourceRequestHandler> handler = DequeueRequestHandler(response.GetId(), response.GetData());
                RealityDataSourceResult requestResult = handler->_Request();
                if (handler->_IsHandled())
                    DequeuePersistHandler(response.GetId(), response.GetData());
                return ResolveResult(response.GetResult(), requestResult);
                }
            break;
            }

        case RealityDataStorageResult::NotFound:
            {
            RefCountedPtr<IRealityDataSourceRequestHandler> handler;
            if (options.RequestFromSource() && (handler = DequeueRequestHandler(response.GetId(), response.GetData())).IsValid())
                {
                RealityDataSourceResult requestResult = handler->_Request();
                if (handler->_IsHandled())
                    DequeuePersistHandler(response.GetId(), response.GetData());
                return ResolveResult(response.GetResult(), requestResult);
                }
            break;
            }

        case RealityDataStorageResult::Queued:
            {
            BeAssert(false);
            break;
            }
        }

    DequeueRequestHandler(response.GetId(), response.GetData());
    DequeuePersistHandler(response.GetId(), response.GetData());
    return ResolveResult(response.GetResult());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult RealityDataCache::GetResult(IRealityDataBase& data, Utf8CP id, RealityDataStorageResult storageResult)
    {
    if (RealityDataStorageResult::Pending == storageResult)
        {
        DequeuePersistHandler(id, data);
        DequeueRequestHandler(id, data);
        }

    RealityDataCacheResult result = ResolveResult(storageResult);
    if (RealityDataCacheResult::RequestQueued == result || RealityDataCacheResult::Error == result)
        return result;

    // unless there was an error or the request was queued, we expect to find the result in the m_results map
    BeMutexHolder lock(m_resultsCS);
    auto iter = m_results.find(&data);
    BeAssert(m_results.end() != iter);
    RealityDataCacheResult returnValue = iter->second;
    m_results.erase(iter);
    return returnValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult RealityDataCache::GetResult(Utf8CP, RealityDataSourceResult sourceResult)
    {
    return ResolveResult(RealityDataStorageResult::NotFound, sourceResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult RealityDataCache::ResolveResult(RealityDataStorageResult storageResult, RealityDataSourceResult sourceResult) const
    {
    switch (storageResult)
        {
        case RealityDataStorageResult::Success:
            return RealityDataCacheResult::Success;

        case RealityDataStorageResult::Queued:
        case RealityDataStorageResult::Pending:
            return RealityDataCacheResult::RequestQueued;

        case RealityDataStorageResult::NotFound:
            switch (sourceResult)
                {
                case RealityDataSourceResult::NotModified:
                case RealityDataSourceResult::Success:
                    return RealityDataCacheResult::Success;
                case RealityDataSourceResult::Queued:
                case RealityDataSourceResult::Pending:
                    return RealityDataCacheResult::RequestQueued;
                case RealityDataSourceResult::Error_NotFound:
                    return RealityDataCacheResult::NotFound;
                default:
                    return RealityDataCacheResult::Error;
                }

        default:
            return RealityDataCacheResult::Error;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static ThreadSafeQueue<IRealityDataBasePtr>::Iterator Find(ThreadSafeQueue<IRealityDataBasePtr> const& queue, Utf8CP id)
    {
    BeMutexHolder lock (queue.GetConditionVariable().GetMutex());
    for (auto iter = queue.begin(); iter != queue.end(); ++iter)
        {
        IRealityDataBasePtr arrival = *iter;
        if (0 == strcmp(arrival->GetId(), id))
            return iter;
        }
    return queue.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::AddToArrivals(IRealityDataBase& data)
    {
    // 0 means arrivals queue is not used
    if (0 == m_arrivalsQueueSize)
        return;

    BeMutexHolder lock (m_arrivals.GetConditionVariable().GetMutex());
    auto iter = Find(m_arrivals, data.GetId());
    if (m_arrivals.end() != iter)
        m_arrivals.Erase(iter);

    // -1 means infinite
    if (-1 != m_arrivalsQueueSize)
        {
        while (m_arrivals.Size() > (unsigned)(m_arrivalsQueueSize - 1))
            m_arrivals.Pop();
        }
    m_arrivals.PushBack(&data);
    RDCLOG(LOG_DEBUG, "Pushed to arrivals: %s. Total: %d", data.GetId(), m_arrivals.Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RealityDataCache::GetFromArrivals(IRealityDataBase& data, Utf8CP id, RealityDataCacheOptions const& options)
    {
    IRealityDataBasePtr arrival;
    if (true)
        {
        BeMutexHolder lock (m_arrivals.GetConditionVariable().GetMutex());
        auto iter = Find(m_arrivals, id);
        if (m_arrivals.end() == iter)
            return ERROR;

        arrival = *iter;
        if (options.RemoveFromArrivals())
            m_arrivals.Erase(iter);
        }
    return data._InitFrom(*arrival, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::Cleanup()
    {
    m_arrivals.Clear();

    if (true)
        {
        BeMutexHolder lock(m_persistHandlersCS);
        m_persistHandlers.clear();
        }
    
    if (true)
        {
        BeMutexHolder lock(m_requestHandlersCS);
        m_requestHandlers.clear();
        }
    }

/*======================================================================================+
|   RealityDataAdmin
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCache& DgnPlatformLib::Host::RealityDataAdmin::GetCache()
    {
    if (m_cache.IsNull())
        {
        m_cache = RealityDataCache::Create(100);

        m_cache->RegisterSource(*HttpRealityDataSource::Create(8));

        BeFileName storageFileName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
        storageFileName.AppendToPath(L"RealityDataCache.db");
        m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(storageFileName));
        }
    return *m_cache;
    }

/*======================================================================================+
|   RealityDataThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataThread::RealityDataThread(Utf8CP threadName) 
    : m_threadId(-1), m_threadName(threadName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::Start()
    {
    BeThreadUtilities::StartNewThread(1024 * 1024, PlatformThreadRunner, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::ThreadRunner(void* arg)
    {
    auto& thread = * (RealityDataThread*) arg;
    thread.AddRef();
    thread.Run();
    thread.Release();
    }

/*---------------------------------------------------------------------------------**//**
* Runs on its own thread.
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::Run()
    {
    m_threadId = BeThreadUtilities::GetCurrentThreadId();

    if (!m_threadName.empty())
        BeThreadUtilities::SetCurrentThreadName(m_threadName.c_str());

    _Run();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t RealityDataThread::GetThreadId() const
    {
    return m_threadId;
    }

/*======================================================================================+
|   RealityDataThreadPool
+======================================================================================*/
#ifdef DEBUG_THREADPOOL
#define THREADPOOL_MSG(msg)     BeDebugLog(Utf8PrintfString("\t %llu\t ThreadPool\t %lld\t %s", BeTimeUtilities::GetCurrentTimeAsUnixMillis(), (Int64)BeThreadUtilities::GetCurrentThreadId(), msg).c_str());
#define THREADPOOL_TIMER(msg)   DebugTimer _debugtimer(msg);  
#else
#define THREADPOOL_MSG(msg) 
#define THREADPOOL_TIMER(msg)
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::QueueWork(RealityDataWork& work)
    {
    THREADPOOL_TIMER("QueueWork")
    THREADPOOL_MSG("QueueWork")

    BeMutexHolder lock(m_workQueueCS);
    auto thread = GetIdleThread();
    if (thread.IsNull())
        {
        THREADPOOL_MSG("QueueWork: No idle threads")
        if (!ShouldCreateNewThread())
            {
            m_workQueue.PushBack(&work);
            THREADPOOL_MSG("QueueWork: Added work item to queue")
            return;
            }
        thread = CreateThread();
        THREADPOOL_MSG("QueueWork: Created a new thread")
        }
    thread->DoWork(work);
    THREADPOOL_MSG("QueueWork: Sent work item to thread")
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataThreadPool::~RealityDataThreadPool()
    {
    THREADPOOL_MSG("~RealityDataThreadPool")
    Terminate();

    // clear the list of threads in a critical section
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        THREADPOOL_MSG("Clear threads list")
        m_threads.clear();
        m_threadsCV.notify_all();
        }

    BeMutexHolder workQueueLock(m_workQueueCS);
    THREADPOOL_MSG("Clear work queue")
    m_workQueue.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::Terminate()
    {
    THREADPOOL_MSG("Terminate")
    if (m_isTerminating)
        {
        THREADPOOL_MSG("Already terminating")
        return;
        }

    m_isTerminating = true;
    m_workQueue.GetConditionVariable().notify_all();

    // copy the threads list in a critical section
    bvector<RealityDataWorkerThreadPtr> threads;
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        for (auto pair : m_threads)
            threads.push_back(pair.first);
        }

    // terminate all threads
    THREADPOOL_MSG("Terminate all threads")
    for (auto thread : threads)
        {
        if (thread->TerminateRequested())
            continue;

        thread->Terminate();
        thread->Release();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThreadPtr RealityDataThreadPool::GetIdleThread() const
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    for (auto pair : m_threads)
        {
        if (!pair.second)
            return pair.first;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThreadPtr RealityDataThreadPool::CreateThread()
    {
    auto thread = RealityDataWorkerThread::Create(this, "BentleyThreadPoolWorker");
    thread->AddRef();
    thread->Start();
    BeMutexHolder lock(m_threadsCV.GetMutex());
    m_threads[thread.get()] = false;
    m_threadsCV.notify_all();
    return thread;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int RealityDataThreadPool::GetThreadsCount() const
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    return(int) m_threads.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataThreadPool::ShouldCreateNewThread() const
    {
    // do not exceed the "max threads" parameter
    if (GetThreadsCount() >= m_maxThreads)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::_OnThreadBusy(RealityDataWorkerThread& thread)
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    THREADPOOL_MSG("_OnThreadBusy: Marked the thread as busy")
    m_threads[&thread] = true;
    m_threadsCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::_OnThreadIdle(RealityDataWorkerThread& thread)
    {
    THREADPOOL_MSG("_OnThreadIdle")

    if (!_AssignWork(thread))
        {
        THREADPOOL_MSG("_OnThreadIdle: No work items in queue")
        BeMutexHolder lock(m_threadsCV.GetMutex());
        int idleThreadsCount = 0;
        for (auto pair : m_threads)
            {
            if (!pair.second)
                idleThreadsCount++;
            }
        
        THREADPOOL_MSG(Utf8PrintfString("_OnThreadIdle: Total idle threads: %d, allowed: %d", idleThreadsCount, m_maxIdleThreads).c_str())
        BeAssert(idleThreadsCount < m_maxIdleThreads || m_maxIdleThreads == 0);
        if (idleThreadsCount >= m_maxIdleThreads)
            {
            THREADPOOL_MSG("_OnThreadIdle removed the thread from list")
            m_threads.erase(&thread);
            if (!thread.TerminateRequested())
                {
                thread.Terminate();
                thread.Release();
                THREADPOOL_MSG("_OnThreadIdle: Terminated the thread")
                }
            }
        else
            {
            m_threads[&thread] = false;
            THREADPOOL_MSG("_OnThreadIdle: Marked the thread as idle")
            }

        m_threadsCV.notify_all();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataThreadPool::_AssignWork(RealityDataWorkerThread& thread)
    {
    if (m_isTerminating)
        return false;

    RealityDataWorkPtr work;
    if (true)
        {
        BeMutexHolder lock(m_workQueueCS);
        m_workQueue.Pop(work);
        }

    if (work.IsValid())
        {
        THREADPOOL_MSG("_OnThreadIdle: Popped work item from queue, send to idle thread")
        thread.DoWork(*work);
        return true;
        }

    return false;
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct AllThreadsIdlePredicate : IConditionVariablePredicate
{
private:
    RealityDataThreadPool const& m_pool;
public:
    AllThreadsIdlePredicate(RealityDataThreadPool const& pool) : m_pool(pool) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override 
        {
        BeMutexHolder lock(m_pool.m_threadsCV.GetMutex());
        for (auto pair : m_pool.m_threads)
            {
            bool isThreadBusy = pair.second;
            if (isThreadBusy && pair.first->GetThreadId() != BeThreadUtilities::GetCurrentThreadId())
                return false;
            }
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::WaitUntilAllThreadsIdle() const
    {
    AllThreadsIdlePredicate predicate(*this);
    m_threadsCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*======================================================================================+
|   RealityDataWorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThread::RealityDataWorkerThread(IStateListener* stateListener, Utf8CP threadName)
    : RealityDataThread(threadName), m_cv(), m_terminate(false), m_stateListener(stateListener), m_idleSince(BeTimeUtilities::GetCurrentTimeAsUnixMillis()), m_busySince(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::SetIsBusy(bool busy)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (busy && 0 != m_busySince || !busy && 0 != m_idleSince)
        return;

    if (busy)
        {
        m_idleSince = 0;
        m_busySince = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        _OnBusy();
        }
    else
        {
        m_idleSince = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        m_busySince = 0;
        _OnIdle();
        }
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_OnBusy()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadBusy(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_OnIdle()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadIdle(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::IsBusy(uint64_t* busyTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_busySince != 0)
        {
        if (NULL != busyTime)
            *busyTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_busySince;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::IsIdle(uint64_t* idleTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_idleSince != 0)
        {
        if (NULL != idleTime)
            *idleTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_idleSince;
        return true;
        }
    return false;
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IsIdlePredicate : IConditionVariablePredicate
{
private:
    RealityDataWorkerThread& m_thread;
public:
    IsIdlePredicate(RealityDataWorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.IsIdle();}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct HasWorkOrTerminatesPredicate : IConditionVariablePredicate
{
private:
    RealityDataWorkerThread& m_thread;

public:
    HasWorkOrTerminatesPredicate(RealityDataWorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.TerminateRequested() || m_thread.m_currentWork.IsValid();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_DoWork(RealityDataWork& work)
    {
    // note: if m_threadId is -1 it means that the thread hasn't been started yet
    // but that also means that we're not on that thread and should wait for idle, etc.
    BeMutexHolder holder(m_cv.GetMutex(), BeMutexHolder::Lock::No);
    if (BeThreadUtilities::GetCurrentThreadId() != GetThreadId())
        {
        IsIdlePredicate predicate(*this);
        holder.lock();
        m_cv.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);
        }

    BeAssert(m_currentWork.IsNull());
    SetIsBusy(true);
    m_currentWork = &work;
    m_cv.notify_all();

    if (BeThreadUtilities::GetCurrentThreadId() != GetThreadId())
        holder.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_Run()
    {
#ifdef BENTLEY_WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
    while (!m_terminate)
        {
        BeMutexHolder holder(m_cv.GetMutex(), BeMutexHolder::Lock::Yes);
        HasWorkOrTerminatesPredicate predicate(*this);
        m_cv.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);

        if (!m_terminate)
            {
            BeAssert(m_currentWork.IsValid());
            auto work = m_currentWork;
            m_currentWork = NULL;
            holder.unlock();
            work->_DoWork();
            }
        else
            holder.unlock();

        SetIsBusy(false);
        }
    SetIsBusy(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::Terminate()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    m_terminate = true;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::TerminateRequested() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_terminate;
    }
