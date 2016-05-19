/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/RealityDataCache_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/RealityDataCache.h>

#include <functional>
#include <memory>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_REALITYDATA

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestWork : RefCounted<Work>
{
typedef std::function<void()> Handler;
private:
    Handler m_handler;
    TestWork(Handler const& handler) : m_handler(handler) {}
protected:
    virtual void _DoWork() override {m_handler();}
public:
    static RefCountedPtr<TestWork> Create(Handler const& handler) {return new TestWork(handler);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void runOnAnotherThread(std::function<void()> const& handler)
    {
    WorkerThreadPtr thread = WorkerThread::Create();
    thread->Start();
    thread->DoWork(*TestWork::Create([handler, thread]()
        {
        handler();
        thread->Terminate();
        }));
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestStorage : Storage
    {
    struct PersistHandler : StoragePersistHandler
        {
        TestStorage& m_storage;
        Payload const& m_data;
        std::function<StorageResult()> m_persistHandler;

        PersistHandler(TestStorage& storage, Payload const& data) : m_storage(storage), m_data(data) {m_storage.NotifyPersistHandlerCreated(*this);}
        void SetPersistHandler(std::function<StorageResult()> const& handler) {m_persistHandler = handler;}
        virtual StorageResult _Persist() const override {return (nullptr != m_persistHandler ? m_persistHandler() : StorageResult::Success);}
        virtual Payload const* _GetData() const override {return &m_data;}
        static RefCountedPtr<PersistHandler> Create(TestStorage& storage, Payload const& data) {return new PersistHandler(storage, data);}
        };

    std::function<StorageResult(Payload&, Utf8CP, Options, CacheR)> m_selectHandler;
    void SetSelectHandler(std::function<StorageResult(Payload&, Utf8CP, Options, CacheR)> const& handler) {m_selectHandler = handler;}

    std::function<void(PersistHandler&)> m_onPersistHandlerCreated;
    void OnPersistHandlerCreated(std::function<void(PersistHandler&)> handler) {m_onPersistHandlerCreated = handler;}
    void NotifyPersistHandlerCreated(PersistHandler& handler)
        {
        if (nullptr != m_onPersistHandlerCreated)
            m_onPersistHandlerCreated(handler);
        }

    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override {return SUCCESS;}
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const override {return SUCCESS;}

    TestStorage() : Storage(1), m_selectHandler(nullptr), m_onPersistHandlerCreated(nullptr) {}
    static RefCountedPtr<TestStorage> Create() {return new TestStorage();}
    StorageResult _Select(Payload& data, Utf8CP id, Options options, CacheR receiver) override
        {
        StorageResult result = (nullptr != m_selectHandler ? m_selectHandler(data, id, options, receiver) : StorageResult::Success);
         receiver._OnResponseReceived(*new StorageResponse(result, id, data), options, false);
        return result;
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestSource : Source
    {
    struct RequestHandler : SourceRequestHandler
    {
    private:
        TestSource&    m_source;
        RefCountedPtr<Payload> m_data;
        Utf8String m_id;
        Options m_options;
        Cache& m_responseReceiver;
        mutable bool m_handled;
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual SourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, m_responseReceiver);}
        virtual Payload const* _GetData() const override {return m_data.get();}
    public:
        RequestHandler(TestSource& source, Payload& data, Utf8CP id, Options options, Cache& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(responseReceiver)
            {}
    };

    std::function<SourceResult(Payload&, bool&, Utf8CP, Options, Cache&)> m_requestHandler;
    void SetRequestHandler(std::function<SourceResult(Payload&, bool&, Utf8CP, Options, Cache&)> const& handler) {m_requestHandler = handler;}

    virtual SourceResult _Request(Payload& data, bool& handled, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return Request(data, handled, id, options, responseReceiver);
        }
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    static Utf8String SourceId() {return "TestSource";}
    static RefCountedPtr<TestSource> Create() {return new TestSource();}
    SourceResult Request(Payload& data, bool& handled, Utf8CP id, Options options, Cache& responseReceiver)
        {
        handled = false;
        if (nullptr != m_requestHandler)
            return m_requestHandler(data, handled, id, options, responseReceiver);
        
        responseReceiver._OnResponseReceived(*new SourceResponse(SourceResult::Success, id, data), options);
        handled = true;
        return SourceResult::Success;
        }

    SourceRequestHandlerPtr _CreateRequestHandler(Payload& data, Utf8CP id, Options options, Cache& responseReceiver) override
        {
        return new RequestHandler(*this, data, id, options, responseReceiver);
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestRealityData : Payload
    {
    struct RequestOptions : Options
        {
        RequestOptions(bool requestFromSource, bool shouldPersist = true) 
            {
            m_requestFromSource = requestFromSource;
            m_forceSynchronous = true;
            }
        };

    Utf8String m_id;
    bool m_expired;
    size_t m_onErrorCalls;
    size_t m_onNotFoundCalls;

    TestRealityData() : m_expired(false), m_onErrorCalls(0), m_onNotFoundCalls(0) {}

    static RefCountedPtr<TestRealityData> Create() {return new TestRealityData();}
    virtual Utf8CP _GetId() const {return m_id.c_str();}
    virtual bool _IsExpired() const {return m_expired;}
    virtual void _OnError() override {++m_onErrorCalls;}
    virtual void _OnNotFound() override {++m_onNotFoundCalls;}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return SUCCESS;}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP id) override {return SUCCESS;}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) override {return SUCCESS;}
    virtual BentleyStatus _Persist(Db& db) const override {return SUCCESS;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct RealityDataCacheTests : ::testing::Test
    {
    CachePtr m_cache;
    RefCountedPtr<TestSource> m_source;
    RefCountedPtr<TestStorage> m_storage;

    virtual void SetUp() override
        {
        m_cache = new Cache();
        m_cache->SetSource(*(m_source = TestSource::Create()));
        m_cache->SetStorage(*(m_storage = TestStorage::Create()));
        }
    virtual void TearDown() override
        {
        m_source = nullptr;
        m_storage = nullptr;
        m_cache = nullptr;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_PassesRequestToStorage)
    {
    Payload const* dataP = nullptr;
    TestRealityData::RequestOptions requestOptions(false);
    m_storage->SetSelectHandler([&requestOptions, &dataP](Payload& data, Utf8CP id, Options options, Cache&)
        {
        BeAssert(0 == strcmp("Get_PassesRequestToStorage", id));
        dataP = &data;
        return StorageResult::Success;
        });
    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_PassesRequestToStorage", requestOptions));
    ASSERT_EQ(dataP, data.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Error)
    {
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::Error;
        });

    BeTest::SetFailOnAssert(false);
    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Error == m_cache->RequestData(*data, "Get_Error", TestRealityData::RequestOptions(false)));
    BeTest::SetFailOnAssert(true);
    EXPECT_EQ(1, data->m_onErrorCalls);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success)
    {
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success", TestRealityData::RequestOptions(false)));
    ASSERT_TRUE(data.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_ReturnsExpired)
    {
    m_storage->SetSelectHandler([](Payload& data, Utf8CP, Options, Cache&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return StorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success_ReturnExpired", TestRealityData::RequestOptions(false)));
    ASSERT_TRUE(data.IsValid());
    ASSERT_TRUE(data->_IsExpired());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenExpired)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](Payload& data, Utf8CP, Options, Cache&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return StorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        didRequest.store(true);
        return SourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success_RequestFromSource_WhenExpired", TestRealityData::RequestOptions(true)));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfExpired)
    {
    m_source->SetRequestHandler([](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        BeAssert(false);
        return SourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success_RequestFromSource_OnlyIfExpired", TestRealityData::RequestOptions(true)));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](Payload& data, Utf8CP, Options, Cache&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return StorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        didRequest.store(true);
        return SourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success_RequestFromSource_WhenFlagSet", TestRealityData::RequestOptions(true)));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](Payload& data, Utf8CP, Options, Cache&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return StorageResult::Success;
        });
    m_source->SetRequestHandler([](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        BeAssert(false);
        return SourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_Success_RequestFromSource_OnlyIfFlagSet", TestRealityData::RequestOptions(false)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound)
    {
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::NotFound;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::NotFound == m_cache->RequestData(*data, "Get_NotFound", TestRealityData::RequestOptions(false)));
    ASSERT_EQ(1, data->m_onNotFoundCalls);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::NotFound;
        });
    m_source->SetRequestHandler([&didRequest](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        didRequest.store(true);
        return SourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::Success == m_cache->RequestData(*data, "Get_NotFound_RequestsFromSource_WhenFlagSet", TestRealityData::RequestOptions(true)));
    ASSERT_TRUE(didRequest);
    ASSERT_EQ(0, data->m_onNotFoundCalls);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::NotFound;
        });
    m_source->SetRequestHandler([](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        BeAssert(false);
        return SourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::NotFound == m_cache->RequestData(*data, "Get_NotFound_RequestsFromSource_OnlyIfFlagSet", TestRealityData::RequestOptions(false)));
    ASSERT_EQ(1, data->m_onNotFoundCalls);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    04/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFoundInSource_ResolvesToNotFoundResult)
    {
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::NotFound;
        });
    m_source->SetRequestHandler([](Payload&, bool&, Utf8CP, Options, Cache&)
        {
        return SourceResult::Error_NotFound;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::NotFound == m_cache->RequestData(*data, "Get_NotFoundInSource_ResolvesToNotFoundResult", TestRealityData::RequestOptions(true)));
    ASSERT_EQ(1, data->m_onNotFoundCalls);
    }

#if defined (NEEDS_WORK_REALITY_CACHE)
//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, SourceResponseHandling_Persists)
    {
    BeAtomic<bool> didPersist (false);
    TestStorage& storage = *m_storage;
    m_storage->SetSelectHandler([](Payload&, Utf8CP, Options, Cache&)
        {
        return StorageResult::NotFound;
        });
    m_storage->OnPersistHandlerCreated([&didPersist](TestStorage::PersistHandler& handler)
        {
        handler.SetPersistHandler([&didPersist]()
            {
            didPersist.store(true);
            return StorageResult::Success;
            });
        });
    m_source->SetRequestHandler([&storage, &didPersist](Payload& data, bool&, Utf8CP id, Options options, Cache& responseReceiver)
        {
        responseReceiver._OnResponseReceived(*new SourceResponse(SourceResult::Success, id, data), options);
        return SourceResult::Queued;
        });

    RefCountedPtr<TestRealityData> data = TestRealityData::Create();
    ASSERT_TRUE(CacheResult::RequestQueued == m_cache->RequestData(*data, "SourceResponseHandling_Persists", TestRealityData::RequestOptions(true)));
    ASSERT_TRUE(didPersist);
    }
#endif

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestCacheDatabase : Storage
    {
    mutable bool m_prepared;
    using Storage::Storage;

    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override { m_prepared = true; return SUCCESS;}
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const override {return SUCCESS;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestBeSQLiteStorageData : Payload
    {
    struct RequestOptions : Options
        {
        RequestOptions(bool synchronous = true) 
            {
            m_forceSynchronous= synchronous;
            }
        };

    std::function<BentleyStatus(BeSQLite::Db& db, Utf8CP key)> m_initFromHandler;
    std::function<BentleyStatus(BeSQLite::Db& db)>  m_persistHandler;

    TestBeSQLiteStorageData() 
        : m_initFromHandler(nullptr), m_persistHandler(nullptr)
        {}
    static RefCountedPtr<TestBeSQLiteStorageData> Create() {return new TestBeSQLiteStorageData();}
    void SetInitFromHandler(std::function<BentleyStatus(BeSQLite::Db& db, Utf8CP key)> const& handler) {m_initFromHandler = handler;}
    void SetPersistHandler(std::function<BentleyStatus(BeSQLite::Db& db)> const& handler) {m_persistHandler = handler;}

    virtual Utf8CP _GetId() const override {return nullptr;}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db,  Utf8CP key) {return (nullptr != m_initFromHandler ? m_initFromHandler(db, key) : SUCCESS);}
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const {return (nullptr != m_persistHandler ? m_persistHandler(db) : SUCCESS);}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return SUCCESS;}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) override {return SUCCESS;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct BeSQLiteRealityDataStorageTests : ::testing::Test
{
protected:
    StoragePtr m_storage;

public:
    virtual void SetUp() override
        {
        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);
        BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory);

        BeFileName filename;
        BeTest::GetHost().GetOutputRoot(filename);
        filename.AppendToPath(L"BeSQLiteRealityDataStorageTests.db");
        m_storage = new TestCacheDatabase(1, SchedulingMethod::LIFO, 0);
        m_storage->OpenAndPrepare(filename);
        }
    virtual void TearDown() override
        {
        m_storage = nullptr;
        }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestStorageResponseReceiver : Cache
    {
    std::function<void(StorageResponse const&, Options)> m_OnResponseReceivedHandler;
    TestStorageResponseReceiver(std::function<void(StorageResponse const&, Options)> const& handler) : m_OnResponseReceivedHandler(handler) {}
    virtual void _OnResponseReceived(StorageResponse const& response, Options options, bool isAsync) override
        {
        if (nullptr != m_OnResponseReceivedHandler)
            m_OnResponseReceivedHandler(response, options);
        }
    static RefCountedPtr<TestStorageResponseReceiver> Create(std::function<void(StorageResponse const&, Options)> const& handler = nullptr) {return new TestStorageResponseReceiver(handler);}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, Select)
    {
    BeAtomic<bool> didInitialize (false);
    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create();
    data->SetInitFromHandler([&didInitialize](BeSQLite::Db&, Utf8CP id)
        {
        BeAssert(0 == strcmp("BeSQLiteRealityDataStorageTests.Select_1", id));
        didInitialize.store(true);
        return SUCCESS;
        });
    ASSERT_EQ(StorageResult::Success, m_storage->_Select(*data, "BeSQLiteRealityDataStorageTests.Select_1", TestBeSQLiteStorageData::RequestOptions(), *TestStorageResponseReceiver::Create()));
    ASSERT_TRUE(didInitialize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, Persist)
    {
    BeAtomic<bool> haltPersist(true);
    BeAtomic<bool> didPersist(false);
    BeConditionVariable cv;
    // create the data in a scope to test if Persist still works (increases refCount) when the data goes out of scope
        {
        RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create();
        data->SetPersistHandler([&haltPersist, &didPersist, &cv](BeSQLite::Db&)
            {
            while (haltPersist);
            BeMutexHolder lock(cv.GetMutex());
            didPersist.store(true);
            cv.notify_all();
            return SUCCESS;
            });
        ASSERT_TRUE(StorageResult::Success == m_storage->Persist(*data));
        }
    haltPersist.store(false);
    BeMutexHolder lock(cv.GetMutex());
    cv.ProtectedWaitOnCondition(lock, nullptr, 10000);
    ASSERT_TRUE(didPersist);
    }

#if defined (NEEDS_WORK_REALITY_CACHE)
//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, DoesPrepareDatabase)
    {
    BeAtomic<bool> didPrepare(false);
    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> handler = TestDatabasePrepareAndCleanupHandler::Create();
    handler->SetPrepareDatabaseHandler([&didPrepare](BeSQLite::Db&)
        {
        didPrepare.store(true);
        return SUCCESS;
        });

    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create(handler.get());
    m_storage->Select(*data, "", TestBeSQLiteStorageData::RequestOptions(), *TestStorageResponseReceiver::Create());
    ASSERT_TRUE(didPrepare);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, DoesCleanupDatabase)
    {
    BeAtomic<bool> didCleanup(false);
    BeConditionVariable cv;
    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> handler = TestDatabasePrepareAndCleanupHandler::Create();
    handler->SetCleanupDatabaseHandler([&didCleanup, &cv](BeSQLite::Db&)
        {
        BeMutexHolder lock(cv.GetMutex());
        didCleanup.store(true);
        cv.notify_all();
        return SUCCESS;
        });

    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create(handler.get());
    m_storage->Persist(*data);
    BeMutexHolder lock(cv.GetMutex());
    cv.ProtectedWaitOnCondition(lock, nullptr, 10000);
    ASSERT_TRUE(didCleanup);
    }
#endif


//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestFileSourceData : Payload
    {
    struct RequestOptions : Options
        {
        RequestOptions(bool synchronous = false) 
            {
            m_forceSynchronous=synchronous;
            }
        };

    std::function<BentleyStatus(Utf8CP, ByteStream const&)> m_initFromHandler;

    TestFileSourceData() : m_initFromHandler(nullptr) {}
    static RefCountedPtr<TestFileSourceData> Create() {return new TestFileSourceData();}
    void SetInitFromHandler(std::function<BentleyStatus(Utf8CP, ByteStream const&)> const& handler) {m_initFromHandler = handler;}
    
    virtual Utf8CP _GetId() const override {return nullptr;}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(Utf8CP id, ByteStream const& data) override {return (nullptr != m_initFromHandler ? m_initFromHandler(id, data) : SUCCESS);}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db,  Utf8CP key) {return SUCCESS;}
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const {return SUCCESS;}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return SUCCESS;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct FileRealityDataSourceTests : ::testing::Test
{
protected:
    FileSourcePtr m_source;
    Utf8String m_fileContent;
    Utf8String m_filePath;

public:
    virtual void SetUp() override
        {
        m_source = new FileSource(1, SchedulingMethod::LIFO);

        BeFileName sourceFile;
        BeTest::GetHost().GetOutputRoot(sourceFile);
        sourceFile.AppendToPath(WPrintfString(L"%f", BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay()).c_str());
        sourceFile.AppendToPath(L"FileRealityDataSourceTests.Request_WithDataOutOfScope");
        sourceFile.BeDeleteFile();
        BeAssert(!sourceFile.DoesPathExist());
        m_filePath = sourceFile.GetNameUtf8();

        BeFileName dir(BeFileName::FileNameParts::DevAndDir, sourceFile.c_str());
        BeFileName::CreateNewDirectory(dir.c_str());
        BeAssert(dir.DoesPathExist());

        // write some data to source file
        m_fileContent = "Test data";
        BeFile f;
        f.Create(m_filePath.c_str());
        f.Write(nullptr, m_fileContent.data(), (uint32_t)(m_fileContent.length() + 1));
        f.Close();
        }
    virtual void TearDown() override
        {
        m_source = nullptr;
        }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestSourceResponseReceiver : Cache
    {
    std::function<void(SourceResponse const&, Options)> mOnResponseReceivedHandler;
    TestSourceResponseReceiver(std::function<void(SourceResponse const&, Options)> const& handler) : mOnResponseReceivedHandler(handler) {}
    virtual void _OnResponseReceived(SourceResponse const& response, Options options) override
        {
        if (nullptr != mOnResponseReceivedHandler)
            mOnResponseReceivedHandler(response, options);
        }
    static RefCountedPtr<TestSourceResponseReceiver> Create(std::function<void(SourceResponse const&, Options)> const& handler = nullptr) {return new TestSourceResponseReceiver(handler);}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (FileRealityDataSourceTests, Request)
    {
    Utf8String filePath = m_filePath;
    Utf8String fileContent = m_fileContent;

    BeAtomic<bool> didInitialize(false);
    RefCountedPtr<TestFileSourceData> data = TestFileSourceData::Create();
    data->SetInitFromHandler([&didInitialize, &filePath, &fileContent](Utf8CP id, ByteStream const& string)
        {
        BeAssert(filePath.Equals(id));
        BeAssert(fileContent.Equals((Utf8CP)string.GetData()));
        didInitialize.store(true);
        return SUCCESS;
        });
    BeAtomic<bool> didReceiveResponse(false);
    BeConditionVariable cv;
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = new TestSourceResponseReceiver([&didReceiveResponse, &cv, &data](SourceResponse const& response, Options options)
        {
        BeAssert(data.get() == &response.GetData());
        BeMutexHolder lock(cv.GetMutex());
        didReceiveResponse.store(true);
        cv.notify_all();
        });
    bool handled;
    ASSERT_TRUE(SourceResult::Queued == m_source->_Request(*data, handled, m_filePath.c_str(), TestFileSourceData::RequestOptions(), *responseReceiver));

    BeMutexHolder lock(cv.GetMutex());
    cv.ProtectedWaitOnCondition(lock, nullptr, 10000);
    ASSERT_TRUE(didInitialize);
    ASSERT_TRUE(didReceiveResponse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (FileRealityDataSourceTests, Request_WithDataOutOfScope)
    {
    Utf8String filePath = m_filePath;
    Utf8String fileContent = m_fileContent;

    BeAtomic<bool> block(true);
    BeAtomic<bool> didInitialize(false);
    BeAtomic<bool> didReceiveResponse(false);
    BeConditionVariable cv;
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = TestSourceResponseReceiver::Create([&didReceiveResponse, &cv](SourceResponse const& response, Options options)
        {
        BeMutexHolder lock(cv.GetMutex());
        didReceiveResponse.store(true);
        cv.notify_all();
        });

        {
        RefCountedPtr<TestFileSourceData> data = TestFileSourceData::Create();
        data->SetInitFromHandler([&didInitialize, &filePath, &fileContent, &block](Utf8CP id, ByteStream const& string)
            {
            while(block);
            BeAssert(filePath.Equals(id));
            BeAssert(fileContent.Equals((Utf8CP)string.GetData()));
            didInitialize.store(true);
            return SUCCESS;
            });
        bool handled;
        ASSERT_TRUE(SourceResult::Queued == m_source->_Request(*data, handled, m_filePath.c_str(), TestFileSourceData::RequestOptions(), *responseReceiver));
        }
        
    block.store(false);
    BeMutexHolder lock(cv.GetMutex());
    cv.ProtectedWaitOnCondition(lock, nullptr, 10000);
    ASSERT_TRUE(didInitialize);
    ASSERT_TRUE(didReceiveResponse);
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            07/2015
//=======================================================================================
struct TestPredicate : IConditionVariablePredicate
    {
    typedef std::function<bool()> Handler;
    Handler m_handler;
    TestPredicate (Handler const& handler) : m_handler(handler) {}
    virtual bool _TestCondition(BeConditionVariable& cv) {return m_handler();}
    };

#if defined (NEEDS_WORK_REALITY_CACHE)
//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    07/2015
//---------------------------------------------------------------------------------------
TEST_F (FileRealityDataSourceTests, SynchronousRequestReturnsDataSynchronouslyAfterQueueing)
    {
    BeAtomic<bool> block(true);
    auto initHandler = [&block](Utf8CP id, ByteStream const& string)
        {
        while (block)
            ;
        return SUCCESS;
        };

    BeAtomic<int> responseCount(0);
    BeConditionVariable responseCV;
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = TestSourceResponseReceiver::Create([&responseCount, &responseCV](SourceResponse const& response, Options options)
        {
        if (++responseCount == 2)
            responseCV.notify_all();
        });
        
    bool handled1 = false;
    RefCountedPtr<TestFileSourceData> data1 = TestFileSourceData::Create();
    data1->SetInitFromHandler(initHandler);
    SourceResult result1 = m_source->_Request(*data1, handled1, m_filePath.c_str(), TestFileSourceData::RequestOptions(), *responseReceiver);
    ASSERT_TRUE(SourceResult::Queued == result1);
    
    BeConditionVariable requestCV;
    BeMutexHolder lock(requestCV.GetMutex());
    runOnAnotherThread([&initHandler, &responseReceiver, &requestCV, this]()
        {
        BeMutexHolder lock(requestCV.GetMutex());

        bool handled2 = false;
        RefCountedPtr<TestFileSourceData> data2 = TestFileSourceData::Create();
        data2->SetInitFromHandler(initHandler);
        SourceResult result2 = m_source->_Request(*data2, handled2, m_filePath.c_str(), TestFileSourceData::RequestOptions(true), *responseReceiver);
        ASSERT_TRUE(SourceResult::Success == result2);
        requestCV.notify_all();
        });
    block.store(false);

    requestCV.ProtectedWaitOnCondition(lock, nullptr, 10000);
    lock.unlock();

    TestPredicate predicate([&responseCount](){return responseCount == 2;});
    responseCV.WaitOnCondition(&predicate, 1000);

    ASSERT_EQ(2, responseCount);
    }
#endif
