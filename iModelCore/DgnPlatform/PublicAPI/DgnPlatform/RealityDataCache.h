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
DGNPLATFORM_REF_COUNTED_PTR(IRealityDataBase)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataCache)

DGNPLATFORM_REF_COUNTED_PTR(IRealityDataStorageResponseReceiver)
DGNPLATFORM_REF_COUNTED_PTR(IRealityDataStorageBase)
DGNPLATFORM_REF_COUNTED_PTR(BeSQLiteRealityDataStorage)
DGNPLATFORM_REF_COUNTED_PTR(InMemoryRealityDataStorage)

DGNPLATFORM_REF_COUNTED_PTR(IRealityDataSourceResponseReceiver)
DGNPLATFORM_REF_COUNTED_PTR(IRealityDataSourceBase)
DGNPLATFORM_REF_COUNTED_PTR(HttpRealityDataSource)
DGNPLATFORM_REF_COUNTED_PTR(FileRealityDataSource)

struct HasWorkOrTerminatesPredicate;
struct IsIdlePredicate;
struct AllThreadsIdlePredicate;
struct FileRealityDataSourceRequest;
struct HttpRealityDataSourceRequest;
struct ThreadPoolQueueNotEmptyPredicate;

#define DEFINE_MEMBER_TYPE_CHECKER(MemberType) \
    template<class T> class has_member_type_##MemberType \
    { \
        struct big { char a[2]; }; \
        template<class C> static big  probe(typename C::MemberType*); \
        template<class C> static char probe(...); \
    public: \
        static const bool value = sizeof(probe<T>(nullptr)) > 1; \
    };
 
#define DEFINE_MEMBER_CHECKER(Member)   \
    template<class T> class has_member_##Member \
    { \
        struct big { char a[2]; }; \
        template<class C> static big  probe(decltype(&C::Member)); \
        template<class C> static char probe(...); \
    public: \
        static const bool value = sizeof(probe<T>(nullptr)) > 1; \
    };

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Options for @ref RealityDataCache::Get request function.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataOptions
{
    bool m_requestFromSource = true;
    bool m_useStorage = true;
    bool m_forceSynchronous = false;
    bool m_removeAfterSelect = false;
    bool m_retryNotFoundRequests = false;
};

//=======================================================================================
//! The virtual base class for all reality data implementations. 
//! @warning Reality data implementations should implement the @ref IRealityData interface
//! instead of this one.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct IRealityDataBase : IRefCounted
{
    friend struct RealityDataCache;
    
    protected:
        virtual ~IRealityDataBase() {}
        virtual Utf8CP _GetId() const = 0;
        virtual bool _IsExpired() const = 0;
        virtual void _OnError() {}
        virtual void _OnNotFound() {}

    public:
        // properties
        Utf8CP GetId() const {return _GetId();}
        bool IsExpired() const {return _IsExpired();}
};

//=======================================================================================
//! The base class for all reality data implementations. The implementation must provide
//! its own type, the preferred storage and source types as template parameters.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
template<typename _Derived, typename _StorageType, typename _SourceType>
struct IRealityData : _StorageType::Data, _SourceType::Data
{
#if !defined (DOCUMENTATION_GENERATOR)
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
#endif

    // export
    typedef _Derived        ThisType;
    typedef _StorageType    StorageType;
    typedef _SourceType     SourceType;

    IRealityData() {}
    virtual ~IRealityData() {}
    uint32_t GetRefCount() const {return m_refCount.load();}
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
    ResultType          m_result;
    Utf8String          m_id;
    IRealityDataBase&   m_data;
protected:
    RealityDataResponse(ResultType result, Utf8CP id, IRealityDataBase& data) : m_result(result), m_id(id), m_data(data) {}
public:
    ResultType GetResult() const {return m_result;}
    Utf8CP GetId() const {return m_id.c_str();}
    IRealityDataBase& GetData() const {return m_data;}
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
        virtual IRealityDataBase const* _GetData() const = 0;
    };

