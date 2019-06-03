/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <UnitTests/BackDoor/DgnPlatform/RepositoryManagerUtil.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryManagerTest : public DgnDbTestFixture, DgnPlatformLib::Host::RepositoryAdmin
{
public:
    typedef IRepositoryManager::Request Request;
    typedef IRepositoryManager::Response Response;
    typedef IBriefcaseManager::ResponseOptions ResponseOptions;

    mutable TestRepositoryManager   m_server;

    RepositoryManagerTest()
        {
        RegisterServer();
        }

    ~RepositoryManagerTest()
        {
        UnregisterServer();
        }

    void RegisterServer() { m_host.SetRepositoryAdmin(this); }
    void UnregisterServer() { m_host.SetRepositoryAdmin(nullptr); }   // GetRepositoryManager() => nullptr; Attempts to contact server => RepositoryStatus::ServerUnavailable

    IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override { return &m_server; }

    virtual void _OnSetupDb(DgnDbR db) { }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId)
        {
        //** Force to copy the file in Sub-Directory of TestCase
        BeFileName testfixtureName(TEST_FIXTURE_NAME,BentleyCharEncoding::Utf8);
        BeFileName outPath;
        BeTest::GetHost().GetOutputRoot(outPath);
        outPath.AppendToPath(testfixtureName);

        WString fileName(TEST_NAME, BentleyCharEncoding::Utf8);
        fileName.append(L".bim");
        BeFileName sourceFilepath(outPath);
        sourceFilepath.AppendToPath(fileName.c_str());

        BeFileName outFileName(outPath);
        outFileName.AppendToPath(testFile);

        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(sourceFilepath, outFileName));
        //BeFileName::BeCopyFile(fullFileName, outFullFileName) != BeFileNameStatus::Success

        //EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFileName.c_str(), __FILE__));
        auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        if (!db.IsValid())
            return nullptr;

        if (bcId.GetValue() != db->GetBriefcaseId().GetValue())
            {
            BeFileName name(db->GetFileName());
            db->SetAsBriefcase(bcId);
            db->SaveChanges();
            db->CloseDb();
            DbResult result = BE_SQLITE_OK;
            db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
            }

        _OnSetupDb(*db);

        return db;
        }

    void SetupMasterFile(bool andClearTxns=true)
        {
        SetupSeedProject();
        if (andClearTxns)
            ClearRevisions(*m_db);
        }

    void ClearRevisions(DgnDbR db)
        {
        // Ensure the seed file doesn't contain any changes pending for a revision
        if (!db.IsMasterCopy())
            {
            DgnRevisionPtr rev = db.Revisions().StartCreateRevision();
            if (rev.IsValid())
                {
                db.Revisions().FinishCreateRevision();
                db.SaveChanges();
                }
            }
        }
        
    DgnModelPtr CreateModel(Utf8CP name, DgnDbR db)
        {
        SubjectCPtr rootSubject = db.Elements().GetRootSubject();
        PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, name);
        EXPECT_TRUE(partition.IsValid());
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireForElementInsert(*partition));
        EXPECT_TRUE(db.Elements().Insert<PhysicalPartition>(*partition).IsValid());
        PhysicalModelPtr model = PhysicalModel::Create(*partition);
        EXPECT_TRUE(model.IsValid());
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));
        auto status = model->Insert();
        EXPECT_EQ(DgnDbStatus::Success, status);
        return DgnDbStatus::Success == status ? model : nullptr;
        }
};

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   02/17
//=======================================================================================
struct LockableSchemas
{
private:
    DgnDbR m_dgndb;
public:
    LockableSchemas(DgnDbR dgndb) : m_dgndb(dgndb) {}
    DgnDbR GetDgnDb() const { return m_dgndb; }
};

typedef LockableSchemas const& LockableSchemasCR;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksManagerTest : RepositoryManagerTest
{
    static LockableId MakeLockableId(DgnElementCR el) { return LockableId(el.GetElementId()); }
    static LockableId MakeLockableId(DgnModelCR model) { return LockableId(model.GetModelId()); }
    static LockableId MakeLockableId(DgnDbCR db) { return LockableId(db); }
    static LockableId MakeLockableId(LockableSchemasCR lockableSchemas) { return LockableId(lockableSchemas.GetDgnDb().Schemas()); }

    static Utf8String MakeLockableName(LockableId lid)
        {
        Utf8CP typeName = nullptr;
        switch (lid.GetType())
            {
            case LockableType::Db: typeName = "DgnDb"; break;
            case LockableType::Model: typeName = "Model"; break;
            case LockableType::Schemas: typeName = "Schemas"; break;
            default: typeName = "Element"; break;
            }

        Utf8Char buf[0x20];
        BeStringUtilities::FormatUInt64(buf, _countof(buf), lid.GetId().GetValue(), HexFormatOptions());
        Utf8String name(typeName);
        name.append(1, ' ');
        name.append(buf);

        return name;
        }

    void ExpectLevel(LockableId id, LockLevel expLevel, DgnDbR db)
        {
        LockLevel actualLevel;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryLockLevel(actualLevel, id));
        EXPECT_EQ(expLevel, actualLevel) << MakeLockableName(id).c_str();
        }

    template<typename T> void ExpectLevel(T const& obj, LockLevel level)
        {
        ExpectLevel(MakeLockableId(obj), level, ExtractDgnDb(obj));
        }

    template<typename T> static DgnDbR ExtractDgnDb(T const& obj) { return obj.GetDgnDb(); }

    Response AcquireResponse(LockRequestR req, DgnDbR db, ResponseOptions opts=ResponseOptions::None)
        {
        auto response = db.BriefcaseManager().AcquireLocks(req, opts);
        return response;
        }

    template<typename T> Response AcquireResponse(T const& obj, LockLevel level)
        {
        LockRequest req;
        req.Insert(obj, level);
        return AcquireResponse(req, ExtractDgnDb(obj), ResponseOptions::LockState);
        }

    template<typename T> RepositoryStatus Acquire(T const& obj, LockLevel level)
        {
        return AcquireResponse(obj, level).Result();
        }

    template<typename T> void ExpectAcquire(T const& obj, LockLevel level)
        {
        EXPECT_EQ(RepositoryStatus::Success, Acquire(obj, level));
        }

    template<typename T> void ExpectDenied(T const& obj, LockLevel level)
        {
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, Acquire(obj, level));
        }

    RepositoryStatus QueryOwnership(DgnLockOwnershipR ownership, LockableId const& id)
        {
        DgnLockInfoSet states;
        LockableIdSet ids;
        ids.insert(id);
        auto stat = m_server.QueryLockStates(states, ids);
        if (RepositoryStatus::Success == stat)
            {
            EXPECT_EQ(1, states.size());
            ownership = states.begin()->GetOwnership();
            }

        return stat;
        }

    template<typename T> void ExpectInDeniedSet(T const& lockedObj, LockLevel level, Response const& response, DgnDbR requestor)
        {
        // Test that locked obj is in response
        auto lockableId = MakeLockableId(lockedObj);
        auto found = response.LockStates().find(DgnLockInfo(lockableId));
        EXPECT_FALSE(found == response.LockStates().end());
        if (response.LockStates().end() != found)
            EXPECT_EQ(found->GetOwnership().GetLockLevel(), level);

        // Test that response matches direct ownership query
        DgnLockOwnership ownership;
        EXPECT_EQ(RepositoryStatus::Success, QueryOwnership(ownership, lockableId));
        EXPECT_EQ(level, ownership.GetLockLevel());
        auto owningBcId = ExtractDgnDb(lockedObj).GetBriefcaseId();
        switch (level)
            {
            case LockLevel::Exclusive:
                EXPECT_EQ(owningBcId, ownership.GetExclusiveOwner());
                break;
            case LockLevel::Shared:
                EXPECT_TRUE(ownership.GetSharedOwners().end() != ownership.GetSharedOwners().find(owningBcId));
                break;
            }
        }

    DgnElementCPtr CreateElement(DgnModelR model, bool acquireLocks=true)
        {
        auto elem = model.Is3d() ? Create3dElement(model) : Create2dElement(model);
        if (acquireLocks)
            {
            IBriefcaseManager::Request req;
            EXPECT_EQ(RepositoryStatus::Success, model.GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *elem, IBriefcaseManager::PrepareAction::Acquire));
            }

        auto persistentElem = elem->Insert();
        return persistentElem;
        }

    DgnElementPtr Create3dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
        return GenericPhysicalObject::Create(*model.ToPhysicalModelP(), categoryId);
        }

    DgnElementPtr Create2dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstDrawingCategoryId(db);
        return AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, model.GetModelId(), classId, categoryId));
        }

    size_t CountLocks(IOwnedLocksIteratorR iter)
        {
        size_t count = 0;
        for (/*iter*/; iter.IsValid(); ++iter)
            ++count;

        return count;
        }
};

