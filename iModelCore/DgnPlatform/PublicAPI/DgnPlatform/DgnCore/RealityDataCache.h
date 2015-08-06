/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RealityDataCache.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>

//__PUBLISH_SECTION_END__
DGNPLATFORM_REF_COUNTED_PTR(RealityDataThread)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataThreadPool)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataWork)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataWorkerThread)

//__PUBLISH_SECTION_START__
DGNPLATFORM_REF_COUNTED_PTR(IRealityDataBase)
DGNPLATFORM_REF_COUNTED_PTR(RealityDataCache)

DGNPLATFORM_REF_COUNTED_PTR(IRealityDataStorageBase)
DGNPLATFORM_REF_COUNTED_PTR(InMemoryRealityDataStorage)
DGNPLATFORM_REF_COUNTED_PTR(BeSQLiteRealityDataStorage)

DGNPLATFORM_REF_COUNTED_PTR(IRealityDataSourceResponseReceiver)
DGNPLATFORM_REF_COUNTED_PTR(IRealityDataSourceBase)
DGNPLATFORM_REF_COUNTED_PTR(HttpRealityDataSource)
DGNPLATFORM_REF_COUNTED_PTR(FileRealityDataSource)

//__PUBLISH_SECTION_END__
struct HasWorkOrTerminatesPredicate;
struct IsIdlePredicate;
struct AllThreadsIdlePredicate;
struct FileRealityDataSourceRequest;
struct HttpRealityDataSourceRequest;
//__PUBLISH_SECTION_START__

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

struct RealityDataCacheOptions;

//=======================================================================================
//! The virtual base class for all reality data implementations. 
//! @warning Reality data implementations should implement the @ref IRealityData interface
//! instead of this one.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct IRealityDataBase : IRefCounted
    {
    friend struct RealityDataCache;

    private:
        bool m_initialized;
        void SetInitialized() {m_initialized=true;}

    protected:
        IRealityDataBase() : m_initialized(false) {}
        virtual ~IRealityDataBase() {}
        virtual Utf8CP _GetId() const = 0;
        virtual bool _IsExpired() const = 0;

    public:
        bool IsInitialized() const {return m_initialized;}

        // properties
        DGNPLATFORM_EXPORT Utf8CP GetId() const;
        DGNPLATFORM_EXPORT bool IsExpired() const;
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
    DEFINE_MEMBER_TYPE_CHECKER(RequestOptions)
    DEFINE_MEMBER_CHECKER(Create)
#endif

    // export
    typedef _Derived        ThisType;
    typedef _StorageType    StorageType;
    typedef _SourceType     SourceType;

    //! The RequestOptions struct which contains SelectOptions of the Storage and 
    //! RequestOptions of the Source. The Request options of the implementor must
    //! derive from this struct.
    struct RequestOptions : StorageType::SelectOptions, SourceType::RequestOptions
        {
        };

    IRealityData() {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}

    //! The virtual destructor which provides some compile-time assertions for the
    //! implementing type.
    virtual ~IRealityData()
        {
        static_assert(has_member_Create<ThisType>::value, "IRealityData implementation must have a static Create() function.");
        static_assert(std::is_same<decltype(ThisType::Create()), RefCountedPtr<ThisType>>::value, "The return type of the Create() function must be RefCountedPtr<ThisType>");
        static_assert(has_member_type_RequestOptions<ThisType>::value, "IRealityData implementation must have an inner struct RequestOptions.");
        static_assert(std::is_base_of<RequestOptions, typename ThisType::RequestOptions>::value && std::is_base_of<RealityDataCacheOptions, typename ThisType::RequestOptions>::value, 
            "RequestOptions must derive from IRealityData::RequestOptions and RealityDataCacheOptions");
        }
    };

//__PUBLISH_SECTION_END__
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
        Item(T const& d, Item* p, Item* n) : prev(p), next(n), data(d) {}
        static ItemPtr Create(T const& d, Item* p, Item* n) {return new Item(d, p, n);}
        };

