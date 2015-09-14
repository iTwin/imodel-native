/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/RealityDataCache_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnCore/RealityDataCache.h>

#include <functional>
#include <memory>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestStorage : IRealityDataStorage<TestStorage>
    {
    struct SelectOptions : IRealityDataStorage::SelectOptions {};
    struct PersistHandler : IRealityDataStoragePersistHandler
        {
        TestStorage& m_storage;
        Data const& m_data;
        std::function<RealityDataStorageResult()> m_persistHandler;

        PersistHandler(TestStorage& storage, Data const& data) : m_storage(storage), m_data(data) {m_storage.NotifyPersistHandlerCreated(*this);}
        void SetPersistHandler(std::function<RealityDataStorageResult()> const& handler) {m_persistHandler = handler;}
        virtual RealityDataStorageResult _Persist() const override {return (nullptr != m_persistHandler ? m_persistHandler() : RealityDataStorageResult::Success);}
        virtual IRealityDataBase const* _GetData() const override {return &m_data;}
        static RefCountedPtr<PersistHandler> Create(TestStorage& storage, Data const& data) {return new PersistHandler(storage, data);}
        };

    std::function<RealityDataStorageResult(Data&, Utf8CP, SelectOptions const&, IRealityDataStorageResponseReceiver&)> m_selectHandler;
    void SetSelectHandler(std::function<RealityDataStorageResult(Data&, Utf8CP, SelectOptions const&, IRealityDataStorageResponseReceiver&)> const& handler) {m_selectHandler = handler;}

    std::function<void(PersistHandler&)> m_onPersistHandlerCreated;
    void OnPersistHandlerCreated(std::function<void(PersistHandler&)> handler) {m_onPersistHandlerCreated = handler;}
    void NotifyPersistHandlerCreated(PersistHandler& handler)
        {
        if (nullptr != m_onPersistHandlerCreated)
            m_onPersistHandlerCreated(handler);
        }

    TestStorage() : m_selectHandler(nullptr), m_onPersistHandlerCreated(nullptr) {}
    virtual Utf8String _GetStorageId() const override {return StorageId();}
    static Utf8String StorageId() {return "TestStorage";}
    static RefCountedPtr<TestStorage> Create() {return new TestStorage();}
    RealityDataStorageResult Select(Data& data, Utf8CP id, SelectOptions const& options, IRealityDataStorageResponseReceiver& receiver) 
        {
        RealityDataStorageResult result = (nullptr != m_selectHandler ? m_selectHandler(data, id, options, receiver) : RealityDataStorageResult::Success);
        receiver._OnResponseReceived(*RealityDataStorageResponse::Create(result, id, data), options, false);
        return result;
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestSource : IRealityDataSource<TestSource>
    {
    struct RequestOptions : IRealityDataSource<TestSource>::RequestOptions {};

    struct RequestHandler : IRealityDataSourceRequestHandler
    {
    private:
        TestSource&    m_source;
        RefCountedPtr<Data> m_data;
        Utf8String m_id;
        RequestOptions const& m_options;
        IRealityDataSourceResponseReceiverPtr m_responseReceiver;
        mutable bool m_handled;
        RequestHandler(TestSource& source, Data& data, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver) 
            : m_source(source), m_data(&data), m_id(id), m_options(options), m_responseReceiver(&responseReceiver)
            {}
    protected:
        virtual bool _IsHandled() const override {return m_handled;}
        virtual RealityDataSourceResult _Request() const override {return m_source.Request(*m_data, m_handled, m_id.c_str(), m_options, *m_responseReceiver);}
        virtual IRealityDataBase const* _GetData() const override {return m_data.get();}
    public:
        static RefCountedPtr<RequestHandler> Create(TestSource& source, Data& data, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
            {
            return new RequestHandler(source, data, id, options, responseReceiver);
            }
    };

    std::function<RealityDataSourceResult(Data&, bool&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&)> m_requestHandler;
    void SetRequestHandler(std::function<RealityDataSourceResult(Data&, bool&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&)> const& handler) {m_requestHandler = handler;}

    virtual RealityDataSourceResult _Request(IRealityDataSource::Data& data, bool& handled, Utf8CP id, IRealityDataSource::RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver) override
        {
        return Request(dynamic_cast<Data&>(data), handled, id, dynamic_cast<RequestOptions const&>(options), responseReceiver);
        }
    virtual Utf8String _GetSourceId() const override {return SourceId();}
    static Utf8String SourceId() {return "TestSource";}
    static RefCountedPtr<TestSource> Create() {return new TestSource();}
    RealityDataSourceResult Request(Data& data, bool& handled, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
        {
        handled = false;
        if (nullptr != m_requestHandler)
            return m_requestHandler(data, handled, id, options, responseReceiver);
        
        responseReceiver._OnResponseReceived(*RealityDataSourceResponse::Create(RealityDataSourceResult::Success, id, data), options);
        handled = true;
        return RealityDataSourceResult::Success;
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestRealityData : IRealityData<TestRealityData, TestStorage, TestSource>
    {
    struct RequestOptions : RealityDataCacheOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    private:
        RequestOptions(bool requestFromSource, bool shouldPersist) : RealityDataCacheOptions(requestFromSource, shouldPersist) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    public:
        static RefCountedPtr<RequestOptions> Create(bool requestFromSource, bool shouldPersist = true) {return new RequestOptions(requestFromSource, shouldPersist);}
    };

    Utf8String m_id;
    bool m_expired;

    TestRealityData() : m_expired(false) {}

    static RefCountedPtr<TestRealityData> Create() {return new TestRealityData();}
    virtual Utf8CP _GetId() const {return m_id.c_str();}
    virtual bool _IsExpired() const {return m_expired;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const&) override {return ERROR;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct RealityDataCacheTests : ::testing::Test
    {
    RealityDataCachePtr m_cache;
    RefCountedPtr<TestSource> m_source;
    RefCountedPtr<TestStorage> m_storage;

    virtual void SetUp() override
        {
        m_cache = RealityDataCache::Create(0);
        m_cache->RegisterSource(*(m_source = TestSource::Create()));
        m_cache->RegisterStorage(*(m_storage = TestStorage::Create()));
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
    TestStorage::Data const* dataP = nullptr;
    RefCountedPtr<TestRealityData::RequestOptions> actualOptions = TestRealityData::RequestOptions::Create(false);
    m_storage->SetSelectHandler([&actualOptions, &dataP](TestStorage::Data& data, Utf8CP id, TestStorage::SelectOptions const& options, IRealityDataStorageResponseReceiver&)
        {
        BeAssert(0 == strcmp("Get_PassesRequestToStorage", id));
        BeAssert(actualOptions.get() == &options);
        dataP = &data;
        return RealityDataStorageResult::Success;
        });
    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_PassesRequestToStorage", *actualOptions));
    ASSERT_EQ(dataP, data.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Error)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::Error;
        });

    BeTest::SetFailOnAssert(false);
    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Error == m_cache->Get(data, "Get_Error", *TestRealityData::RequestOptions::Create(false)));
    BeTest::SetFailOnAssert(true);
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success", *TestRealityData::RequestOptions::Create(false)));
    ASSERT_TRUE(data.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_ReturnsExpired)
    {
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success_ReturnExpired", *TestRealityData::RequestOptions::Create(false)));
    ASSERT_TRUE(data.IsValid());
    ASSERT_TRUE(data->IsExpired());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenExpired)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success_RequestFromSource_WhenExpired", *TestRealityData::RequestOptions::Create(true)));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfExpired)
    {
    m_source->SetRequestHandler([](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success_RequestFromSource_OnlyIfExpired", *TestRealityData::RequestOptions::Create(true)));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success_RequestFromSource_WhenFlagSet", *TestRealityData::RequestOptions::Create(true)));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_Success_RequestFromSource_OnlyIfFlagSet", *TestRealityData::RequestOptions::Create(false)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::NotFound;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::NotFound == m_cache->Get(data, "Get_NotFound", *TestRealityData::RequestOptions::Create(false)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest (false);
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::Success == m_cache->Get(data, "Get_NotFound_RequestsFromSource_WhenFlagSet", *TestRealityData::RequestOptions::Create(true)));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::NotFound == m_cache->Get(data, "Get_NotFound_RequestsFromSource_OnlyIfFlagSet", *TestRealityData::RequestOptions::Create(false)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    04/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFoundInSource_ResolvesToNotFoundResult)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([](TestSource::Data&, bool&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        return RealityDataSourceResult::Error_NotFound;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::NotFound == m_cache->Get(data, "Get_NotFoundInSource_ResolvesToNotFoundResult", *TestRealityData::RequestOptions::Create(true)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, SourceResponseHandling_Persists)
    {
    BeAtomic<bool> didPersist (false);
    TestStorage& storage = *m_storage;
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&, IRealityDataStorageResponseReceiver&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_storage->OnPersistHandlerCreated([&didPersist](TestStorage::PersistHandler& handler)
        {
        handler.SetPersistHandler([&didPersist]()
            {
            didPersist = true;
            return RealityDataStorageResult::Success;
            });
        });
    m_source->SetRequestHandler([&storage, &didPersist](TestSource::Data& data, bool&, Utf8CP id, TestSource::RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
        {
        responseReceiver._OnResponseReceived(*RealityDataSourceResponse::Create(RealityDataSourceResult::Success, id, data), options);
        return RealityDataSourceResult::Queued;
        });

    RefCountedPtr<TestRealityData> data;
    ASSERT_TRUE(RealityDataCacheResult::RequestQueued == m_cache->Get(data, "SourceResponseHandling_Persists", *TestRealityData::RequestOptions::Create(true)));
    ASSERT_TRUE(didPersist);
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestDatabasePrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
    {
    mutable bool m_prepared;
    std::function<BentleyStatus(BeSQLite::Db&)> m_prepareDatabaseHandler;
    std::function<BentleyStatus(BeSQLite::Db&, double percentage)> m_cleanupDatabaseHandler;

    TestDatabasePrepareAndCleanupHandler() : m_prepareDatabaseHandler(nullptr), m_cleanupDatabaseHandler(nullptr), m_prepared(false) {}
    static RefCountedPtr<TestDatabasePrepareAndCleanupHandler> Create() {return new TestDatabasePrepareAndCleanupHandler();}
    void SetPrepareDatabaseHandler(std::function<BentleyStatus(BeSQLite::Db&)> const& prepareDatabaseHandler) {m_prepareDatabaseHandler = prepareDatabaseHandler;}
    void SetCleanupDatabaseHandler(std::function<BentleyStatus(BeSQLite::Db&, double percentage)> const& cleanupDatabaseHandler) {m_cleanupDatabaseHandler = cleanupDatabaseHandler;}

    virtual bool _IsPrepared() const override {return m_prepared;}
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override { m_prepared = true; return (nullptr != m_prepareDatabaseHandler ? m_prepareDatabaseHandler(db) : SUCCESS);}
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db, double percentage) const override {return (nullptr != m_cleanupDatabaseHandler ? m_cleanupDatabaseHandler(db, percentage) : SUCCESS);}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestBeSQLiteStorageData : RefCounted<BeSQLiteRealityDataStorage::Data>
    {
    struct RequestOptions : BeSQLiteRealityDataStorage::SelectOptions 
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    private:
        RequestOptions(bool synchronous) 
            {
            DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT
            SetForceSynchronousRequest(synchronous);
            }
    public:
        static RefCountedPtr<RequestOptions> Create(bool synchronous = true) {return new RequestOptions(synchronous);}
    };

    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> m_prepareAndCleanupHandler;
    std::function<BentleyStatus(BeSQLite::Db& db, BeMutex& cs, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)> m_initFromHandler;
    std::function<BentleyStatus(BeSQLite::Db& db, BeMutex& cs)>  m_persistHandler;

    TestBeSQLiteStorageData(TestDatabasePrepareAndCleanupHandler* prepareAndCleanupHandler) 
        : m_prepareAndCleanupHandler(prepareAndCleanupHandler), m_initFromHandler(nullptr), m_persistHandler(nullptr)
        {}
    static RefCountedPtr<TestBeSQLiteStorageData> Create(TestDatabasePrepareAndCleanupHandler* prepareAndCleanupHandler = nullptr) {return new TestBeSQLiteStorageData(prepareAndCleanupHandler);}
    void SetInitFromHandler(std::function<BentleyStatus(BeSQLite::Db& db, BeMutex& cs, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)> const& handler) {m_initFromHandler = handler;}
    void SetPersistHandler(std::function<BentleyStatus(BeSQLite::Db& db, BeMutex& cs)> const& handler) {m_persistHandler = handler;}

    virtual Utf8CP _GetId() const override {return nullptr;}
    virtual bool _IsExpired() const override {return false;}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return (m_prepareAndCleanupHandler.IsValid() ? m_prepareAndCleanupHandler : TestDatabasePrepareAndCleanupHandler::Create());}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options) {return (nullptr != m_initFromHandler ? m_initFromHandler(db, cs, key, options) : SUCCESS);}
    virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const {return (nullptr != m_persistHandler ? m_persistHandler(db, cs) : SUCCESS);}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const&) override {return ERROR;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct BeSQLiteRealityDataStorageTests : ::testing::Test
{
protected:
    BeSQLiteRealityDataStoragePtr m_storage;

public:
    virtual void SetUp() override
        {
        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);
        BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory);

        BeFileName filename;
        BeTest::GetHost().GetOutputRoot(filename);
        filename.AppendToPath(L"BeSQLiteRealityDataStorageTests.db");

        m_storage = BeSQLiteRealityDataStorage::Create(filename, 0, 1);
        }
    virtual void TearDown() override
        {
        m_storage = nullptr;
        }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestStorageResponseReceiver : IRealityDataStorageResponseReceiver
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    std::function<void(RealityDataStorageResponse const&)> m_onResponseReceivedHandler;
    TestStorageResponseReceiver(std::function<void(RealityDataStorageResponse const&)> const& handler) : m_onResponseReceivedHandler(handler) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    virtual void _OnResponseReceived(RealityDataStorageResponse const& response, IRealityDataStorageBase::SelectOptions const&, bool isAsync) override
        {
        if (nullptr != m_onResponseReceivedHandler)
            m_onResponseReceivedHandler(response);
        }
    static RefCountedPtr<TestStorageResponseReceiver> Create(std::function<void(RealityDataStorageResponse const&)> const& handler = nullptr) {return new TestStorageResponseReceiver(handler);}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, Select)
    {
    BeAtomic<bool> didInitialize (false);
    RefCountedPtr<TestBeSQLiteStorageData::RequestOptions> options = TestBeSQLiteStorageData::RequestOptions::Create();
    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create();
    data->SetInitFromHandler([&didInitialize, &options](BeSQLite::Db&, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& opts)
        {
        BeAssert(0 == strcmp("BeSQLiteRealityDataStorageTests.Select_1", id));
        BeAssert(options.get() == &opts);
        didInitialize = true;
        return SUCCESS;
        });
    ASSERT_TRUE(RealityDataStorageResult::Success == m_storage->Select(*data, "BeSQLiteRealityDataStorageTests.Select_1", *options, *TestStorageResponseReceiver::Create()));
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
        data->SetPersistHandler([&haltPersist, &didPersist, &cv](BeSQLite::Db&, BeMutex& cs)
            {
            while(haltPersist);
            didPersist = true;
            cv.notify_all();
            return SUCCESS;
            });
        ASSERT_TRUE(RealityDataStorageResult::Success == m_storage->Persist(*data));
        }
    haltPersist = false;
    cv.WaitOnCondition(nullptr, 10000);
    ASSERT_TRUE(didPersist);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, DoesPrepareDatabase)
    {
    BeAtomic<bool> didPrepare(false);
    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> handler = TestDatabasePrepareAndCleanupHandler::Create();
    handler->SetPrepareDatabaseHandler([&didPrepare](BeSQLite::Db&)
        {
        didPrepare = true;
        return SUCCESS;
        });

    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create(handler.get());
    m_storage->Select(*data, "", *TestBeSQLiteStorageData::RequestOptions::Create(), *TestStorageResponseReceiver::Create());
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
    handler->SetCleanupDatabaseHandler([&didCleanup, &cv](BeSQLite::Db&, double percentage)
        {
        didCleanup = true;
        cv.notify_all();
        return SUCCESS;
        });

    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create(handler.get());
    m_storage->Persist(*data);
    cv.WaitOnCondition(nullptr, 10000);
    ASSERT_TRUE(didCleanup);
    }


//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestFileSourceData : RefCounted<FileRealityDataSource::Data>
    {
    struct RequestOptions : FileRealityDataSource::RequestOptions
        {
        DEFINE_BENTLEY_REF_COUNTED_MEMBERS
        private:
            RequestOptions() {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
        public:
            static RefCountedPtr<RequestOptions> Create() {return new RequestOptions();}
        };

    std::function<BentleyStatus(Utf8CP, bvector<Byte> const&, FileRealityDataSource::RequestOptions const&)> m_initFromHandler;

    TestFileSourceData() : m_initFromHandler(nullptr) {}
    static RefCountedPtr<TestFileSourceData> Create() {return new TestFileSourceData();}
    void SetInitFromHandler(std::function<BentleyStatus(Utf8CP, bvector<Byte> const&, FileRealityDataSource::RequestOptions const&)> const& handler) {m_initFromHandler = handler;}
    
    virtual Utf8CP _GetId() const override {return nullptr;}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(Utf8CP id, bvector<Byte> const& data, FileRealityDataSource::RequestOptions const& options) override {return (nullptr != m_initFromHandler ? m_initFromHandler(id, data, options) : SUCCESS);}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const&) override {return ERROR;}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct FileRealityDataSourceTests : ::testing::Test
{
protected:
    FileRealityDataSourcePtr m_source;
    Utf8String m_fileContent;
    Utf8String m_filePath;

public:
    virtual void SetUp() override
        {
        m_source = FileRealityDataSource::Create(1);

        BeFileName sourceFile;
        BeTest::GetHost().GetOutputRoot(sourceFile);
        sourceFile.AppendToPath(L"FileRealityDataSourceTests.Request_WithDataOutOfScope");
        sourceFile.BeDeleteFile();
        BeAssert(!sourceFile.DoesPathExist());
        m_filePath = sourceFile.GetNameUtf8();

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
struct TestSourceResponseReceiver : IRealityDataSourceResponseReceiver
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    std::function<void(RealityDataSourceResponse const&)> m_onResponseReceivedHandler;
    TestSourceResponseReceiver(std::function<void(RealityDataSourceResponse const&)> const& handler) : m_onResponseReceivedHandler(handler) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    virtual void _OnResponseReceived(RealityDataSourceResponse const& response, IRealityDataSourceBase::RequestOptions const&) override
        {
        if (nullptr != m_onResponseReceivedHandler)
            m_onResponseReceivedHandler(response);
        }
    static RefCountedPtr<TestSourceResponseReceiver> Create(std::function<void(RealityDataSourceResponse const&)> const& handler = nullptr) {return new TestSourceResponseReceiver(handler);}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (FileRealityDataSourceTests, Request)
    {
    Utf8String filePath = m_filePath;
    Utf8String fileContent = m_fileContent;

    BeAtomic<bool> didInitialize(false);
    RefCountedPtr<TestFileSourceData::RequestOptions> options = TestFileSourceData::RequestOptions::Create();
    RefCountedPtr<TestFileSourceData> data = TestFileSourceData::Create();
    data->SetInitFromHandler([&didInitialize, &filePath, &options, &fileContent](Utf8CP id, bvector<Byte> const& string, FileRealityDataSource::RequestOptions const& opts)
        {
        BeAssert(filePath.Equals(id));
        BeAssert(options.get() == &opts);
        BeAssert(fileContent.Equals((Utf8CP)string.data()));
        didInitialize = true;
        return SUCCESS;
        });
    BeAtomic<bool> didReceiveResponse(false);
    BeConditionVariable cv;
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = TestSourceResponseReceiver::Create([&didReceiveResponse, &cv, &data](RealityDataSourceResponse const& response)
        {
        BeAssert(data.get() == &response.GetData());
        didReceiveResponse = true;
        cv.notify_all();
        });
    bool handled;
    ASSERT_TRUE(RealityDataSourceResult::Queued == m_source->Request(*data, handled, m_filePath.c_str(), *options, *responseReceiver));

    cv.WaitOnCondition(nullptr, 10000);
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
    RefCountedPtr<TestFileSourceData::RequestOptions> options = TestFileSourceData::RequestOptions::Create();
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = TestSourceResponseReceiver::Create([&didReceiveResponse, &cv](RealityDataSourceResponse const& response)
        {
        didReceiveResponse = true;
        cv.notify_all();
        });

        {
        RefCountedPtr<TestFileSourceData> data = TestFileSourceData::Create();
        data->SetInitFromHandler([&didInitialize, &filePath, &options, &fileContent, &block](Utf8CP id, bvector<Byte> const& string, FileRealityDataSource::RequestOptions const& opts)
            {
            while(block);
            BeAssert(filePath.Equals(id));
            BeAssert(options.get() == &opts);
            BeAssert(fileContent.Equals((Utf8CP)string.data()));
            didInitialize = true;
            return SUCCESS;
            });
        bool handled;
        ASSERT_TRUE(RealityDataSourceResult::Queued == m_source->Request(*data, handled, m_filePath.c_str(), *options, *responseReceiver));
        }
        
    block = false;
    cv.WaitOnCondition(nullptr, 10000);
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
    virtual bool _TestCondition(BeConditionVariable& cv) {BeMutexHolder lock(cv.GetMutex()); return m_handler();}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    07/2015
//---------------------------------------------------------------------------------------
TEST_F (FileRealityDataSourceTests, SynchronousRequestReturnsDataSynchronouslyAfterQueueing)
    {
    BeAtomic<bool> block(true);
    auto initHandler = [&block](Utf8CP id, bvector<Byte> const& string, FileRealityDataSource::RequestOptions const& opts)
        {
        while (block)
            ;
        return SUCCESS;
        };

    BeAtomic<int> responseCount(0);
    BeConditionVariable responseCV;
    RefCountedPtr<TestSourceResponseReceiver> responseReceiver = TestSourceResponseReceiver::Create([&responseCount, &responseCV](RealityDataSourceResponse const& response)
        {
        if (++responseCount == 2)
            responseCV.notify_all();
        });
        
    bool handled1 = false;
    RefCountedPtr<TestFileSourceData> data1 = TestFileSourceData::Create();
    data1->SetInitFromHandler(initHandler);
    RefCountedPtr<TestFileSourceData::RequestOptions> options1 = TestFileSourceData::RequestOptions::Create();
    RealityDataSourceResult result1 = m_source->Request(*data1, handled1, m_filePath.c_str(), *options1, *responseReceiver);
    ASSERT_TRUE(RealityDataSourceResult::Queued == result1);
    
    BeConditionVariable requestCV;
    BeMutexHolder lock(requestCV.GetMutex());
    BackDoor::RealityData::RunOnAnotherThread([&initHandler, &responseReceiver, &requestCV, this]()
        {
        BeMutexHolder lock(requestCV.GetMutex());

        bool handled2 = false;
        RefCountedPtr<TestFileSourceData> data2 = TestFileSourceData::Create();
        data2->SetInitFromHandler(initHandler);
        RefCountedPtr<TestFileSourceData::RequestOptions> options2 = TestFileSourceData::RequestOptions::Create();
        options2->SetForceSynchronousRequest(true);
        RealityDataSourceResult result2 = m_source->Request(*data2, handled2, m_filePath.c_str(), *options2, *responseReceiver);
        ASSERT_TRUE(RealityDataSourceResult::Success == result2);
        requestCV.notify_all();
        });
    block = false;

    requestCV.ProtectedWaitOnCondition(lock, nullptr, 10000);
    lock.unlock();

    TestPredicate predicate([&responseCount](){return responseCount == 2;});
    responseCV.WaitOnCondition(&predicate, 1000);

    ASSERT_EQ(2, responseCount);
    }