//=======================================================================================
//! The virtual base class for all reality data storages.
//! @warning Reality data storage implementations should implement the @ref IRealityDataStorage 
//! interface instead of this one.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IRealityDataStorageBase : RefCountedBase
{
    friend struct RealityDataCache;
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : virtual IRealityDataBase
        {
        protected:
            virtual ~Data() {}
        };
    
protected:
    virtual DGNPLATFORM_EXPORT ~IRealityDataStorageBase();

    //! The unique id of this reality data storage. 
    //! @warning This id must match the id returned by the static StorageId() function.
    virtual Utf8String _GetStorageId() const = 0;

    //! Called during the cache termination process. The storage is supposed to terminate all
    //! its worker threads here.
    virtual void _Terminate() {}
    
public:
    DGNPLATFORM_EXPORT void Terminate();
};

//=======================================================================================
//! The base class for all reality data storages. The implementation must provide its own
//! type as a template parameter.
//!
//! All IRealityDataStorage implementations have these requirements:
//! - Define a nested `SelectOptions` structure.
//! - Define a nested `PersistHandler` structure which implements @ref IRealityDataStoragePersistHandler
//!   and has a constructor which takes a reference of the implementation instance as the 
//!   first parameter and a const reference of the data as the second parameter.
//! - Define a function which returns the ID of the storage implementation:
//!   @code static Utf8String StorageId() @endcode
//! - Define a member function: @code RealityDataStorageResult Select(Data&, Utf8CP, SelectOptions const&, IRealityDataStorageResponseReceiver&)
//!   @endcode
//! - Call the `_OnResponseReceived` callback function of the response receiver
//!   that is passed to the `Request` function.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
template<class Derived> struct IRealityDataStorage : IRealityDataStorageBase
    {
    DEFINE_MEMBER_TYPE_CHECKER(PersistHandler)
    DEFINE_MEMBER_CHECKER(StorageId)
    DEFINE_MEMBER_CHECKER(Select)

    //! Virtual destructor provides some compile-time assertions for the implementation.
    virtual ~IRealityDataStorage()
        {
        static_assert(has_member_type_PersistHandler<Derived>::value, "IRealityDataStorage implementation must have an inner struct PersistHandler.");
        static_assert(std::is_base_of<IRealityDataStoragePersistHandler, typename Derived::PersistHandler>::value, "PersistHandler must implement IRealityDataStoragePersistHandler.");
        static_assert(has_member_StorageId<Derived>::value, "IRealityDataStorage implementation must have a static Utf8String StorageId() function.");
        static_assert(std::is_same<decltype(Derived::StorageId()), Utf8String>::value, "The return type of StorageId() function must be Utf8String.");
        static_assert(has_member_Select<Derived>::value, "IRealityDataStorage implementation must have a static RealityDataStorageResult Select(Data&, Utf8CP, SelectOptions const&) function.");
        }
    };

//======================================================================================
//! The response of reality data storage.
// @bsiclass                                        Grigas.Petraitis           04/2015
//======================================================================================
struct RealityDataStorageResponse : RealityDataResponse<RealityDataStorageResult>
{
private:
    RealityDataStorageResponse(RealityDataStorageResult result, Utf8CP id, IRealityDataBase& data) : RealityDataResponse(result, id, data) {}
public:
    static RefCountedPtr<RealityDataStorageResponse> Create(RealityDataStorageResult result, Utf8CP id, IRealityDataBase& data) {return new RealityDataStorageResponse(result, id, data);}
};

//======================================================================================
//! Interface for a reality data storage response receiver.
// @bsiclass                                        Grigas.Petraitis           04/2015
//======================================================================================
struct IRealityDataStorageResponseReceiver : virtual IRefCounted
    {
    //! Called to notify the receiver with a response.
    virtual void _OnResponseReceived(RealityDataStorageResponse const&, RealityDataOptions, bool isAsync) = 0;
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
        virtual IRealityDataBase const* _GetData() const = 0;
    };

