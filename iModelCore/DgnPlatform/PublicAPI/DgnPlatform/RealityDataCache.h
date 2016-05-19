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

#define BEGIN_BENTLEY_REALITYDATA_NAMESPACE  BEGIN_BENTLEY_DGN_NAMESPACE namespace RealityData {
#define END_BENTLEY_REALITYDATA_NAMESPACE    } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_BENTLEY_REALITYDATA using namespace BentleyApi::Dgn::RealityData;

BEGIN_BENTLEY_REALITYDATA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Cache)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Thread)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreadPool)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Work)
DEFINE_POINTER_SUFFIX_TYPEDEFS(WorkerThread)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Cache)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Storage)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Source)
DEFINE_POINTER_SUFFIX_TYPEDEFS(HttpSource)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileSource)
DEFINE_POINTER_SUFFIX_TYPEDEFS(SourceRequestHandler)

DEFINE_REF_COUNTED_PTR(Thread)
DEFINE_REF_COUNTED_PTR(ThreadPool)
DEFINE_REF_COUNTED_PTR(Work)
DEFINE_REF_COUNTED_PTR(WorkerThread)
DEFINE_REF_COUNTED_PTR(Cache)
DEFINE_REF_COUNTED_PTR(Storage)
DEFINE_REF_COUNTED_PTR(Source)

DEFINE_REF_COUNTED_PTR(HttpSource)
DEFINE_REF_COUNTED_PTR(FileSource)
DEFINE_REF_COUNTED_PTR(SourceRequestHandler)

//=======================================================================================
//! Options for request function.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct Options
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
struct Payload : RefCountedBase
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

    Payload() {}
    virtual ~Payload() {}
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
struct Thread : RefCountedBase
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
    DGNPLATFORM_EXPORT Thread(Utf8CP threadName = NULL);
    virtual void _Run() = 0;