/*---------------------------------------------------------------------------------**//**
* gcc errs if defined inside the class: explicit specialization in non-namespace scope 
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<> DgnDbR LocksManagerTest::ExtractDgnDb(DgnDb const& obj) { return const_cast<DgnDbR>(obj); }

template<> TestRepositoryManager::Response LocksManagerTest::AcquireResponse(LockableSchemasCR const& lockableSchemas, LockLevel level)
    {
    LockRequest req;
    req.InsertSchemasLock(lockableSchemas.GetDgnDb());
    return AcquireResponse(req, lockableSchemas.GetDgnDb(), ResponseOptions::LockState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SingleBriefcaseLocksTest : LocksManagerTest
{
    DEFINE_T_SUPER(LocksManagerTest);

    // Our DgnDb is masquerading as a briefcase...it has no actual master copy.
    BeBriefcaseId   m_bcId;
    DgnModelId      m_modelId;
    DgnElementId    m_elemId;

    void _OnSetupDb(DgnDbR db) override
        {
        m_db = &db;
        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
        }

    void SetUp()
        {
        SetupMasterFile();
        }

    SingleBriefcaseLocksTest() : m_bcId(2) { }
    ~SingleBriefcaseLocksTest() {if (m_db.IsValid()) m_db->CloseDb();}

    DgnModelPtr CreateModel(Utf8CP name) { return T_Super::CreateModel(name, *m_db); }
    void DoIterateOwnedLocks(bool bulkMode);
    void DoDisconnectedWorkflow(bool bulkMode);
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocks)
    {
    SetupDb(L"AcquireLocksTest.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    LockableSchemas lockableSchemas(db);

    ExpectAcquire(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);

    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared); // a shared lock on a model => shared lock on DB

    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);

    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // An exclusive model lock results in exclusive locks on all of its elements
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models, elements and schemas
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);

    // If we obtain an exclusive lock on a model, exclusive locks on its elements should be retained after refresh
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    06/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, ElementSpecificLockTests)
    {
    SetupDb(L"AcquireLocksTest.bim", m_bcId);
    DgnDbR db = *m_db;

    // TFS#901239: Test category and its subcategory insertion inside the _OnInserted call, need locks for sub category
    SpatialCategory cat(db.GetDictionaryModel(), "SpatialCategoryTestInsert", DgnCategory::Rank::Domain); 
    DgnDbStatus catStat;
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementInsert(req, cat, IBriefcaseManager::PrepareAction::Acquire));
    DgnSubCategory::Appearance appearance;
    cat.Insert(appearance, &catStat);
    EXPECT_EQ(catStat, DgnDbStatus::Success);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
TEST_F(SingleBriefcaseLocksTest, ClearUserHeldCodesLocks)
    {
    SetupDb(L"ClearUserHeldCodesLocksTest.bim", m_bcId);

    // Create a new element - requires locking the dictionary model + the db
    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    LockableSchemas lockableSchemas(db);

    ExpectLevel(db, LockLevel::None);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);

    ExpectAcquire(model, LockLevel::Shared);
    ExpectAcquire(el, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared); // a shared lock on a model => shared lock on DB

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ClearUserHeldCodesLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    }

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention ... bulk mode version.
* @bsimethod                                                    Paul.Connelly   10/15
* @bsimethod                                                    Sam.Wilson      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocksBulkMode)
    {
    SetupDb(L"AcquireLocksTestBulkMode.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    LockableSchemas lockableSchemas(db);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ASSERT_TRUE(db.BriefcaseManager().IsBulkOperation()) << "StartBulkOperation should start bulk ops mode";
    ExpectAcquire(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ASSERT_FALSE(db.BriefcaseManager().IsBulkOperation()) << "SaveChanges should end bulk ops mode";
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // An exclusive model lock results in exclusive locks on all of its elements
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models, elements and schemas
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);

    // If we obtain an exclusive lock on a model, exclusive locks on its elements should be retained after refresh
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, RelinquishLocks)
    {
    SetupDb(L"RelinquishLocksTest.bim", m_bcId);

    // Create a new element - requires locking the dictionary model + the db
    DgnDbR db = *m_db;
    auto txnPos = db.Txns().GetCurrentTxnId();
    CategorySelector element(db.GetDictionaryModel(), TEST_NAME);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireForElementInsert(element));
    EXPECT_TRUE(element.Insert().IsValid());

    // Cannot relinquish locks with uncommitted changes
    EXPECT_EQ(RepositoryStatus::PendingTransactions, db.BriefcaseManager().RelinquishLocks());

    // Cannot relinquish locks if we have changed the locked object
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());
    EXPECT_TRUE(db.Txns().IsUndoPossible());
    EXPECT_EQ(RepositoryStatus::LockUsed, db.BriefcaseManager().RelinquishLocks());

    // Undo local changes
    EXPECT_EQ(DgnDbStatus::Success, db.Txns().ReverseTo(txnPos));
    EXPECT_TRUE(db.Txns().IsRedoPossible());
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    // Now we can relinquish locks because we have not changed the objects
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // Cannot undo/redo after relinquishing locks
    EXPECT_FALSE(db.Txns().IsRedoPossible());
    EXPECT_FALSE(db.Txns().IsUndoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* Newly-created models and elements are implicitly exclusively locked by that briefcase.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, LocallyCreatedObjects)
    {
    SetupDb(L"LocallyCreatedObjectsTest.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr elem = db.Elements().GetElement(m_elemId);

    // Create a new model.
    DgnModelPtr newModel = CreateModel("NewModel");

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    // Our exclusive locks are only known locally, because they refer to elements not known to the server
    // If we refresh our local locks from server, we will need to re-obtain them.
    // Server *will* know about shared locks on locally-created models
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(*newModel, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);

    // Create a new element in our new model
    DgnElementCPtr newElem = CreateElement(*newModel);

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(*newElem, LockLevel::None);

    ExpectAcquire(*newElem, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);

    ExpectAcquire(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectAcquire(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* Newly-created models and elements are implicitly exclusively locked by that briefcase ... bulk mode version.
* @bsimethod                                                    Paul.Connelly   11/15
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, LocallyCreatedObjectsBulkMode)
    {
    SetupDb(L"LocallyCreatedObjectsTestBulkMode.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr elem = db.Elements().GetElement(m_elemId);

    // Create a new model.
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    DgnModelPtr newModel = CreateModel("NewModel");
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(*newModel, LockLevel::None);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);
    db.BriefcaseManager().EndBulkOperation();               // <<<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    // Note: EndBulkOperation actually acquired the (exclusive) lock on the new model from the server.

    // Create a new element in our new model
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    DgnElementCPtr newElem = CreateElement(*newModel);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    // TFS#788401: Since we now use model exclusive locking to know if an element is exclusively locked
    // we will have results here that show that the new element is exclusively locked by default, even though
    // the bulk operation hasn't occurred yet.
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);
    db.BriefcaseManager().EndBulkOperation();               // <<<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    // TFS#904880: Even though the QueryLockLevel calls return that we have newElem exclusively locked, this is inferred
    // by the element's model being exclusively locked. If we iterate over the owned locks locally, then we should find that
    // newElem is actually not locked, but its lock is inferred by the locked state of its model
    IBriefcaseManagerR bc = db.BriefcaseManager();
    IOwnedLocksIteratorPtr pIter = bc.GetOwnedLocks();
    for (;pIter->IsValid();++(*pIter))
        {
        DgnLock lock = **pIter;
        EXPECT_FALSE(lock.GetId() == newElem->GetElementId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleBriefcaseLocksTest::DoIterateOwnedLocks(bool bulkMode)
    {
    WPrintfString bcname(L"IterateOwnedLocksTest%ls.bim", (bulkMode? L"BulkMode": L""));
    SetupDb(bcname.c_str(), m_bcId);
    DgnDbR db = *m_db;
    IBriefcaseManagerR bc = db.BriefcaseManager();

    // We hold no locks
    IOwnedLocksIteratorPtr pIter = bc.GetOwnedLocks();
    ASSERT_TRUE(pIter.IsValid());
    EXPECT_FALSE(pIter->IsValid());
    EXPECT_EQ(0, CountLocks(*pIter));

    // Create a new model
    if (bulkMode)
        db.BriefcaseManager().StartBulkOperation();

    DgnModelPtr model = CreateModel("NewModel");
    pIter = bc.GetOwnedLocks();

    if (bulkMode)
        {
        EXPECT_EQ(0, CountLocks(*pIter));
        db.BriefcaseManager().EndBulkOperation();
        pIter->Reset();
        }

    // Creating a model involves creating a partition, which involves:
    //  - Shared lock on db
    //  - Shared lock on model containing partition element
    //  - Exclusive lock on partition element
    //  - Exclusive lock on newly-created model
    EXPECT_EQ(4, CountLocks(*pIter));

    // Iteration consumes the iterator - must manually reset to restart iteration
    EXPECT_FALSE(pIter->IsValid());
    pIter->Reset();
    EXPECT_TRUE(pIter->IsValid());

    // Confirm the locks
    bool haveModel = false,
         haveDb = false,
         havePartitionModel = false,
         havePartitionElement = false;

    for (auto& iter = *pIter; iter.IsValid(); ++iter)
        {
        DgnLock lock = *iter;
        switch (iter->GetType())
            {
            case LockableType::Db:
                {
                EXPECT_FALSE(haveDb);
                EXPECT_FALSE(lock.IsExclusive());
                haveDb = true;
                break;
                }
            case LockableType::Model:
                {
                if (lock.GetId() == model->GetModelId())
                    {
                    EXPECT_FALSE(haveModel);
                    EXPECT_TRUE(lock.IsExclusive());
                    haveModel = true;
                    }
                else
                    {
                    // Partition element's model
                    EXPECT_FALSE(havePartitionModel);
                    EXPECT_FALSE(lock.IsExclusive());
                    havePartitionModel = true;
                    }
                break;
                }
            case LockableType::Element:
                {
                // Partition element
                EXPECT_FALSE(havePartitionElement);
                EXPECT_TRUE(lock.IsExclusive());
                havePartitionElement = true;
                break;
                }
            }
        }

    EXPECT_TRUE(haveDb);
    EXPECT_TRUE(haveModel);
    EXPECT_TRUE(havePartitionElement);
    EXPECT_TRUE(havePartitionModel);

    pIter = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, IterateOwnedLocks)
    {
    DoIterateOwnedLocks(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/17
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, IterateOwnedLocksBulkMode)
    {
    DoIterateOwnedLocks(true);
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleBriefcaseLocksTest::DoDisconnectedWorkflow(bool bulkMode)
    {
    BeFileName filename;
    DgnModelId newModelId;
        {
        SetupDb(L"DisconnectedWorkflowTest.bim", m_bcId);

        DgnDbR db = *m_db;
        DgnModelPtr pModel = db.Models().GetModel(m_modelId);
        DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);

        if (bulkMode)
            db.BriefcaseManager().StartBulkOperation();

        // Acquire locks with server available
        EXPECT_TRUE(nullptr != T_HOST.GetRepositoryAdmin()._GetRepositoryManager(db));
        ExpectAcquire(*cpEl, LockLevel::Exclusive);
        if (bulkMode)
            {
            ExpectLevel(db, LockLevel::None);
            ExpectLevel(*pModel, LockLevel::None);
            ExpectLevel(*cpEl, LockLevel::None);
            }
        else
            {
            ExpectLevel(db, LockLevel::Shared);
            ExpectLevel(*pModel, LockLevel::Shared);
            ExpectLevel(*cpEl, LockLevel::Exclusive);
            }

        // Create a new model (implicitly exclusively locked)
        DgnModelPtr newModel = CreateModel("NewModel");
        newModelId = newModel->GetModelId();
        if (bulkMode)
            ExpectLevel(*newModel, LockLevel::None);
        else
            ExpectLevel(*newModel, LockLevel::Exclusive);

        // Close the db and unregister the server
        m_db->SaveChanges();

        if (bulkMode)
            {
            // in build operation mode, all locks are acquired by savechanges
            ExpectLevel(db, LockLevel::Shared);
            ExpectLevel(*pModel, LockLevel::Shared);
            ExpectLevel(*cpEl, LockLevel::Exclusive);
            ExpectLevel(*newModel, LockLevel::Exclusive);
            }

        filename = m_db->GetFileName();
        pModel = nullptr;
        newModel = nullptr;
        cpEl = nullptr;
        m_db->CloseDb();
        m_db = nullptr;
        UnregisterServer();
        }

    // Reopen the db and expect that the locks we acquired remain acquired
        {
        m_db = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());
        EXPECT_EQ(nullptr, T_HOST.GetRepositoryAdmin()._GetRepositoryManager(*m_db));

        DgnModelPtr pModel = m_db->Models().GetModel(m_modelId);
        DgnElementCPtr cpEl = m_db->Elements().GetElement(m_elemId);
        ASSERT_TRUE(pModel.IsValid());
        ASSERT_TRUE(cpEl.IsValid());

        ExpectLevel(*m_db, LockLevel::Shared);
        ExpectLevel(*pModel, LockLevel::Shared);
        ExpectLevel(*cpEl, LockLevel::Exclusive);

        // Expect we cannot acquire new locks due to unavailability of server
        EXPECT_EQ(RepositoryStatus::ServerUnavailable, Acquire(*pModel, LockLevel::Exclusive));

        // Expect that we implicitly acquire locks for locally-created objects despite unavailability of server
        DgnModelPtr pNewModel = m_db->Models().GetModel(newModelId);
        ASSERT_TRUE(pNewModel.IsValid());
        ExpectLevel(*pNewModel, LockLevel::Exclusive);
        DgnElementCPtr newElem = CreateElement(*pNewModel);
        ASSERT_TRUE(newElem.IsValid());
        ExpectLevel(*newElem, LockLevel::Exclusive);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, DisconnectedWorkflow)
    {
    DoDisconnectedWorkflow(false);
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, DisconnectedWorkflowBulkMode)
    {
    DoDisconnectedWorkflow(true);
    }

/*---------------------------------------------------------------------------------**//**
* Simulate two briefcases operating against the same master DgnDb.
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DoubleBriefcaseTest : LocksManagerTest
{
    DEFINE_T_SUPER(LocksManagerTest);

    DgnDbPtr m_dbA;
    DgnDbPtr m_dbB;

    DgnModelId m_modelIds[2];
    DgnElementId m_elemIds[4];

    ~DoubleBriefcaseTest() 
        {
        if (m_dbA.IsValid())
            m_dbA->SaveChanges();
        if (m_dbB.IsValid())
            m_dbB->SaveChanges();
        }

    DgnModelId Model3dId() { return m_modelIds[0]; }
    DgnModelId Model2dId() { return m_modelIds[1]; }

    DgnElementId Elem3dId1() { return m_elemIds[0]; } 
    DgnElementId Elem3dId2() { return m_elemIds[1]; } 
    DgnElementId Elem2dId1() { return m_elemIds[2]; } 
    DgnElementId Elem2dId2() { return m_elemIds[3]; } 

    DgnModelPtr GetModel(DgnDbR db, bool twoD) { return db.Models().GetModel(twoD ? Model2dId() : Model3dId()); }
    DgnElementCPtr GetElement(DgnDbR db, DgnElementId id) { return db.Elements().GetElement(id); }

    bool LookupElementIds(DgnElementId* ids, DgnModelR model)
        {
        uint32_t nElemsFound = 0;
        for (auto const& elems : model.MakeIterator())
            {
            ids[nElemsFound++] = elems.GetElementId();
            if (2 == nElemsFound)
                return true;
            }

        return false;
        }

    void SetUp()
        {
        SetupMasterFile(false);
        auto defaultModel = GetDefaultPhysicalModel();
        CreateElement(*defaultModel, false);
        CreateElement(*defaultModel, false);
        DgnModelPtr modelA2d0 = CreateModel("Model2d", *m_db);
        CreateElement(*modelA2d0.get(), false);
        CreateElement(*modelA2d0.get(), false);
        _InitMasterFile();
        m_db->SaveChanges();
        ClearRevisions(*m_db);
        }

    virtual void _InitMasterFile() { };

    void SetupDbs(uint32_t baseBcId=2)
        {
        m_dbA = SetupDb(L"DbA.bim", BeBriefcaseId(baseBcId));
        m_dbB = SetupDb(L"DbB.bim", BeBriefcaseId(baseBcId+1));

        ASSERT_TRUE(m_dbA.IsValid());
        ASSERT_TRUE(m_dbB.IsValid());

        // Model + element IDs may vary each time we convert the v8 file...need to look them up.
        DgnModelId model2d, model3d;
        for (ModelIteratorEntryCR entry : m_dbA->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
            {
            auto model = m_dbA->Models().GetModel(entry.GetModelId());
            if (model.IsValid())
                {
                if (!model3d.IsValid() && LookupElementIds(m_elemIds, *model))
                    {
                    model3d = entry.GetModelId();
                    }
                else if (!model2d.IsValid() && LookupElementIds(m_elemIds+2, *model))
                    {
                    model2d = entry.GetModelId();
                    break;
                    }
                }
            }

        ASSERT_TRUE(model2d.IsValid() && model3d.IsValid());
        m_modelIds[0] = model3d;
        m_modelIds[1] = model2d;
        }

    void RelinquishAll()
        {
        EXPECT_EQ(RepositoryStatus::Success, m_dbA->BriefcaseManager().RelinquishLocks());
        EXPECT_EQ(RepositoryStatus::Success, m_dbB->BriefcaseManager().RelinquishLocks());
        }
};

/*---------------------------------------------------------------------------------**//**
* Acquisition of locks may result in contention with another briefcase.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, Contention)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;
    DgnModelPtr modelA2d = GetModel(dbA, true);
    DgnModelPtr modelB2d = GetModel(dbB, true);
    DgnElementCPtr elemA2d2 = GetElement(dbA, Elem2dId2());
    DgnElementCPtr elemB2d2 = GetElement(dbB, Elem2dId2());

    // Both briefcases can hold shared locks
    ExpectAcquire(*modelA2d, LockLevel::Shared);
    ExpectAcquire(*elemB2d2, LockLevel::Exclusive);
    ExpectLevel(*modelA2d, LockLevel::Shared);
    ExpectLevel(dbA, LockLevel::Shared);
    ExpectLevel(*elemA2d2, LockLevel::None);
    ExpectLevel(*modelB2d, LockLevel::Shared);
    ExpectLevel(*elemB2d2, LockLevel::Exclusive);
    ExpectLevel(dbB, LockLevel::Shared);

    // No briefcase can exclusively lock a model if a shared lock already held on it by another briefcase
    ExpectDenied(*modelA2d, LockLevel::Exclusive);
    ExpectDenied(*elemA2d2, LockLevel::Exclusive);

    // One briefcase can acquire exclusive lock on a model
    DgnModelPtr modelA3d = GetModel(dbA, false);
    DgnModelPtr modelB3d = GetModel(dbB, false);
    ExpectAcquire(*modelA3d, LockLevel::Exclusive);
    ExpectDenied(*modelB3d, LockLevel::Shared);
    ExpectDenied(*modelB3d, LockLevel::Exclusive);

    // One briefcase cannot obtain an exclusive lock on an element in a model exclusively locked by another briefcase
    DgnElementCPtr elemA3d2 = GetElement(dbA, Elem3dId2());
    DgnElementCPtr elemB3d2 = GetElement(dbB, Elem3dId2());
    ExpectDenied(*elemB3d2, LockLevel::Exclusive);
    ExpectLevel(*modelA3d, LockLevel::Exclusive);
    ExpectLevel(*elemA3d2, LockLevel::Exclusive);
    ExpectLevel(*modelB3d, LockLevel::None);
    ExpectLevel(*elemB3d2, LockLevel::None);

    // No briefcase can lock the db unless no shared locks exist
    ExpectDenied(dbA, LockLevel::Exclusive);
    ExpectDenied(dbB, LockLevel::Exclusive);

    // If one briefcase relinquishes its locks they become available to others
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().RelinquishLocks());
    ExpectAcquire(dbB, LockLevel::Exclusive);
    ExpectLevel(dbA, LockLevel::None);
    ExpectLevel(*modelA3d, LockLevel::None);
    ExpectLevel(*modelA2d, LockLevel::None);
    ExpectLevel(*elemA3d2, LockLevel::None);
    ExpectLevel(dbB, LockLevel::Exclusive);
    ExpectLevel(*modelB3d, LockLevel::Exclusive);
    ExpectLevel(*elemB3d2, LockLevel::Exclusive);

    // B's exclusive Db lock prevents A from acquiring any locks, shared or otherwise
    ExpectDenied(dbA, LockLevel::Shared);
    ExpectDenied(*modelA3d, LockLevel::Shared);
    ExpectDenied(*elemA3d2, LockLevel::Shared);
    ExpectDenied(*elemA2d2, LockLevel::Shared);
    ExpectDenied(dbA, LockLevel::Exclusive);
    ExpectDenied(*modelA3d, LockLevel::Exclusive);
    ExpectDenied(*elemA3d2, LockLevel::Exclusive);
    ExpectDenied(*elemA2d2, LockLevel::Exclusive);

    // B's exclusive locks imply shared ownership
    ExpectAcquire(dbB, LockLevel::Shared);
    ExpectAcquire(*modelB3d, LockLevel::Shared);
    ExpectAcquire(*elemB3d2, LockLevel::Shared);
    ExpectAcquire(*elemB2d2, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* Operations on elements and models USED TO automatically acquire locks.
* Now, they do NOT - they only verify that the requisite locks were acquired prior to the
* operation. Caller must explicitly acquire those locks.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, AutomaticLocking)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;

    // Creating a new model requires shared ownership of Db, implies exclusive ownership of new model
    DgnModelPtr newModelA = CreateModel("New Model A", dbA);
    ASSERT_TRUE(newModelA.IsValid());
    ExpectLevel(dbA, LockLevel::Shared);
    ExpectLevel(*newModelA, LockLevel::Exclusive);

    // Creating a new element requires shared ownership of model, implies exclusive ownership of new element
    DgnModelPtr modelB3d = GetModel(dbB, false);
    DgnElementCPtr newElemB3d = CreateElement(*modelB3d);
    ASSERT_TRUE(newElemB3d.IsValid());
    ExpectLevel(dbB, LockLevel::Shared);
    ExpectLevel(*modelB3d, LockLevel::Shared);
    ExpectLevel(*newElemB3d, LockLevel::Exclusive);

    // Deleting a model requires exclusive ownership
    DgnModelPtr modelA2d = GetModel(dbA, true);
    ExpectLevel(*modelA2d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, modelA2d->Delete());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().PrepareForModelDelete(req, *modelA2d, IBriefcaseManager::PrepareAction::Acquire));
    ExpectLevel(*modelA2d, LockLevel::Exclusive);
    EXPECT_EQ(DgnDbStatus::Success, modelA2d->Delete());
    DgnModelPtr modelA3d = GetModel(dbA, false);
    ExpectLevel(*modelA3d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, modelA3d->Delete()); // because B has a shared lock on it
    ExpectLevel(*modelA3d, LockLevel::None);

    // Adding an element to a model requires shared ownership
    DgnModelPtr modelB2d = GetModel(dbB, true);
    ExpectLevel(*modelB2d, LockLevel::None);
    DgnElementCPtr newElemB2d = CreateElement(*modelB2d, false);
    EXPECT_TRUE(newElemB2d.IsNull());   // no return status to check...
    ExpectLevel(*modelB2d, LockLevel::None);

    // At this point, ownership is:
    //  Object      A   B
    //  Model2d     Ex  No      // deleted
    //    Elem1     Ex  No      // deleted
    //    Elem2     Ex  No      // deleted
    //  Model3d     Sh  Sh
    //    Elem1     No  No
    //    Elem2     No  No

    // Modifying an element requires shared ownership of model and exclusive ownership of element
    DgnElementCPtr cpElemB2d = GetElement(dbB, Elem2dId1());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, cpElemB2d->Delete());
    DgnElementPtr pElemB2d = cpElemB2d->CopyForEdit();
    DgnDbStatus status;
    EXPECT_TRUE(pElemB2d->Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, status);
    ExpectLevel(*cpElemB2d, LockLevel::None);
    ExpectLevel(*modelB2d, LockLevel::None);

    DgnElementCPtr cpElemB3d = GetElement(dbB, Elem3dId1());
    ExpectLevel(*cpElemB3d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, cpElemB3d->Delete());
    LockRequest lockReq;
    lockReq.Insert(*cpElemB3d, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(lockReq).Result());
    ExpectLevel(*cpElemB3d, LockLevel::Exclusive);
    EXPECT_EQ(DgnDbStatus::Success, cpElemB3d->Delete());

    DgnElementPtr pElemB3d2 = GetElement(dbB, Elem3dId2())->CopyForEdit();
    ExpectLevel(*pElemB3d2, LockLevel::None);
    EXPECT_FALSE(pElemB3d2->Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, status);
    lockReq.Clear();
    lockReq.Insert(*pElemB3d2, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(lockReq).Result());
    ExpectLevel(*pElemB3d2, LockLevel::Exclusive);
    EXPECT_TRUE(pElemB3d2->Update(&status).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Locks are not required for dynamic operations, as all changes will be reverted when
* the operation completes.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, Dynamics)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;

    // Briefcase A exclusively locks model
    auto modelA = GetModel(dbA, true);
    auto elemA = GetElement(dbA, Elem2dId1());
    ExpectAcquire(*modelA, LockLevel::Exclusive);
    ExpectAcquire(*elemA, LockLevel::Exclusive);

    // Briefcase B cannot obtain locks during normal transactions
    auto modelB = GetModel(dbB, true);
    auto elemB = GetElement(dbB, Elem2dId1());
    ExpectDenied(*modelB, LockLevel::Shared);
    ExpectDenied(*elemB, LockLevel::Exclusive);

    // Briefcase B starts a dynamic operation
    auto& txns = dbB.Txns();
    txns.BeginDynamicOperation();
        // within dynamics, Briefcase A can make temporary changes without acquiring locks
        // delete an element within the model
        EXPECT_EQ(DgnDbStatus::Success, elemB->Delete());
        ExpectLevel(*elemB, LockLevel::None);
        ExpectLevel(*modelB, LockLevel::None);
        EXPECT_EQ(DgnDbStatus::Success, modelB->Delete());
        ExpectLevel(*modelB, LockLevel::None);
    txns.EndDynamicOperation();

    ExpectLevel(*modelB, LockLevel::None);
    ExpectLevel(*modelA, LockLevel::Exclusive);
    ExpectLevel(*elemB, LockLevel::None);
    ExpectLevel(*elemA, LockLevel::Exclusive);
    }

/*---------------------------------------------------------------------------------**//**
* Test that:
*   - Reponse from AcquireLocks() includes set of denied locks (if option specified in request)
*   - QueryOwnership response matches that set of denied locks
*   - ReformulateRequest produces a set of locks which are not in denied set
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, ReformulateRequest)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;
    DgnModelPtr modelA2d = GetModel(dbA, true);
    DgnModelPtr modelB2d = GetModel(dbB, true);
    DgnElementCPtr elemA2d2 = GetElement(dbA, Elem2dId2());
    DgnElementCPtr elemB2d2 = GetElement(dbB, Elem2dId2());

    LockableId dbLockId = MakeLockableId(dbB);

    // Model: Locked exclusively
        {
        ExpectAcquire(*modelA2d, LockLevel::Exclusive);
        auto response = AcquireResponse(*modelB2d, LockLevel::Shared);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
        EXPECT_EQ(1, response.LockStates().size());

        ExpectInDeniedSet(*modelA2d, LockLevel::Exclusive, response, dbB);
        // Reformulate request => remove request for model
        Request request;
        request.Locks().Insert(*modelB2d, LockLevel::Shared);
        dbB.BriefcaseManager().ReformulateRequest(request, response);
        EXPECT_EQ(1, request.Locks().Size());
        EXPECT_TRUE(request.Locks().Contains(dbLockId));
        }

    RelinquishAll();

    // Model: Locked shared by both
        {
        ExpectAcquire(*modelA2d, LockLevel::Shared);
        ExpectAcquire(*modelB2d, LockLevel::Shared);

        // Try to lock exclusively
        auto response = AcquireResponse(*modelB2d, LockLevel::Exclusive);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
        EXPECT_EQ(1, response.LockStates().size());
        ExpectInDeniedSet(*modelA2d, LockLevel::Shared, response, dbB);

        // Expect two shared owners
        DgnLockOwnership ownership;
        auto modelLockId = MakeLockableId(*modelA2d);
        EXPECT_EQ(RepositoryStatus::Success, QueryOwnership(ownership, modelLockId));
        EXPECT_EQ(LockLevel::Shared, ownership.GetLockLevel());
        auto const& sharedOwners = ownership.GetSharedOwners();
        EXPECT_EQ(2, sharedOwners.size());
        EXPECT_TRUE(sharedOwners.find(dbA.GetBriefcaseId()) != sharedOwners.end());
        EXPECT_TRUE(sharedOwners.find(dbB.GetBriefcaseId()) != sharedOwners.end());

        // Reformulate request => demote exclusive to shared
        Request request;
        request.Locks().Insert(*modelB2d, LockLevel::Exclusive);
        dbB.BriefcaseManager().ReformulateRequest(request, response);
        EXPECT_EQ(2, request.Locks().Size());
        EXPECT_EQ(LockLevel::Shared, request.Locks().GetLockLevel(modelLockId));
        EXPECT_TRUE(request.Locks().Contains(dbLockId));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(request.Locks()).Result());
        }

    RelinquishAll();

    // One element locked, one not
        {
        ExpectAcquire(*elemA2d2, LockLevel::Exclusive);

        // Try to lock both elems
        DgnElementCPtr elemB2d1 = GetElement(dbB, Elem2dId1());
        Request req;
        req.Locks().Insert(*elemB2d2, LockLevel::Exclusive);
        req.Locks().Insert(*elemB2d1, LockLevel::Exclusive);
        auto response = AcquireResponse(req.Locks(), dbB, ResponseOptions::LockState);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());

        // Reformulate request => remove one element
        dbB.BriefcaseManager().ReformulateRequest(req, response);
        EXPECT_EQ(LockLevel::Exclusive, req.Locks().GetLockLevel(MakeLockableId(*elemB2d1)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*elemB2d2)));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(req.Locks()).Result());
        }

    RelinquishAll();

    // One model exclusively locked, one not
        {
        DgnModelPtr modelA3d = GetModel(dbA, false);
        ExpectAcquire(*modelA2d, LockLevel::Exclusive);

        // Try to lock one elem from each model
        DgnElementCPtr elemB3d2 = GetElement(dbB, Elem3dId2());
        Request req;
        req.Locks().Insert(*elemB2d2, LockLevel::Exclusive);
        req.Locks().Insert(*elemB3d2, LockLevel::Exclusive);
        auto response = AcquireResponse(req.Locks(), dbB, ResponseOptions::LockState);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());

        // Reformulate request => remove element in locked model
        dbB.BriefcaseManager().ReformulateRequest(req, response);
        EXPECT_EQ(LockLevel::Exclusive, req.Locks().GetLockLevel(MakeLockableId(*elemB3d2)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*elemB2d2)));
        EXPECT_EQ(LockLevel::Shared, req.Locks().GetLockLevel(MakeLockableId(*modelA3d)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*modelB2d)));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(req.Locks()).Result());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void touchElement(DgnElementCR el)
    {
    BeThreadUtilities::BeSleep(1);
    auto mod = el.CopyForEdit();
    mod->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, DemoteLocks)
    {
    SetupDbs();
    DgnDbR db = *m_dbA;
    DgnModelPtr model2d = GetModel(db, true);
    DgnModelPtr model3d = GetModel(db, false);
    DgnElementCPtr elem2d2 = GetElement(db, Elem2dId2());
    DgnElementCPtr elem3d2 = GetElement(db, Elem3dId2());

    LockableId model2dLockId = MakeLockableId(*model2d);
    LockableId elem2d2LockId = MakeLockableId(*elem2d2);

    // Lock model + elements exclusively
    ExpectAcquire(*elem2d2, LockLevel::Exclusive);
    ExpectAcquire(*elem3d2, LockLevel::Exclusive);
    ExpectAcquire(*model2d, LockLevel::Exclusive);
    ExpectAcquire(*model3d, LockLevel::Exclusive);

    // We can demote model lock from exclusive to shared while retaining element lock
    DgnLockSet toRelease;
    toRelease.insert(DgnLock(model2dLockId, LockLevel::Shared));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model2d, LockLevel::Shared);
    ExpectLevel(*elem2d2, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    // Releasing shared lock on model also release lock on element within that model (but not the element in the other model)
    toRelease.clear();
    toRelease.insert(DgnLock(model2dLockId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model2d, LockLevel::None);
    ExpectLevel(*elem2d2, LockLevel::None);
    ExpectLevel(*elem3d2, LockLevel::Exclusive);
    ExpectLevel(*model3d, LockLevel::Exclusive);

    // Modify the 3d elem
    auto txnId = db.Txns().GetCurrentTxnId();
    touchElement(*elem3d2);
    db.SaveChanges();

    // Can demote model lock to shared while retaining lock on modified element within i
    LockableId model3dLockId = MakeLockableId(*model3d);
    toRelease.clear();
    toRelease.insert(DgnLock(model3dLockId, LockLevel::Shared));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model3d, LockLevel::Shared);
    ExpectLevel(*elem3d2, LockLevel::Exclusive);

    // Cannot release model lock if elements within it have been modified
    toRelease.clear();
    toRelease.insert(DgnLock(model3dLockId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::LockUsed, db.BriefcaseManager().DemoteLocks(toRelease));

    // If we undo the element change, we can release the locks
    EXPECT_EQ(DgnDbStatus::Success, db.Txns().ReverseTo(txnId));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*elem3d2, LockLevel::None);
    ExpectLevel(*model3d, LockLevel::None);
    
    // After releasing lock, redo of change to previously-locked element is not possible
    EXPECT_FALSE(db.Txns().IsRedoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* A 'standalone briefcase' is a transactable DgnDb which did not originate from a master
* copy - i.e., it is not associated with a repository on a server somewhere. Generally
* used for tests; for developer activities operating on local files; or for initializing
* a DgnDb which will later become promoted to be the master DgnDb for a new repository
* on a server.
* TxnManager is enabled; no requirement to acquire locks/codes.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleBriefcaseTest, StandaloneBriefcase)
    {
    SetupDbs(BeBriefcaseId::Master());

    DgnDbR master = *m_dbA; // master copy
    DgnDbR brief  = *m_dbB; // standalone briefcase

    EXPECT_TRUE(master.IsMasterCopy());
    EXPECT_FALSE(master.Txns().IsUndoPossible());

    // Locks unconditionally granted for both...
    ExpectLevel(master, LockLevel::Exclusive);
    ExpectLevel(*GetElement(master, Elem2dId2()), LockLevel::Exclusive);
    ExpectAcquire(*GetElement(master, Elem3dId2()), LockLevel::Exclusive);

    ExpectLevel(brief, LockLevel::Exclusive);
    ExpectLevel(*GetElement(brief, Elem2dId2()), LockLevel::Exclusive);
    ExpectAcquire(*GetElement(brief, Elem3dId2()), LockLevel::Exclusive);

    // So are codes...
    AnnotationTextStyle style(master.GetDictionaryModel());
    style.SetName("MyStyle");
    DgnCode code = style.GetCode(); // the only reason we created the style...
    EXPECT_EQ(RepositoryStatus::Success, master.BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::Success, brief.BriefcaseManager().ReserveCode(code));

    // TxnManager is ONLY enabled for briefcase...
    EXPECT_TRUE(style.Insert().IsValid());
    EXPECT_EQ(BE_SQLITE_OK, master.SaveChanges());
    EXPECT_FALSE(master.Txns().IsUndoPossible());

    AnnotationTextStyle briefStyle(brief.GetDictionaryModel());
    briefStyle.SetName("MyStyle");
    EXPECT_TRUE(briefStyle.Insert().IsValid());
    EXPECT_EQ(BE_SQLITE_OK, brief.SaveChanges());
    EXPECT_TRUE(brief.Txns().IsUndoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, CodeSpecsLock)
    {
    SetupDbs();

    // briefcase A
        {
        DgnDbR dbA = *m_dbA;
        CodeSpecPtr codeSpecA1 = CodeSpec::Create(dbA, "A1", CodeScopeSpec::CreateModelScope());
        CodeSpecPtr codeSpecA2 = CodeSpec::Create(dbA, "A2", CodeScopeSpec::CreateModelScope());
        EXPECT_TRUE(codeSpecA1.IsValid());
        EXPECT_TRUE(codeSpecA2.IsValid());

        EXPECT_EQ(DgnDbStatus::Success, codeSpecA1->Insert());

        dbA.BriefcaseManager().StartBulkOperation();
        EXPECT_EQ(DgnDbStatus::Success, codeSpecA2->Insert());
        dbA.SaveChanges();

        EXPECT_TRUE(dbA.CodeSpecs().GetCodeSpec(codeSpecA1->GetName().c_str()).IsValid());
        EXPECT_TRUE(dbA.CodeSpecs().GetCodeSpec(codeSpecA2->GetName().c_str()).IsValid());
        }

    // briefcase B
        {
        DgnDbR dbB = *m_dbB;
        CodeSpecPtr codeSpecB = CodeSpec::Create(dbB, "B", CodeScopeSpec::CreateModelScope());
        EXPECT_TRUE(codeSpecB.IsValid());
        BeTest::SetFailOnAssert(false);
        DgnDbStatus insertStatus = codeSpecB->Insert();
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::LockNotHeld, insertStatus);
        }
    }

/*---------------------------------------------------------------------------------**//**
* For queries which need to be fast and not necessarily 100% up to date with server,
* IBriefcaseManager supplies a FastQuery option which queries a local copy of locks+codes
* known to be unavailable to this briefcase, either because another briefcase owns them
* or because a required revision has not been pulled.
* The primary use case for this is tools which want to filter out elements for which the
* lock is not available.
* Test that:
*   - While the cache is in sync with server, fast and slow queries both return same results
*   - When cache becomes out of sync, fast queries continue to return previous results
*   - When cache is out of sync, attempts to actually acquire locks/codes are validated against the server, not the local cache
*   - After refreshing local cache, fast queries are again in sync with server state
* @bsistruct                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct FastQueryTest : DoubleBriefcaseTest
{
    DEFINE_T_SUPER(DoubleBriefcaseTest);

    typedef IBriefcaseManager::FastQuery FastQuery;
    typedef IBriefcaseManager::RequestPurpose Purpose;

    template<typename T> static Utf8String ToString(T const& t)
        {
        Json::Value v;
        t.ToJson(v);
        return Json::FastWriter::ToString(v);
        }

    void ExpectEqual(DgnLockInfo const& a, DgnLockInfo const& b)
        {
        EXPECT_EQ(a.IsTracked(), b.IsTracked());
        EXPECT_EQ(a.IsOwned(), b.IsOwned());
        EXPECT_EQ(a.GetOwnership().GetLockLevel(), b.GetOwnership().GetLockLevel());
        }

    void ExpectEqual(DgnCodeInfo const& a, DgnCodeInfo const& b)
        {
        EXPECT_EQ(a.IsAvailable(), b.IsAvailable());
        EXPECT_EQ(a.IsReserved(), b.IsReserved());
        EXPECT_EQ(a.IsUsed(), b.IsUsed());
        EXPECT_EQ(a.IsDiscarded(), b.IsDiscarded());
        }

    static bool AreSameEntity(DgnLockInfo const& a, DgnLockInfo const& b) { return a.GetLockableId() == b.GetLockableId(); }
    static bool AreSameEntity(DgnCodeInfo const& a, DgnCodeInfo const& b) { return a.GetCode() == b.GetCode(); }

    template<typename T> void ExpectEqual(T const& a, T const& b)
        {
        EXPECT_EQ(a.size(), b.size());

        for (auto const& aInfo : a)
            {
            auto pbInfo = std::find_if(b.begin(), b.end(), [&](decltype(aInfo) arg) { return AreSameEntity(aInfo, arg); });
            EXPECT_FALSE(b.end() == pbInfo) << "Present in a only: " << ToString(aInfo).c_str();
            if (b.end() != pbInfo)
                ExpectEqual(aInfo, *pbInfo);
            }

        for (auto const& bInfo : b)
            {
            auto paInfo = std::find_if(a.begin(), a.end(), [&](decltype(bInfo) arg) { return AreSameEntity(bInfo, arg); });
            EXPECT_FALSE(a.end() == paInfo);
            }
        }

    void ExpectResponsesEqual(Response const& a, Response const& b)
        {
        EXPECT_EQ(a.Result(), b.Result());
        ExpectEqual(a.LockStates(), b.LockStates());
        ExpectEqual(a.CodeStates(), b.CodeStates());
        }

    void ExpectResponsesEqual(Request& req, DgnDbR db)
        {
        Request fastReq = req; // function modifies input Request...
        Response response(Purpose::Query, req.Options()), fastResponse(Purpose::FastQuery, req.Options());
        EXPECT_EQ(db.BriefcaseManager().AreResourcesAvailable(req, &response, FastQuery::No), db.BriefcaseManager().AreResourcesAvailable(fastReq, &fastResponse, FastQuery::Yes));
        ExpectResponsesEqual(response, fastResponse);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FastQueryTest, CacheLocks)
    {
    SetupDbs();

    // dbA will acquire locks. dbB will make fast queries.
    DgnDbR dbA = *m_dbA,
           dbB = *m_dbB;
    DgnModelPtr modelA1 = GetModel(dbA, true),
                modelA2 = GetModel(dbA, false),
                modelB1 = GetModel(dbB, true),
                modelB2 = GetModel(dbB, false);
    DgnElementCPtr elemA1 = GetElement(dbA, Elem2dId2()),
                   elemA2 = GetElement(dbA, Elem3dId2()),
                   elemB1 = GetElement(dbB, Elem2dId2()),
                   elemB2 = GetElement(dbB, Elem3dId2());

    ExpectAcquire(*modelA1, LockLevel::Exclusive);
    ExpectAcquire(*modelA2, LockLevel::Shared);
    ExpectAcquire(*elemA1, LockLevel::Exclusive);

    // Make sure local state is pulled for dbB
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().RefreshFromRepository());

    // Confirm fast queries return same results as queries against server
    Request req(ResponseOptions::LockState);
    ExpectResponsesEqual(req, dbB);

    req.Reset();
    req.SetOptions(ResponseOptions::LockState);
    req.Locks().Insert(*modelB1, LockLevel::Shared);    // unavailable
    req.Locks().Insert(*modelB2, LockLevel::Shared);    // available
    req.Locks().Insert(*elemB1, LockLevel::Exclusive);  // unavailable - but not in denied set, because in exclusively locked model
    req.Locks().Insert(*elemB2, LockLevel::Exclusive);  // available
    ExpectResponsesEqual(req, dbB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FastQueryTest, CacheCodes)
    {
    SetupDbs();

    // dbA will acquire codes. dbB will make fast queries
    DgnDbR dbA = *m_dbA,
           dbB = *m_dbB;

    DefinitionModelR dictionaryA = dbA.GetDictionaryModel();

    DgnCode code1 = RenderMaterial::CreateCode(dictionaryA, "Palette", "One"),
            code2 = RenderMaterial::CreateCode(dictionaryA, "Palette", "Two");

    // reserve codes
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().ReserveCodes(codes).Result());

    // Make sure local state is pulled for dbB
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().RefreshFromRepository());

    // Confirm fast queries return same results as queries against server
    Request req(ResponseOptions::CodeState);
    ExpectResponsesEqual(req, dbB);

    DgnCode code3 = RenderMaterial::CreateCode(dictionaryA, "Palette", "Three");
    req.Reset();
    req.SetOptions(ResponseOptions::CodeState);
    req.Codes().insert(code1);  // unavailable
    req.Codes().insert(code3);  // available
    ExpectResponsesEqual(req, dbB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractLocksTest : SingleBriefcaseLocksTest
{
    DgnElementCPtr  m_onRootChangedElement = nullptr;

    DgnDbStatus ExtractLocks(LockRequestR req, bool extractInserted = true, bool avoidExclusiveModelElements = true)
        {
        if (BE_SQLITE_OK != m_db->SaveChanges())
            return DgnDbStatus::WriteError;

        RevisionStatus revStat;
        DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision(&revStat);
        if (rev.IsNull())
            {
            if (RevisionStatus::NoTransactions == revStat)
                {
                req.Clear();
                return DgnDbStatus::Success;
                }
            return DgnDbStatus::BadRequest;
            }

        req.FromRevision(*rev, *m_db, extractInserted, avoidExclusiveModelElements);
        m_db->Revisions().AbandonCreateRevision();
        return DgnDbStatus::Success;
        }

    //-------------------------------------------------------------------------------------------
    // @bsimethod                                                 Diego.Pinate     12/17
    //-------------------------------------------------------------------------------------------
    DgnDbStatus Commit()
        {
        if (BE_SQLITE_OK != m_db->SaveChanges())
            return DgnDbStatus::WriteError;

        RevisionStatus revStat;
        DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision(&revStat);
        if (rev.IsNull())
            {
            if (RevisionStatus::NoTransactions == revStat)
                return DgnDbStatus::Success;
            return DgnDbStatus::BadRequest;
            }
        
        return m_db->Revisions().FinishCreateRevision() == RevisionStatus::Success ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
        }
};

/*---------------------------------------------------------------------------------**//**
* Test functions which query the set of locks which are required by the changes actually
* made in the briefcase. (Excluding locks obtained but not actually used).
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtractLocksTest, UsedLocks)
    {
    struct AssertScope
    {
        AssertScope() { BeTest::SetFailOnAssert(false); }
        ~AssertScope() { BeTest::SetFailOnAssert(true); }
    };

    struct UndoScope
    {
        DgnDbR m_db;
        TxnManager::TxnId m_id;
        UndoScope(DgnDbR db) : m_db(db), m_id(db.Txns().GetCurrentTxnId()) { }
        ~UndoScope() { m_db.Txns().ReverseTo(m_id); }
    };

    // Need Test domain for ElementDrivesElement dependency tests
    //DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);

    AssertScope V_V_V_Asserts;
    m_db = SetupDb(L"UsedLocksTests.bim", m_bcId);

    // Import domain
    //SchemaStatus schemaStatus = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db);

    DgnDbR db = *m_db;
    DgnModelR model = *db.Models().GetModel(m_modelId);
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    DgnModelPtr testModel = CreateModel("TestModelLocks");
    DgnElementCPtr testElem = CreateElement(*testModel);
    EXPECT_EQ(DgnDbStatus::Success, Commit());

    LockRequest req;
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Modify an elem (it's a SpatialCategory...)
        {
        UndoScope V_V_V_Undo(db);
        auto pEl = cpEl->CopyForEdit();
        DgnCode newCode = SpatialCategory::CreateCode(db.GetDictionaryModel(), "RenamedCategory");
        EXPECT_EQ(DgnDbStatus::Success, pEl->SetCode(newCode));
        IBriefcaseManager::Request bcreq;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementUpdate(bcreq, *pEl, IBriefcaseManager::PrepareAction::Acquire));
        cpEl = pEl->Update();
        ASSERT_TRUE(cpEl.IsValid());

        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
        EXPECT_FALSE(req.IsEmpty());
        EXPECT_EQ(4, req.Size());
        // We add exclusive model locks even if no changes present but user has lock over it
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*testModel)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(model)));
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*pEl)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Test creating a new model and a new element inside the new model WITHOUT extracting inserted locks
        {
        UndoScope V_V_V_Undo(db);
        // Create a new model
        DgnModelPtr newModel = CreateModel("NewModel");
        DgnElementCPtr newElem = CreateElement(*newModel);

        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req, false));
        // A lock for the db, a lock for the root subject which the new model is created in
        EXPECT_EQ(3, req.Size());
        // TFS#788401: Now, we don't extract locks for inserted elements in the revision
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(LockableId(*newModel)));
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(LockableId(*newElem)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Test creating a new model and a new element inside the new model extracting inserted locks
        {
        UndoScope V_V_V_Undo(db);
        // Create a new model
        DgnModelPtr newModel = CreateModel("NewModel");
        DgnElementCPtr newElem = CreateElement(*newModel);

        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req, true));
        EXPECT_EQ(6, req.Size());
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newModel)));
        // TFS#788401: We imply that since the new model is locked exclusively, we don't extract locks for those elements
        // contained in a exclusively locked model, as they are exclusively locked by default
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(LockableId(*newElem)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel (LockableId(db)));
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*testModel)));
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Test creating an element inside an existing model WITHOUT extracting inserted locks
        {
        UndoScope V_V_V_Undo(db);
        DgnElementCPtr newElem2 = CreateElement(*testModel);

        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req, false));
        // Locks: Db and existing model (all shared)
        EXPECT_EQ(4, req.Size());
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(LockableId(*newElem2)));
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*testModel)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Test deleting an element in the existing model
        {
        UndoScope V_V_V_Undo(db);
        // Delete the new element
        LockableId testElemId (*testElem);
        EXPECT_EQ(DgnDbStatus::Success, testElem->Delete());
        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
        EXPECT_EQ(4, req.Size());
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*testModel)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(testElemId)); // Model is exclusive, so lock for element not present
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Delete the new model
        {
        UndoScope V_V_V_Undo(db);
        LockableId testModelId (*testModel);
        LockableId testElemId (*testElem);
        EXPECT_EQ(DgnDbStatus::Success, testModel->Delete());
        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
        // Locks for db, model, the element that was in the model, and the root where the model was
        EXPECT_EQ(4, req.Size());
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(testModelId));
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(testElemId)); // Model is exclusive, so lock for element not present
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

#if defined NOTNOW
    // Indirect changes shouldn't be extracted as required locks
        {
        UndoScope V_V_V_Undo(db);
        // Create two elements and insert the relationship between them
        DgnElementCPtr root = CreateElement(*testModel);
        DgnElementCPtr dependent = CreateElement(*testModel);
        TestElementDrivesElementHandler::SetCallback(this);
        TestElementDrivesElementHandler::Insert(db, root->GetElementId(), dependent->GetElementId());
        // Set the testElem in the Db as the element that we will update in the _OnRootChanged call
        m_onRootChangedElement = testElem;
        // Trigger a change in the root which will call our Callback on this element, causing an update on an element
        DgnElementPtr rootEdit = root->CopyForEdit();
        rootEdit->SetUserLabel("New Root Label");
        EXPECT_TRUE(db.Elements().Update<DgnElement>(*rootEdit).IsValid());
        // Calling ExtractLocks below will trigger the _OnRootChange called when saving changes to the Db and extracting necessary locks
        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));

        // Result of dependency callback, we shouldn't extract locks for the update done to this element
        EXPECT_EQ(LockLevel::None, req.GetLockLevel(LockableId(m_onRootChangedElement->GetElementId())));
        // Get rid of the static callback pointer
        TestElementDrivesElementHandler::SetCallback(nullptr);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesManagerTest : RepositoryManagerTest
{
    void SetUp()
        {
        SetupMasterFile();
        }

    static DgnCodeInfo MakeAvailable(DgnCodeCR code) { return DgnCodeInfo(code); }
    static DgnCodeInfo MakeUsed(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, false); }
    static DgnCodeInfo MakeDiscarded(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, true); }
    static DgnCodeInfo MakeReserved(DgnCodeCR code, DgnDbR db)
        {
        DgnCodeInfo info(code);
        info.SetReserved(db.GetBriefcaseId());
        return info;
        }

    static DgnCodeInfo MakeUsedOrDiscarded(DgnCodeCR code, Utf8StringCR rev, bool discarded)
        {
        DgnCodeInfo info(code);
        if (discarded)
            info.SetDiscarded(rev);
        else
            info.SetUsed(rev);
        return info;
        }

    void ExpectState(DgnCodeInfoCR expect, DgnDbR db)
        {
        DgnCodeInfoSet expectInfos;
        expectInfos.insert(expect);
        ExpectStates(expectInfos, db);
        }

    void ExpectStates(DgnCodeInfoSet const& expect, DgnDbR db)
        {
        DgnCodeInfoSet actual;
        DgnCodeSet codes;
        for (auto const& info : expect)
            codes.insert(info.GetCode());

        EXPECT_STATUS(Success, m_server.QueryCodeStates(actual, codes));
        ExpectEqual(expect, actual);
        }

    void ExpectEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual)
        {
        EXPECT_EQ(expected.size(), actual.size());
        for (auto expIter = expected.begin(), actIter = actual.begin(); expIter != expected.end() && actIter != actual.end(); ++expIter, ++actIter)
            ExpectEqual(*expIter, *actIter);
        }

    void ExpectEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act)
        {
        EXPECT_EQ(exp.GetCode(), act.GetCode());
        ExpectEqual(static_cast<DgnCodeState>(exp), static_cast<DgnCodeState>(act));
        }

    void ExpectEqual(DgnCodeStateCR exp, DgnCodeStateCR act)
        {
        if (exp.IsAvailable())
            {
            EXPECT_TRUE(act.IsAvailable());
            }
        else if (exp.IsReserved())
            {
            EXPECT_TRUE(act.IsReserved());
            EXPECT_EQ(exp.GetReservedBy().GetValue(), act.GetReservedBy().GetValue());
            }
        else
            {
            EXPECT_EQ(exp.IsDiscarded(), act.IsDiscarded());
            EXPECT_EQ(exp.GetRevisionId(), act.GetRevisionId());
            EXPECT_FALSE(exp.GetRevisionId().empty());
            }
        }

    Utf8String CommitRevision(DgnDbR db);

    static DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db, bool expectSuccess = true)
        {
        AnnotationTextStyle style(db.GetDictionaryModel());
        style.SetName(name);
        IBriefcaseManager::Request req;
        EXPECT_EQ(expectSuccess, RepositoryStatus::Success == db.BriefcaseManager().PrepareForElementInsert(req, style, IBriefcaseManager::PrepareAction::Acquire));
        DgnDbStatus status;
        style.DgnElement::Insert(&status);
        return status;
        }

    static PhysicalElementPtr InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, BeGuidCR federationGuid=BeGuid(), DgnCodeCR code=DgnCode())
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
        EXPECT_TRUE(element.IsValid());
        element->SetFederationGuid(federationGuid);
        element->SetCode(code);
        EXPECT_TRUE(element->Insert().IsValid());
        return element;
        }

    static DgnCode MakeStyleCode(Utf8CP name, DgnDbR db)
        {
        // Because AnnotationTextStyle::CreateCodeFromName() is private for some reason...
        AnnotationTextStyle style(db.GetDictionaryModel());
        style.SetName(name);
        return style.GetCode();
        }

    DgnCodeSet GetReservedCodes(DgnDbR db)
        {
        DgnCodeSet codes;
        EXPECT_STATUS(Success, m_server._QueryCodes(codes, db));
        return codes;
        }

    bool IsCodeReserved(IBriefcaseManager& briefcaseManager, DgnCodeCR code)
        {
        DgnCodeSet codes;
        codes.insert(code);
        return briefcaseManager.AreCodesReserved(codes);
        }

    void AcquireSharedLockOnModel(DgnDbR db, DgnModelId modelId)
        {
        DgnModelPtr model = db.Models().GetModel(modelId);
        EXPECT_TRUE(model.IsValid());
        IBriefcaseManager::Request request;
        request.Locks().Insert(*model, LockLevel::Shared);
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().Acquire(request).Result());
        }

    DgnElementCPtr CreateGeometryPart(DefinitionModelR model, Utf8String name)
        {
        DgnDbR db = model.GetDgnDb();
        DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(db, DgnGeometryPart::CreateCode(model, name.c_str()));
        // Need to append some geometry or else the part is invalid
        GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(db, true);
        DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
        ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
        GeometricPrimitivePtr elmGeom = GeometricPrimitive::Create(*curvePrimitive);
        builder->Append(*elmGeom);
        EXPECT_EQ(SUCCESS, builder->Finish(*geomPart));
        IBriefcaseManager::Request req;
        auto persistentElem = db.Elements().Insert<DgnGeometryPart>(*geomPart);
        EXPECT_TRUE(persistentElem.IsValid());
        return persistentElem;
        }

};

/*---------------------------------------------------------------------------------**//**
* Simulate pushing a revision to the server.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodesManagerTest::CommitRevision(DgnDbR db)
    {
    Utf8String revId;
    DgnRevisionPtr rev;
    if (BE_SQLITE_OK != db.SaveChanges() || (rev = db.Revisions().StartCreateRevision()).IsNull())
        return revId;

    if (RevisionStatus::Success == db.Revisions().FinishCreateRevision())
        {
        m_server.OnFinishRevision(*rev, db);
        revId = rev->GetId();
        }

    return revId;
    }

/*---------------------------------------------------------------------------------**//**
* Exercise the basic functions: reserve + release codes, query code state
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, ReserveQueryRelinquish)
    {
    DgnDbPtr pDb = SetupDb(L"ReserveQueryRelinquishTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    DefinitionModelR dictionary = db.GetDictionaryModel();
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    // Empty request
    DgnCodeSet req;
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    // Reserve single code
    DgnCode code = RenderMaterial::CreateCode(dictionary, "Palette", "Material");
    req.insert(code);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());
    ExpectState(MakeReserved(code, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code), db);

    // Reserve 2 codes
    DgnCode code2 = SpatialCategory::CreateCode(dictionary, "Category");
    req.insert(code2);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());
    ExpectState(MakeReserved(code, db), db);
    ExpectState(MakeReserved(code2, db), db);

    // Release one code
    DgnCodeSet codes;
    codes.insert(code);
    EXPECT_STATUS(Success, mgr.ReleaseCodes(codes));
    ExpectState(MakeAvailable(code), db);
    ExpectState(MakeReserved(code2, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code2), db);
    }

/*---------------------------------------------------------------------------------**//**
* We treat certain codes as internal only and filter them out of requests when in
* bulk mode (e.g. Geometry Parts)
* Necessary for bridges workflow to avoid creating huge code request of internal
* geometry parts
* @bsimethod                                                    Diego.Pinate    03/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, FilteredCodes)
    {
    DgnDbPtr pDb = SetupDb(L"FilteredCodesTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    // Start bulk operation
    db.BriefcaseManager().StartBulkOperation();
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(db, "FilteredCodesTestPhysicalModel");
    DgnElementCPtr geomPart = CreateGeometryPart(db.GetDictionaryModel(), "FC_GeometryPart1");
    // Both should be available before ending the bulk operation
    ExpectState(MakeAvailable(geomPart->GetCode()), db);
    ExpectState(MakeAvailable(physicalModel->GetModeledElement()->GetCode()), db);
    // End bulk operation
    db.BriefcaseManager().EndBulkOperation();
    // Filtered out since we filter geometry part codes
    ExpectState(MakeAvailable(geomPart->GetCode()), db);
    // A regular modeled element should still be reserved
    ExpectState(MakeReserved(physicalModel->GetModeledElement()->GetCode(), db), db);

    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Test that any INSERT or UPDATE will fail if the object's code has not been reserved.
* NOTE: Previously, insert/update would attempt to reserve the code automatically for us.
* That is no longer the case - if we don't explicitly reserve it prior to insert/update,
* the operation will fail with DgnDbStatus::CodeNotReserved.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, AutoReserveCodes)
    {
    DgnDbPtr pDb = SetupDb(L"AutoReserveCodesTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    // Simulate a pre-existing style having been committed in a previous revision
    static const Utf8String s_initialRevisionId("InitialRevision");
    DgnCode existingStyleCode = MakeStyleCode("Preexisting", db);
    m_server.MarkUsed(existingStyleCode, s_initialRevisionId);

    ExpectState(MakeUsed(existingStyleCode, s_initialRevisionId), db);
    EXPECT_EQ(0, GetReservedCodes(db).size());

    // When we insert an element without having explicitly reserved its code, an attempt to reserve it will automatically occur
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("MyStyle", db));
    EXPECT_EQ(1, GetReservedCodes(db).size());
    ExpectState(MakeReserved(MakeStyleCode("MyStyle", db), db), db);

    // An attempt to insert an element with the same code as an already-used code will fail
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, InsertStyle(existingStyleCode.GetValueUtf8().c_str(), db, false));

    // Updating an element and changing its code will NOT reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), "MyStyle")->CreateCopy();
    DgnDbStatus status;
    pStyle->SetName("MyRenamedStyle");
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    // Explicitly reserve the code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(pStyle->GetCode()));
    EXPECT_TRUE(pStyle->Update().IsValid());
    EXPECT_EQ(2, GetReservedCodes(db).size());
    ExpectState(MakeReserved(pStyle->GetCode(), db), db);
    pStyle = nullptr;

    // Attempting to change code to an already-used code will fail on update
    pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), "MyRenamedStyle")->CreateCopy();
    pStyle->SetName(existingStyleCode.GetValueUtf8().c_str());
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);

    pStyle = nullptr;

    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkCodesAreReserved(DgnDbR db, DgnCodeSet const& codes, bool expectedValue)
    {
    DgnCodeInfoSet codeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(codeStates, codes));
    for (DgnCodeInfo const& codeState : codeStates)
        {
        EXPECT_EQ(expectedValue, codeState.IsReserved());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, AutoReserveCodesBulkOpMode)
    {
    DgnDbPtr pDb = SetupDb(L"PlantScenarioTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    db.BriefcaseManager().StartBulkOperation();
    ASSERT_TRUE(db.BriefcaseManager().IsBulkOperation());

    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(db, "TestPhysicalModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "TestSpatialCategory");

    DgnCodeSet codes;
    codes.insert(physicalModel->GetModeledElement()->GetCode());
    codes.insert(db.Elements().GetElement(categoryId)->GetCode());
    checkCodesAreReserved(db, codes, false);

    db.BriefcaseManager().EndBulkOperation();

    checkCodesAreReserved(db, codes, true);

    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* When we commit a revision, the changes made to the revision are processed such that:
*   - Any codes which were newly-assigned become marked as "used", and
*   - Any previously-used codes which became no-longer-used are marked as "discarded"
* In both cases the server records the revision ID.
* Codes which become "used" were necessarily previously "reserved" by the briefcase.
* After committing the revision, they are no longer "reserved".
* Codes which became "discarded" were necessarily previously "used".
* After committing the revision, they become available for reserving by any briefcase,
* provided the briefcase has pulled the revision in which they became discarded.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, CodesInRevisions)
    {
    DgnDbPtr pDb = SetupDb(L"CodesInRevisionsTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    // Reserve some codes
    DgnCode unusedCode = MakeStyleCode("Unused", db);
    DgnCode usedCode = MakeStyleCode("Used", db);
    DgnCodeSet req;
    req.insert(unusedCode);
    req.insert(usedCode);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db));
    ExpectState(MakeReserved(unusedCode, db), db);
    ExpectState(MakeReserved(usedCode, db), db);

    // Commit the change as a revision
    Utf8String rev1 = CommitRevision(db);
    EXPECT_FALSE(rev1.empty());

    // The used code should not be marked as such
    ExpectState(MakeUsed(usedCode, rev1), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // We cannot reserve a used code
    EXPECT_STATUS(CodeUnavailable, mgr.ReserveCode(usedCode));

    // Swap the code so that "Used" becomes "Unused"
    auto pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), "Used")->CreateCopy();
    pStyle->SetName("Unused");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the revision
    Utf8String rev2 = CommitRevision(db);
    EXPECT_FALSE(rev2.empty());

    // "Used" is now discarded; "Unused" is now used; both in the same revision
    ExpectState(MakeUsed(unusedCode, rev2), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // Delete the style => its code becomes discarded
    // Ugh except you are not allowed to delete text styles...rename it again instead
    pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), "Unused")->CreateCopy();
    pStyle->SetName("Deleted");
    // Will fail because we haven't reserved code...
    DgnDbStatus status;
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(MakeStyleCode("Deleted", db)));
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::Success, status);
    pStyle = nullptr;

    // Cannot release codes if transactions are pending
    EXPECT_EQ(1, GetReservedCodes(db).size());
    DgnCodeSet codes;
    codes.insert(MakeStyleCode("Deleted", db));
    EXPECT_STATUS(PendingTransactions, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(PendingTransactions, mgr.RelinquishCodes());

    // Cannot release a code which is used locally
    db.SaveChanges();
    EXPECT_EQ(1, GetReservedCodes(db).size());
    EXPECT_STATUS(CodeUsed, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(CodeUsed, mgr.RelinquishCodes());

    Utf8String rev3 = CommitRevision(db);
    EXPECT_FALSE(rev3.empty());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // We can reserve either code, since they are discarded and we have the latest revision
    EXPECT_STATUS(Success, mgr.ReserveCode(usedCode));
    EXPECT_STATUS(Success, mgr.ReserveCode(unusedCode));
    ExpectState(MakeReserved(usedCode, db), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // If we release these codes, they should return to "Discarded" and retain the most recent revision ID in which they were discarded.
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);
    }

/*---------------------------------------------------------------------------------**//**
* It is common in Plant to reserve codes before elements exist.
* @bsimethod                                                    Shaun.Sewall    05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, PlantScenario)
    {
    DgnDbPtr pDb = SetupDb(L"PlantScenarioTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    EXPECT_FALSE(db.BriefcaseManager().IsBulkOperation());
    db.BriefcaseManager().StartBulkOperation();
    EXPECT_TRUE(db.BriefcaseManager().IsBulkOperation());

    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(db, "TestPhysicalModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "TestSpatialCategory");

    CodeSpecPtr unitCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Unit", CodeScopeSpec::CreateModelScope());
    EXPECT_TRUE(unitCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, unitCodeSpec->Insert());
    EXPECT_TRUE(unitCodeSpec->IsModelScope());
    EXPECT_FALSE(unitCodeSpec->GetScope().IsFederationGuidRequired());

    Utf8CP fakeRelationship = "Fake.EquipmentScopedByUnit"; // relationship name not validated by CodeScopeSpec yet
    CodeSpecPtr equipmentCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Equipment", CodeScopeSpec::CreateRelatedElementScope(fakeRelationship, CodeScopeSpec::ScopeRequirement::FederationGuid));
    EXPECT_TRUE(equipmentCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, equipmentCodeSpec->Insert());
    EXPECT_TRUE(equipmentCodeSpec->IsRelatedElementScope());
    EXPECT_STREQ(equipmentCodeSpec->GetScope().GetRelationship().c_str(), fakeRelationship);
    EXPECT_TRUE(equipmentCodeSpec->GetScope().IsFederationGuidRequired());

    CodeSpecPtr nozzleCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Nozzle", CodeScopeSpec::CreateParentElementScope(CodeScopeSpec::ScopeRequirement::FederationGuid));
    EXPECT_TRUE(nozzleCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, nozzleCodeSpec->Insert());
    EXPECT_TRUE(nozzleCodeSpec->IsParentElementScope());
    EXPECT_TRUE(nozzleCodeSpec->GetScope().IsFederationGuidRequired());

    CodeSpecPtr externalCodeSpec = CodeSpec::Create(db, "CodesManagerTest.External", CodeScopeSpec::CreateRepositoryScope(CodeScopeSpec::ScopeRequirement::FederationGuid));
    EXPECT_TRUE(externalCodeSpec.IsValid());
    externalCodeSpec->SetIsManagedWithDgnDb(false); // indicates these codes are managed externally (not by iModelHub with the DgnDb)
    EXPECT_EQ(DgnDbStatus::Success, externalCodeSpec->Insert());
    EXPECT_TRUE(externalCodeSpec->IsRepositoryScope());
    EXPECT_FALSE(externalCodeSpec->IsManagedWithDgnDb());

    db.SaveChanges("1");

    BeGuid unitGuid(true);
    BeGuid equipment1Guid(true);
    BeGuid nozzle11Guid(true);
    BeGuid nozzle12Guid(true);
    BeGuid equipment2Guid(true);
    BeGuid nozzle21Guid(true);
    BeGuid nozzle22Guid(true);
    BeGuid nozzle23Guid(true);

    DgnCode unitCode = unitCodeSpec->CreateCode(*physicalModel, "U1");
    DgnCode equipment1Code = equipmentCodeSpec->CreateCode(unitGuid, "U1-E1");
    DgnCode nozzle11Code = nozzleCodeSpec->CreateCode(equipment1Guid, "N1");
    DgnCode nozzle12Code = nozzleCodeSpec->CreateCode(equipment1Guid, "N2");
    DgnCode equipment2Code = equipmentCodeSpec->CreateCode(unitGuid, "U1-E2");
    DgnCode nozzle21Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N1");
    DgnCode nozzle22Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N2");
    DgnCode nozzle23Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N3");
    DgnCode nozzle24Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N4");
    DgnCode nozzle25Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N5");
    DgnCode nozzleNullCode = nozzleCodeSpec->CreateCode(equipment2Guid, "");

    DgnCodeSet codesToReserve;
    codesToReserve.insert(unitCode);
    codesToReserve.insert(equipment1Code);
    codesToReserve.insert(nozzle11Code);
    codesToReserve.insert(nozzle12Code);
    codesToReserve.insert(equipment2Code);
    codesToReserve.insert(nozzle21Code);
    codesToReserve.insert(nozzle22Code);
    codesToReserve.insert(nozzle23Code);
    codesToReserve.insert(nozzle24Code);
    codesToReserve.insert(nozzle25Code);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesToReserve).Result());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesToReserve));

    DgnCodeSet codesFromServer = GetReservedCodes(db);
    for (DgnCodeCR code : codesToReserve)
        {
        EXPECT_TRUE(codesFromServer.end() != codesFromServer.find(code));
        }

    DgnCodeInfoSet codeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(codeStates, codesToReserve));
    for (DgnCodeInfo const& codeState : codeStates)
        {
        EXPECT_TRUE(codeState.IsReserved());
        }

    IBriefcaseManager::Request request1, request2;
    request1.Codes() = codesToReserve;
    Json::Value requestJson(Json::objectValue);
    request1.ToJson(requestJson);
    EXPECT_TRUE(request2.FromJson(requestJson));
    EXPECT_EQ(request2.Codes().size(), codesToReserve.size());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(request2.Codes()));

    DgnCodeSet wrongCodes;
    wrongCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U2")); // wrong name
    wrongCodes.insert(unitCodeSpec->CreateCode(db.GetDictionaryModel(), "U1")); // wrong model
    wrongCodes.insert(DgnCode(unitCodeSpec->GetCodeSpecId(), DgnElementId(), "U1")); // invalid scope
    wrongCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N3")); // wrong scope

    for (DgnCodeCR wrongCode : wrongCodes)
        {
        BeTest::SetFailOnAssert(false);
        bool isCodeReserved = IsCodeReserved(db.BriefcaseManager(), wrongCode);
        BeTest::SetFailOnAssert(true);

        EXPECT_FALSE(isCodeReserved) << "wrongCode should not be reserved";
        EXPECT_TRUE(codesFromServer.end() == codesFromServer.find(wrongCode)) << "Should not find wrongCode on server";
        }

    DgnCodeSet availableCodes;
    availableCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U2"));
    availableCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U3"));
    availableCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N3"));
    availableCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N4"));

    IBriefcaseManager::Request availableCodesRequest;
    availableCodesRequest.Codes() = availableCodes;
    EXPECT_TRUE(db.BriefcaseManager().AreResourcesAvailable(availableCodesRequest));
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(availableCodesRequest.Codes()));

    DgnCodeInfoSet availableCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(availableCodeStates, availableCodes));
    for (DgnCodeInfo const& codeState : availableCodeStates)
        {
        EXPECT_TRUE(codeState.IsAvailable());
        }

    DgnCodeSet codesEmpty;
    DgnCodeSet codesSkipped;
    codesSkipped.insert(DgnCode::CreateEmpty());
    codesSkipped.insert(DgnCode());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesEmpty)) << "Should return true because there is nothing to reserve (empty list of codes)";
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesSkipped)) << "Should return true because there is nothing to reserve (effectively an empty list of codes)";

    DgnCodeSet codesToRelease;
    codesToRelease.insert(nozzle24Code);
    codesToRelease.insert(nozzle25Code);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReleaseCodes(codesToRelease));
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(codesToRelease));

    db.BriefcaseManager().StartBulkOperation();

    PhysicalElementPtr unitElement = InsertPhysicalElement(*physicalModel, categoryId, unitGuid, unitCode);
    PhysicalElementPtr equipment1Element = InsertPhysicalElement(*physicalModel, categoryId, equipment1Guid, equipment1Code);
    PhysicalElementPtr nozzle11Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle11Guid, nozzle11Code);
    PhysicalElementPtr nozzle12Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle12Guid, nozzle12Code);
    PhysicalElementPtr equipment2Element = InsertPhysicalElement(*physicalModel, categoryId, equipment2Guid, equipment2Code);
    PhysicalElementPtr nozzle21Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle21Guid, nozzle21Code);
    PhysicalElementPtr nozzle22Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle22Guid, nozzle22Code);
    PhysicalElementPtr nozzle23Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle23Guid, nozzle23Code);
    PhysicalElementPtr nozzleElementNullCode1 = InsertPhysicalElement(*physicalModel, categoryId, BeGuid(), nozzleNullCode);
    PhysicalElementPtr nozzleElementNullCode2 = InsertPhysicalElement(*physicalModel, categoryId, BeGuid(), nozzleNullCode);

    EXPECT_EQ(unitCode, unitElement->GetCode());
    EXPECT_EQ(equipment1Code, equipment1Element->GetCode());
    EXPECT_EQ(nozzle11Code, nozzle11Element->GetCode());
    EXPECT_EQ(nozzle12Code, nozzle12Element->GetCode());
    EXPECT_EQ(equipment2Code, equipment2Element->GetCode());
    EXPECT_EQ(nozzle21Code, nozzle21Element->GetCode());
    EXPECT_EQ(nozzle22Code, nozzle22Element->GetCode());
    EXPECT_EQ(nozzle23Code, nozzle23Element->GetCode());
    EXPECT_EQ(nozzleNullCode, nozzleElementNullCode1->GetCode());
    EXPECT_EQ(nozzleNullCode, nozzleElementNullCode2->GetCode());

    DgnCodeSet reservedCodes;
    reservedCodes.insert(unitElement->GetCode());
    reservedCodes.insert(equipment1Element->GetCode());
    reservedCodes.insert(nozzle11Element->GetCode());
    reservedCodes.insert(nozzle12Element->GetCode());
    reservedCodes.insert(equipment2Element->GetCode());
    reservedCodes.insert(nozzle21Element->GetCode());
    reservedCodes.insert(nozzle22Element->GetCode());
    reservedCodes.insert(nozzle23Element->GetCode());

    DgnCodeInfoSet reservedCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(reservedCodeStates, reservedCodes));
    for (DgnCodeInfo const& codeState : reservedCodeStates)
        {
        EXPECT_TRUE(codeState.IsReserved());
        }

    DgnCodeSet nullCodes;
    nullCodes.insert(nozzleNullCode);
    nullCodes.insert(DgnCode::CreateEmpty());

    DgnCodeInfoSet nullCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(nullCodeStates, nullCodes));
    for (DgnCodeInfo const& codeState : nullCodeStates)
        {
        EXPECT_FALSE(codeState.IsReserved());
        }

    db.SaveChanges("2");
    CommitRevision(db); // simulates pushing revision to server which should mark codes as used

    DgnCodeSet usedCodes;
    usedCodes.insert(unitCode);
    usedCodes.insert(equipment1Code);
    usedCodes.insert(nozzle11Code);
    usedCodes.insert(nozzle12Code);
    usedCodes.insert(equipment2Code);
    usedCodes.insert(nozzle21Code);
    usedCodes.insert(nozzle22Code);
    usedCodes.insert(nozzle23Code);

    DgnCodeInfoSet usedCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(usedCodeStates, usedCodes));
    for (DgnCodeInfo const& codeState : usedCodeStates)
        {
        EXPECT_TRUE(codeState.IsUsed());
        }

    DgnCode externalCode1 = externalCodeSpec->CreateCode("External1");
    DgnCode externalCode2 = externalCodeSpec->CreateCode("External2");
    DgnCode externalCode3 = externalCodeSpec->CreateCode("External3");

    DgnCodeSet reservedExternalCodes;
    reservedExternalCodes.insert(externalCode1);
    reservedExternalCodes.insert(externalCode2);
    reservedExternalCodes.insert(externalCode3);
    IBriefcaseManager::Response response(db.BriefcaseManager().ReserveCodes(reservedExternalCodes, ResponseOptions::CodeState));
    EXPECT_EQ(RepositoryStatus::Success, response.Result());
    EXPECT_TRUE(reservedExternalCodes.empty()) << "Expect external codes to be culled from request";

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(DgnCode())); // ReserveCode returns Success on invalid Code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(externalCode1)); // Therefore, it should also return Success for an external code (even though it wasn't actually reserved)

    DgnCodeInfoSet externalCodeStates;
    DgnCodeSet externalCodes;
    externalCodes.insert(externalCode1);
    externalCodes.insert(externalCode2);
    externalCodes.insert(externalCode3);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(externalCodeStates, externalCodes));
    EXPECT_EQ(externalCodes.size(), externalCodeStates.size());
    for (DgnCodeInfoCR codeState : externalCodeStates)
        {
        EXPECT_TRUE(codeState.IsAvailable()); // External codes are not known to iModelHub
        }
    }

/*---------------------------------------------------------------------------------**//**
* Code value uniqueness constraint uses COLLATE NOCASE, which means case-insensitive strictly for
* the ASCII uppercase characters A-Z.
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, Collation)
    {
    DgnDbPtr pDb = SetupDb(L"CodeCollationTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    DgnCode abc = MakeStyleCode("abc", db);
    DgnCodeSet req;
    req.insert(abc);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    DgnCode ABC = MakeStyleCode("ABC", db);
    EXPECT_TRUE(ABC == abc);
    EXPECT_TRUE(abc == ABC);
    EXPECT_FALSE(ABC < abc);
    EXPECT_FALSE(abc < ABC);
    EXPECT_FALSE(ABC != abc);
    EXPECT_FALSE(abc != ABC);

    EXPECT_TRUE(IsCodeReserved(mgr, ABC));

    req.clear();
    req.insert(ABC);
    EXPECT_EQ(1, req.size());
    req.insert(abc);
    EXPECT_EQ(1, req.size());

    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("ABC", db));
    db.SaveChanges();

    ExpectState(MakeReserved(abc, db), db);
    ExpectState(MakeReserved(ABC, db), db);

    EXPECT_EQ(DgnDbStatus::DuplicateCode, InsertStyle("abc", db, false));

    EXPECT_TRUE(db.Elements().QueryElementIdByCode(abc).IsValid());
    EXPECT_TRUE(db.Elements().QueryElementIdByCode(ABC).IsValid());
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct IndirectLocksTest : DoubleBriefcaseTest
{
    DgnElementId    m_displayStyleId;

    void _InitMasterFile() override
        {
        DisplayStyle2d style(m_db->GetDictionaryModel(), "MyDisplayStyle");
        style.Insert();
        m_displayStyleId = style.GetElementId();
        ASSERT_TRUE(m_displayStyleId.IsValid());
        }

    void Acquire(DgnElementCR el, BeSQLite::DbOpcode op)
        {
        IBriefcaseManager::Request req;
        EXPECT_STATUS(Success, el.PopulateRequest(req, op));
        EXPECT_STATUS(Success, el.GetDgnDb().BriefcaseManager().Acquire(req).Result());
        }

    bool DeleteDisplayStyle(DgnDbR db);
    bool CreateView(DgnDbR db);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IndirectLocksTest::DeleteDisplayStyle(DgnDbR db)
    {
    DgnCode styleCode = DisplayStyle::CreateCode(db.GetDictionaryModel(), "MyDisplayStyle");
    auto style = db.Elements().Get<DisplayStyle>(db.Elements().QueryElementIdByCode(styleCode));
    EXPECT_TRUE(style.IsValid());
    if (!style.IsValid())
        return false;

    Acquire(*style, BeSQLite::DbOpcode::Delete);
    return DgnDbStatus::Success == style->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IndirectLocksTest::CreateView(DgnDbR db)
    {
    CategorySelectorPtr categorySelector = new CategorySelector(db.GetDictionaryModel(), "MyCategorySelector");
    Acquire(*categorySelector, BeSQLite::DbOpcode::Insert);
    if (!categorySelector->Insert().IsValid())
        return false;

    auto displayStyleA = db.Elements().GetForEdit<DisplayStyle2d>(m_displayStyleId);
    EXPECT_TRUE(displayStyleA.IsValid());
    if (!displayStyleA.IsValid())
        return false;

    DrawingViewDefinition view(db.GetDictionaryModel(), "MyView", Model2dId(), *categorySelector, *displayStyleA);
    Acquire(view, BeSQLite::DbOpcode::Insert);
    return view.Insert().IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IndirectLocksTest, UseThenDelete)
    {
    SetupDbs();

    EXPECT_TRUE(CreateView(*m_dbA));
    EXPECT_FALSE(DeleteDisplayStyle(*m_dbB));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IndirectLocksTest, DeleteThenUse)
    {
    SetupDbs();

    EXPECT_TRUE(DeleteDisplayStyle(*m_dbB));
    EXPECT_FALSE(CreateView(*m_dbA));
    }