//=======================================================================================
//! The virtual base class for all reality data sources.
//! @warning Reality data source implementations should implement the @ref IRealityDataSource
//! interface instead of this one.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IRealityDataSourceBase : RefCountedBase
{   
    friend struct RealityDataCache;
    friend struct CombinedRealityDataSourceBase;

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : virtual IRealityDataBase
        {
        protected:
            virtual ~Data() {}
        };
    
protected:
    DGNPLATFORM_EXPORT virtual ~IRealityDataSourceBase();

    //! The unique id of this reality data source. 
    //! @warning This id must match the id returned by the static SourceId() function.
    virtual Utf8String _GetSourceId() const = 0;

    //! Called during the cache termination process. The source is supposed to terminate all
    //! its worker threads here.
    virtual void _Terminate() {}

    //! The abstract function that might be called to request data. The imeplementations should simply
    //! forward requests to their more specialized `Request` functions.
    //! @note It is guaranteed that the arguments can be cast to their derived classes, provided by the implementation.
    virtual RealityDataSourceResult _Request(Data&, bool& handled, Utf8CP, RealityDataOptions, IRealityDataSourceResponseReceiver&) = 0;

public:
    DGNPLATFORM_EXPORT void Terminate();
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data&, bool& handled, Utf8CP, RealityDataOptions, IRealityDataSourceResponseReceiver&);
};

//=======================================================================================
//! The base class for all reality data sources. The implementation must provide its own
//! type as a template parameter.
//!
//! All IRealityDataSource implementations have these requirements:
//! - Define a nested `RequestOptions` structure.
//! - Define a nested `RequestHandler` structure which implements @ref IRealityDataSourceRequestHandler;
//! - Define a function which returns the ID of the source implementation:
//!   @code static Utf8String SourceId() @endcode
//! - Define a member function: @code RealityDataSourceResult Request(Data&, bool&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&)
//!   @endcode
//! - Call the `_OnResponseReceived` callback function of the response receiver
//!   that is passed to the `Request` function.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
template<class Derived> struct IRealityDataSource : IRealityDataSourceBase
    {
    DEFINE_MEMBER_CHECKER(SourceId)
    DEFINE_MEMBER_CHECKER(Request)

    //! Virtual destructor provides some compile-time assertions for the implementation.
    virtual ~IRealityDataSource()
        {
        static_assert(has_member_SourceId<Derived>::value, "IRealityDataSource implementation must have a static Utf8String SourceId() function.");
        static_assert(std::is_same<decltype(Derived::SourceId()), Utf8String>::value, "The return type of SourceId() function must be Utf8String.");
        static_assert(has_member_Request<Derived>::value, "IRealityDataSource implementation must have a static RealityDataSourceResult Request(Data&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&) function.");
        }
    };

//======================================================================================
//! The response of reality data source.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct RealityDataSourceResponse : RealityDataResponse<RealityDataSourceResult>
{
private:
    RealityDataSourceResponse(RealityDataSourceResult result, Utf8CP id, IRealityDataBase& data) : RealityDataResponse(result, id, data) {}
public:
    static RefCountedPtr<RealityDataSourceResponse> Create(RealityDataSourceResult result, Utf8CP id, IRealityDataBase& data) {return new RealityDataSourceResponse(result, id, data);}
};

//======================================================================================
//! Interface for a reality data source response receiver.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct IRealityDataSourceResponseReceiver : virtual IRefCounted
    {
    //! Called to notify the receiver with a response.
    virtual void _OnResponseReceived(RealityDataSourceResponse const&, RealityDataOptions) = 0;
    };

//=======================================================================================
//! The base class for all combined reality data sources.
//! @warning Combined reality data source implementations should derive from the 
//! @ref CombinedRealityDataSource class instead of this one.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct CombinedRealityDataSourceBase : IRealityDataSourceBase
{
    //===================================================================================
    //! Interface for CombinedRealityDataSource data. All reality data using this kind of 
    //! source must implement this interface.
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct Data : virtual IRealityDataSourceBase::Data
        {
        friend struct CombinedRealityDataSourceBase;
        protected:
            virtual IRealityDataSourceBase& _PickDataSource(Utf8CP id, bmap<Utf8String, IRealityDataSourceBasePtr> availableSources) = 0;
        };

private:
    bmap<Utf8String, IRealityDataSourceBasePtr> m_sources;
protected:
    DGNPLATFORM_EXPORT virtual void _Terminate() override;
    DGNPLATFORM_EXPORT static Utf8String GetIdFormat(int numSources);
    DGNPLATFORM_EXPORT void RegisterSource(IRealityDataSourceBase&);
public:
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data&, bool&, Utf8CP, RealityDataOptions, IRealityDataSourceResponseReceiver&);
};

