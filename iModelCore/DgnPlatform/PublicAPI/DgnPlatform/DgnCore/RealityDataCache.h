/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RealityDataCache.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnCore.h"

#define TABLE_NAME_TiledRaster  "TiledRaster"

DGNPLATFORM_REF_COUNTED_PTR (IRealityData)
DGNPLATFORM_REF_COUNTED_PTR (IRealityDataStorage)
DGNPLATFORM_REF_COUNTED_PTR (IRealityDataSource)
DGNPLATFORM_REF_COUNTED_PTR (BeSQLiteRealityDataStorage)
DGNPLATFORM_REF_COUNTED_PTR (IHttpRequestOptionsProvider)
DGNPLATFORM_REF_COUNTED_PTR (RealityDataCache)
DGNPLATFORM_REF_COUNTED_PTR (HttpRealityDataSource)

DGNPLATFORM_REF_COUNTED_PTR (RealityDataThread)
DGNPLATFORM_REF_COUNTED_PTR (RealityDataThreadPool)
DGNPLATFORM_REF_COUNTED_PTR (RealityDataWork)
DGNPLATFORM_REF_COUNTED_PTR (RealityDataWorkerThread)

struct HasWorkOrTerminatesPredicate;
struct IsIdlePredicate;
struct AllThreadsIdlePredicate;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class RealityDataType
    {
    TiledRaster,
    };