private:
    BeConditionVariable m_cv;
    ItemPtr m_first;
    ItemPtr m_last;

public:
    BeConditionVariable& GetConditionVariable() {return m_cv;}

public:
    DGNPLATFORM_EXPORT void Push(T const& element);
    DGNPLATFORM_EXPORT bool Pop(T& element);
    DGNPLATFORM_EXPORT void Clear();
    DGNPLATFORM_EXPORT bool IsEmpty() const;
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
    IStateListener*             m_stateListener;

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
    DGNPLATFORM_EXPORT void Terminate();
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
    mutable BeConditionVariable m_threadsCV;
    mutable BeMutex             m_workQueueCS;
    bmap<RealityDataWorkerThread*, bool> m_threads;
    ThreadSafeQueue<RealityDataWorkPtr> m_workQueue;

private:
    RealityDataThreadPool(int maxThreads, int maxIdleThreads) : m_maxThreads(maxThreads), m_maxIdleThreads(maxIdleThreads), m_threadsCV() {}
    RealityDataWorkerThreadPtr CreateThread();
    RealityDataWorkerThreadPtr GetIdleThread() const;
    bool ShouldCreateNewThread() const;

protected:
    DGNPLATFORM_EXPORT virtual void _OnThreadBusy(RealityDataWorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual void _OnThreadIdle(RealityDataWorkerThread& thread) override;

public:
    //! NOT PUBLISHED: needed for tests
    DGNPLATFORM_EXPORT void WaitUntilAllThreadsIdle() const;

public:
    static RealityDataThreadPoolPtr Create(int maxThreads, int maxIdleThreads) {return new RealityDataThreadPool(maxThreads, maxIdleThreads);}
    DGNPLATFORM_EXPORT virtual ~RealityDataThreadPool();
    DGNPLATFORM_EXPORT int  GetThreadsCount() const;
    DGNPLATFORM_EXPORT void QueueWork(RealityDataWork& work);
};

//__PUBLISH_SECTION_START__
//=======================================================================================
//! The result of reality data storage operations.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class RealityDataStorageResult
    {
    Error,
    NotFound,
    Success,
    };

//=======================================================================================
//! Interface for a persist operation.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct IRealityDataStoragePersistHandler
    {
    friend struct RealityDataCache;
    protected:
        virtual ~IRealityDataStoragePersistHandler() {}

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
struct IRealityDataStorageBase : RefCountedBase
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
    //! The unique id of this reality data storage. 
    //! @warning This id must match the id returned by the static StorageId() function.
    virtual Utf8CP _GetStorageId() const = 0;
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
//!   @code static Utf8CP StorageId() @endcode
//! - Define a member function: @code RealityDataStorageResult Select(Data&, Utf8CP, SelectOptions const&)
//!   @endcode
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
template<class Derived>
struct IRealityDataStorage : IRealityDataStorageBase
    {
    DEFINE_MEMBER_TYPE_CHECKER(SelectOptions)
    DEFINE_MEMBER_TYPE_CHECKER(PersistHandler)
    DEFINE_MEMBER_CHECKER(StorageId)
    DEFINE_MEMBER_CHECKER(Select)

    //! Virtual destructor provides some compile-time assertions for the implementation.
    virtual ~IRealityDataStorage()
        {
        static_assert(has_member_type_SelectOptions<Derived>::value, "IRealityDataStorage implementation must have an inner struct SelectOptions.");
        static_assert(has_member_type_PersistHandler<Derived>::value, "IRealityDataStorage implementation must have an inner struct PersistHandler.");
        static_assert(std::is_base_of<IRealityDataStoragePersistHandler, typename Derived::PersistHandler>::value, "PersistHandler must implement IRealityDataStoragePersistHandler.");
        static_assert(has_member_StorageId<Derived>::value, "IRealityDataStorage implementation must have a static Utf8CP StorageId() function.");
        static_assert(std::is_same<decltype(Derived::StorageId()), Utf8CP>::value, "The return type of StorageId() function must be Utf8CP.");
        static_assert(has_member_Select<Derived>::value, "IRealityDataStorage implementation must have a static RealityDataStorageResult Select(Data&, Utf8CP, SelectOptions const&) function.");
        }
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
    Pending,
    NotModified,
    Success,
    };

//=======================================================================================
//! The virtual base class for all reality data sources.
//! @warning Reality data source implementations should implement the @ref IRealityDataSource
//! interface instead of this one.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IRealityDataSourceBase : RefCountedBase
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
    //! The unique id of this reality data source. 
    //! @warning This id must match the id returned by the static SourceId() function.
    virtual Utf8CP _GetSourceId() const = 0;
};