//=======================================================================================
//! Reality data source which takes multiple data sources as template parameters and 
//! allows the data to choose which source it should be initialized from.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
template<class... Sources>
struct CombinedRealityDataSource : CombinedRealityDataSourceBase
{
    struct Data : CombinedRealityDataSourceBase::Data, Sources::Data... {};

private:
    CombinedRealityDataSource() {}
    template<class Source> void RegisterSources(Source* source) {RegisterSource(*source);}
    template<class Source> void RegisterSources(Source* source, Sources*... sources)
        {
        if (nullptr != source)
            {
            RegisterSources(source);
            RegisterSources(sources...);
            }
        }

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    virtual RealityDataSourceResult _Request(IRealityDataSourceBase::Data& data, bool& handled, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver) override
        {
        return Request(dynamic_cast<CombinedRealityDataSourceBase::Data&>(data), handled, id, options, responseReceiver);
        }

public:
    //! The id of this storage.
    static Utf8String SourceId() {return Utf8PrintfString(GetIdFormat(sizeof...(Sources)).c_str(), Sources::SourceId()...);}

    //! Create a new instance of CombinedRealityDataSource.
    static RefCountedPtr<CombinedRealityDataSource<Sources...>> Create(Sources&... sources)
        {
        CombinedRealityDataSource<Sources...>* source = new CombinedRealityDataSource<Sources...>();
        source->RegisterSources(&sources...);
        return source;
        }
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
//! The reality data cache class which manages two types of objects - storages and sources.
//! Storages are responsible for caching the reality data and returning it quickly on demand.
//! Sources are responsible for retrieving the reality data from its primary source, e.g.
//! the internet.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RealityDataCache : NonCopyableClass, IRealityDataStorageResponseReceiver, IRealityDataSourceResponseReceiver
{    
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

    template<typename T> struct RefCountedPtrComparer
        {
        bool operator()(RefCountedPtr<T> const& lhs, RefCountedPtr<T> const& rhs) const {return lhs.get() < rhs.get();}
        };
    
private:
    bmap<Utf8String, IRealityDataStorageBasePtr> m_storages;
    bmap<Utf8String, IRealityDataSourceBasePtr>  m_sources;
    bmap<Utf8String, bset<RefCountedPtr<IRealityDataStoragePersistHandler>, RefCountedPtrComparer<IRealityDataStoragePersistHandler>>> m_persistHandlers;
    BeMutex m_persistHandlersCS;
    bmap<Utf8String, bset<RefCountedPtr<IRealityDataSourceRequestHandler>, RefCountedPtrComparer<IRealityDataSourceRequestHandler>>> m_requestHandlers;
    BeMutex m_requestHandlersCS;
    mutable bmap<void const*, RealityDataCacheResult> m_results;
    mutable BeMutex m_resultsCS;
    
    ~RealityDataCache();

    // note: these might be called from any thread!
    virtual void _OnResponseReceived(RealityDataSourceResponse const& response, RealityDataOptions) override;
    virtual void _OnResponseReceived(RealityDataStorageResponse const& response, RealityDataOptions, bool isAsync) override;

    RealityDataCacheResult HandleStorageResponse(RealityDataStorageResponse const& response, RealityDataOptions);
    RealityDataCacheResult ResolveResult(RealityDataStorageResult storageResult, RealityDataSourceResult sourceResult = RealityDataSourceResult::Error_NotFound) const;
    RefCountedPtr<IRealityDataSourceRequestHandler> DequeueRequestHandler(Utf8CP id, IRealityDataBase const&);
    RefCountedPtr<IRealityDataStoragePersistHandler> DequeuePersistHandler(Utf8CP id, IRealityDataBase const&);

private:
    DGNPLATFORM_EXPORT IRealityDataStorageBase* GetStorage(Utf8CP id) const;
    DGNPLATFORM_EXPORT IRealityDataSourceBase* GetSource(Utf8CP id) const;
    template<typename StorageType> StorageType* GetStorage() const {return(StorageType*)GetStorage(StorageType::StorageId().c_str());}
    template<typename SourceType> SourceType* GetSource() const {return(SourceType*)GetSource(SourceType::SourceId().c_str());}

    DGNPLATFORM_EXPORT void QueuePersistHandler(Utf8CP id, IRealityDataStoragePersistHandler&);
    DGNPLATFORM_EXPORT void QueueRequestHandler(Utf8CP id, IRealityDataSourceRequestHandler&);

    DGNPLATFORM_EXPORT RealityDataCacheResult GetResult(IRealityDataBase& data, Utf8CP id, RealityDataStorageResult storageResult);
    DGNPLATFORM_EXPORT RealityDataCacheResult GetResult(Utf8CP id, RealityDataSourceResult sourceResult);
    
public:
    //! Create a new reality data cache.
    DGNPLATFORM_EXPORT RealityDataCache() {}

    //! Cleans up local caches
    DGNPLATFORM_EXPORT void Cleanup();

    //! Register a storage in this cache.
    DGNPLATFORM_EXPORT void RegisterStorage(IRealityDataStorageBase& storage);

    //! Register a source in this cache.
    DGNPLATFORM_EXPORT void RegisterSource(IRealityDataSourceBase& source);

    //! Request reality data from this cache. The type of the reality data is provided as a template parameter.
    //! @param[out] data       An reference object to receive the requested data.
    //! @param[in] id       The id of the requested data.
    //! @param[in] options  The request options.
    template<typename DataType> RealityDataCacheResult Get(DataType& data, Utf8CP id, RealityDataOptions options)
        {
        RefCountedPtr<DataType> temp(&data);

        if (!options.m_useStorage)
            {
            if (!options.m_requestFromSource)
                return RealityDataCacheResult::NotFound;

            bool handled;
            return GetResult(id, GetSource<typename DataType::SourceType>()->Request(data, handled, id, options, *this));
            }
        
        QueueRequestHandler(id, *DataType::SourceType::RequestHandler::Create(*GetSource<typename DataType::SourceType>(), data, id, options, *this));
        QueuePersistHandler(id, *DataType::StorageType::PersistHandler::Create(*GetStorage<typename DataType::StorageType>(), data));
        return GetResult(data, id, GetStorage<typename DataType::StorageType>()->Select(data, id, options, *this));
        }
};

//=======================================================================================
//! Reality data storage which uses a SQLite database for storing cached data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BeSQLiteRealityDataStorage : IRealityDataStorage<BeSQLiteRealityDataStorage>, NonCopyableClass
{
    //===================================================================================
    //! Interface for BeSQLiteRealityDataStorage data. All reality data using this kind of 
    //! storage must implement this interface.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : IRealityDataStorage::Data
        {
        friend struct BeSQLiteRealityDataStorage;
        protected:
            //! Initialize data from the database using key and select options.
            virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key) = 0;

            //! Persist this data in the database.
            virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const = 0;
        };
    
