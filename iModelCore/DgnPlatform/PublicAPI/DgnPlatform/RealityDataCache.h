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

DGNPLATFORM_REF_COUNTED_PTR(RealityDataThread)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataThreadPool)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataWork)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataWorkerThread)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataCache)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataStorage)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataSource)

DGNPLATFORM_REF_COUNTED_PTR(HttpRealityDataSource)
DGNPLATFORM_REF_COUNTED_PTR(FileRealityDataSource)

struct HasWorkOrTerminatesPredicate;
struct IsIdlePredicate;
struct AllThreadsIdlePredicate;
struct ThreadPoolQueueNotEmptyPredicate;

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Options for request function.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataOptions
{
    bool m_requestFromSource = true;
    bool m_forceSynchronous = false;
    bool m_removeAfterSelect = false;
    bool m_retryNotFoundRequests = false;
};

//=======================================================================================
//! The base class for all reality data implementations. 
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct RealityData  : RefCountedBase
{
private:
    DateTime    m_expirationDate;
    Utf8String  m_entityTag;

public:
    void ParseExpirationDateAndETag(bmap<Utf8String, Utf8String> const& header);
    void SetExpirationDate(DateTime const& date) {m_expirationDate = date;}
    void SetEntityTag(Utf8CP eTag) {m_entityTag = eTag;}
    DateTime const& GetExpirationDate() const {return m_expirationDate;}
    Utf8CP GetEntityTag() const {return m_entityTag.c_str();}

    RealityData() {}
    virtual ~RealityData() {}
    virtual Utf8CP _GetId() const = 0;
    virtual bool _IsExpired() const = 0;
    virtual void _OnError() {}
    virtual void _OnNotFound() {}

    //! Initialize data from the database using key and select options.
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP key) = 0;

    //! Persist this data in the database.
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const = 0;

    //! Initializes this data with file path, content and request options.
    //! @param[in] filepath     The path of the file that the data was read from.
    //! @param[in] data         The file content.
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) = 0;

    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) = 0;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