public:
    DGNPLATFORM_EXPORT intptr_t GetThreadId() const;
    DGNPLATFORM_EXPORT void Run();   //!< Call this to invoke the _Run method in the current thread
    DGNPLATFORM_EXPORT void Start(); //!< Call this to start the thread
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct Work : IRefCounted
{
friend struct WorkerThread;
protected:
    virtual void _DoWork() = 0;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct WorkerThread : Thread
{
    friend struct ThreadPool;
    friend struct HasWorkOrTerminatesPredicate;
    friend struct IsIdlePredicate;
        
    struct IStateListener : IRefCounted
    {
    friend struct WorkerThread;
    protected:
        virtual void _OnThreadBusy(WorkerThread& thread) {};
        virtual void _OnThreadIdle(WorkerThread& thread) {};
    };
    typedef RefCountedPtr<IStateListener> IStateListenerPtr;

private:
    mutable BeConditionVariable m_cv;
    bool       m_terminate;
    WorkPtr    m_currentWork;
    uint64_t   m_idleSince;
    uint64_t   m_busySince;
    IStateListenerPtr   m_stateListener;

    void SetIsBusy(bool);

protected:
    DGNPLATFORM_EXPORT WorkerThread(IStateListener* stateListener, Utf8CP threadName);
    DGNPLATFORM_EXPORT virtual void _OnBusy();
    DGNPLATFORM_EXPORT virtual void _OnIdle();
    DGNPLATFORM_EXPORT virtual void _DoWork(Work& work);
                       virtual void _Run() override;

protected:

public:
    Work& DoWork(Work& work) {_DoWork(work); return work;}
    DGNPLATFORM_EXPORT bool IsBusy(uint64_t* busyTime = NULL) const;
    DGNPLATFORM_EXPORT bool IsIdle(uint64_t* idleTime = NULL) const;
    DGNPLATFORM_EXPORT bool Terminate();
    DGNPLATFORM_EXPORT bool TerminateRequested() const;
    //! Create a new WorkerThread thread.
    //! note This function does not start the new thrdad. You must call the Start method on the returned thread object in order to start it.
    static WorkerThreadPtr Create(IStateListener* stateListener = NULL, Utf8CP threadName = NULL) {return new WorkerThread(stateListener, threadName);}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreadPool : RefCounted<WorkerThread::IStateListener>
{       
    friend struct AllThreadsIdlePredicate;

private:
    int                         m_maxIdleThreads;
    int                         m_maxThreads;
    bool                        m_isTerminating;
    mutable BeConditionVariable m_threadsCV;
    mutable BeMutex             m_workQueueCS;
    bmap<WorkerThread*, bool> m_threads;
    ThreadSafeQueue<WorkPtr> m_workQueue;
    
protected:
    ThreadPool(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod) 
        : m_workQueue(schedulingMethod), m_maxThreads(maxThreads), m_maxIdleThreads(maxIdleThreads), m_isTerminating(false)
        {}
    DGNPLATFORM_EXPORT virtual void _OnThreadBusy(WorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual void _OnThreadIdle(WorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual bool _AssignWork(WorkerThread& thread);
    WorkerThreadPtr CreateThread();
    WorkerThreadPtr GetIdleThread() const;
    bool ShouldCreateNewThread() const;

public:
    //! NOT PUBLISHED: needed for tests
    DGNPLATFORM_EXPORT void WaitUntilAllThreadsIdle() const;

public:
    ThreadSafeQueue<WorkPtr>& GetQueue() {return m_workQueue;}
    bool IsTerminating() const {return m_isTerminating;}

    static ThreadPoolPtr Create(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod) {return new ThreadPool(maxThreads, maxIdleThreads, schedulingMethod);}

    DGNPLATFORM_EXPORT virtual ~ThreadPool();
    DGNPLATFORM_EXPORT int  GetThreadsCount() const;
    DGNPLATFORM_EXPORT void QueueWork(Work& work);
    DGNPLATFORM_EXPORT void Terminate();
};

//======================================================================================
//! The response of reality data requests.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
template<typename ResultType> struct Response : RefCountedBase
{
private:
    ResultType   m_result;
    Utf8String   m_id;
    Payload& m_data;
protected:
    Response(ResultType result, Utf8CP id, Payload& data) : m_result(result), m_id(id), m_data(data) {}
public:
    ResultType GetResult() const {return m_result;}
    Utf8CP GetId() const {return m_id.c_str();}
    Payload& GetData() const {return m_data;}
};

//=======================================================================================
//! The result of reality data storage operations.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class StorageResult
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
struct StoragePersistHandler : RefCountedBase
    {
    friend struct Cache;
    protected:
        //! Called to persist the reality data into reality data storage.
        virtual StorageResult _Persist() const = 0;

        //! Return the data object.
        virtual Payload const* _GetData() const = 0;
    };

//======================================================================================
//! The response of reality data storage.
// @bsiclass                                        Grigas.Petraitis           04/2015
//======================================================================================
struct StorageResponse : Response<StorageResult>
{
    StorageResponse(StorageResult result, Utf8CP id, Payload& data) : Response(result, id, data) {}
};

//=======================================================================================
//! The result of reality data source operations.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class SourceResult
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
struct SourceRequestHandler : RefCountedBase
{
    friend struct Cache;
protected:
    //! Called to request reality data from reality data source.
    virtual SourceResult _Request() const = 0;

    //! Is the request already handled.
    virtual bool _IsHandled() const = 0;

    //! Return the data object.
    virtual Payload const* _GetData() const = 0;
};


//=======================================================================================
//! The base class for all reality data sources.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Source : RefCountedBase, NonCopyableClass
{   
    friend struct Cache;

protected:
    virtual ~Source() {}

    //! The unique id of this reality data source. 
    //! @warning This id must match the id returned by the static SourceId() function.
    virtual Utf8String _GetSourceId() const = 0;

    //! The abstract function that might be called to request data. The imeplementations should simply
    //! forward requests to their more specialized `Request` functions.
    //! @note It is guaranteed that the arguments can be cast to their derived classes, provided by the implementation.
    virtual SourceResult _Request(Payload&, bool& handled, Utf8CP, Options, Cache&) = 0;

    virtual SourceRequestHandlerPtr _CreateRequestHandler(Payload& data, Utf8CP id, Options options, Cache& responseReceiver) = 0;
};

//======================================================================================
//! The response of reality data source.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct SourceResponse : Response<SourceResult>
{
    SourceResponse(SourceResult result, Utf8CP id, Payload& data) : Response(result, id, data) {}
};

//=======================================================================================
//! The result of Cache::Get.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
enum class CacheResult
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
struct EXPORT_VTABLE_ATTRIBUTE Storage  : RefCountedBase, NonCopyableClass
{
    //===================================================================================
    //! @ref StoragePersistHandler implementation which knows how to persist
    //! reality data in the Storage.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct PersistHandler : StoragePersistHandler
    {
    private:
        Storage& m_storage;
        RefCountedPtr<Payload const>  m_data;
    protected:
        virtual StorageResult _Persist() const override {return m_storage.Persist(*m_data);}
        virtual Payload const* _GetData() const override {return m_data.get();}
    public:
        PersistHandler(Storage& storage, Payload const& data) : m_storage(storage), m_data(&data) {}        
    };

    RefCountedPtr<PersistHandler> CreatePersistHandler(Payload const& data) {return new PersistHandler(*this, data);}

    struct CleanAndSaveChangesWork;
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct BeSQLiteStorageThreadPool : ThreadPool
    {
    private:
        Storage& m_storage;
    protected:
        virtual bool _AssignWork(WorkerThread& thread) override;
    public:
        BeSQLiteStorageThreadPool(Storage& storage, int numThreads, SchedulingMethod schedulingMethod) 
            : ThreadPool(numThreads, numThreads, schedulingMethod), m_storage(storage) 
            {}
        void QueueIdleWork(Work& work);
        ThreadSafeQueue<WorkPtr>& GetQueue() {return ThreadPool::GetQueue();}
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct StorageWork : RefCounted<Work>
    {
    protected:
        StoragePtr m_storage;
        StorageWork(Storage& storage) : m_storage(&storage) {}
    };

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct SelectDataWork : StorageWork
    {
    private:
        Utf8String m_id;
        RefCountedPtr<Payload> m_data;
        Options m_options;
        Cache& m_responseReceiver;
        bool m_hasResult;
        mutable StorageResult m_result;
        mutable BeConditionVariable m_resultCV;
    protected:
        virtual void _DoWork() override;
    public:
        SelectDataWork(Storage& storage, Utf8CP id, Payload& data, Options options, Cache& responseReceiver) 
            : StorageWork(storage), m_id(id), m_data(&data), m_options(options), m_hasResult(false), m_responseReceiver(responseReceiver)
            {} 
        StorageResult GetResult() const;
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct PersistDataWork : StorageWork
    {
    private:
        RefCountedPtr<Payload const>  m_data;
    protected:
        virtual void _DoWork() override;
    public:
        PersistDataWork(Storage& storage, Payload const& data) : StorageWork(storage), m_data(&data) {} 
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
        CleanAndSaveChangesWork(Storage& storage, uint32_t idleTime) : StorageWork(storage), m_idleTime(idleTime) {}
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
    void wt_Persist(Payload const& data);
    StorageResult wt_Select(Payload& data, Utf8CP id, Options options, Cache& responseReceiver);
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
    //! Creates a new Storage.
    //! @param[in] numThreads   The number of worker threads used by this storage.
    //! @param[in] schedulingMethod The order of handled requests.
    //! @param[in] idleTime     Time (in miliseconds) for the worker thread to wait before commiting changes to the database.
    DGNPLATFORM_EXPORT Storage(int numThreads, SchedulingMethod schedulingMethod = SchedulingMethod::FIFO, uint32_t idleTime=1000);
    ~Storage() {m_retry = nullptr; Terminate();}

    //! Initialize data object with the data in the database.
    DGNPLATFORM_EXPORT virtual StorageResult _Select(Payload& data, Utf8CP id, Options options, Cache& responseReceiver);

    //! Persist the data object in the database.
    DGNPLATFORM_EXPORT StorageResult Persist(Payload const& data);

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
struct EXPORT_VTABLE_ATTRIBUTE Cache : RefCountedBase, NonCopyableClass
{    
    template<typename T> struct RefCountedPtrComparer
        {
        bool operator()(RefCountedPtr<T> const& lhs, RefCountedPtr<T> const& rhs) const {return lhs.get() < rhs.get();}
        };
    
private:
    StoragePtr  m_storage;
    SourcePtr  m_source;
    bmap<Utf8String, bset<RefCountedPtr<StoragePersistHandler>, RefCountedPtrComparer<StoragePersistHandler>>> m_persistHandlers;
    BeMutex m_persistHandlersCS;
    bmap<Utf8String, bset<RefCountedPtr<SourceRequestHandler>, RefCountedPtrComparer<SourceRequestHandler>>> m_requestHandlers;
    BeMutex m_requestHandlersCS;
    mutable bmap<void const*, CacheResult> m_results;
    mutable BeMutex m_resultsCS;

    CacheResult HandleStorageResponse(StorageResponse const& response, Options);
    CacheResult ResolveResult(StorageResult storageResult, SourceResult sourceResult = SourceResult::Error_NotFound) const;
    RefCountedPtr<SourceRequestHandler> DequeueRequestHandler(Utf8CP id, Payload const&);
    RefCountedPtr<StoragePersistHandler> DequeuePersistHandler(Utf8CP id, Payload const&);

private:
    DGNPLATFORM_EXPORT void QueuePersistHandler(Utf8CP id, StoragePersistHandler&);
    DGNPLATFORM_EXPORT void QueueRequestHandler(Utf8CP id, SourceRequestHandler&);

    DGNPLATFORM_EXPORT CacheResult GetResult(Payload& data, Utf8CP id, StorageResult storageResult);
    DGNPLATFORM_EXPORT CacheResult GetResult(Utf8CP id, SourceResult sourceResult);
    
public:
    //! Create a new reality data cache.
    Cache() {}

    //! Cleans up local caches
    DGNPLATFORM_EXPORT void Cleanup();

    // note: these might be called from any thread!
    DGNPLATFORM_EXPORT virtual void _OnResponseReceived(SourceResponse const& response, Options);
    DGNPLATFORM_EXPORT virtual void _OnResponseReceived(StorageResponse const& response, Options, bool isAsync);

    //! Set storage in this cache.
    void SetStorage(Storage& storage) {BeAssert(!m_storage.IsValid()); m_storage = &storage; }

    //! Register a source in this cache.
    void SetSource(Source& source) {BeAssert(!m_source.IsValid()); m_source = &source;}

    //! Request reality data from this cache. The type of the reality data is provided as a template parameter.
    //! @param[out] data       An reference object to receive the requested data.
    //! @param[in] id       The id of the requested data.
    //! @param[in] options  The request options.
    CacheResult RequestData(Payload& data, Utf8CP id, Options options)
        {
        RefCountedPtr<Payload> temp(&data);

        if (!m_storage.IsValid())
            {
            if (!options.m_requestFromSource)
                return CacheResult::NotFound;

            bool handled;
            return GetResult(id, m_source->_Request(data, handled, id, options, *this));
            }
        
        QueueRequestHandler(id, *m_source->_CreateRequestHandler(data, id, options, *this));
        QueuePersistHandler(id, *m_storage->CreatePersistHandler(data));
        return GetResult(data, id, m_storage->_Select(data, id, options, *this));
        }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            05/2015
//=======================================================================================
struct IAsyncRequestCancellationToken
{
    friend struct AsyncSourceRequest;
protected:
    virtual ~IAsyncRequestCancellationToken() {}
    virtual bool _ShouldCancel() const = 0;
};

//=======================================================================================
//! A request interface for @ref AsyncSource.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct AsyncSourceRequest : RefCountedBase
{
    struct SynchronousRequestPredicate;

protected:
    IAsyncRequestCancellationToken* m_cancellationToken;
    Options m_options;

    mutable RefCountedPtr<SourceResponse> m_response; // set only for sync requests

    AsyncSourceRequest(IAsyncRequestCancellationToken* cancellationToken, Options options) : m_cancellationToken(cancellationToken), m_options(options) {}

    //! Returns true if the request was cancelled.
    DGNPLATFORM_EXPORT bool ShouldCancelRequest() const;

    //! Return the if of reality data which is handled by this request.
    virtual Utf8CP _GetId() const = 0;

    //! Handle the request (e.g. read file content and initialize reality data from it).
    virtual RefCountedPtr<SourceResponse> _Handle() const = 0;

public:
    Utf8CP GetId() const {return _GetId();}
    RefCountedPtr<SourceResponse> Handle(BeMutex& cs) const;
    void SetCancellationToken(IAsyncRequestCancellationToken& token) {m_cancellationToken = &token;}
    Options GetRequestOptions() const {return m_options;}
};

//=======================================================================================
//! Base class for asynchronous reality data sources.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsyncSource : Source
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestHandler : RefCounted<Work>, IAsyncRequestCancellationToken
    {
    private:
        RefCountedPtr<AsyncSource> m_source;
        RefCountedPtr<AsyncSourceRequest const> m_request;
        Cache& m_responseReceiver;
        
        RequestHandler(AsyncSource& source, AsyncSourceRequest const& request, Cache& responseReceiver)
            : m_source(&source), m_request(&request), m_responseReceiver(responseReceiver)
            {}
        void SendResponse(SourceResponse const& response, Options options) {m_responseReceiver._OnResponseReceived(response, options);}

    protected:
        virtual void _DoWork() override;
        virtual bool _ShouldCancel() const override;

    public:
        static RefCountedPtr<RequestHandler> Create(AsyncSource& source, AsyncSourceRequest const& request, Cache& responseReceiver) 
            {
            return new RequestHandler(source, request, responseReceiver); 
            }
    };

private:
    ThreadPoolPtr m_threadPool;
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
    virtual ~AsyncSource() 
        {
        m_terminateRequested = true;
        m_threadPool->Terminate();
        m_threadPool->WaitUntilAllThreadsIdle();
        }
    AsyncSource(int numThreads, SchedulingMethod schedulingMethod) 
        : m_threadPool(ThreadPool::Create(numThreads, numThreads, schedulingMethod)), m_ignoreRequestsUntil(0), m_terminateRequested(false)
        {}

protected:
    //! Queues a request for handling on the work thread. Derived classes should use this
    //! function to queue their requests to be handled asynchronously.
    SourceResult QueueRequest(AsyncSourceRequest& request, bool& handled, Cache& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses files (on the disc) as the source of reality data.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FileSource : AsyncSource
{
    //===================================================================================
    //! @ref SourceRequestHandler implementation which knows how to request
    //! reality data from the @ref FileSource.
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct RequestHandler : SourceRequestHandler
    {
    private:
        mutable bool m_handled;
        FileSource&    m_source;
        RefCountedPtr<Payload> m_data;
        Utf8String m_id;
        Options m_options;
        Cache& m_responseReceiver;
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual SourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, m_responseReceiver);}
        virtual Payload const* _GetData() const override {return m_data.get();}
    public:
        RequestHandler(FileSource& source, Payload& data, Utf8CP id, Options options, Cache& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(responseReceiver), m_handled(false)
            {}
    };

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}

    DGNPLATFORM_EXPORT SourceResult Request(Payload& data, bool& handled, Utf8CP filepath, Options options, Cache& responseReceiver);

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "File";}

    //! Create a new instance of FileSource.
    //! @param[in] numThreads   Number of worker threads to use for reading files. Must be at least 1.
    //! @param[in] schedulingMethod The order of handled requests.
    FileSource(int numThreads, SchedulingMethod schedulingMethod) : AsyncSource(numThreads, schedulingMethod) {}

    virtual SourceResult _Request(Payload& data, bool& handled, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return Request(data, handled, id, options, responseReceiver);
        }

    SourceRequestHandlerPtr _CreateRequestHandler(Payload& data, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return new RequestHandler(*this, data, id, options, responseReceiver);
        }
};

//=======================================================================================
//! Reality data source which uses HTTP protocol to download content and use it as 
//! the source of reality data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HttpSource : AsyncSource
{
    //===================================================================================
    //! @ref SourceRequestHandler implementation which knows how to request
    //! reality data from the @ref HttpSource.
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct RequestHandler : SourceRequestHandler
    {
    private:
        mutable bool m_handled;
        HttpSource&    m_source;
        RefCountedPtr<Payload> m_data;
        Utf8String m_id;
        Options m_options;
        Cache& m_responseReceiver;
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual SourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, m_responseReceiver);}
        virtual Payload const* _GetData() const override {return m_data.get();}

public:
        RequestHandler(HttpSource& source, Payload& data, Utf8CP id, Options options, Cache& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(responseReceiver), m_handled(false)
            {}  
    };

private:

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    virtual SourceResult _Request(Payload& data, bool& handled, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return Request(data, handled, id, options, responseReceiver);
        }

    SourceRequestHandlerPtr _CreateRequestHandler(Payload& data, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return new RequestHandler(*this, data, id, options, responseReceiver);
        }

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "Http";}
    
    HttpSource(int numThreads, SchedulingMethod schedulingMethod) : AsyncSource(numThreads, schedulingMethod) {}
    
    //! Request the data to be initialized from the content at the specified url
    DGNPLATFORM_EXPORT SourceResult Request(Payload& data, bool& handled, Utf8CP url, Options options, Cache& responseReceiver);
};

END_BENTLEY_REALITYDATA_NAMESPACE