//=======================================================================================
//! The base class for all reality data sources. The implementation must provide its own
//! type as a template parameter.
//!
//! All IRealityDataSource implementations have these requirements:
//! - Define a nested `RequestOptions` structure.
//! - Define a function which returns the ID of the source implementation:
//!   @code static Utf8CP SourceId() @endcode
//! - Define a member function: @code RealityDataSourceResult Request(Data&, Utf8CP, RequestOptions const&, IRealityDataResponseReceiver&)
//!   @endcode
//! - Call the `_OnResponseReceived` callback function of the response receiver
//!   that is passed to the `Request` function.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
template<class Derived>
struct IRealityDataSource : IRealityDataSourceBase
    {
    DEFINE_MEMBER_TYPE_CHECKER(RequestOptions)
    DEFINE_MEMBER_CHECKER(SourceId)
    DEFINE_MEMBER_CHECKER(Request)

    //! Virtual destructor provides some compile-time assertions for the implementation.
    virtual ~IRealityDataSource()
        {
        static_assert(has_member_type_RequestOptions<Derived>::value, "IRealityDataSource implementation must have an inner struct RequestOptions.");
        static_assert(has_member_SourceId<Derived>::value, "IRealityDataSource implementation must have a static Utf8CP SourceId() function.");
        static_assert(std::is_same<decltype(Derived::SourceId()), Utf8CP>::value, "The return type of SourceId() function must be Utf8CP.");
        static_assert(has_member_Request<Derived>::value, "IRealityDataSource implementation must have a static RealityDataSourceResult Request(Data&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&) function.");
        }
    };

//======================================================================================
//! The response of reality data source.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct RealityDataSourceResponse
{
private:
    RealityDataSourceResult     m_result;
    Utf8String                  m_id;
    IRealityDataBase*           m_data;
public:
    RealityDataSourceResponse(RealityDataSourceResult result, Utf8CP id, IRealityDataBase* data = nullptr) : m_result(result), m_id(id), m_data(data) {}
    RealityDataSourceResult GetResult() const {return m_result;}
    Utf8CP GetId() const {return m_id.c_str();}
    IRealityDataBase* GetData() const {return m_data;}
};

//======================================================================================
//! Interface for a reality data source response receiver.
// @bsiclass                                        Grigas.Petraitis           03/2015
//======================================================================================
struct IRealityDataSourceResponseReceiver : RefCountedBase
    {
    //! Called to notify the receiver with a response.
    virtual void _OnResponseReceived(RealityDataSourceResponse const&) = 0;
    };

//=======================================================================================
//! Options for @ref RealityDataCache::Get request function.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataCacheOptions : virtual IRefCounted
    {
    private:
        bool m_returnExpired;
        bool m_requestFromSource;

    public:
        //! Creates the options object.
        //! @param[in] returnExpired        Should the cache return reality data even if it's expired.
        //! @param[in] requestFromSource    Should the cache request for fresh data if it's expired or doesn't exist.
        RealityDataCacheOptions(bool returnExpired, bool requestFromSource) : m_returnExpired(returnExpired), m_requestFromSource(requestFromSource) {}
        
        //! Should the cache return reality data even if it's expired.
        bool ReturnExpired() const {return m_returnExpired;}
        void SetReturnExpired(bool value) {m_returnExpired = value;}
        
        //! Should the cache request for fresh data if it's expired or doesn't exist.
        bool RequestFromSource() const {return m_requestFromSource;}
        void SetRequestFromSource(bool value) {m_requestFromSource = value;}
    };

