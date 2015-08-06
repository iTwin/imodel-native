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

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestStorage : IRealityDataStorage<TestStorage>
    {
    struct SelectOptions {};
    struct PersistHandler : IRealityDataStoragePersistHandler
        {
        TestStorage& m_storage;
        Data const& m_data;
        std::function<RealityDataStorageResult()> m_persistHandler;

        PersistHandler(TestStorage& storage, Data const& data) : m_storage(storage), m_data(data) {m_storage.AddPersistHandler(this);}
        ~PersistHandler() {m_storage.RemovePersistHandler(this);}
        void SetPersistHandler(std::function<RealityDataStorageResult()> const& handler) {m_persistHandler = handler;}
        virtual RealityDataStorageResult _Persist() const override {return (nullptr != m_persistHandler ? m_persistHandler() : RealityDataStorageResult::Success);}
        virtual IRealityDataBase const* _GetData() const override {return &m_data;}
        };

    std::function<RealityDataStorageResult(Data&, Utf8CP, SelectOptions const&)> m_selectHandler;
    void SetSelectHandler(std::function<RealityDataStorageResult(Data&, Utf8CP, SelectOptions const&)> const& handler) {m_selectHandler = handler;}

    bset<PersistHandler*> m_persistHandlers;
    void AddPersistHandler(PersistHandler* caller) {m_persistHandlers.insert(caller);}
    void RemovePersistHandler(PersistHandler* caller) {m_persistHandlers.erase(caller);}

    virtual Utf8CP _GetStorageId() const {return StorageId();}
    static Utf8CP StorageId() {return "TestStorage";}
    static RefCountedPtr<TestStorage> Create() {return new TestStorage();}
    RealityDataStorageResult Select(Data& data, Utf8CP id, SelectOptions const& options) {return (nullptr != m_selectHandler ? m_selectHandler(data, id, options) : RealityDataStorageResult::Success);}
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestSource : IRealityDataSource<TestSource>
    {
    struct RequestOptions {};

    std::function<RealityDataSourceResult(Data&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&)> m_requestHandler;
    void SetRequestHandler(std::function<RealityDataSourceResult(Data&, Utf8CP, RequestOptions const&, IRealityDataSourceResponseReceiver&)> const& handler) {m_requestHandler = handler;}

    virtual Utf8CP _GetSourceId() const {return SourceId();}
    static Utf8CP SourceId() {return "TestSource";}
    static RefCountedPtr<TestSource> Create() {return new TestSource();}
    RealityDataSourceResult Request(Data& data, Utf8CP id, RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
        {
        if (nullptr != m_requestHandler)
            return m_requestHandler(data, id, options, responseReceiver);
        
        responseReceiver._OnResponseReceived(RealityDataSourceResponse(RealityDataSourceResult::Success, id, &data));
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
        RequestOptions(bool returnExpired, bool requestFromSource) : RealityDataCacheOptions(returnExpired, requestFromSource) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    public:
        static RefCountedPtr<RequestOptions> Create(bool returnExpired, bool requestFromSource) {return new RequestOptions(returnExpired, requestFromSource);}
    };

    Utf8String m_id;
    bool m_expired;

    TestRealityData() : m_expired(false) {}

    static RefCountedPtr<TestRealityData> Create() {return new TestRealityData();}
    virtual Utf8CP _GetId() const {return m_id.c_str();}
    virtual bool _IsExpired() const {return m_expired;}
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
        m_cache = RealityDataCache::Create();
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
    RefCountedPtr<TestRealityData::RequestOptions> actualOptions = TestRealityData::RequestOptions::Create(false, false);
    m_storage->SetSelectHandler([&actualOptions, &dataP](TestStorage::Data& data, Utf8CP id, TestStorage::SelectOptions const& options)
        {
        BeAssert(!data.IsInitialized());
        BeAssert(0 == strcmp("Get_PassesRequestToStorage", id));
        BeAssert(actualOptions.get() == &options);
        dataP = &data;
        return RealityDataStorageResult::Success;
        });
    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_PassesRequestToStorage", *actualOptions);
    ASSERT_EQ(dataP, data.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Error)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::Error;
        });

    BeTest::SetFailOnAssert(false);
    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Error", *TestRealityData::RequestOptions::Create(false, false));
    BeTest::SetFailOnAssert(true);

    ASSERT_TRUE(data.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success", *TestRealityData::RequestOptions::Create(false, false));
    ASSERT_TRUE(data.IsValid());
    ASSERT_TRUE(data->IsInitialized());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_ReturnExpired)
    {
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_ReturnExpired", *TestRealityData::RequestOptions::Create(true, false));
    ASSERT_TRUE(data.IsValid());
    ASSERT_TRUE(data->IsExpired());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_DontReturnExpired)
    {
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_DontReturnExpired", *TestRealityData::RequestOptions::Create(false, false));
    ASSERT_TRUE(data.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenExpired)
    {
    BeAtomic<bool> didRequest(false);
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_RequestFromSource_WhenExpired", *TestRealityData::RequestOptions::Create(false, true));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfExpired)
    {
    m_source->SetRequestHandler([](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_RequestFromSource_OnlyIfExpired", *TestRealityData::RequestOptions::Create(false, true));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest(false);
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_RequestFromSource_WhenFlagSet", *TestRealityData::RequestOptions::Create(false, true));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_Success_RequestFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](TestStorage::Data& data, Utf8CP, TestStorage::SelectOptions const&)
        {
        static_cast<TestRealityData&>(data).m_expired = true;
        return RealityDataStorageResult::Success;
        });
    m_source->SetRequestHandler([](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_Success_RequestFromSource_OnlyIfFlagSet", *TestRealityData::RequestOptions::Create(false, false));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::NotFound;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_NotFound", *TestRealityData::RequestOptions::Create(false, false));
    ASSERT_TRUE(data.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_WhenFlagSet)
    {
    BeAtomic<bool> didRequest(false);
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([&didRequest](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        didRequest = true;
        return RealityDataSourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_NotFound_RequestsFromSource_WhenFlagSet", *TestRealityData::RequestOptions::Create(false, true));
    ASSERT_TRUE(didRequest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, Get_NotFound_RequestsFromSource_OnlyIfFlagSet)
    {
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([](TestSource::Data&, Utf8CP, TestSource::RequestOptions const&, IRealityDataSourceResponseReceiver&)
        {
        BeAssert(false);
        return RealityDataSourceResult::Error_Unknown;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("Get_NotFound_RequestsFromSource_OnlyIfFlagSet", *TestRealityData::RequestOptions::Create(false, false));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (RealityDataCacheTests, SourceResponseHandling_Persists)
    {
    BeAtomic<bool> didPersist(false);
    TestStorage& storage = *m_storage;
    m_storage->SetSelectHandler([](TestStorage::Data&, Utf8CP, TestStorage::SelectOptions const&)
        {
        return RealityDataStorageResult::NotFound;
        });
    m_source->SetRequestHandler([&storage, &didPersist](TestSource::Data& data, Utf8CP id, TestSource::RequestOptions const& options, IRealityDataSourceResponseReceiver& responseReceiver)
        {
        BeAssert(storage.m_persistHandlers.size() == 1 && nullptr != *storage.m_persistHandlers.begin());
        TestStorage::PersistHandler* handler = *storage.m_persistHandlers.begin();
        handler->SetPersistHandler([&didPersist]()
            {
            didPersist = true;
            return RealityDataStorageResult::Success;
            });
        responseReceiver._OnResponseReceived(RealityDataSourceResponse(RealityDataSourceResult::Success, id, &data));
        return RealityDataSourceResult::Success;
        });

    RefCountedPtr<TestRealityData> data = m_cache->Get<TestRealityData>("SourceResponseHandling_Persists", *TestRealityData::RequestOptions::Create(false, true));
    ASSERT_TRUE(didPersist);
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TestDatabasePrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
    {
    std::function<BentleyStatus(BeSQLite::Db&)> m_prepareDatabaseHandler;
    std::function<BentleyStatus(BeSQLite::Db&, double percentage)> m_cleanupDatabaseHandler;

    TestDatabasePrepareAndCleanupHandler() : m_prepareDatabaseHandler(nullptr), m_cleanupDatabaseHandler(nullptr) {}
    void SetPrepareDatabaseHandler(std::function<BentleyStatus(BeSQLite::Db&)> const& prepareDatabaseHandler) {m_prepareDatabaseHandler = prepareDatabaseHandler;}
    void SetCleanupDatabaseHandler(std::function<BentleyStatus(BeSQLite::Db&, double percentage)> const& cleanupDatabaseHandler) {m_cleanupDatabaseHandler = cleanupDatabaseHandler;}

    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override { return (nullptr != m_prepareDatabaseHandler ? m_prepareDatabaseHandler(db) : SUCCESS);}
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
        RequestOptions() {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    public:
        static RefCountedPtr<RequestOptions> Create() {return new RequestOptions();}
    };

    TestDatabasePrepareAndCleanupHandler* m_prepareAndCleanupHandler;
    std::function<BentleyStatus(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)> m_initFromHandler;
    std::function<BentleyStatus(BeSQLite::Db& db)>  m_persistHandler;

    TestBeSQLiteStorageData(TestDatabasePrepareAndCleanupHandler* prepareAndCleanupHandler) 
        : m_prepareAndCleanupHandler(prepareAndCleanupHandler), m_initFromHandler(nullptr), m_persistHandler(nullptr)
        {}
    static RefCountedPtr<TestBeSQLiteStorageData> Create(TestDatabasePrepareAndCleanupHandler* prepareAndCleanupHandler = nullptr) {return new TestBeSQLiteStorageData(prepareAndCleanupHandler);}
    void SetInitFromHandler(std::function<BentleyStatus(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)> const& handler) {m_initFromHandler = handler;}
    void SetPersistHandler(std::function<BentleyStatus(BeSQLite::Db& db)> const& handler) {m_persistHandler = handler;}

    virtual Utf8CP _GetId() const override {return nullptr;}
    virtual bool _IsExpired() const override {return false;}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return (nullptr != m_prepareAndCleanupHandler ? m_prepareAndCleanupHandler : new TestDatabasePrepareAndCleanupHandler());}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options) {return (nullptr != m_initFromHandler ? m_initFromHandler(db, key, options) : SUCCESS);}
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const {return (nullptr != m_persistHandler ? m_persistHandler(db) : SUCCESS);}
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

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, Select)
    {
    BeAtomic<bool> didInitialize(false);
    RefCountedPtr<TestBeSQLiteStorageData::RequestOptions> options = TestBeSQLiteStorageData::RequestOptions::Create();
    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create();
    data->SetInitFromHandler([&didInitialize, &options](BeSQLite::Db&, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& opts)
        {
        BeAssert(0 == strcmp("BeSQLiteRealityDataStorageTests.Select_1", id));
        BeAssert(options.get() == &opts);
        didInitialize = true;
        return SUCCESS;
        });
    ASSERT_EQ(RealityDataStorageResult::Success, m_storage->Select(*data, "BeSQLiteRealityDataStorageTests.Select_1", *options));
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
            while(haltPersist);
            didPersist = true;
            cv.notify_all();
            return SUCCESS;
            });
        ASSERT_EQ(RealityDataStorageResult::Success, m_storage->Persist(*data));
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
    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> handler = new TestDatabasePrepareAndCleanupHandler();
    handler->SetPrepareDatabaseHandler([&didPrepare](BeSQLite::Db&)
        {
        didPrepare = true;
        return SUCCESS;
        });

    RefCountedPtr<TestBeSQLiteStorageData> data = TestBeSQLiteStorageData::Create(handler.get());
    m_storage->Select(*data, "", *TestBeSQLiteStorageData::RequestOptions::Create());
    ASSERT_TRUE(didPrepare);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    03/2015
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteRealityDataStorageTests, DoesCleanupDatabase)
    {
    BeAtomic<bool> didCleanup(false);
    BeConditionVariable cv;
    RefCountedPtr<TestDatabasePrepareAndCleanupHandler> handler = new TestDatabasePrepareAndCleanupHandler();
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
struct TestResponseReceiver : IRealityDataSourceResponseReceiver
    {
    std::function<void(RealityDataSourceResponse const&)> m_onResponseReceivedHandler;
    TestResponseReceiver(std::function<void(RealityDataSourceResponse const&)> const& handler) : m_onResponseReceivedHandler(handler) {}
    virtual void _OnResponseReceived(RealityDataSourceResponse const& response) override
        {
        if (nullptr != m_onResponseReceivedHandler)
            m_onResponseReceivedHandler(response);
        }
    static RefCountedPtr<TestResponseReceiver> Create(std::function<void(RealityDataSourceResponse const&)> const& handler = nullptr) {return new TestResponseReceiver(handler);}
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
    RefCountedPtr<TestResponseReceiver> responseReceiver = TestResponseReceiver::Create([&didReceiveResponse, &cv, &data](RealityDataSourceResponse const& response)
        {
        BeAssert(data.get() == response.GetData());
        didReceiveResponse = true;
        cv.notify_all();
        });
    ASSERT_EQ(RealityDataSourceResult::Success, m_source->Request(*data, m_filePath.c_str(), *options, *responseReceiver));

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
        RefCountedPtr<TestResponseReceiver> responseReceiver = TestResponseReceiver::Create([&didReceiveResponse, &cv, &data](RealityDataSourceResponse const& response)
            {
            BeAssert(data.get() == response.GetData());
            didReceiveResponse = true;
            cv.notify_all();
            });
        ASSERT_EQ(RealityDataSourceResult::Success, m_source->Request(*data, m_filePath.c_str(), *options, *responseReceiver));
        }

    block = false;
    cv.WaitOnCondition(nullptr, 10000);
    ASSERT_TRUE(didInitialize);
    ASSERT_TRUE(didReceiveResponse);
    }