enum class SchedulingMethod
{
    FIFO,   // queue
    LIFO    // stack
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
template<typename T> struct ThreadSafeQueue
{
    struct Item;
    typedef RefCountedPtr<Item> ItemPtr;
    struct Item : RefCountedBase
        {
        ItemPtr prev;
        ItemPtr next;
        T       data;
        Item(T d, Item* p, Item* n) : prev(p), next(n), data(d) {}
        static ItemPtr Create(T d, Item* p, Item* n) {return new Item(d, p, n);}
        };

    struct Iterator
    {
    friend struct ThreadSafeQueue<T>;
    private:
        Item* m_curr;
        Iterator(Item* first) : m_curr(first) {}
    public:
        bool IsValid() const {return nullptr != m_curr;}
        Iterator& operator++() {m_curr = m_curr->next.get(); return *this;}
        T const& operator*() {return m_curr->data;}
        bool operator!=(Iterator const& other) const {return m_curr != other.m_curr;}
        bool operator==(Iterator const& other) const {return m_curr == other.m_curr;}
    };

private:
    mutable BeConditionVariable m_cv;
    ItemPtr m_first;
    ItemPtr m_last;
    unsigned m_count;
    SchedulingMethod m_schedulingMethod;

private:
    bool PopBack(T* element);
    bool PopFront(T* element);
    bool Pop(T* element);
    
public:
    ThreadSafeQueue(SchedulingMethod defaultSchedulingMethod = SchedulingMethod::FIFO) : m_count(0), m_schedulingMethod(defaultSchedulingMethod) {}
    DGNPLATFORM_EXPORT void PushBack(T element);
    DGNPLATFORM_EXPORT void PushFront(T element);
    DGNPLATFORM_EXPORT bool Pop(T& element);
    DGNPLATFORM_EXPORT bool PopBack(T& element);
    DGNPLATFORM_EXPORT bool PopFront(T& element);
    DGNPLATFORM_EXPORT bool Pop();
    DGNPLATFORM_EXPORT bool PopBack();
    DGNPLATFORM_EXPORT bool PopFront();
    DGNPLATFORM_EXPORT bool IsEmpty() const;
    DGNPLATFORM_EXPORT unsigned Size() const;
    BeConditionVariable& GetConditionVariable() const {return m_cv;}
    DGNPLATFORM_EXPORT Iterator begin() const;
    DGNPLATFORM_EXPORT Iterator end() const;
    DGNPLATFORM_EXPORT void Erase(Iterator const& iterator);
    DGNPLATFORM_EXPORT void Clear();
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataThread : RefCountedBase
{
private:
    intptr_t    m_threadId;
    Utf8String  m_threadName;
    
private:
    static void ThreadRunner(void* arg);
#if defined(__unix__)
    static void* PlatformThreadRunner(void* arg) {ThreadRunner(arg); return NULL;}
#else
    static unsigned __stdcall PlatformThreadRunner(void* arg) {ThreadRunner(arg); return 0;}
#endif
    
protected:
    DGNPLATFORM_EXPORT RealityDataThread(Utf8CP threadName = NULL);
    virtual void _Run() = 0;

public:
    DGNPLATFORM_EXPORT intptr_t GetThreadId() const;
    DGNPLATFORM_EXPORT void     Run();   //!< Call this to invoke the _Run method in the current thread
    DGNPLATFORM_EXPORT void     Start(); //!< Call this to start the thread
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataWork : IRefCounted
{
friend struct RealityDataWorkerThread;
protected:
    virtual void _DoWork() = 0;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataWorkerThread : RealityDataThread
{
    friend struct RealityDataThreadPool;
    friend struct ::HasWorkOrTerminatesPredicate;
    friend struct ::IsIdlePredicate;
        
    struct IStateListener : IRefCounted
    {
    friend struct RealityDataWorkerThread;
    protected:
        virtual void _OnThreadBusy(RealityDataWorkerThread& thread) {};
        virtual void _OnThreadIdle(RealityDataWorkerThread& thread) {};
    };
    typedef RefCountedPtr<IStateListener> IStateListenerPtr;

private:
    mutable BeConditionVariable m_cv;
    bool                        m_terminate;
    RealityDataWorkPtr          m_currentWork;
    uint64_t                    m_idleSince;
    uint64_t                    m_busySince;
    IStateListenerPtr           m_stateListener;

    void SetIsBusy(bool);

protected:
    DGNPLATFORM_EXPORT RealityDataWorkerThread(IStateListener* stateListener, Utf8CP threadName);
    DGNPLATFORM_EXPORT virtual void _OnBusy();
    DGNPLATFORM_EXPORT virtual void _OnIdle();
    DGNPLATFORM_EXPORT virtual void _DoWork(RealityDataWork& work);
                       virtual void _Run() override;

protected:
    DGNPLATFORM_EXPORT bool TerminateRequested() const;

public:
                       RealityDataWork& DoWork(RealityDataWork& work) {_DoWork(work); return work;}
    DGNPLATFORM_EXPORT bool IsBusy(uint64_t* busyTime = NULL) const;
    DGNPLATFORM_EXPORT bool IsIdle(uint64_t* idleTime = NULL) const;
    DGNPLATFORM_EXPORT bool Terminate();
    //! Create a new RealityDataWorkerThread thread.
    //! note This function does not start the new thrdad. You must call the Start method on the returned thread object in order to start it.
    static RealityDataWorkerThreadPtr Create(IStateListener* stateListener = NULL, Utf8CP threadName = NULL) {return new RealityDataWorkerThread(stateListener, threadName);}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RealityDataThreadPool : RefCounted<RealityDataWorkerThread::IStateListener>
{       
    friend struct ::AllThreadsIdlePredicate;
private:
    int                         m_maxIdleThreads;
    int                         m_maxThreads;
    bool                        m_isTerminating;
    mutable BeConditionVariable m_threadsCV;
    mutable BeMutex             m_workQueueCS;
    bmap<RealityDataWorkerThread*, bool> m_threads;
    ThreadSafeQueue<RealityDataWorkPtr> m_workQueue;
    
protected:
    RealityDataThreadPool(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod) 
        : m_workQueue(schedulingMethod), m_maxThreads(maxThreads), m_maxIdleThreads(maxIdleThreads), m_isTerminating(false)
        {}
    ThreadSafeQueue<RealityDataWorkPtr>& GetQueue() {return m_workQueue;}
    DGNPLATFORM_EXPORT virtual void _OnThreadBusy(RealityDataWorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual void _OnThreadIdle(RealityDataWorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual bool _AssignWork(RealityDataWorkerThread& thread);
    RealityDataWorkerThreadPtr CreateThread();
    RealityDataWorkerThreadPtr GetIdleThread() const;
    bool ShouldCreateNewThread() const;
    bool IsTerminating() const {return m_isTerminating;}

public:
    //! NOT PUBLISHED: needed for tests
    DGNPLATFORM_EXPORT void WaitUntilAllThreadsIdle() const;

public:
    static RealityDataThreadPoolPtr Create(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod) {return new RealityDataThreadPool(maxThreads, maxIdleThreads, schedulingMethod);}
    DGNPLATFORM_EXPORT virtual ~RealityDataThreadPool();
    DGNPLATFORM_EXPORT int  GetThreadsCount() const;
    DGNPLATFORM_EXPORT void QueueWork(RealityDataWork& work);
    DGNPLATFORM_EXPORT void Terminate();
};

//======================================================================================
//! The response of reality data requests.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
template<typename ResultType> struct RealityDataResponse : RefCountedBase
{
private:
    ResultType   m_result;
    Utf8String   m_id;
    RealityData& m_data;
protected:
    RealityDataResponse(ResultType result, Utf8CP id, RealityData& data) : m_result(result), m_id(id), m_data(data) {}
public:
    ResultType GetResult() const {return m_result;}
    Utf8CP GetId() const {return m_id.c_str();}
    RealityData& GetData() const {return m_data;}
};

//=======================================================================================
//! The result of reality data storage operations.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class RealityDataStorageResult
    {
    Error,
    NotFound,
    Success,
    Queued,
    Pending,
    };

//=======================================================================================
//! Interface for a persist operation.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct IRealityDataStoragePersistHandler : RefCountedBase
    {
    friend struct RealityDataCache;
    protected:
        //! Called to persist the reality data into reality data storage.
        virtual RealityDataStorageResult _Persist() const = 0;

        //! Return the data object.
        virtual RealityData const* _GetData() const = 0;
    };

//======================================================================================
//! The response of reality data storage.
// @bsiclass                                        Grigas.Petraitis           04/2015
//======================================================================================
struct RealityDataStorageResponse : RealityDataResponse<RealityDataStorageResult>
{
    RealityDataStorageResponse(RealityDataStorageResult result, Utf8CP id, RealityData& data) : RealityDataResponse(result, id, data) {}
};

//=======================================================================================
//! The result of reality data source operations.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class RealityDataSourceResult
    {
    Error_Unknown,
    Error_CouldNotResolveHost,
    Error_NoConnection,
    Error_GatewayTimeout,
    Error_NotFound,
    Error_AccessDenied,
    
    Cancelled,
    Ignored,
    Pending,
    NotModified,
    Success,
    Queued,
    };

//=======================================================================================
//! Interface for a request operation.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct IRealityDataSourceRequestHandler : RefCountedBase
{
    friend struct RealityDataCache;
protected:
    //! Called to request reality data from reality data source.
    virtual RealityDataSourceResult _Request() const = 0;

    //! Is the request already handled.
    virtual bool _IsHandled() const = 0;

    //! Return the data object.
    virtual RealityData const* _GetData() const = 0;
};

//=======================================================================================
//! The base class for all reality data sources.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RealityDataSource : RefCountedBase, NonCopyableClass
{   
    friend struct RealityDataCache;

protected:
    virtual ~RealityDataSource() {}

    //! The unique id of this reality data source. 
    //! @warning This id must match the id returned by the static SourceId() function.
    virtual Utf8String _GetSourceId() const = 0;

    //! The abstract function that might be called to request data. The imeplementations should simply
    //! forward requests to their more specialized `Request` functions.
    //! @note It is guaranteed that the arguments can be cast to their derived classes, provided by the implementation.
    virtual RealityDataSourceResult _Request(RealityData&, bool& handled, Utf8CP, RealityDataOptions, RealityDataCache&) = 0;

    virtual IRealityDataSourceRequestHandler* _CreateRequestHandler(RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) = 0;
};

//======================================================================================
//! The response of reality data source.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct RealityDataSourceResponse : RealityDataResponse<RealityDataSourceResult>
{
    RealityDataSourceResponse(RealityDataSourceResult result, Utf8CP id, RealityData& data) : RealityDataResponse(result, id, data) {}
};

//=======================================================================================
//! The result of RealityDataCache::Get.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
enum class RealityDataCacheResult
    {
    Error,
    NotFound,
    Success,
    RequestQueued,
    };

//=======================================================================================
//! Reality data storage which uses a SQLite database for storing cached data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RealityDataStorage  : RefCountedBase, NonCopyableClass
{
    //===================================================================================
    //! @ref IRealityDataStoragePersistHandler implementation which knows how to persist
    //! reality data in the RealityDataStorage.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct PersistHandler : IRealityDataStoragePersistHandler
    {
    private:
        RealityDataStorage& m_storage;
        RefCountedPtr<RealityData const>  m_data;
    protected:
        virtual RealityDataStorageResult _Persist() const override {return m_storage.Persist(*m_data);}
        virtual RealityData const* _GetData() const override {return m_data.get();}
    public:
        PersistHandler(RealityDataStorage& storage, RealityData const& data) : m_storage(storage), m_data(&data) {}        
    };

    RefCountedPtr<PersistHandler> CreatePersistHandler(RealityData const& data) {return new PersistHandler(*this, data);}

    struct CleanAndSaveChangesWork;
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct BeSQLiteStorageThreadPool : RealityDataThreadPool
    {
        friend struct CleanAndSaveChangesWork;
        friend struct ::ThreadPoolQueueNotEmptyPredicate;
    private:
        RealityDataStorage& m_storage;
    protected:
        ThreadSafeQueue<RealityDataWorkPtr>& GetQueue() {return RealityDataThreadPool::GetQueue();}
        virtual bool _AssignWork(RealityDataWorkerThread& thread) override;
    public:
        BeSQLiteStorageThreadPool(RealityDataStorage& storage, int numThreads, SchedulingMethod schedulingMethod) 
            : RealityDataThreadPool(numThreads, numThreads, schedulingMethod), m_storage(storage) 
            {}
        void QueueIdleWork(RealityDataWork& work);
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct StorageWork : RefCounted<RealityDataWork>
    {
    protected:
        RealityDataStoragePtr m_storage;
        StorageWork(RealityDataStorage& storage) : m_storage(&storage) {}
    };

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct SelectDataWork : StorageWork
    {
    private:
        Utf8String m_id;
        RefCountedPtr<RealityData> m_data;
        RealityDataOptions m_options;
        RealityDataCache& m_responseReceiver;
        bool m_hasResult;
        mutable RealityDataStorageResult m_result;
        mutable BeConditionVariable m_resultCV;
    protected:
        virtual void _DoWork() override;
    public:
        SelectDataWork(RealityDataStorage& storage, Utf8CP id, RealityData& data, RealityDataOptions options, RealityDataCache& responseReceiver) 
            : StorageWork(storage), m_id(id), m_data(&data), m_options(options), m_hasResult(false), m_responseReceiver(responseReceiver)
            {} 
        RealityDataStorageResult GetResult() const;
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct PersistDataWork : StorageWork
    {
    private:
        RefCountedPtr<RealityData const>  m_data;
    protected:
        virtual void _DoWork() override;
    public:
        PersistDataWork(RealityDataStorage& storage, RealityData const& data) : StorageWork(storage), m_data(&data) {} 
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct CleanAndSaveChangesWork : StorageWork
    {
    private:
        uint32_t m_idleTime;
    protected:
        virtual void _DoWork() override;
    public:
        CleanAndSaveChangesWork(RealityDataStorage& storage, uint32_t idleTime) : StorageWork(storage), m_idleTime(idleTime) {}
    };
    
private:
    RefCountedPtr<BeSQLiteStorageThreadPool> m_threadPool;
    BeAtomic<bool> m_hasChanges;
    BeMutex m_saveChangesMux;
    BeSQLite::Db m_database;
    bset<Utf8String> m_activeRequests;
    BeMutex m_activeRequestsMux;
    uint32_t m_idleTime;
    RefCountedPtr<BeSQLite::BusyRetry> m_retry;

private:
    void wt_Cleanup();
    void wt_Persist(RealityData const& data);
    RealityDataStorageResult wt_Select(RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver);
    void wt_SaveChanges();

public:
    //! NOT PUBLISHED: needed for tests
    RefCountedPtr<BeSQLiteStorageThreadPool> GetThreadPool() const {return m_threadPool;}

protected:
    //! Called to prepare the database for storing specific type of data (the type 
    //! must be known to the implementation).
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const = 0;

    //! Called to free some space in the database.
    //! @param[in] db           The database to cleanup.
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const = 0;

public:
    //! Creates a new RealityDataStorage.
    //! @param[in] numThreads   The number of worker threads used by this storage.
    //! @param[in] schedulingMethod The order of handled requests.
    //! @param[in] idleTime     Time (in miliseconds) for the worker thread to wait before commiting changes to the database.
    DGNPLATFORM_EXPORT RealityDataStorage(int numThreads, SchedulingMethod schedulingMethod = SchedulingMethod::FIFO, uint32_t idleTime=1000);
    ~RealityDataStorage() {m_retry = nullptr; Terminate();}

    //! Initialize data object with the data in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Select(RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver);

    //! Persist the data object in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Persist(RealityData const& data);

    DGNPLATFORM_EXPORT BentleyStatus OpenAndPrepare(BeFileNameCR cacheName);

    DGNPLATFORM_EXPORT void Terminate();
};

//=======================================================================================
//! The reality data cache class which manages two objects - a source and optionally a storage.
//! Storages are responsible for caching the reality data and returning it quickly on demand.
//! Sources are responsible for retrieving the reality data from its primary source, e.g.
//! the internet.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RealityDataCache : RefCountedBase, NonCopyableClass
{    
    template<typename T> struct RefCountedPtrComparer
        {
        bool operator()(RefCountedPtr<T> const& lhs, RefCountedPtr<T> const& rhs) const {return lhs.get() < rhs.get();}
        };
    
private:
    RealityDataStoragePtr  m_storage;
    RealityDataSourcePtr  m_source;
    bmap<Utf8String, bset<RefCountedPtr<IRealityDataStoragePersistHandler>, RefCountedPtrComparer<IRealityDataStoragePersistHandler>>> m_persistHandlers;
    BeMutex m_persistHandlersCS;
    bmap<Utf8String, bset<RefCountedPtr<IRealityDataSourceRequestHandler>, RefCountedPtrComparer<IRealityDataSourceRequestHandler>>> m_requestHandlers;
    BeMutex m_requestHandlersCS;
    mutable bmap<void const*, RealityDataCacheResult> m_results;
    mutable BeMutex m_resultsCS;

    RealityDataCacheResult HandleStorageResponse(RealityDataStorageResponse const& response, RealityDataOptions);
    RealityDataCacheResult ResolveResult(RealityDataStorageResult storageResult, RealityDataSourceResult sourceResult = RealityDataSourceResult::Error_NotFound) const;
    RefCountedPtr<IRealityDataSourceRequestHandler> DequeueRequestHandler(Utf8CP id, RealityData const&);
    RefCountedPtr<IRealityDataStoragePersistHandler> DequeuePersistHandler(Utf8CP id, RealityData const&);

private:
    DGNPLATFORM_EXPORT void QueuePersistHandler(Utf8CP id, IRealityDataStoragePersistHandler&);
    DGNPLATFORM_EXPORT void QueueRequestHandler(Utf8CP id, IRealityDataSourceRequestHandler&);

    DGNPLATFORM_EXPORT RealityDataCacheResult GetResult(RealityData& data, Utf8CP id, RealityDataStorageResult storageResult);
    DGNPLATFORM_EXPORT RealityDataCacheResult GetResult(Utf8CP id, RealityDataSourceResult sourceResult);
    
public:
    //! Create a new reality data cache.
    RealityDataCache() {}

    //! Cleans up local caches
    DGNPLATFORM_EXPORT void Cleanup();

    // note: these might be called from any thread!
    DGNPLATFORM_EXPORT virtual void _OnResponseReceived(RealityDataSourceResponse const& response, RealityDataOptions);
    DGNPLATFORM_EXPORT virtual void _OnResponseReceived(RealityDataStorageResponse const& response, RealityDataOptions, bool isAsync);

    //! Set storage in this cache.
    void SetStorage(RealityDataStorage& storage) {BeAssert(!m_storage.IsValid()); m_storage = &storage; }

    //! Register a source in this cache.
    void SetSource(RealityDataSource& source) {BeAssert(!m_source.IsValid()); m_source = &source;}

    //! Request reality data from this cache. The type of the reality data is provided as a template parameter.
    //! @param[out] data       An reference object to receive the requested data.
    //! @param[in] id       The id of the requested data.
    //! @param[in] options  The request options.
    RealityDataCacheResult RequestData(RealityData& data, Utf8CP id, RealityDataOptions options)
        {
        RefCountedPtr<RealityData> temp(&data);

        if (!m_storage.IsValid())
            {
            if (!options.m_requestFromSource)
                return RealityDataCacheResult::NotFound;

            bool handled;
            return GetResult(id, m_source->_Request(data, handled, id, options, *this));
            }
        
        QueueRequestHandler(id, *m_source->_CreateRequestHandler(data, id, options, *this));
        QueuePersistHandler(id, *m_storage->CreatePersistHandler(data));
        return GetResult(data, id, m_storage->Select(data, id, options, *this));
        }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            05/2015
//=======================================================================================
struct IAsyncRequestCancellationToken
{
    friend struct AsyncRealityDataSourceRequest;
protected:
    virtual ~IAsyncRequestCancellationToken() {}
    virtual bool _ShouldCancel() const = 0;
};

//=======================================================================================
//! A request interface for @ref AsyncRealityDataSource.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct AsyncRealityDataSourceRequest : RefCountedBase
{
    struct SynchronousRequestPredicate;

protected:
    IAsyncRequestCancellationToken* m_cancellationToken;
    RealityDataOptions m_options;

    mutable RefCountedPtr<RealityDataSourceResponse> m_response; // set only for sync requests

    AsyncRealityDataSourceRequest(IAsyncRequestCancellationToken* cancellationToken, RealityDataOptions options) : m_cancellationToken(cancellationToken), m_options(options) {}

    //! Returns true if the request was cancelled.
    DGNPLATFORM_EXPORT bool ShouldCancelRequest() const;

    //! Return the if of reality data which is handled by this request.
    virtual Utf8CP _GetId() const = 0;

    //! Handle the request (e.g. read file content and initialize reality data from it).
    virtual RefCountedPtr<RealityDataSourceResponse> _Handle() const = 0;

public:
    Utf8CP GetId() const {return _GetId();}
    RefCountedPtr<RealityDataSourceResponse> Handle(BeMutex& cs) const;
    void SetCancellationToken(IAsyncRequestCancellationToken& token) {m_cancellationToken = &token;}
    RealityDataOptions GetRequestOptions() const {return m_options;}
};

//=======================================================================================
//! Base class for asynchronous reality data sources.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsyncRealityDataSource : RealityDataSource
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestHandler : RefCounted<RealityDataWork>, IAsyncRequestCancellationToken
    {
    private:
        RefCountedPtr<AsyncRealityDataSource> m_source;
        RefCountedPtr<AsyncRealityDataSourceRequest const> m_request;
        RealityDataCache& m_responseReceiver;
        
        RequestHandler(AsyncRealityDataSource& source, AsyncRealityDataSourceRequest const& request, RealityDataCache& responseReceiver)
            : m_source(&source), m_request(&request), m_responseReceiver(responseReceiver)
            {}
        void SendResponse(RealityDataSourceResponse const& response, RealityDataOptions options) {m_responseReceiver._OnResponseReceived(response, options);}

    protected:
        virtual void _DoWork() override;
        virtual bool _ShouldCancel() const override;

    public:
        static RefCountedPtr<RequestHandler> Create(AsyncRealityDataSource& source, AsyncRealityDataSourceRequest const& request, RealityDataCache& responseReceiver) 
            {
            return new RequestHandler(source, request, responseReceiver); 
            }
    };

private:
    RealityDataThreadPoolPtr m_threadPool;
    BeMutex                  m_requestsCS;
    bset<Utf8String>         m_activeRequests;
    bset<Utf8String>         m_notFoundRequests;
    BeAtomic<uint64_t>       m_ignoreRequestsUntil;
    BeConditionVariable      m_synchronizationCV;
    bool                     m_terminateRequested;

private:
    void SetIgnoreRequests(uint32_t ignoreTime);
    bool ShouldIgnoreRequests() const;

protected:
    virtual ~AsyncRealityDataSource() 
        {
        m_terminateRequested = true;
        m_threadPool->Terminate();
        m_threadPool->WaitUntilAllThreadsIdle();
        }
    AsyncRealityDataSource(int numThreads, SchedulingMethod schedulingMethod) 
        : m_threadPool(RealityDataThreadPool::Create(numThreads, numThreads, schedulingMethod)), m_ignoreRequestsUntil(0), m_terminateRequested(false)
        {}

protected:
    //! Queues a request for handling on the work thread. Derived classes should use this
    //! function to queue their requests to be handled asynchronously.
    RealityDataSourceResult QueueRequest(AsyncRealityDataSourceRequest& request, bool& handled, RealityDataCache& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses files (on the disc) as the source of reality data.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FileRealityDataSource : AsyncRealityDataSource
{
    //===================================================================================
    //! @ref IRealityDataSourceRequestHandler implementation which knows how to request
    //! reality data from the @ref FileRealityDataSource.
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct RequestHandler : IRealityDataSourceRequestHandler
    {
    private:
        mutable bool m_handled;
        FileRealityDataSource&    m_source;
        RefCountedPtr<RealityData> m_data;
        Utf8String m_id;
        RealityDataOptions m_options;
        RealityDataCache& m_responseReceiver;
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual RealityDataSourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, m_responseReceiver);}
        virtual RealityData const* _GetData() const override {return m_data.get();}
    public:
        RequestHandler(FileRealityDataSource& source, RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(responseReceiver), m_handled(false)
            {}
    };

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}

    DGNPLATFORM_EXPORT RealityDataSourceResult Request(RealityData& data, bool& handled, Utf8CP filepath, RealityDataOptions options, RealityDataCache& responseReceiver);

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "File";}

    //! Create a new instance of FileRealityDataSource.
    //! @param[in] numThreads   Number of worker threads to use for reading files. Must be at least 1.
    //! @param[in] schedulingMethod The order of handled requests.
    FileRealityDataSource(int numThreads, SchedulingMethod schedulingMethod) : AsyncRealityDataSource(numThreads, schedulingMethod) {}

    virtual RealityDataSourceResult _Request(RealityData& data, bool& handled, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) override
        {
        return Request(data, handled, id, options, responseReceiver);
        }

    IRealityDataSourceRequestHandler* _CreateRequestHandler(RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) override
        {
        return new RequestHandler(*this, data, id, options, responseReceiver);
        }
};

//=======================================================================================
//! Reality data source which uses HTTP protocol to download content and use it as 
//! the source of reality data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HttpRealityDataSource : AsyncRealityDataSource
{
    //===================================================================================
    //! @ref IRealityDataSourceRequestHandler implementation which knows how to request
    //! reality data from the @ref HttpRealityDataSource.
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct RequestHandler : IRealityDataSourceRequestHandler
    {
    private:
        mutable bool m_handled;
        HttpRealityDataSource&    m_source;
        RefCountedPtr<RealityData> m_data;
        Utf8String m_id;
        RealityDataOptions m_options;
        RealityDataCache& m_responseReceiver;
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual RealityDataSourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, m_responseReceiver);}
        virtual RealityData const* _GetData() const override {return m_data.get();}

public:
        RequestHandler(HttpRealityDataSource& source, RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(responseReceiver), m_handled(false)
            {}  
    };

private:

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    virtual RealityDataSourceResult _Request(RealityData& data, bool& handled, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) override
        {
        return Request(data, handled, id, options, responseReceiver);
        }

    IRealityDataSourceRequestHandler* _CreateRequestHandler(RealityData& data, Utf8CP id, RealityDataOptions options, RealityDataCache& responseReceiver) override
        {
        return new RequestHandler(*this, data, id, options, responseReceiver);
        }

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "Http";}
    
    HttpRealityDataSource(int numThreads, SchedulingMethod schedulingMethod) : AsyncRealityDataSource(numThreads, schedulingMethod) {}
    
    //! Request the data to be initialized from the content at the specified url
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(RealityData& data, bool& handled, Utf8CP url, RealityDataOptions options, RealityDataCache& responseReceiver);
};

END_BENTLEY_DGN_NAMESPACE