//=======================================================================================
//! The reality data cache class which manages two types of objects - storages and sources.
//! Storages are responsible for caching the reality data and returning it quickly on demand.
//! Sources are responsible for retrieving the reality data from its primary source, e.g.
//! the internet.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataCache : NonCopyableClass, IRealityDataSourceResponseReceiver
{    
//__PUBLISH_SECTION_END__
private:
    bmap<Utf8String, IRealityDataStorageBasePtr> m_storages;
    bmap<Utf8String, IRealityDataSourceBasePtr>  m_sources;
    bmap<Utf8String, bset<IRealityDataStoragePersistHandler const*>> m_persistHandlers;
    BeMutex m_persistHandlersCS;
    
    // note: this might be called from any thread!
    virtual void _OnResponseReceived(RealityDataSourceResponse const& response) override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
private:
    DGNPLATFORM_EXPORT IRealityDataStorageBase* GetStorage(Utf8CP id) const;
    DGNPLATFORM_EXPORT IRealityDataSourceBase* GetSource(Utf8CP id) const;
    template<typename StorageType> StorageType* GetStorage() const {return(StorageType*)GetStorage(StorageType::StorageId());}
    template<typename SourceType> SourceType* GetSource() const {return(SourceType*)GetSource(SourceType::SourceId());}

    DGNPLATFORM_EXPORT void QueuePersistHandler(Utf8CP id, IRealityDataStoragePersistHandler const&);
    DGNPLATFORM_EXPORT IRealityDataStoragePersistHandler const* DequeuePersistHandler(Utf8CP id, IRealityDataBase const*, bool dealloc = false);

    template<typename DataType> void RequestData(DataType& data, Utf8CP id, typename DataType::RequestOptions const& options)
        {
        QueuePersistHandler(id, *new typename DataType::StorageType::PersistHandler(*GetStorage<typename DataType::StorageType>(), data));
        if (RealityDataSourceResult::Success != GetSource<typename DataType::SourceType>()->Request(data, id, options, *this))
            DequeuePersistHandler(id, &data, true);
        }

public:
    //! Create a new reality data cache.
    DGNPLATFORM_EXPORT static RealityDataCachePtr Create();

    //! Register a storage in this cache.
    DGNPLATFORM_EXPORT void RegisterStorage(IRealityDataStorageBase& storage);

    //! Register a source in this cache.
    DGNPLATFORM_EXPORT void RegisterSource(IRealityDataSourceBase& source);

    //! Request reality data from this cache. The type of the reality data is provided as a template parameter.
    //! @param[in] id       The id of the requested data.
    //! @param[in] options  The request options.
    template<typename DataType> RefCountedPtr<DataType> Get(Utf8CP id, typename DataType::RequestOptions const& options)
        {
        RefCountedPtr<DataType> data = DataType::Create();
        RealityDataStorageResult result = GetStorage<typename DataType::StorageType>()->Select(*data, id, options);
        switch(result)
            {
            case RealityDataStorageResult::Success:
                {
                data->SetInitialized();
                if (data->IsExpired())
                    {
                    if (options.RequestFromSource())
                        RequestData(*data, id, options);
                    if (!options.ReturnExpired())
                        return nullptr;
                    }
                return data;
                }
            case RealityDataStorageResult::NotFound:
                {
                if (options.RequestFromSource())
                    RequestData(*data, id, options);
                return nullptr;
                }
            case RealityDataStorageResult::Error:
                BeAssert(false);
                return nullptr;
            }
        return nullptr;
        }
};

