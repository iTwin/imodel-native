/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#ifndef BENTLEY_WINRT
    #include <curl/curl.h>
    #ifdef GetCurrentTime
        #undef GetCurrentTime
    #endif
#endif

DPILOG_DEFINE(RealityDataCache)
#define RDCLOG(sev,...) {if (RealityDataCache_getLogger().isSeverityEnabled(sev)) {RealityDataCache_getLogger().messagev(sev, __VA_ARGS__);}}

USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
template<typename T> void ThreadSafeQueue<T>::Push(T const& element)
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
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop(T& element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_first.IsNull())
        return false;

    element = m_first->data;
    
    if (m_first.Equals(m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_first->next->prev = NULL;
        m_first = m_first->next;
        }

    m_cv.notify_all();
    return true;
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

// explicitly implement for testing purposes, 
// note: must be done AFTER all template functions are defined
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
template struct ThreadSafeQueue<int>;
END_BENTLEY_DGNPLATFORM_NAMESPACE
    
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
    m_idleTracker = WorkerThreadIdleTracker::Create(*this);
    m_worker = WorkerThread::Create(*this, m_idleTracker.get());
    m_worker->Start();
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct WorkerQueueNotEmptyPredicate : IConditionVariablePredicate
    {
    ThreadSafeQueue<RealityDataWorkPtr> const& m_queue;
    WorkerQueueNotEmptyPredicate(ThreadSafeQueue<RealityDataWorkPtr> const& queue) : m_queue(queue) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override 
        {
        BeMutexHolder lock(cv.GetMutex());
        return !m_queue.IsEmpty();
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
    auto& queue = m_storage->m_worker->m_workQueue;
    WorkerQueueNotEmptyPredicate predicate(queue);
    if (queue.GetConditionVariable().WaitOnCondition(&predicate, m_idleTime) || m_storage->m_worker->TerminateRequested())
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
    m_result = m_storage->wt_Select(m_data, m_id, m_options);
    m_hasResult = true;
    m_resultCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::SelectDataWork::GetResult() const
    {
    SelectDataWorkHasResultPredicate predicate(m_hasResult);
    auto result = m_resultCV.WaitOnCondition(&predicate, 1000);
    BeAssert(result);
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStorage::~BeSQLiteRealityDataStorage()
    {
    m_worker->Terminate();

    for (auto& cleanupHandler : m_cleanupHandlers)
        cleanupHandler->Release();

    m_database.CloseDb();
    delete &m_database;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeSQLiteRealityDataStorage::wt_Prepare(DatabasePrepareAndCleanupHandler const& prepareHandler)
    {
    BeSQLite::DbResult result;
    if (!m_initialized)
        {
        if (BeSQLite::BE_SQLITE_OK != (result = m_database.OpenBeSQLiteDb(m_filename, Db::OpenParams(Db::OpenMode::ReadWrite))))
            {
            RDCLOG(LOG_INFO, "%s: %s", Utf8String(m_filename).c_str(), BeSQLite::Db::InterpretDbResult(result));
            if (m_filename.DoesPathExist())
                {
                BeFileNameStatus deleteStatus = m_filename.BeDeleteFile();
                if (deleteStatus != BeFileNameStatus::Success)
                    {
                    RDCLOG(LOG_ERROR, "%s: %s", Utf8String(m_filename).c_str(), deleteStatus);
                    BeAssert(false);
                    return false;
                    }
                }

            if (BeSQLite::BE_SQLITE_OK != (result = m_database.CreateNewDb(m_filename)))
                {
                RDCLOG(LOG_ERROR, "%s: %s", Utf8String(m_filename).c_str(), BeSQLite::Db::InterpretDbResult(result));
                BeAssert(false);
                return false;
                }
            }

        // Stop all current transactions with savepoint <commit>. Enable auto VACUUM for database. Resume operations with <begin>.
        BeSQLite::Savepoint* savepoint = m_database.GetSavepoint(0);
        if (NULL != savepoint)
            savepoint->Commit(nullptr);

        m_database.TryExecuteSql("PRAGMA auto_vacuum = FULL");
        m_database.TryExecuteSql("VACUUM");

        if (NULL != savepoint)
            savepoint->Begin();
        
        m_initialized = true;
        }

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
        RDCLOG(LOG_ERROR, "%s: %s", Utf8String(m_filename).c_str(), BeSQLite::Db::InterpretDbResult(result));
        BeAssert(false);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Mantas.Ragauskas               01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Cleanup()
    {
    uint64_t realCacheSize;
    auto realFileSizeResult = m_filename.GetFileSize(realCacheSize);
    BeAssert(BeFileNameStatus::Success == realFileSizeResult);
   
    if ((0 != m_cacheSize) && (realCacheSize > m_cacheSize))
        {
        // percentage of cache to delete
        double pct = 100.0 * (realCacheSize - m_cacheSize) / realCacheSize;
        if (pct > 100)
            pct = 100;

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
    auto result = m_database.SaveChanges();
    BeAssert(BeSQLite::BE_SQLITE_OK == result);
    m_hasChanges = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::wt_Select(Data& data, Utf8CP id, SelectOptions const& options)
    {
    if (!wt_Prepare(*data._GetDatabasePrepareAndCleanupHandler()))
        return RealityDataStorageResult::Error;

    if (SUCCESS != data._InitFrom(m_database, id, options))
        return RealityDataStorageResult::NotFound;
    
    return RealityDataStorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::Select(Data& data, Utf8CP id, SelectOptions const& options)
    {
    auto work = SelectDataWork::Create(*this, id, data, options);
    m_worker->DoWork(*work);
    return work->GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Persist(Data const& data)
    {
    if (!wt_Prepare(*data._GetDatabasePrepareAndCleanupHandler()))
        return;

    BentleyStatus result = data._Persist(m_database);
    BeAssert(SUCCESS == result);
    m_hasChanges = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::Persist(Data const& data)
    {
    m_worker->DoWork(*PersistDataWork::Create(*this, data));
    return RealityDataStorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::PersistHandler::_Persist() const
    {
    return m_storage.Persist(*m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataBase const* BeSQLiteRealityDataStorage::PersistHandler::_GetData() const {return m_data.get();}

/*======================================================================================+
|   BeSQLiteRealityDataStorage::WorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThread::_DoWork(RealityDataWork& work)
    {
    if (TerminateRequested())
        return;

    if (IsBusy())
        m_workQueue.Push(&work);
    else
        {
        m_storage->AddRef();
        RealityDataWorkerThread::_DoWork(work);
        m_storage->Release();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThread::_OnIdle()
    {
    RealityDataWorkPtr work;
    if (m_workQueue.Pop(work))
        {
        DoWork(*work);
        return;
        }

    RealityDataWorkerThread::_OnIdle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThreadIdleTracker::_OnThreadIdle(RealityDataWorkerThread& thread)
    {
    if (m_storage.m_hasChanges)
        {
        // give some idle time before actually calling SaveChanges (default 5 seconds)
        m_storage.m_worker->DoWork(*CleanAndSaveChangesWork::Create(m_storage, m_storage.m_idleTime));
        }
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
RealityDataStorageResult InMemoryRealityDataStorage::Select(Data& data, Utf8CP id, SelectOptions const& options)
    {
    BeMutexHolder lock(m_cs);
    auto iter = m_map.find(id);
    if (m_map.end() != iter)
        {
        data._InitFrom(id, iter->second, options);
        if (options.GetRemoveAfterSelect())
            {
            Data const* d = (Data const*)iter->second;
            d->Release();
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
|   AsyncRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AsyncRealityDataSourceRequest::GetId() const {return _GetId();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataSourceResponse AsyncRealityDataSourceRequest::Handle() const {return _Handle();}

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
RealityDataSourceResult AsyncRealityDataSource<Derived>::QueueRequest(AsyncRealityDataSourceRequest const& request, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    if (ShouldIgnoreRequests())
        return RealityDataSourceResult::Error_Unknown;

    if (true)
        {
        BeMutexHolder lock(m_requestsCS);
        auto requestIter = m_activeRequests.find(request.GetId());
        if (requestIter != m_activeRequests.end())
            return RealityDataSourceResult::Pending;

        m_activeRequests.insert(request.GetId());
        }
    
    m_threadPool->QueueWork(*RequestHandler::Create(*this, request, responseReceiver));
    return RealityDataSourceResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Derived>
void AsyncRealityDataSource<Derived>::RequestHandler::_DoWork()
    {
    RealityDataSourceResponse response = m_request->Handle();

    switch(response.GetResult())
        {
        case RealityDataSourceResult::Error_CouldNotResolveHost:
        case RealityDataSourceResult::Error_NoConnection:
            m_source->SetIgnoreRequests(10 * 1000);
            break;
        case RealityDataSourceResult::Error_GatewayTimeout:
            m_source->SetIgnoreRequests(3 * 1000);
            break;
        }

    SendResponse(response);

    BeMutexHolder lock(m_source->m_requestsCS);
    auto requestIter = m_source->m_activeRequests.find(m_request->GetId());
    BeAssert(requestIter != m_source->m_activeRequests.end());
    m_source->m_activeRequests.erase(requestIter);
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
    Utf8String                    m_filename;
    RefCountedPtr<FileRealityDataSource::RequestOptions const> m_requestOptions;
    
    FileRealityDataSourceRequest(FileRealityDataSource::Data& data, Utf8CP url, FileRealityDataSource::RequestOptions const& requestOptions)
        : m_data(&data), m_filename(url), m_requestOptions(&requestOptions)
        {}

protected:
    virtual Utf8CP _GetId() const {return m_filename.c_str();}
    virtual RealityDataSourceResponse _Handle() const override
        {
        // open the file
        FILE* file = fopen (m_filename.c_str (), "rb");
        if (NULL == file)
            {
            BeAssert (false);
            return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_filename.c_str());
            }
    
        // get file size
        fseek (file, 0, SEEK_END);
        long size = ftell (file);
        rewind (file);

        // read file content
        bvector<Byte> data;
        data.resize(size);
        size_t bytesRead = fread (&data[0], sizeof (Utf8Char), size, file);
        fclose(file);
        if (bytesRead != size)
            {
            BeAssert (false);
            return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_filename.c_str());
            }
        
        if (SUCCESS != m_data->_InitFrom(m_filename.c_str(), data, *m_requestOptions))
            {
            BeAssert (false);
            return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_filename.c_str());
            }

        return RealityDataSourceResponse(RealityDataSourceResult::Success, m_filename.c_str(), m_data.get());
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
RealityDataSourceResult FileRealityDataSource::Request(Data& data, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    return QueueRequest(*FileRealityDataSourceRequest::Create(data, id, options), responseReceiver);
    }

/*======================================================================================+
|   HttpRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime GetExpirationDateFromCacheControlStr(Utf8CP str)
    {
    int maxage = 0, smaxage = 0;    // in seconds.
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

    int64_t maxage64 = (0 != smaxage ? smaxage : maxage);   // use int64_t to avoid overflow when converting to milliseconds.
    DateTime expirationDate;
    DateTime::FromUnixMilliseconds(expirationDate, currentTime + (1000 * maxage64));
    return expirationDate;
    }

#ifndef BENTLEY_WINRT
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
#endif

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRealityDataSource::Initialize()
    {
#ifndef BENTLEY_WINRT
    if (m_initialized)
        return SUCCESS;

    curl_global_init(CURL_GLOBAL_ALL);
    m_initialized = true;
    return SUCCESS;

#else
    return ERROR;
#endif
    }

#ifndef BENTLEY_WINRT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t HttpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp)
    {
    size_t totalSize = size * nmemb;
    auto buffer = (bvector<Byte>*) userp;
    buffer->insert(buffer->end(),(Byte*) ptr,(Byte*) ptr + totalSize);
    return totalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t HttpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData)
    {
    auto& header = * (bmap<Utf8String, Utf8String>*) userData;
    auto totalSize = size * nItems;

    Utf8String line(buffer, totalSize);
    line.Trim();
    if (Utf8String::IsNullOrEmpty(line.c_str()))
        return totalSize;
    
    if (header.empty())
        {
        // expecting the "HTTP/1.x 200 OK" header
        header["HTTP"] = line;
        }
    else
        {
        auto delimiterPos = line.find(':');
        if (Utf8String::npos == delimiterPos)
            {
            RDCLOG(LOG_INFO, "Malformed HTTP header: %s", line.c_str());
            BeAssert(false);
            return totalSize;
            }

        Utf8String key = line.substr(0, delimiterPos);
        
        // Not sure if we should keep or filter out fields with empty value. Maybe they mean something even if empty?
        // Anyway do not try to read beyond the input string when it happens.
        if(delimiterPos + 1 == line.size())
            {
            header[key] = "";
            }
        else
            {
            Utf8String value = line.substr(delimiterPos+2/* skip ':' and leading space*/);
            header[key] = value;
            }
        }
    
    return totalSize;
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct CurlHolder
{
private:
    CURL* m_curl;

public:
    CurlHolder() : m_curl(curl_easy_init()) {}
    ~CurlHolder() {curl_easy_cleanup(m_curl);}
    CURL* Get() const {return m_curl;}
};
#endif

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct HttpRealityDataSourceRequest : AsyncRealityDataSourceRequest
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
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual RealityDataSourceResponse _Handle() const override 
        {
#ifndef BENTLEY_WINRT
        CurlHolder curl;

        bmap<Utf8String, Utf8String> header;
        bvector<Byte> body;
        curl_easy_setopt(curl.Get(), CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, HttpBodyParser);
        curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl.Get(), CURLOPT_HEADERFUNCTION, HttpHeaderParser);
        curl_easy_setopt(curl.Get(), CURLOPT_HEADERDATA, &header);
        long nosignal=1;
        curl_easy_setopt(curl.Get(), CURLOPT_NOSIGNAL, &nosignal);

        curl_slist* httpHeaders = NULL;
        if (m_data->IsInitialized())
            {
            BeAssert(DateTime::Kind::Utc == m_data->GetExpirationDate().GetInfo().GetKind());
            httpHeaders = curl_slist_append(httpHeaders, Utf8PrintfString("If-Modified-Since: %s", GetAnsiCFormattedDateTime(m_data->GetExpirationDate()).c_str()).c_str());

            if (!Utf8String::IsNullOrEmpty(m_data->GetEntityTag()))
                httpHeaders = curl_slist_append(httpHeaders, Utf8PrintfString("If-None-Match: %s", m_data->GetEntityTag()).c_str());

            curl_easy_setopt(curl.Get(), CURLOPT_HTTPHEADER, httpHeaders);
            }

        RDCLOG(LOG_TRACE, "[%lld] GET %s",(uint64_t)BeThreadUtilities::GetCurrentThreadId(), m_url.c_str());

        CURLcode res = curl_easy_perform(curl.Get());
        curl_slist_free_all(httpHeaders);
        if (CURLE_OK != res)
            {
            switch(res)
                {
                case CURLE_COULDNT_RESOLVE_HOST:
                    RDCLOG(LOG_TRACE, "[%lld] CURLE_COULDNT_RESOLVE_HOST - waiting 10 seconds to re-try",(uint64_t)BeThreadUtilities::GetCurrentThreadId());
                    return RealityDataSourceResponse(RealityDataSourceResult::Error_CouldNotResolveHost, m_url.c_str());
                case CURLE_COULDNT_CONNECT:
                case CURLE_RECV_ERROR:
                    RDCLOG(LOG_TRACE, "[%lld] CURLE_COULDNT_CONNECT - waiting 10 seconds to re-try",(uint64_t)BeThreadUtilities::GetCurrentThreadId());
                    return RealityDataSourceResponse(RealityDataSourceResult::Error_NoConnection, m_url.c_str());
                default:
                    RDCLOG(LOG_ERROR, "[%lld] Unkown curl error %d",(uint64_t)BeThreadUtilities::GetCurrentThreadId(), res);
                    BeAssert(false && "All CURL errors should be handled");
                    return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_url.c_str());
                }
            }

        long responseCode;
        if (CURLE_OK != curl_easy_getinfo(curl.Get(), CURLINFO_RESPONSE_CODE, &responseCode) ||(0 == responseCode))
            {
            BeAssert(false && "All CURL errors should be handled");
            return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_url.c_str());
            }

        RDCLOG(LOG_TRACE, "[%lld] ResponseCode=%d",(uint64_t)BeThreadUtilities::GetCurrentThreadId(), responseCode);

        switch(responseCode)
            {
            case 200:   // ok
                {
                BeAssert(!m_data->IsInitialized() || m_data->IsExpired());
                m_data->ParseExpirationDateAndETag(header);
                if (SUCCESS != m_data->_InitFrom(m_url.c_str(), header, body, *m_requestOptions))
                    {
                    RDCLOG(LOG_INFO, "[%lld] response 200 but unable to initialize reality data",(uint64_t)BeThreadUtilities::GetCurrentThreadId());
                    return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_url.c_str());
                    }
                return RealityDataSourceResponse(RealityDataSourceResult::Success, m_url.c_str(), m_data.get());
                }

            case 304:   // not modified
                {
                BeAssert(m_data->IsInitialized());
                m_data->ParseExpirationDateAndETag(header);
                return RealityDataSourceResponse(RealityDataSourceResult::NotModified, m_url.c_str(), m_data.get());
                }

            case 404:   // not found
                return RealityDataSourceResponse(RealityDataSourceResult::Error_NotFound, m_url.c_str());

            case 504:   // gateway timeout
                RDCLOG(LOG_TRACE, "504 - waiting 3 seconds to re-try");
                return RealityDataSourceResponse(RealityDataSourceResult::Error_GatewayTimeout, m_url.c_str());

            default:    // not expected
                RDCLOG(LOG_ERROR, "Unhandled HTTP response: GET %s -> %d", m_url.c_str(), responseCode);
                return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_url.c_str());
            }
#else
        return RealityDataSourceResponse(RealityDataSourceResult::Error_Unknown, m_url.c_str());
#endif
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
RealityDataSourceResult HttpRealityDataSource::Request(Data& data, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
    {
    if (SUCCESS != Initialize())
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        }

    QueueRequest(*HttpRealityDataSourceRequest::Create(data, id, options), responseReceiver);
    return RealityDataSourceResult::Success;
    }

/*======================================================================================+
|   RealityDataCache
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCachePtr RealityDataCache::Create() {return new RealityDataCache();}

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
    BeAssert(nullptr == GetStorage(storage._GetStorageId()));
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
    BeAssert(nullptr == GetSource(source._GetSourceId()));
    m_sources[source._GetSourceId()] = &source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::QueuePersistHandler(Utf8CP id, IRealityDataStoragePersistHandler const& caller)
    {
    BeMutexHolder lock(m_persistHandlersCS);
    m_persistHandlers[id].insert(&caller);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataStoragePersistHandler const* RealityDataCache::DequeuePersistHandler(Utf8CP id, IRealityDataBase const* data, bool dealloc)
    {
    BeMutexHolder lock(m_persistHandlersCS);
    auto setIter = m_persistHandlers.find(id);
    if (m_persistHandlers.end() == setIter)
        return nullptr;

    bset<IRealityDataStoragePersistHandler const*>& set = setIter->second;
    for (auto handlerIter = set.begin(); handlerIter != set.end(); handlerIter++)
        {
        IRealityDataStoragePersistHandler const* handler = *handlerIter;
        if (nullptr == data || handler->_GetData() == data)
            {
            set.erase(handlerIter);
            if (!dealloc)
                return handler;

            delete handler;
            break;
            }        
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::_OnResponseReceived(RealityDataSourceResponse const& response)
    {
    IRealityDataStoragePersistHandler const* persistHandler = DequeuePersistHandler(response.GetId(), response.GetData());
    if (nullptr == persistHandler)
        BeAssert(false);

    switch(response.GetResult())
        {
        case RealityDataSourceResult::Success:
        case RealityDataSourceResult::NotModified:
            {
            BeAssert(nullptr != response.GetData());
            BeAssert(response.GetData() == persistHandler->_GetData());
            response.GetData()->SetInitialized();
            RealityDataStorageResult result = persistHandler->_Persist();
            BeAssert(RealityDataStorageResult::Success == result);
            break;
            }
        case RealityDataSourceResult::Error_GatewayTimeout:
        case RealityDataSourceResult::Error_CouldNotResolveHost:
        case RealityDataSourceResult::Error_NoConnection:
        case RealityDataSourceResult::Error_NotFound:
        case RealityDataSourceResult::Error_Unknown:
            break;
        default:
            BeAssert(false);
        }

    delete persistHandler;

    // WIP: need a callback???
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
        m_cache = RealityDataCache::Create();

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
    // Don't try to start the new thread here in the constructor. The sub-class vtable has not been set up yet.
    // If the thread actually starts running before this constructor returns, then PlatformThreadRunner will 
    // call this->_Run, and that will still be a pure virtual function. In fact, that's exactly what happens in iOS.
    //BeThreadUtilities::StartNewThread(1024 * 1024, PlatformThreadRunner, this);
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
#define THREADPOOL_MSG(msg)     BeDebugLog(Utf8PrintfString("\t %llu\t ThreadPool\t %lld\t %s", BeTimeUtilities::GetCurrentTimeAsUnixMillis(),(Int64) BeThreadUtilities::GetCurrentThreadId(), msg).c_str());
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
            m_workQueue.Push(&work);
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
    bvector<RealityDataWorkerThreadPtr> threads;

    // copy the threads list in a critical section
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        for (auto pair : m_threads)
            threads.push_back(pair.first);
        }

    // terminate all threads
    for (auto thread : threads)
        {
        thread->Terminate();
        thread->Release();
        }

    // clear the list of threads in a critical section
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        m_threads.clear();
        m_threadsCV.notify_all();
        }

    BeMutexHolder workQueueLock(m_workQueueCS);
    m_workQueue.Clear();
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
        }
    else
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
            m_threads.erase(&thread);
            thread.Release();
            thread.Terminate();
            THREADPOOL_MSG("_OnThreadIdle: Terminated the thread")
            }
        else
            {
            m_threads[&thread] = false;
            THREADPOOL_MSG("_OnThreadIdle: Marked the thread as idle")
            }

        m_threadsCV.notify_all();
        }
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
            if (pair.second)
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
    if (NULL != m_stateListener && !m_terminate)
        {
        m_stateListener->AddRef();
        m_stateListener->_OnThreadBusy(*this);
        m_stateListener->Release();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_OnIdle()
    {
    if (NULL != m_stateListener && !m_terminate)
        {
        m_stateListener->AddRef();
        m_stateListener->_OnThreadIdle(*this);
        m_stateListener->Release();
        }
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_Run()
    {
    while (!m_terminate)
        {
        BeMutexHolder holder(m_cv.GetMutex());
        HasWorkOrTerminatesPredicate predicate(*this);
        m_cv.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);

        if (!m_terminate)
            {
            BeAssert(m_currentWork.IsValid());
            auto work = m_currentWork;
            m_currentWork = NULL;
            holder.unlock();

            work->_DoWork();
            SetIsBusy(false);
            }
        }
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