    //===================================================================================
    //! @ref IRealityDataStoragePersistHandler implementation which knows how to persist
    //! reality data in the @ref BeSQLiteRealityDataStorage.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct PersistHandler : IRealityDataStoragePersistHandler
    {
    private:
        BeSQLiteRealityDataStorage& m_storage;
        RefCountedPtr<Data const>   m_data;
        PersistHandler(BeSQLiteRealityDataStorage& storage, Data const& data) : m_storage(storage), m_data(&data) {}        
    protected:
        DGNPLATFORM_EXPORT virtual RealityDataStorageResult _Persist() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        static RefCountedPtr<PersistHandler> Create(BeSQLiteRealityDataStorage& storage, Data const& data) {return new PersistHandler(storage, data);}
    };

    struct CleanAndSaveChangesWork;
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct BeSQLiteStorageThreadPool : RealityDataThreadPool
    {
    friend struct CleanAndSaveChangesWork;
    friend struct ::ThreadPoolQueueNotEmptyPredicate;
    private:
        BeSQLiteRealityDataStoragePtr m_storage;
    protected:
        BeSQLiteStorageThreadPool(BeSQLiteRealityDataStorage& storage) : RealityDataThreadPool(4, 0, SchedulingMethod::LIFO), m_storage(&storage) {}
        ThreadSafeQueue<RealityDataWorkPtr>& GetQueue() {return RealityDataThreadPool::GetQueue();}
        virtual bool _AssignWork(RealityDataWorkerThread& thread) override;
    public:
        static RefCountedPtr<BeSQLiteStorageThreadPool> Create(BeSQLiteRealityDataStorage& storage) {return new BeSQLiteStorageThreadPool(storage);}
        void QueueIdleWork(RealityDataWork& work);
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct StorageWork : RefCounted<RealityDataWork>
    {
    protected:
        BeSQLiteRealityDataStoragePtr m_storage;
        StorageWork(BeSQLiteRealityDataStorage& storage) : m_storage(&storage) {}
    };

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct SelectDataWork : StorageWork
    {
    private:
        Utf8String m_id;
        RefCountedPtr<Data> m_data;
        RealityDataOptions m_options;
        IRealityDataStorageResponseReceiverPtr m_responseReceiver;
        bool m_hasResult;
        mutable RealityDataStorageResult m_result;
        mutable BeConditionVariable m_resultCV;
    protected:
        SelectDataWork(BeSQLiteRealityDataStorage& storage, Utf8CP id, Data& data, RealityDataOptions options, IRealityDataStorageResponseReceiver& responseReceiver) 
            : StorageWork(storage), m_id(id), m_data(&data), m_options(options), m_hasResult(false), m_responseReceiver(&responseReceiver)
            {} 
        virtual void _DoWork() override;
    public:
        static RefCountedPtr<SelectDataWork> Create(BeSQLiteRealityDataStorage& storage, Utf8CP id, Data& data, RealityDataOptions options, IRealityDataStorageResponseReceiver& responseReceiver)
            {
            return new SelectDataWork(storage, id, data, options, responseReceiver);
            }
        RealityDataStorageResult GetResult() const;
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct PersistDataWork : StorageWork
    {
    private:
        RefCountedPtr<Data const>  m_data;
    protected:
        PersistDataWork(BeSQLiteRealityDataStorage& storage, Data const& data) : StorageWork(storage), m_data(&data) {} 
        virtual void _DoWork() override;
    public:
        static RefCountedPtr<PersistDataWork> Create(BeSQLiteRealityDataStorage& storage, Data const& data) {return new PersistDataWork(storage, data);}
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct CleanAndSaveChangesWork : StorageWork
    {
    private:
        uint32_t m_idleTime;
    protected:
        CleanAndSaveChangesWork(BeSQLiteRealityDataStorage& storage, uint32_t idleTime) : StorageWork(storage), m_idleTime(idleTime) {}
        virtual void _DoWork() override;
    public:
        static RefCountedPtr<CleanAndSaveChangesWork> Create(BeSQLiteRealityDataStorage& storage, uint32_t idleTime) {return new CleanAndSaveChangesWork(storage, idleTime);}
    };
    
private:
    bool m_initialized;
    Utf8String m_filename;
    RefCountedPtr<BeSQLiteStorageThreadPool> m_threadPool;
    BeAtomic<bool> m_hasChanges;
    BeMutex m_saveChangesCS;
    BeSQLite::Db* m_database;
    BeMutex m_databaseCS;
    bset<Utf8String> m_activeRequests;
    BeMutex m_activeRequestsCS;
    uint32_t m_idleTime;
    RefCountedPtr<BeSQLite::BusyRetry> m_retry;

private:
    void wt_Prepare();
    void wt_Cleanup();
    void wt_Persist(Data const& data);
    RealityDataStorageResult wt_Select(Data& data, Utf8CP id, RealityDataOptions options, IRealityDataStorageResponseReceiver& responseReceiver);
    void wt_SaveChanges();

public:
    //! NOT PUBLISHED: needed for tests
    RefCountedPtr<BeSQLiteStorageThreadPool> GetThreadPool() const {return m_threadPool;}

protected:
    virtual Utf8String _GetStorageId() const override {return StorageId();}
    DGNPLATFORM_EXPORT virtual void _Terminate() override;

    //! Called to check if the database is prepared to store specific type of data 
    //! (the type must be known to the implementation).
    virtual bool _IsPrepared() const = 0;

    //! Called to prepare the database for storing specific type of data (the type 
    //! must be known to the implementation).
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const = 0;

    //! Called to free some space in the database.
    //! @param[in] db           The database to cleanup.
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const = 0;

public:
    //! Creates a new BeSQLiteRealityDataStorage.
    //! @param[in] filename     File name.
    //! @param[in] idleTime     Time (in miliseconds) for the worker thread to wait before commiting changes to the database.
    DGNPLATFORM_EXPORT BeSQLiteRealityDataStorage(BeFileName const& filename, uint32_t idleTime=5000);
    DGNPLATFORM_EXPORT ~BeSQLiteRealityDataStorage();

    //! The id of this storage.
    static Utf8String StorageId() {return "BeSQLite";}

    //! Initialize data object with the data in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Select(Data& data, Utf8CP id, RealityDataOptions options, IRealityDataStorageResponseReceiver& responseReceiver);

    //! Persist the data object in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Persist(Data const& data);
};

//=======================================================================================
//! Reality data storage which stores reality data in memory.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InMemoryRealityDataStorage : IRealityDataStorage<InMemoryRealityDataStorage>, NonCopyableClass
    {
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct Data : IRealityDataStorage::Data
        {
        friend struct InMemoryRealityDataStorage;
        protected:
            //! Initialize data from the void pointer and select options.
            virtual BentleyStatus _InitFrom(Utf8CP id, Data const& data) = 0;
        };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct PersistHandler : IRealityDataStoragePersistHandler
    {
    private:
        InMemoryRealityDataStorage& m_storage;
        RefCountedPtr<Data const>   m_data;
        PersistHandler(InMemoryRealityDataStorage& storage, Data const& data) : m_storage(storage), m_data(&data) {}    
    protected:
        DGNPLATFORM_EXPORT virtual RealityDataStorageResult _Persist() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        static RefCountedPtr<PersistHandler> Create(InMemoryRealityDataStorage& storage, Data const& data) {return new PersistHandler(storage, data);}
    };

private:
    bmap<Utf8String, Data const*> m_map;
    BeMutex m_cs;

private:
    //! Persist the data object in memory.
    DGNPLATFORM_EXPORT RealityDataStorageResult Persist(Data const& data);
    
protected:
    virtual Utf8String _GetStorageId() const override {return StorageId();}

public:
    //! The id of this storage.
    static Utf8String StorageId() {return "InMemory";}
    
    //! Creates a new instance of InMemoryRealityDataStorage.
    DGNPLATFORM_EXPORT static InMemoryRealityDataStoragePtr Create();
    DGNPLATFORM_EXPORT ~InMemoryRealityDataStorage();
    DGNPLATFORM_EXPORT RealityDataStorageResult Select(Data& data, Utf8CP id, RealityDataOptions options, IRealityDataStorageResponseReceiver& responseReceiver);
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
template<class Derived> struct EXPORT_VTABLE_ATTRIBUTE AsyncRealityDataSource : IRealityDataSource<Derived>, NonCopyableClass
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestHandler : RefCounted<RealityDataWork>, IAsyncRequestCancellationToken
    {
    private:
        RefCountedPtr<AsyncRealityDataSource> m_source;
        RefCountedPtr<AsyncRealityDataSourceRequest const> m_request;
        IRealityDataSourceResponseReceiver* m_responseReceiver;
        
        RequestHandler(AsyncRealityDataSource& source, AsyncRealityDataSourceRequest const& request, IRealityDataSourceResponseReceiver& responseReceiver)
            : m_source(&source), m_request(&request), m_responseReceiver(&responseReceiver)
            {}
        void SendResponse(RealityDataSourceResponse const& response, RealityDataOptions options) {m_responseReceiver->_OnResponseReceived(response, options);}

    protected:
        virtual void _DoWork() override;
        virtual bool _ShouldCancel() const override;

    public:
        static RefCountedPtr<RequestHandler> Create(AsyncRealityDataSource& source, AsyncRealityDataSourceRequest const& request, IRealityDataSourceResponseReceiver& responseReceiver) 
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
    virtual void _Terminate() override
        {
        m_terminateRequested = true;
        m_threadPool->Terminate();
        m_threadPool->WaitUntilAllThreadsIdle();
        }
    AsyncRealityDataSource(int numThreads) 
        : m_threadPool(RealityDataThreadPool::Create(numThreads, 0, SchedulingMethod::LIFO)), m_ignoreRequestsUntil(0), m_terminateRequested(false)
        {}

protected:
    //! Queues a request for handling on the work thread. Derived classes should use this
    //! function to queue their requests to be handled asynchronously.
    RealityDataSourceResult QueueRequest(AsyncRealityDataSourceRequest& request, bool& handled, IRealityDataSourceResponseReceiver& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses files (on the disc) as the source of reality data.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FileRealityDataSource : AsyncRealityDataSource<FileRealityDataSource>
{
    //===================================================================================
    //! Interface for FileRealityDataSource data. All reality data using this kind of 
    //! source must implement this interface.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : virtual IRealityDataSource::Data
        {
        friend struct ::FileRealityDataSourceRequest;
        friend struct FileRealityDataSource;
        protected:
            //! Initializes this data with file path, content and request options.
            //! @param[in] filepath     The path of the file that the data was read from.
            //! @param[in] data         The file content.
            virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) = 0;
        };

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
        RefCountedPtr<Data> m_data;
        Utf8String m_id;
        RealityDataOptions m_options;
        IRealityDataSourceResponseReceiver* m_responseReceiver;
        RequestHandler(FileRealityDataSource& source, Data& data, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(&responseReceiver), m_handled(false)
            {}
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        DGNPLATFORM_EXPORT virtual RealityDataSourceResult _Request() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        static RefCountedPtr<RequestHandler> Create(FileRealityDataSource& source, Data& data, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver)
            {
            return new RequestHandler(source, data, id, options, responseReceiver);
            }
    };

private:
    FileRealityDataSource(int numThreads) : AsyncRealityDataSource(numThreads) {}

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    virtual RealityDataSourceResult _Request(IRealityDataSourceBase::Data& data, bool& handled, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver) override
        {
        return Request(dynamic_cast<Data&>(data), handled, id, options, responseReceiver);
        }

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "File";}

    //! Create a new instance of FileRealityDataSource.
    //! @param[in] numThreads   Number of worker threads to use for reading files. Must be at least 1.
    DGNPLATFORM_EXPORT static RefCountedPtr<FileRealityDataSource> Create(int numThreads);

    //! Request the data to be initialized from the specified file.
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data& data, bool& handled, Utf8CP filepath, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses HTTP protocol to download content and use it as 
//! the source of reality data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HttpRealityDataSource : AsyncRealityDataSource<HttpRealityDataSource>
{
    //===================================================================================
    //! Interface for HttpRealityDataSource data. All reality data using this kind of 
    //! source must implement this interface.
    //! @note Implementation must set expiration date and entity tag (see @ref SetExpirationDate
    //! and @ref SetEntityTag) when it initializes the data from the reality data storage.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : virtual IRealityDataSource::Data
        {
        friend struct ::HttpRealityDataSourceRequest;
        friend struct HttpRealityDataSource;
        private:
            DateTime    m_expirationDate;
            Utf8String  m_entityTag;
            void ParseExpirationDateAndETag(bmap<Utf8String, Utf8String> const& header);
        protected:
            void SetExpirationDate(DateTime const& date) {m_expirationDate = date;}
            void SetEntityTag(Utf8CP eTag) {m_entityTag = eTag;}
            virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) = 0;
        public:
            DateTime const& GetExpirationDate() const {return m_expirationDate;}
            Utf8CP GetEntityTag() const {return m_entityTag.c_str();}
        };
        
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
        RefCountedPtr<Data> m_data;
        Utf8String m_id;
        RealityDataOptions m_options;
        IRealityDataSourceResponseReceiver* m_responseReceiver;
        RequestHandler(HttpRealityDataSource& source, Data& data, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(&responseReceiver), m_handled(false)
            {}  
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        DGNPLATFORM_EXPORT virtual RealityDataSourceResult _Request() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        static RefCountedPtr<RequestHandler> Create(HttpRealityDataSource& source, Data& data, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver)
            {
            return new RequestHandler(source, data, id, options, responseReceiver);
            }   
    };

private:
    HttpRealityDataSource(int numThreads) : AsyncRealityDataSource(numThreads) {}

protected:
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    virtual RealityDataSourceResult _Request(IRealityDataSourceBase::Data& data, bool& handled, Utf8CP id, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver) override
        {
        return Request(dynamic_cast<Data&>(data), handled, id, options, responseReceiver);
        }

public:
    //! The id of this reality data source.
    static Utf8String SourceId() {return "Http";}
    
    //! Create a new instance of HttpRealityDataSource.
    static HttpRealityDataSourcePtr Create(int numThreads) {return new HttpRealityDataSource(numThreads);}
    
    //! Request the data to be initialized from the content at the specified url
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data& data, bool& handled, Utf8CP url, RealityDataOptions options, IRealityDataSourceResponseReceiver& responseReceiver);
};

END_BENTLEY_DGN_NAMESPACE