//=======================================================================================
//! Reality data storage which uses a SQLite database for storing cached data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct BeSQLiteRealityDataStorage : IRealityDataStorage<BeSQLiteRealityDataStorage>, NonCopyableClass
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct SelectOptions : virtual IRefCounted
        {
        protected:
            virtual ~SelectOptions() {}
        };
    
    //===================================================================================
    //! Interface for a class which prepares the database for storing specific type of data
    //! and knows how to clean it up.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct DatabasePrepareAndCleanupHandler : RefCountedBase
        {
        //__PUBLISH_SECTION_END__
        struct Comparer
            {
            bool operator()(DatabasePrepareAndCleanupHandler const* lhs, DatabasePrepareAndCleanupHandler const* rhs) const
                {
                size_t lhs_hash = typeid(*lhs).hash_code();
                size_t rhs_hash = typeid(*rhs).hash_code();
                return lhs_hash < rhs_hash;
                }
            };

        //__PUBLISH_SECTION_START__
        friend struct BeSQLiteRealityDataStorage;
        protected:
            //! Called to prepare the database for storing specific type of data (the type 
            //! must be known to the implementation).
            virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const = 0;

            //! Called to free some space in the database.
            //! @param[in] db           The database to cleanup.
            //! @param[in] percentage   The percentage of data that should be freed.
            virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db, double percentage) const = 0;
        };
    typedef RefCountedPtr<DatabasePrepareAndCleanupHandler> DatabasePrepareAndCleanupHandlerPtr;

    //===================================================================================
    //! Interface for BeSQLiteRealityDataStorage data. All reality data using this kind of 
    //! storage must implement this interface.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : IRealityDataStorage::Data
        {
        friend struct BeSQLiteRealityDataStorage;
        protected:
            //! Return @ref DatabasePrepareAndCleanupHandler which knows how to prepare and cleanup
            //! database for this data type.
            virtual DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const = 0;

            //! Initialize data from the database using key and select options.
            virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP key, SelectOptions const& options) = 0;

            //! Persist this data in the database.
            virtual BentleyStatus _Persist(BeSQLite::Db& db) const = 0;
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
    protected:
        DGNPLATFORM_EXPORT virtual RealityDataStorageResult _Persist() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        PersistHandler(BeSQLiteRealityDataStorage& storage, Data const& data) : m_storage(storage), m_data(&data) {}        
    };

    //__PUBLISH_SECTION_END__
    struct CleanAndSaveChangesWork;
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct WorkerThread : RealityDataWorkerThread
    {
    friend struct CleanAndSaveChangesWork;
    private:
        ThreadSafeQueue<RealityDataWorkPtr> m_workQueue;
        BeSQLiteRealityDataStorage*         m_storage;
    protected:
        WorkerThread(BeSQLiteRealityDataStorage& storage, IStateListener* listener) 
            : RealityDataWorkerThread(listener, "BentleyBeSQLiteRealityDataStorageWorker"), m_storage(&storage) 
            {}
        virtual void _DoWork(RealityDataWork& work) override;
        virtual void _OnIdle() override;
    public:
        static RefCountedPtr<WorkerThread> Create(BeSQLiteRealityDataStorage& storage, IStateListener* listener) {return new WorkerThread(storage, listener);}
    };
    typedef RefCountedPtr<WorkerThread> WorkerThreadPtr;
        
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct WorkerThreadIdleTracker : RefCounted<RealityDataWorkerThread::IStateListener>
    {
    private:
        BeSQLiteRealityDataStorage& m_storage;
        WorkerThreadIdleTracker(BeSQLiteRealityDataStorage& storage) : m_storage(storage) {} 

    protected:
        virtual void _OnThreadIdle(RealityDataWorkerThread& thread) override;

    public:
        static RefCountedPtr<WorkerThreadIdleTracker> Create(BeSQLiteRealityDataStorage& storage) {return new WorkerThreadIdleTracker(storage);}
    };
    typedef RefCountedPtr<WorkerThreadIdleTracker> WorkerThreadIdleTrackerPtr;

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
        Utf8CP  m_id;
        Data&  m_data;
        SelectOptions const& m_options;

        bool                             m_hasResult;
        mutable RealityDataStorageResult m_result;
        mutable BeConditionVariable      m_resultCV;
    protected:
        SelectDataWork(BeSQLiteRealityDataStorage& storage, Utf8CP id, Data& data, SelectOptions const& options) : StorageWork(storage), m_id(id), m_data(data), m_options(options), m_hasResult(false) {} 
        virtual void _DoWork() override;
    public:
        static RefCountedPtr<SelectDataWork> Create(BeSQLiteRealityDataStorage& storage, Utf8CP id, Data& data, SelectOptions const& options) {return new SelectDataWork(storage, id, data, options);}
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
    bool                        m_initialized;
    bset<DatabasePrepareAndCleanupHandler const*, DatabasePrepareAndCleanupHandler::Comparer> m_cleanupHandlers;
    BeFileName                  m_filename;
    WorkerThreadPtr             m_worker;
    WorkerThreadIdleTrackerPtr  m_idleTracker;
    bool                        m_hasChanges;
    BeSQLite::Db&               m_database;
    uint32_t m_idleTime; 
    uint64_t m_cacheSize;

