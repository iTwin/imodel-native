#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>

#include "iModelHubMock.h"
#include "../CompatibilityTestFixture.h"

struct IModelHubMockTestFixture : public ::testing::Test, DgnPlatformLib::Host::RepositoryAdmin 
{
private:
    static bool                     s_isInitialized;
    static ScopedDgnHost*           s_host;
    mutable TestRepositoryManager   m_server;
    BeFileName                      m_tempPath;
    BeFileName                      m_outputRoot;
    IModelHubMock*                  m_mock;

    void Initialize() 
        {
        BeFileName assets;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assets);
        DgnDb::Initialize(m_tempPath, &assets);
        s_isInitialized = true;
        }
public:
    IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override {return &m_server;}
    IModelHubMock* GetMock() {return m_mock;}
    BeFileNameCR GetTempPath() {return m_tempPath;}
    BeFileNameCR GetOutputPath() {return m_outputRoot;}

    void SetUp() override 
        {
        if (!s_isInitialized)
            Initialize();
        s_host = new ScopedDgnHost();
        s_host->SetRepositoryAdmin(this);
        BeTest::GetHost().GetOutputRoot(m_outputRoot);
        BeTest::GetHost().GetTempDir(m_tempPath);
        m_mock = new IModelHubMock(m_outputRoot);
        }

    void TearDown() override 
        {
        delete m_mock;
        delete s_host;
        }
};
ScopedDgnHost* IModelHubMockTestFixture::s_host;
bool IModelHubMockTestFixture::s_isInitialized = false;

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, CreateiModel)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, CreateMultipleiModels)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto id2 = GetMock()->CreateiModel("test2");
    ASSERT_TRUE(id2.IsValid());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, AcquireBriefcase)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    auto status = GetMock()->AcquireBriefcase(id, briefcasePath);
    ASSERT_TRUE(BeFileName::DoesPathExist(briefcasePath));
    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());
    ASSERT_TRUE(db->IsBriefcase());
    ASSERT_FALSE(db->IsMasterCopy());
    ASSERT_FALSE(db->IsStandaloneBriefcase());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, AcquireMultipleBriefcase)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));
    ASSERT_TRUE(BeFileName::DoesPathExist(briefcasePath));
    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());
    ASSERT_TRUE(db->IsBriefcase());
    ASSERT_FALSE(db->IsMasterCopy());
    ASSERT_FALSE(db->IsStandaloneBriefcase());

    // Acquire another briefcase
    briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase2.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));
    ASSERT_TRUE(BeFileName::DoesPathExist(briefcasePath));
    auto db2 = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(db2.IsValid());
    ASSERT_TRUE(db2->IsBriefcase());
    ASSERT_FALSE(db2->IsMasterCopy());
    ASSERT_FALSE(db2->IsStandaloneBriefcase());

    ASSERT_NE(db->GetBriefcaseId(), db2->GetBriefcaseId());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, CreateAndPushChangeset)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    auto status = GetMock()->AcquireBriefcase(id, briefcasePath);

    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
    auto res = db->BriefcaseManager().LockDb(LockLevel::Exclusive);

    SubjectCPtr rootSubject = db->Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, "myPartition");
    auto code = PhysicalPartition::CreateCode(*rootSubject, "myPartition");
    DgnDbStatus insStatus;
    auto resStatus = db->BriefcaseManager().ReserveCode(code);
    partition->Insert(&insStatus);

    ASSERT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    auto modelId = model->GetModelId();

    DgnElement::CreateParams cp = DgnElement::CreateParams::CreateParams(*db, modelId, DgnClassId(db->Schemas().GetClass("BisCore", "AnnotationElement2d")->GetId()));
    auto element = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(cp));
    element->Insert();

    db->SaveChanges("test");
    auto rev = db->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev, id).empty());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, MergeChangesets)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));

    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    auto res = db->BriefcaseManager().LockDb(LockLevel::Exclusive);

    SubjectCPtr rootSubject = db->Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, "myPartition");
    auto code = PhysicalPartition::CreateCode(*rootSubject, "myPartition");
    DgnDbStatus insStatus;
    auto resStatus = db->BriefcaseManager().ReserveCode(code);
    partition->Insert(&insStatus);

    ASSERT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    auto modelId = model->GetModelId();

    DgnElement::CreateParams cp =  DgnElement::CreateParams::CreateParams(*db, modelId, DgnClassId(db->Schemas().GetClass("BisCore", "AnnotationElement2d")->GetId()));
    auto element = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(cp));
    element->Insert();

    db->SaveChanges("test");
    auto rev = db->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev, id).empty());

    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging a changeset with correct iModelId and parent should succeed";
    GetMock()->ClearStoredRevisions(id);
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging 0 changesets should succeed always";

    partition = PhysicalPartition::Create(*rootSubject, "myPartition2");
    resStatus = db->BriefcaseManager().ReserveCode(PhysicalPartition::CreateCode(*rootSubject, "myPartition2"));
    partition->Insert(&insStatus);

    ASSERT_TRUE(partition.IsValid());
    model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    modelId = model->GetModelId();

    DgnElement::CreateParams cp2 =  DgnElement::CreateParams::CreateParams(*db, modelId, DgnClassId(db->Schemas().GetClass("BisCore", "AnnotationElement2d")->GetId()));
    auto element2 = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(cp2));
    element2->Insert();
    db->SaveChanges("test2");
    RevisionStatus revStatus;
    rev = db->Revisions().StartCreateRevision(&revStatus);
    ASSERT_TRUE(rev.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev, id).empty());

    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging a changeset with correct iModelId and parent should succeed";
    GetMock()->ClearStoredRevisions(id);
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging 0 changesets should succeed always";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, Locking)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));

    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(RepositoryStatus::Success,  db->BriefcaseManager().LockDb(LockLevel::Exclusive).Result());

    auto briefcasePath2 = BeFileName(GetTempPath()).AppendToPath(L"briefcase2.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath2));

    auto db2 = DgnDb::OpenDgnDb(&stat, briefcasePath2, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(RepositoryStatus::LockAlreadyHeld, db2->BriefcaseManager().LockDb(LockLevel::Exclusive).Result());

    db->BriefcaseManager().RelinquishLocks();
    ASSERT_EQ(RepositoryStatus::Success, db2->BriefcaseManager().LockDb(LockLevel::Exclusive).Result());

    db2->BriefcaseManager().RelinquishLocks();
    ASSERT_EQ(RepositoryStatus::Success, db->BriefcaseManager().LockDb(LockLevel::Shared).Result());
    ASSERT_EQ(RepositoryStatus::Success, db2->BriefcaseManager().LockDb(LockLevel::Shared).Result());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, MergeChangesetsFromMultipleClients)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"briefcase.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));
    DbResult stat;
    DgnDbStatus insStatus;
    {
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(RepositoryStatus::Success, db->BriefcaseManager().LockDb(LockLevel::Exclusive).Result());

    // First briefcase changes
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db->Elements().GetRootSubject(), "myPartition");
    ASSERT_EQ(RepositoryStatus::Success, db->BriefcaseManager().ReserveCode(PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), "myPartition")));
    partition->Insert(&insStatus);

    ASSERT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());

    auto cp = DgnElement::CreateParams(*db, model->GetModelId(), DgnClassId(db->Schemas().GetClass("BisCore", "AnnotationElement2d")->GetId()));
    auto element = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(cp));
    element->Insert();
    db->SaveChanges("test");
    auto rev = db->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev, id).empty());
    db->BriefcaseManager().RelinquishLocks();
    db->CloseDb();
    }
    // Merging changesets
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging a changeset with correct iModelId and parent should succeed";
    GetMock()->ClearStoredRevisions(id);

    // Second briefcase changes
    {
    auto briefcasePath2 = BeFileName(GetTempPath()).AppendToPath(L"briefcase2.bim");
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath2));
    auto db2 = DgnDb::OpenDgnDb(&stat, briefcasePath2, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(RepositoryStatus::Success, db2->BriefcaseManager().LockDb(LockLevel::Exclusive).Result());
    auto partition = PhysicalPartition::Create(*db2->Elements().GetRootSubject(), "myPartition2");
    ASSERT_EQ(RepositoryStatus::Success, db2->BriefcaseManager().ReserveCode(PhysicalPartition::CreateCode(*db2->Elements().GetRootSubject(), "myPartition2")));
    partition->Insert(&insStatus);

    ASSERT_TRUE(partition.IsValid());
    auto model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());

    auto cp2 = DgnElement::CreateParams(*db2, model->GetModelId(), DgnClassId(db2->Schemas().GetClass("BisCore", "AnnotationElement2d")->GetId()));
    auto element = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(cp2));
    element->Insert();

    db2->SaveChanges("test2");
    auto rev = db2->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db2->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev, id).empty());
    db2->BriefcaseManager().RelinquishLocks();

    // Merging changesets
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging a changeset with correct iModelId and parent should succeed";
    GetMock()->ClearStoredRevisions(id);
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id)) << "Merging 0 changesets should succeed always";
    db2->CloseDb();
    }
    }