//=======================================================================================
// @bsiclass                                        Sam.Wilson                  11/2014
//=======================================================================================
struct RealityDataGetCachedOption
    {
    bool m_evenIfExpired;
    RealityDataGetCachedOption (bool evenIfExpired=false) : m_evenIfExpired(evenIfExpired) {;}
    bool ShouldReturnExpired() const {return m_evenIfExpired;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IHttpRequestOptionsProvider : IRefCounted
{
friend struct IRealityDataSource;
protected:
    virtual void _ProvideHttpRequestOptions (bmap<Utf8String, Utf8String>& options) {};
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IRealityData : IHttpRequestOptionsProvider
    {
    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            10/2014
    //===================================================================================
    struct RequestOptions : RefCountedBase
        {
        virtual ~RequestOptions() {}
        };
    typedef RefCountedPtr<RequestOptions> RequestOptionsPtr;

    // Factory
    static IRealityDataPtr  Create (RealityDataType type);
    
    // Http-specific
protected:
    virtual void            _ProvideHttpRequestOptions (bmap<Utf8String, Utf8String>& options) {};
public:
    virtual BentleyStatus   InitFrom (Utf8CP key, bmap<Utf8String, Utf8String> const& header, bvector<byte> const& body, RequestOptions const& options) { return ERROR; }
    virtual BentleyStatus   UpdateFrom (bmap<Utf8String, Utf8String> const& header) { return ERROR; }

    // BeSQLite-specific
    virtual BentleyStatus   InitFrom (BeSQLite::Db& db, Utf8CP key) { return ERROR; }
    virtual BentleyStatus   Persist (BeSQLite::Db& db) const { return ERROR; }

    // General
    virtual RealityDataType GetDataType () const = 0;
    virtual Utf8CP          GetKey () const = 0;
    virtual bool            IsExpired () const = 0;
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
template<typename T> struct RealityDataQueue
{
    struct Item;
    typedef RefCountedPtr<Item> ItemPtr;
    struct Item : RefCountedBase
        {
        ItemPtr prev;
        ItemPtr next;
        T       data;
        Item (T const& d, Item* p, Item* n) : prev (p), next (n), data (d) {}
        static ItemPtr Create (T const& d, Item* p, Item* n) { return new Item (d, p, n); }
        };

private:
    BeConditionVariable m_cv;
    ItemPtr m_first;
    ItemPtr m_last;

public:
    BeConditionVariable& GetConditionVariable () { return m_cv; }

public:
    DGNPLATFORM_EXPORT void Push (T const& element);
    DGNPLATFORM_EXPORT bool Pop (T& element);
    DGNPLATFORM_EXPORT void Clear ();
    DGNPLATFORM_EXPORT bool IsEmpty () const;
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
    static void ThreadRunner (void* arg);
#if defined (__unix__)
    static void* PlatformThreadRunner (void* arg) { ThreadRunner (arg); return NULL; }
#else
    static unsigned __stdcall PlatformThreadRunner (void* arg) { ThreadRunner (arg); return 0; }
#endif
    
protected:
    DGNPLATFORM_EXPORT RealityDataThread (Utf8CP threadName = NULL);
    virtual void _Run () = 0;

public:
    DGNPLATFORM_EXPORT intptr_t GetThreadId () const;
    DGNPLATFORM_EXPORT void     Run ();   //!< Call this to invoke the _Run method in the current thread
    DGNPLATFORM_EXPORT void     Start (); //!< Call this to start the thread
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataWork : IRefCounted
{
friend struct RealityDataWorkerThread;
protected:
    virtual void _DoWork () = 0;
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
        virtual void _OnThreadBusy (RealityDataWorkerThread& thread) { };
        virtual void _OnThreadIdle (RealityDataWorkerThread& thread) { };
    };
    typedef RefCountedPtr<IStateListener> IStateListenerPtr;

private:
    mutable BeCriticalSection   m_cs;
    BeConditionVariable         m_cv;
    bool                        m_terminate;
    RealityDataWorkPtr          m_currentWork;
    UInt64                      m_idleSince;
    UInt64                      m_busySince;
    IStateListenerPtr           m_stateListener;

    void SetIsBusy (bool);
    bool TerminateRequested () const;

protected:
    DGNPLATFORM_EXPORT RealityDataWorkerThread (IStateListener* stateListener, Utf8CP threadName);
    DGNPLATFORM_EXPORT virtual void _OnBusy ();
    DGNPLATFORM_EXPORT virtual void _OnIdle ();
    DGNPLATFORM_EXPORT virtual void _DoWork (RealityDataWork& work);
                       virtual void _Run () override;

public:
                       void DoWork (RealityDataWork& work) { _DoWork (work); }
    DGNPLATFORM_EXPORT bool IsBusy (UInt64* busyTime = NULL) const;
    DGNPLATFORM_EXPORT bool IsIdle (UInt64* idleTime = NULL) const;
    DGNPLATFORM_EXPORT void Terminate ();
    //! Create a new RealityDataWorkerThread thread.
    //! note This function does not start the new thrdad. You must call the Start method on the returned thread object in order to start it.
    static RealityDataWorkerThreadPtr Create (IStateListener* stateListener = NULL, Utf8CP threadName = NULL) { return new RealityDataWorkerThread (stateListener, threadName); }
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
    mutable BeCriticalSection   m_threadsCS;
    mutable BeConditionVariable m_threadsCV;
    mutable BeCriticalSection   m_workQueueCS;
    bmap<RealityDataWorkerThread*, bool> m_threads;
    RealityDataQueue<RealityDataWorkPtr> m_workQueue;

private:
    RealityDataThreadPool (int maxThreads, int maxIdleThreads) : m_maxThreads (maxThreads), m_maxIdleThreads (maxIdleThreads), m_threadsCV (&m_threadsCS) {}
    RealityDataWorkerThreadPtr CreateThread ();
    RealityDataWorkerThreadPtr GetIdleThread () const;
    bool ShouldCreateNewThread () const;

protected:
    DGNPLATFORM_EXPORT virtual void _OnThreadBusy (RealityDataWorkerThread& thread) override;
    DGNPLATFORM_EXPORT virtual void _OnThreadIdle (RealityDataWorkerThread& thread) override;

public:
    //! NOT PUBLISHED: needed for tests
    DGNPLATFORM_EXPORT void WaitUntilAllThreadsIdle () const;

public:
    static RealityDataThreadPoolPtr Create (int maxThreads, int maxIdleThreads) { return new RealityDataThreadPool (maxThreads, maxIdleThreads); }
    DGNPLATFORM_EXPORT virtual ~RealityDataThreadPool ();
    DGNPLATFORM_EXPORT int  GetThreadsCount () const;
    DGNPLATFORM_EXPORT void QueueWork (RealityDataWork& work);
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
enum class RealityDataStorageResult
    {
    Error,
    NotFound,
    Expired,
    Success,
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IRealityDataStorage : IRefCounted
{
public:
    virtual RealityDataStorageResult Select (IRealityDataPtr& data, RealityDataType type, Utf8CP id) = 0;
    virtual BentleyStatus            Persist (IRealityData const& data) = 0;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct BeSQLiteRealityDataStorage : RefCounted<IRealityDataStorage>, BeSQLite::Db
{
    struct SaveChangesWork;

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct WorkerThread : RealityDataWorkerThread
    {
    friend struct SaveChangesWork;

    private:
        RealityDataQueue<RealityDataWorkPtr> m_workQueue;
        BeSQLiteRealityDataStoragePtr        m_storage;

    protected:
        WorkerThread (BeSQLiteRealityDataStorage& storage, IStateListener* listener) 
            : RealityDataWorkerThread (listener, "BentleyBeSQLiteRealityDataStorageWorker"), m_storage (&storage) 
            {}
        virtual void _DoWork (RealityDataWork& work) override;
        virtual void _OnIdle () override;

    public:
        static RefCountedPtr<WorkerThread> Create (BeSQLiteRealityDataStorage& storage, IStateListener* listener) { return new WorkerThread (storage, listener); }
    };
    typedef RefCountedPtr<WorkerThread> WorkerThreadPtr;
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct WorkerThreadIdleTracker : RefCounted<RealityDataWorkerThread::IStateListener>
    {
    private:
        BeSQLiteRealityDataStorage& m_storage;
        WorkerThreadIdleTracker (BeSQLiteRealityDataStorage& storage) : m_storage (storage) { } 

    protected:
        virtual void _OnThreadIdle (RealityDataWorkerThread& thread) override;

    public:
        static RefCountedPtr<WorkerThreadIdleTracker> Create (BeSQLiteRealityDataStorage& storage) { return new WorkerThreadIdleTracker (storage); }
    };
    typedef RefCountedPtr<WorkerThreadIdleTracker> WorkerThreadIdleTrackerPtr;

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct StorageWork : RefCounted<RealityDataWork>
    {
    protected:
        BeSQLiteRealityDataStoragePtr m_storage;
        StorageWork (BeSQLiteRealityDataStorage& storage) : m_storage (&storage) { }
    };

    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct SelectDataWork : StorageWork
    {
    private:
        RealityDataType     m_type;
        Utf8CP              m_id;
        IRealityDataPtr&    m_data;
        bool                             m_hasResult;
        mutable RealityDataStorageResult m_result;
        mutable BeConditionVariable      m_resultCV;
    protected:
        SelectDataWork (BeSQLiteRealityDataStorage& storage, RealityDataType type, Utf8CP id, IRealityDataPtr& data) : StorageWork (storage), m_type (type), m_id (id), m_data (data), m_hasResult (false) { } 
        virtual void _DoWork () override;
    public:
        static RefCountedPtr<SelectDataWork> Create (BeSQLiteRealityDataStorage& storage, RealityDataType type, Utf8CP id, IRealityDataPtr& data) { return new SelectDataWork (storage, type, id, data); }
        RealityDataStorageResult GetResult () const;
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct PersistDataWork : StorageWork
    {
    private:
        RefCountedPtr<IRealityData const> m_data;
    protected:
        PersistDataWork (BeSQLiteRealityDataStorage& storage, IRealityData const& data) : StorageWork (storage), m_data (&data) { } 
        virtual void _DoWork () override;
    public:
        static RefCountedPtr<PersistDataWork> Create (BeSQLiteRealityDataStorage& storage, IRealityData const& data) { return new PersistDataWork (storage, data); }
    };
    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        11/2014
    //===================================================================================
    struct SaveChangesWork : StorageWork
    {
    private:
        UInt32          m_idleTime;
    protected:
        SaveChangesWork (BeSQLiteRealityDataStorage& storage, UInt32 idleTime) : StorageWork (storage), m_idleTime (idleTime) { } 
        virtual void _DoWork () override;
    public:
        static RefCountedPtr<SaveChangesWork> Create (BeSQLiteRealityDataStorage& storage, UInt32 idleTime) { return new SaveChangesWork (storage, idleTime); }
    };

private:
    bool                        m_initialized;
    BeFileName                  m_filename;
    WorkerThreadPtr             m_worker;
    WorkerThreadIdleTrackerPtr  m_idleTracker;
    bool                        m_hasChanges;

private:
    void                     wt_Initialize ();
    void                     wt_Persist (IRealityData const& data);
    RealityDataStorageResult wt_Select (RealityDataType type, Utf8CP id, IRealityDataPtr& data);
    void                     wt_SaveChanges ();
    
protected:
    BeSQLiteRealityDataStorage (BeFileNameCR filename)
        : m_initialized (false), m_filename (filename), m_hasChanges (false)
        {
        m_idleTracker = WorkerThreadIdleTracker::Create (*this);
        m_worker = WorkerThread::Create (*this, m_idleTracker.get ());
        m_worker->Start();
        }

public:
    static BeSQLiteRealityDataStoragePtr Create (BeFileNameCR filename) { return new BeSQLiteRealityDataStorage (filename); }
    virtual ~BeSQLiteRealityDataStorage ();

    // IRealityDataStorage implementation:
    virtual RealityDataStorageResult Select (IRealityDataPtr& data, RealityDataType type, Utf8CP id) override;
    virtual BentleyStatus            Persist (IRealityData const& data) override;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataSourceResponse
{
    enum class Status
        {
        Error_Unknown,
        Error_CouldNotResolveHost,
        Error_NoConnection,
        Error_GatewayTimeout,
        Error_NotFound,
        NotModified,
        Success,
        };

private:
    Status                              m_status;
    bmap<Utf8String, Utf8String> const* m_header;
    IRealityDataPtr                     m_realityData;

public:
    RealityDataSourceResponse (Status status) : m_status (status), m_header (NULL) {}
    RealityDataSourceResponse (Status status, bmap<Utf8String, Utf8String> const& header) : m_status (status), m_header (&header) {}
    RealityDataSourceResponse (Status status, IRealityDataPtr realityData) : m_status (status), m_header (NULL), m_realityData (realityData) {}
    Status                              GetStatus () const { return m_status; }
    bmap<Utf8String, Utf8String> const* GetHeader () const { return m_header; }
    IRealityDataPtr                     GetRealityData () const { return m_realityData; }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IRealityDataSource : IRefCounted
    {
    struct IResponseReceiver : IRefCounted
        {
        friend struct IRealityDataSource;
        protected:
            virtual void _OnResponseReceived (RealityDataType type, Utf8CP id, RealityDataSourceResponse const& response) = 0;
        };
    typedef RefCountedPtr<IResponseReceiver> IResponseReceiverPtr;
    
protected:
    void SendResponse (IResponseReceiver& receiver, RealityDataType type, Utf8CP id, RealityDataSourceResponse const& response) { receiver._OnResponseReceived (type, id, response); }
    void GetHttpRequestOptions (IHttpRequestOptionsProvider& provider, bmap<Utf8String, Utf8String>& options) { provider._ProvideHttpRequestOptions (options); }

public:
    virtual BentleyStatus Request (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& requestOptions, IHttpRequestOptionsProvider* optionsProvider, IResponseReceiver& callbackReceiver) = 0;
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct HttpRealityDataSource : RefCounted<IRealityDataSource>
{    
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestHandler : RefCounted<RealityDataWork>
    {
    private:
        HttpRealityDataSourcePtr        m_source;
        RealityDataType                 m_type;
        Utf8String                      m_url;
        IHttpRequestOptionsProviderPtr  m_optionsProvider;
        IResponseReceiverPtr            m_responseReceiver;
        RefCountedPtr<IRealityData::RequestOptions const> m_requestOptions;

        RequestHandler (HttpRealityDataSource& source, RealityDataType type, Utf8CP url, IRealityData::RequestOptions const& requestOptions, IHttpRequestOptionsProvider* optionsProvider, IResponseReceiver& responseReceiver)
            : m_source (&source), m_type (type), m_url (url), m_requestOptions (&requestOptions), m_optionsProvider (optionsProvider), m_responseReceiver (&responseReceiver)
            { }

        void SendResponse (RealityDataSourceResponse const& response) { m_source->SendResponse (*m_responseReceiver, m_type, m_url.c_str (), response); }

    protected:
        virtual void _DoWork () override;

    public:
        static RefCountedPtr<RequestHandler> Create (HttpRealityDataSource& source, RealityDataType type, Utf8CP url, IRealityData::RequestOptions const& requestOptions, IHttpRequestOptionsProvider* optionsProvider, IResponseReceiver& responseReceiver) 
            {
            return new RequestHandler (source, type, url, requestOptions, optionsProvider, responseReceiver); 
            }
    };

private:
    bool                     m_initialized;
    RealityDataThreadPoolPtr m_threadPool;
    BeCriticalSection        m_requestsCS;
    bmap<Utf8String, bool>   m_activeRequests;
    BeAtomic<UInt64>         m_ignoreRequestsUntil;

private:
    BentleyStatus Initialize ();
    void          SetIgnoreRequests (UInt32 ignoreTime);

protected:
    HttpRealityDataSource ()
        : m_initialized (false), m_ignoreRequestsUntil (0), m_threadPool (RealityDataThreadPool::Create (8, 0))
        {}

public:
    static RefCountedPtr<HttpRealityDataSource> Create () { return new HttpRealityDataSource (); }

    // IRealityDataSource implementation
    virtual BentleyStatus Request (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& requestOptions, IHttpRequestOptionsProvider* optionsProvider, IResponseReceiver& callbackReceiver) override;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
template<class T> struct RefCountedRealityDataQueue : RealityDataQueue<T>, RefCountedBase 
    {
    static RefCountedPtr<RefCountedRealityDataQueue> Create () { return new RefCountedRealityDataQueue (); }
    virtual ~RefCountedRealityDataQueue () {}
    };
typedef RefCountedPtr<RefCountedRealityDataQueue<Utf8String>> StringRealityDataQueuePtr;
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IRealityDataCache
{
protected:
    virtual IRealityDataPtr             _GetCached (RealityDataType type, Utf8CP id, RealityDataGetCachedOption const&) = 0;
    virtual IRealityDataPtr             _Get (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& options, RealityDataGetCachedOption const&) = 0;
    virtual StringRealityDataQueuePtr   _Arrivals (RealityDataType type) = 0;

public:
    virtual ~IRealityDataCache(){}
    template<class T> IRealityDataPtr           GetCached (Utf8CP id, RealityDataGetCachedOption const& copt=RealityDataGetCachedOption()) { return _GetCached (T::Type (), id, copt); }
    template<class T> IRealityDataPtr           Get (Utf8CP id, typename T::RequestOptions const& options, RealityDataGetCachedOption const& copt=RealityDataGetCachedOption()) { return _Get (T::Type (), id, options, copt); }
    template<class T> StringRealityDataQueuePtr Arrivals () { return _Arrivals (T::Type ()); }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct RealityDataCache : IRealityDataCache
{
    struct ResponseReceiver;

private:
    IRealityDataStorage&        m_storage;
    IRealityDataSource&         m_source;
    ResponseReceiver*           m_responseReceiver;
    BeCriticalSection                                m_arrivalsCS;
    bmap<RealityDataType, StringRealityDataQueuePtr> m_arrivals;
    
private:
    void _OnResponseReceived (RealityDataType type, Utf8CP id, RealityDataSourceResponse const& response);

protected:
    // IRealityDataCache implementation:
    virtual IRealityDataPtr             _GetCached (RealityDataType type, Utf8CP id, RealityDataGetCachedOption const&) override;
    virtual IRealityDataPtr             _Get (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& options, RealityDataGetCachedOption const&) override;
    virtual StringRealityDataQueuePtr   _Arrivals (RealityDataType type) override;

public:
    RealityDataCache (IRealityDataStorage& storage, IRealityDataSource& source);
    virtual ~RealityDataCache ();
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct TiledRaster : RefCounted<IRealityData>
{
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestOptions : IRealityData::RequestOptions
    {
    private:
        ImageUtilities::RgbImageInfo m_expectedImageInfo;
        RequestOptions (ImageUtilities::RgbImageInfo expectedImageInfo) : m_expectedImageInfo (expectedImageInfo) { }
    public:
        static RefCountedPtr<RequestOptions> Create (ImageUtilities::RgbImageInfo expectedImageInfo) { return new RequestOptions (expectedImageInfo); }
        ImageUtilities::RgbImageInfo const& ExpectedImageInfo () const { return m_expectedImageInfo; }
    };

private:
    Utf8String      m_url;
    bvector<byte>   m_data;
    DateTime        m_creationDate;
    DateTime        m_expirationDate;
    Utf8String      m_entityTag;
    Utf8String      m_contentType;
    ImageUtilities::RgbImageInfo m_rasterInfo;

private:
    static Utf8String SerializeRasterInfo (ImageUtilities::RgbImageInfo const&);
    static ImageUtilities::RgbImageInfo DeserializeRasterInfo (Utf8CP);

public:
    static BentleyStatus InitializeDb (BeSQLite::Db& db);
    static RealityDataType Type () { return RealityDataType::TiledRaster; }

protected:
    // IRealityDataSource::IRequestOptionsProvider implementation
    virtual void                    _ProvideHttpRequestOptions (bmap<Utf8String, Utf8String>& options) override;

public:
    static RefCountedPtr<TiledRaster> Create () { return new TiledRaster (); }
    virtual ~TiledRaster () {}

    bvector<byte> const&            GetData () const { return m_data; }
    DateTime                        GetCreationDate () const { return m_creationDate; }
    DateTime                        GetExpirationDate () const { return m_expirationDate; }
    Utf8CP                          GetEntityTag () const { return m_entityTag.c_str (); }
    ImageUtilities::RgbImageInfo const& GetImageInfo () const { return m_rasterInfo; }
    Utf8String                      GetContentType() const {return m_contentType;}

    // IRealityData implementation
    virtual BentleyStatus           InitFrom (Utf8CP key, bmap<Utf8String, Utf8String> const& header, bvector<byte> const& body, IRealityData::RequestOptions const& options) override;
    virtual BentleyStatus           UpdateFrom (bmap<Utf8String, Utf8String> const& header) override;
    virtual BentleyStatus           InitFrom (BeSQLite::Db& db, Utf8CP key) override;
    virtual BentleyStatus           Persist (BeSQLite::Db& db) const override;
    virtual RealityDataType         GetDataType () const override { return RealityDataType::TiledRaster; }
    virtual Utf8CP                  GetKey () const override { return m_url.c_str (); }
    virtual bool                    IsExpired () const override { return DateTime::CompareResult::EarlierThan == DateTime::Compare (m_expirationDate, DateTime::GetCurrentTime ()); }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