private:
    BeSQLiteRealityDataStorage(BeFileName const& filename, uint32_t idleTime, uint64_t cacheSize);
    bool                     wt_Prepare(DatabasePrepareAndCleanupHandler const& prepareHandler);
    void                     wt_Cleanup();
    void                     wt_Persist(Data const& data);
    RealityDataStorageResult wt_Select(Data& data, Utf8CP id, SelectOptions const& options);
    void                     wt_SaveChanges();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    virtual Utf8CP _GetStorageId() const override {return StorageId();}

public:
    //! The id of this storage.
    static Utf8CP StorageId() {return "BeSQLite";}

    //! Creates a new BeSQLiteRealityDataStorage.
    //! @param[in] filename     the file to create
    //! @param[in] idleTime     Time (in miliseconds) for the worker thread to wait before commiting changes to the database.
    //! @param[in] cacheSize    Sets max size (in bytes) which is allowed for RealityDataCache database.
    DGNPLATFORM_EXPORT static BeSQLiteRealityDataStoragePtr Create(BeFileName const& filename, uint32_t idleTime = 5000, uint64_t cacheSize = 0);
    DGNPLATFORM_EXPORT ~BeSQLiteRealityDataStorage();

    //! Initialize data object with the data in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Select(Data& data, Utf8CP id, SelectOptions const& options);

    //! Persist the data object in the database.
    DGNPLATFORM_EXPORT RealityDataStorageResult Persist(Data const& data);
};

