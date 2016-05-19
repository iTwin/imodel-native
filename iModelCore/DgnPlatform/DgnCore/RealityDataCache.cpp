/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_BENTLEY_REALITYDATA_NAMESPACE
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IsIdlePredicate : IConditionVariablePredicate
{
private:
    WorkerThread& m_thread;
public:
    IsIdlePredicate(WorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.IsIdle();}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct HasWorkOrTerminatesPredicate : IConditionVariablePredicate
{
private:
    WorkerThread& m_thread;

public:
    HasWorkOrTerminatesPredicate(WorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.TerminateRequested() || m_thread.m_currentWork.IsValid();}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct AllThreadsIdlePredicate : IConditionVariablePredicate
{
private:
    ThreadPool const& m_pool;
public:
    AllThreadsIdlePredicate(ThreadPool const& pool) : m_pool(pool) {}
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
END_BENTLEY_REALITYDATA_NAMESPACE

USING_NAMESPACE_BENTLEY_REALITYDATA

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::PushFront(T element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = Item::Create(element, nullptr, m_first.get());
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
    auto item = Item::Create(element, m_last.get(), nullptr);
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
        m_first = m_last = nullptr;
        }
    else
        {
        m_first->next->prev = nullptr;
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
        m_first = m_last = nullptr;
        }
    else
        {
        m_last->prev->next = nullptr;
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
        temp->next = nullptr;
        temp->prev = nullptr;
        }
    m_first = nullptr;
    m_last = nullptr;
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
BEGIN_BENTLEY_REALITYDATA_NAMESPACE
template struct ThreadSafeQueue<int>;
END_BENTLEY_REALITYDATA_NAMESPACE

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct StorageBusyRetry : BeSQLite::BusyRetry
    {
    virtual int _OnBusy(int count) const
        {
        if (count > 100)
            {
            BeAssert(false && "Exceeded maximum retries count");
            return 0;
            }
        
        int sleepTime = 10 * (count + 1);
        RDCLOG(LOG_DEBUG, "Database is busy. Waiting for %d ms before retry...", sleepTime);
        BeThreadUtilities::BeSleep(sleepTime);
        return 1;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Storage::Storage(int numThreads, SchedulingMethod schedulingMethod, uint32_t idleTime) : m_hasChanges(false), m_idleTime(idleTime)
    {
    m_retry = new StorageBusyRetry();
    m_threadPool = new StorageThreadPool(*this, numThreads, schedulingMethod);
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct ThreadPoolQueueNotEmptyPredicate : IConditionVariablePredicate
    {
    Storage::StorageThreadPool& m_pool;
    ThreadPoolQueueNotEmptyPredicate( Storage::StorageThreadPool& pool) : m_pool(pool) {}
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
void Storage::CleanAndSaveChangesWork::_DoWork()
    {
    auto& queue = m_storage->m_threadPool->GetQueue();
    ThreadPoolQueueNotEmptyPredicate predicate(*m_storage->m_threadPool);
    if (queue.GetConditionVariable().WaitOnCondition(&predicate, m_idleTime))
        return;

    BeMutexHolder lock(m_storage->m_saveChangesMux);
    if (!m_storage->m_hasChanges)
        return;

    m_storage->wt_Cleanup();
    m_storage->wt_SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::PersistDataWork::_DoWork()
    {
    m_storage->wt_Persist(*m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::SelectDataWork::_DoWork()
    {
    BeMutexHolder lock(m_resultCV.GetMutex());
    m_result = m_storage->wt_Select(*m_data, m_options, m_responseReceiver);
    m_hasResult = true;
    m_resultCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StorageResult Storage::SelectDataWork::GetResult() const
    {
    SelectDataWorkHasResultPredicate predicate(m_hasResult);
    auto result = m_resultCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    BeAssert(result);
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::Terminate()
    {
    m_threadPool->Terminate();
    m_threadPool->WaitUntilAllThreadsIdle();

    if (m_database.IsDbOpen())
        m_database.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Storage::OpenAndPrepare(BeFileNameCR fileName)
    {
    DbResult result = m_database.OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes, m_retry.get()));

    if (BE_SQLITE_OK != result)
        {
        Db::CreateParams createParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, false, DefaultTxn::Yes, m_retry.get());
        if (BeSQLite::BE_SQLITE_OK != (result = m_database.CreateNewDb(fileName, BeSQLite::BeGuid(), createParams)))
            {
            RDCLOG(LOG_ERROR, "%s: %s", fileName.GetNameUtf8().c_str(), Db::InterpretDbResult(result));
            BeAssert(false);
            return ERROR;
            }
        }        

    if (SUCCESS != _PrepareDatabase(m_database))
        {
        BeAssert(false);
        return ERROR;
        }
    
    if (BE_SQLITE_OK != (result = m_database.SaveChanges()))
        {
        RDCLOG(LOG_ERROR, "%s: %s", fileName.GetNameUtf8().c_str(), Db::InterpretDbResult(result));
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Mantas.Ragauskas               01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::wt_Cleanup()
    {
    BentleyStatus result = _CleanupDatabase(m_database);
    BeAssert(SUCCESS == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::wt_SaveChanges()
    {
    auto result = m_database.SaveChanges();
    BeAssert(BeSQLite::BE_SQLITE_OK == result);
    m_hasChanges.store(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StorageResult Storage::wt_Select(Payload& data, Options options, Cache& responseReceiver)
    {
    StorageResult result;
    if (SUCCESS != data._LoadFromStorage(m_database))
        {
        responseReceiver._OnResponseReceived(*new StorageResponse(StorageResult::NotFound, data), options, !options.m_forceSynchronous);
        result = StorageResult::NotFound;
        }
    else
        {
        responseReceiver._OnResponseReceived(*new StorageResponse(StorageResult::Success, data), options, !options.m_forceSynchronous);
        result = StorageResult::Success;
        }

    if (true)
        {
        BeMutexHolder lock(m_activeRequestsMux);
        BeAssert(m_activeRequests.end() != m_activeRequests.find(data.GetPayloadId()));
        m_activeRequests.erase(m_activeRequests.find(data.GetPayloadId()));
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StorageResult Storage::_Select(Payload& data, Options options, Cache& responseReceiver)
    {
    if (true)
        {
        BeMutexHolder lock(m_activeRequestsMux);
        if (m_activeRequests.end() != m_activeRequests.find(data.GetPayloadId()))
            return StorageResult::Pending;
        m_activeRequests.insert(data.GetPayloadId());
        }

    auto work = new SelectDataWork(*this, data, options, responseReceiver);
    m_threadPool->QueueWork(*work);

    if (options.m_forceSynchronous)
        return work->GetResult();

    return StorageResult::Queued;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Storage::wt_Persist(Payload const& data)
    {
    BentleyStatus result = data._PersistToStorage(m_database);
    BeAssert(SUCCESS == result);
    m_hasChanges.store(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StorageResult Storage::Persist(Payload const& data)
    {
    m_threadPool->QueueWork(*new PersistDataWork(*this, data));
    return StorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Storage::StorageThreadPool::_AssignWork(WorkerThread& thread)
    {
    if (ThreadPool::_AssignWork(thread))
        return true;

    if (m_storage.m_hasChanges && !IsTerminating())
        {
        thread.DoWork(*new CleanAndSaveChangesWork(m_storage, m_storage.m_idleTime));
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<SourceResponse> AsyncSourceRequest::Handle(BeMutex& cs) const
    {
    RefCountedPtr<SourceResponse> response = _Handle();
    BeMutexHolder lock(cs);
    m_response = response;
    return m_response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncSourceRequest::ShouldCancelRequest() const {return nullptr != m_cancellationToken && m_cancellationToken->_ShouldCancel();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncSource::SetIgnoreRequests(uint32_t ignoreTime)
    {
    m_ignoreRequestsUntil.store(BeTimeUtilities::GetCurrentTimeAsUnixMillis() + ignoreTime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncSource::ShouldIgnoreRequests() const
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_ignoreRequestsUntil;
    }

//===================================================================================
// @bsiclass                                        Grigas.Petraitis        04/2015
//===================================================================================
struct AsyncSourceRequest::SynchronousRequestPredicate : IConditionVariablePredicate
{
private:
    AsyncSourceRequest const& m_request;
public:
    SynchronousRequestPredicate(AsyncSourceRequest const& request) : m_request(request) {}
    SourceResult GetResult() const {return m_request.m_response->GetResult();}
    virtual bool _TestCondition(BeConditionVariable &cv) override {BeMutexHolder lock(cv.GetMutex()); return m_request.m_response.IsValid();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SourceResult AsyncSource::QueueRequest(AsyncSourceRequest& request, Cache& responseReceiver)
    {
    if (!request.GetRequestOptions().m_retryNotFoundRequests)
        {
        BeMutexHolder lock(m_requestsCS);
        if (m_notFoundRequests.end() != m_notFoundRequests.find(request.GetPayload().GetPayloadId()))
            return SourceResult::Error_NotFound;
        }

    if (ShouldIgnoreRequests())
        return SourceResult::Ignored;

    if (!request.GetRequestOptions().m_forceSynchronous)
        {
        BeMutexHolder lock(m_requestsCS);
        auto requestIter = m_activeRequests.find(request.GetPayload().GetPayloadId());
        if (requestIter != m_activeRequests.end())
            return SourceResult::Pending;

        m_activeRequests.insert(request.GetPayload().GetPayloadId());
        }
    
    RefCountedPtr<RequestHandler> handler = RequestHandler::Create(*this, request, responseReceiver);
    request.SetCancellationToken(*handler);
    m_threadPool->QueueWork(*handler);

    if (!request.GetRequestOptions().m_forceSynchronous)
        return SourceResult::Queued;

    AsyncSourceRequest::SynchronousRequestPredicate predicate(request);
    m_synchronizationCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    return predicate.GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncSource::RequestHandler::_DoWork()
    {
    RefCountedPtr<SourceResponse> response = m_request->Handle(m_source->m_synchronizationCV.GetMutex());
    switch (response->GetResult())
        {
        case SourceResult::Error_CouldNotResolveHost:
        case SourceResult::Error_NoConnection:
            m_source->SetIgnoreRequests(10 * 1000);
            break;
        case SourceResult::Error_GatewayTimeout:
            m_source->SetIgnoreRequests(3 * 1000);
            break;
        case SourceResult::Error_NotFound:
            {
            if (!m_request->GetRequestOptions().m_retryNotFoundRequests)
                {
                BeMutexHolder lock(m_source->m_requestsCS);
                m_source->m_notFoundRequests.insert(m_request->GetPayload().GetPayloadId());
                }
            }
        }

    if (!m_source->m_terminateRequested)
        SendResponse(*response, m_request->GetRequestOptions());

    if (!m_request->GetRequestOptions().m_forceSynchronous)
        {
        BeMutexHolder lock(m_source->m_requestsCS);
        auto requestIter = m_source->m_activeRequests.find(m_request->GetPayload().GetPayloadId());
        BeAssert(requestIter != m_source->m_activeRequests.end());
        m_source->m_activeRequests.erase(requestIter);
        }

    m_source->m_synchronizationCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncSource::RequestHandler::_ShouldCancel() const
    {
    return m_source->m_terminateRequested;
    }

//===================================================================================
// @bsiclass                                        Grigas.Petraitis        03/2015
//===================================================================================
struct FileSourceRequest : AsyncSourceRequest
{
protected:
    virtual RefCountedPtr<SourceResponse> _Handle() const override
        {
        if (!BeFileName::DoesPathExist(BeFileName(m_payload->GetPayloadId())))
            return new SourceResponse(SourceResult::Error_NotFound, *m_payload);

        // open the file
        BeFile dataFile;
        if (BeFileStatus::Success != dataFile.Open(m_payload->GetPayloadId().c_str(), BeFileAccess::Read))
            {
            BeAssert(false);
            return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
            }
    
        // read file content
        ByteStream data;
        if (BeFileStatus::Success != dataFile.ReadEntireFile(data))
            {
            BeAssert(false);
            return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
            }

        if (ShouldCancelRequest())
            return new SourceResponse(SourceResult::Cancelled, *m_payload);
        
        if (SUCCESS != m_payload->_LoadFromFile(data))
            {
            BeAssert(false);
            return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
            }

        return new SourceResponse(SourceResult::Success, *m_payload);
        }

public:
    FileSourceRequest(Payload& data, Options options) : AsyncSourceRequest(nullptr, options, data) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SourceResult FileSource::_Request(Payload& data, Options options, Cache& responseReceiver)
    {
    return QueueRequest(*new FileSourceRequest(data, options), responseReceiver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime GetExpirationDateFromCacheControlStr(Utf8CP str)
    {
    int maxage = 0;
    int start = 0;
    int i = 0;
    while (true)
        {
        if (str[i] == ',' || str[i] == '\0')
            {
            // means we passed an option
            Utf8String option(str + start, i - start);
            option.Trim();

            if (option.Equals("no-cache") || option.Equals("no-store"))
                return DateTime::GetCurrentTimeUtc(); 
            
            if (option.length() >= 7 && 0 == strncmp("max-age", option.c_str(), 7))
                {
                Utf8CP value = option.c_str() + 8;
                sscanf(value, "%d", &maxage);
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
    DateTime::FromUnixMilliseconds(expirationDate, currentTime + 1000 * maxage);
    return expirationDate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetAnsiCFormattedDateTime(DateTime const& dateTime)
    {
    Utf8CP wkday = "";
    switch (dateTime.GetDayOfWeek())
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
    switch (dateTime.GetMonth())
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
* @bsimethod                                     Grigas.Petraitis               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime GetDefaultExpirationDate()
    {
    DateTime now = DateTime::GetCurrentTimeUtc();
    return DateTime(now.GetInfo().GetKind(), now.GetYear() + 1, now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Payload::ParseExpirationDateAndETag(bmap<Utf8String, Utf8String> const& header)
    {
    auto cacheControlIter = header.find("Cache-Control");
    SetExpirationDate((cacheControlIter != header.end()) ? GetExpirationDateFromCacheControlStr(cacheControlIter->second.c_str()) : GetDefaultExpirationDate());

    auto etagIter = header.find("ETag");
    if (header.end() != etagIter)
        SetEntityTag(etagIter->second.c_str());
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct HttpSourceRequest : AsyncSourceRequest, IHttpRequestCancellationToken
{
    virtual bool _ShouldCancelHttpRequest() const override {return ShouldCancelRequest();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis               03/2015
    +---------------+---------------+---------------+---------------+-----------+------*/
    virtual RefCountedPtr<SourceResponse> _Handle() const override 
        {
        bmap<Utf8String, Utf8String> header;
        if (m_payload->GetExpirationDate().IsValid())
            {
            BeAssert(DateTime::Kind::Utc == m_payload->GetExpirationDate().GetInfo().GetKind());
            header["If-Modified-Since"] = GetAnsiCFormattedDateTime(m_payload->GetExpirationDate());
            }
        if (!Utf8String::IsNullOrEmpty(m_payload->GetEntityTag()))
            header["If-None-Match"] = m_payload->GetEntityTag();

        Utf8StringCR url = m_payload->GetPayloadId();
        HttpResponsePtr response;
        HttpRequestStatus requestStatus = HttpHandler::Instance().Request(response, HttpRequest(url.c_str(), header), this);
        switch (requestStatus)
            {
            case HttpRequestStatus::NoConnection:
                return new SourceResponse(SourceResult::Error_NoConnection, *m_payload);
            case HttpRequestStatus::CouldNotResolveHost:
            case HttpRequestStatus::CouldNotResolveProxy:
                return new SourceResponse(SourceResult::Error_CouldNotResolveHost,  *m_payload);
            case HttpRequestStatus::UnknownError:
                BeAssert(false && "All HTTP errors should be handled");
                return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
            }

        if (requestStatus == HttpRequestStatus::Aborted || ShouldCancelRequest())
            return new SourceResponse(SourceResult::Cancelled, *m_payload);

        BeAssert(response.IsValid());
        switch (response->GetStatus())
            {
            case HttpResponseStatus::Success:
                {
                m_payload->ParseExpirationDateAndETag(response->GetHeader());
                if (SUCCESS != m_payload->_LoadFromHttp(response->GetHeader(), response->GetBody()))
                    {
                    RDCLOG(LOG_INFO, "[%lld] response 200 but unable to initialize reality data",(uint64_t)BeThreadUtilities::GetCurrentThreadId());
                    return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
                    }
                return new SourceResponse(SourceResult::Success, *m_payload);
                }

            case HttpResponseStatus::NotModified:
                {
                m_payload->ParseExpirationDateAndETag(header);
                return new SourceResponse(SourceResult::NotModified, *m_payload);
                }

            case HttpResponseStatus::NotFound:
                return new SourceResponse(SourceResult::Error_NotFound, *m_payload);

            case HttpResponseStatus::Forbidden:
                return new SourceResponse(SourceResult::Error_AccessDenied, *m_payload);

            case HttpResponseStatus::GatewayTimeout:
                return new SourceResponse(SourceResult::Error_GatewayTimeout, *m_payload);

            default:
                return new SourceResponse(SourceResult::Error_Unknown, *m_payload);
            }
        }

    HttpSourceRequest(Payload& data, Options options) : AsyncSourceRequest(nullptr, options, data) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SourceResult HttpSource::_Request(Payload& data, Options options, Cache& responseReceiver)
    {
    return QueueRequest(*new HttpSourceRequest(data, options), responseReceiver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::_OnResponseReceived(SourceResponse const& response, Options options)
    {
    switch (response.GetResult())
        {
        case SourceResult::Success:
        case SourceResult::NotModified:
            {
            if (!m_storage.IsValid())
                return;

            StorageResult result = m_storage->Persist(response.GetPayload());
            BeAssert(StorageResult::Success == result);
            break;
            }

        case SourceResult::Error_NotFound:
            response.GetPayload()._OnNotFound();
            break;

        case SourceResult::Error_GatewayTimeout:
        case SourceResult::Error_CouldNotResolveHost:
        case SourceResult::Error_NoConnection:
        case SourceResult::Error_AccessDenied:
        case SourceResult::Error_Unknown:
        case SourceResult::Cancelled:
            response.GetPayload()._OnError();
            break;

        default:
            BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::_OnResponseReceived(StorageResponse const& response, Options options, bool isAsync)
    {
    CacheResult result = HandleStorageResponse(response, options);
    
    if (!isAsync)
        {
        BeMutexHolder lock(m_resultsMux);
        BeAssert(m_results.end() == m_results.find(&response.GetPayload()));
        m_results[&response.GetPayload()] = result;
        }

    switch (result)
        {
        case CacheResult::NotFound:
            response.GetPayload()._OnNotFound();
            break;

        case CacheResult::Error:
            response.GetPayload()._OnError();
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheResult Cache::HandleStorageResponse(StorageResponse const& response, Options options)
    {    
    switch (response.GetResult())
        {
        case StorageResult::Success:
            {
            if (response.GetPayload()._IsExpired() && options.m_requestFromSource)
                {
                SourceResult requestResult = m_source->_Request(response.GetPayload(), options, *this);
                return ResolveResult(response.GetResult(), requestResult);
                }
            break;
            }

        case StorageResult::NotFound:
            {
            SourceResult requestResult = m_source->_Request(response.GetPayload(), options, *this);
            return ResolveResult(response.GetResult(), requestResult);
            }

        case StorageResult::Queued:
            {
            BeAssert(false);
            break;
            }
        }

    return ResolveResult(response.GetResult());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheResult Cache::GetResult(Payload& data, StorageResult storageResult)
    {
    CacheResult result = ResolveResult(storageResult);
    if (CacheResult::RequestQueued == result || CacheResult::Error == result)
        return result;

    // unless there was an error or the request was queued, we expect to find the result in the m_results map
    BeMutexHolder lock(m_resultsMux);
    auto iter = m_results.find(&data);
    BeAssert(m_results.end() != iter);
    CacheResult returnValue = iter->second;
    m_results.erase(iter);
    return returnValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheResult Cache::GetResult(SourceResult sourceResult)
    {
    return ResolveResult(StorageResult::NotFound, sourceResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheResult Cache::ResolveResult(StorageResult storageResult, SourceResult sourceResult) const
    {
    switch (storageResult)
        {
        case StorageResult::Success:
            return CacheResult::Success;

        case StorageResult::Queued:
        case StorageResult::Pending:
            return CacheResult::RequestQueued;

        case StorageResult::NotFound:
            switch (sourceResult)
                {
                case SourceResult::NotModified:
                case SourceResult::Success:
                    return CacheResult::Success;
                case SourceResult::Queued:
                case SourceResult::Pending:
                    return CacheResult::RequestQueued;
                case SourceResult::Error_NotFound:
                    return CacheResult::NotFound;
                default:
                    return CacheResult::Error;
                }

        default:
            return CacheResult::Error;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Thread::Thread(Utf8CP threadName) : m_threadId(-1), m_threadName(threadName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Thread::Start()
    {
    BeThreadUtilities::StartNewThread(1024 * 1024, PlatformThreadRunner, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Thread::ThreadRunner(void* arg)
    {
    ThreadPtr thread = (Thread*)arg;
    thread->Run();
    }

/*---------------------------------------------------------------------------------**//**
* Runs on its own thread.
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Thread::Run()
    {
    m_threadId = BeThreadUtilities::GetCurrentThreadId();

    if (!m_threadName.empty())
        BeThreadUtilities::SetCurrentThreadName(m_threadName.c_str());

    DgnDb::SetThreadId(DgnDb::ThreadId::RealityData);
    _Run();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t Thread::GetThreadId() const
    {
    return m_threadId;
    }

/*======================================================================================+
|   ThreadPool
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
void ThreadPool::QueueWork(Work& work)
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
ThreadPool::~ThreadPool()
    {
    THREADPOOL_MSG("~ThreadPool")
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
void ThreadPool::Terminate()
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
    bvector<WorkerThreadPtr> threads;
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        for (auto pair : m_threads)
            threads.push_back(pair.first);
        }

    // terminate all threads
    THREADPOOL_MSG(Utf8PrintfString("Terminate all threads: %d", threads.size()).c_str());
    while (!threads.empty())
        {
        WorkerThreadPtr thread = threads[0];
        threads.erase(threads.begin());

        if (thread->Terminate())
            thread->Release();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThreadPtr ThreadPool::GetIdleThread() const
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    for (auto pair : m_threads)
        {
        if (!pair.second)
            return pair.first;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThreadPtr ThreadPool::CreateThread()
    {
    auto thread = WorkerThread::Create(this, "RealityData");
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
int ThreadPool::GetThreadsCount() const
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    return (int) m_threads.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ThreadPool::ShouldCreateNewThread() const
    {
    // do not exceed the "max threads" parameter
    if (GetThreadsCount() >= m_maxThreads)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::_OnThreadBusy(WorkerThread& thread)
    {
    BeMutexHolder lock(m_threadsCV.GetMutex());
    THREADPOOL_MSG("_OnThreadBusy: Marked the thread as busy")
    m_threads[&thread] = true;
    m_threadsCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::_OnThreadIdle(WorkerThread& thread)
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
            if (thread.Terminate())
                {
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
bool ThreadPool::_AssignWork(WorkerThread& thread)
    {
    if (m_isTerminating)
        return false;

    WorkPtr work;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::WaitUntilAllThreadsIdle() const
    {
    AllThreadsIdlePredicate predicate(*this);
    m_threadsCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*======================================================================================+
|   WorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThread::WorkerThread(IStateListener* stateListener, Utf8CP threadName)
    : Thread(threadName), m_cv(), m_terminate(false), m_stateListener(stateListener), m_idleSince(BeTimeUtilities::GetCurrentTimeAsUnixMillis()), m_busySince(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WorkerThread::SetIsBusy(bool busy)
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
void WorkerThread::_OnBusy()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadBusy(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WorkerThread::_OnIdle()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadIdle(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorkerThread::IsBusy(uint64_t* busyTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_busySince != 0)
        {
        if (nullptr != busyTime)
            *busyTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_busySince;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorkerThread::IsIdle(uint64_t* idleTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_idleSince != 0)
        {
        if (nullptr != idleTime)
            *idleTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_idleSince;
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WorkerThread::_DoWork(Work& work)
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
void WorkerThread::_Run()
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
            m_currentWork = nullptr;
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
bool WorkerThread::Terminate()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_terminate)
        return false;

    m_terminate = true;
    m_cv.notify_all();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorkerThread::TerminateRequested() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_terminate;
    }