//=======================================================================================
//! Reality data storage which stores reality data in memory.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct InMemoryRealityDataStorage : IRealityDataStorage<InMemoryRealityDataStorage>, NonCopyableClass
    {
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct SelectOptions : virtual IRefCounted
    {
    private:
        bool m_removeAfterSelect;
    public:
        virtual ~SelectOptions() {}
        bool GetRemoveAfterSelect() const {return m_removeAfterSelect;}
        void SetRemoveAfterSelect(bool value) {m_removeAfterSelect = value;}
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct Data : IRealityDataStorage::Data
        {
        friend struct InMemoryRealityDataStorage;
        protected:
            //! Initialize data from the void pointer and select options.
            virtual BentleyStatus _InitFrom(Utf8CP id, void const* data, SelectOptions const& options) = 0;
        };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        04/2015
    //===================================================================================
    struct PersistHandler : IRealityDataStoragePersistHandler
    {
    private:
        InMemoryRealityDataStorage& m_storage;
        RefCountedPtr<Data const>   m_data;
    protected:
        DGNPLATFORM_EXPORT virtual RealityDataStorageResult _Persist() const override;
        DGNPLATFORM_EXPORT virtual IRealityDataBase const* _GetData() const override;
    public:
        PersistHandler(InMemoryRealityDataStorage& storage, Data const& data) : m_storage(storage), m_data(&data) {}    
    };

//__PUBLISH_SECTION_END__
private:
    bmap<Utf8String, void const*> m_map;
    BeMutex m_cs;

private:
    //! Persist the data object in memory.
    DGNPLATFORM_EXPORT RealityDataStorageResult Persist(Data const& data);
    
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    virtual Utf8CP _GetStorageId() const override {return StorageId();}

public:
    //! The id of this storage.
    static Utf8CP StorageId() {return "InMemory";}
    
    //! Creates a new instance of InMemoryRealityDataStorage.
    DGNPLATFORM_EXPORT static InMemoryRealityDataStoragePtr Create();
    DGNPLATFORM_EXPORT RealityDataStorageResult Select(Data& data, Utf8CP id, SelectOptions const& options);
};

//===================================================================================
//! A request interface for @ref AsyncRealityDataSource.
// @bsiclass                                        Grigas.Petraitis        03/2015
//===================================================================================
struct AsyncRealityDataSourceRequest : RefCountedBase
{
protected:
    //! Return the if of reality data which is handled by this request.
    virtual Utf8CP _GetId() const = 0;

    //! Handle the request (e.g. read file content and initialize reality data from it).
    virtual RealityDataSourceResponse _Handle() const = 0;

//__PUBLISH_SECTION_END__
public:
    Utf8CP GetId() const;
    RealityDataSourceResponse Handle() const;
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! Base class for asynchronous reality data sources.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
template<class Derived>
struct AsyncRealityDataSource : IRealityDataSource<Derived>, NonCopyableClass
{
//__PUBLISH_SECTION_END__
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestHandler : RefCounted<RealityDataWork>
    {
    private:
        RefCountedPtr<AsyncRealityDataSource> m_source;
        RefCountedPtr<AsyncRealityDataSourceRequest const> m_request;
        IRealityDataSourceResponseReceiverPtr m_responseReceiver;
        
        RequestHandler(AsyncRealityDataSource& source, AsyncRealityDataSourceRequest const& request, IRealityDataSourceResponseReceiver& responseReceiver)
            : m_source(&source), m_request(&request), m_responseReceiver(&responseReceiver)
            {}
        void SendResponse(RealityDataSourceResponse const& response) {m_responseReceiver->_OnResponseReceived(response);}

    protected:
        virtual void _DoWork() override;

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
    BeAtomic<uint64_t>       m_ignoreRequestsUntil;

private:
    void SetIgnoreRequests(uint32_t ignoreTime);
    bool ShouldIgnoreRequests() const;

protected:
    AsyncRealityDataSource(int numThreads) : m_threadPool(RealityDataThreadPool::Create(numThreads, 0)), m_ignoreRequestsUntil(0) {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    //! Queues a request for handling on the work thread. Derived classes should use this
    //! function to queue their requests to be handled asynchronously.
    RealityDataSourceResult QueueRequest(AsyncRealityDataSourceRequest const& request, IRealityDataSourceResponseReceiver& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses files (on the disc) as the source of reality data.
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct FileRealityDataSource : AsyncRealityDataSource<FileRealityDataSource>
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct RequestOptions : virtual IRefCounted
        {
        public:
            virtual ~RequestOptions() {}
        };
    
    //===================================================================================
    //! Interface for FileRealityDataSource data. All reality data using this kind of 
    //! source must implement this interface.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : IRealityDataSource::Data
        {
        //__PUBLISH_SECTION_END__
        friend struct ::FileRealityDataSourceRequest;
        //__PUBLISH_SECTION_START__
        friend struct FileRealityDataSource;
        protected:
            //! Initializes this data with file path, content and request options.
            //! @param[in] filepath     The path of the file that the data was read from.
            //! @param[in] data         The file content.
            //! @param[in] options      The request options.
            virtual BentleyStatus _InitFrom(Utf8CP filepath, bvector<Byte> const& data, RequestOptions const& options) = 0;
        };

//__PUBLISH_SECTION_END__
private:
    FileRealityDataSource(int numThreads) : AsyncRealityDataSource(numThreads) {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    virtual Utf8CP _GetSourceId() const override {return SourceId();}
public:
    //! The id of this reality data source.
    static Utf8CP SourceId() {return "File";}

    //! Create a new instance of FileRealityDataSource.
    //! @param[in] numThreads   Number of worker threads to use for reading files. Must be at least 1.
    DGNPLATFORM_EXPORT static RefCountedPtr<FileRealityDataSource> Create(int numThreads);

    //! Request the data to be initialized from the specified file.
    //! @param[in] data             The reality data to initialize.
    //! @param[in] filepath         The path of the file to initialize data from.
    //! @param[in] options          The request options.
    //! @param[in] responseReceiver The object that will receive the response of this request.
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data& data, Utf8CP filepath, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver);
};

//=======================================================================================
//! Reality data source which uses HTTP protocol to download content and use it as 
//! the source of reality data.
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct HttpRealityDataSource : AsyncRealityDataSource<HttpRealityDataSource>
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct RequestOptions : virtual IRefCounted
        {
        protected:
            virtual ~RequestOptions() {}
        };
    
    //===================================================================================
    //! Interface for HttpRealityDataSource data. All reality data using this kind of 
    //! source must implement this interface.
    //! @note Implementation must set expiration date and entity tag (see @ref SetExpirationDate
    //! and @ref SetEntityTag) when it initializes the data from the reality data storage.
    // @bsiclass                                        Grigas.Petraitis        03/2015
    //===================================================================================
    struct Data : IRealityDataSource::Data
        {
        //__PUBLISH_SECTION_END__
        friend struct ::HttpRealityDataSourceRequest;
        //__PUBLISH_SECTION_START__
        friend struct HttpRealityDataSource;
        private:
            DateTime    m_expirationDate;
            Utf8String  m_entityTag;
            void ParseExpirationDateAndETag(bmap<Utf8String, Utf8String> const& header);
        protected:
            DGNPLATFORM_EXPORT void SetExpirationDate(DateTime const& date);
            DGNPLATFORM_EXPORT void SetEntityTag(Utf8CP eTag);
            virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, RequestOptions const& options) = 0;
        public:
            DGNPLATFORM_EXPORT DateTime const& GetExpirationDate() const;
            DGNPLATFORM_EXPORT Utf8CP GetEntityTag() const;
        };
        
//__PUBLISH_SECTION_END__
private:
    bool m_initialized;
    
private:
    HttpRealityDataSource(int numThreads) : AsyncRealityDataSource(numThreads), m_initialized(false) {}
    BentleyStatus Initialize();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    virtual Utf8CP _GetSourceId() const override {return SourceId();}

public:
    //! The id of this reality data source.
    static Utf8CP SourceId() {return "Http";}
    
    //! Create a new instance of HttpRealityDataSource.
    DGNPLATFORM_EXPORT static HttpRealityDataSourcePtr Create(int numThreads);
    
    //! Request the data to be initialized from the content at the specified url
    //! @param[in] data             The reality data to initialize.
    //! @param[in] url              The URL to download content from.
    //! @param[in] options          The request options.
    //! @param[in] responseReceiver The object that will receive the response of this request.
    DGNPLATFORM_EXPORT RealityDataSourceResult Request(Data& data, Utf8CP url, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver);
};
END_BENTLEY_DGN_NAMESPACE
