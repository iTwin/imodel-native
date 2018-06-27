#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/UnitTests/RepositoryManagerUtil.h>

#include "iModelHubMock.h"
#include "../CompatibilityTestFixture.h"


struct IModelHubMockTestFixture : public ::testing::Test, DgnPlatformLib::Host::RepositoryAdmin {
    static bool s_isInitialized;
    static ScopedDgnHost* s_host;
    mutable TestRepositoryManager m_server;
private:
    void Initialize() 
        {
        BeFileName fn;
        BeFileName temp;
        BeFileName assets;
        BeTest::GetHost().GetOutputRoot(fn);
        BeTest::GetHost().GetTempDir(temp);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assets);
        DgnDb::Initialize(temp, &assets);
        s_isInitialized = true;
        }
public:
    IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override {return &m_server;}
    void SetUp() override 
        {
        if (!s_isInitialized)
            Initialize();
        s_host = new ScopedDgnHost();
        s_host->SetRepositoryAdmin(this);
        }
    void TearDown() override 
        {
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
    BeFileName fn;
    BeFileName temp;
    IModelHubMock mock(fn);
    auto id = mock.CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, AcquireBriefcase)
    {
    BeFileName fn;
    BeTest::GetHost().GetOutputRoot(fn);
    BeFileName temp;
    BeTest::GetHost().GetTempDir(temp);
    IModelHubMock mock(fn);
    auto id = mock.CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(temp).AppendToPath(L"briefcase.bim");
    auto status = mock.AcquireBriefcase(id, briefcasePath);
    ASSERT_TRUE(BeFileName::DoesPathExist(briefcasePath));
    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(db->IsBriefcase());
    ASSERT_FALSE(db->IsMasterCopy());
    ASSERT_FALSE(db->IsStandaloneBriefcase());
    ASSERT_TRUE(db.IsValid());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, CreateAndPushChangeset)
    {
    BeFileName fn;
    BeTest::GetHost().GetOutputRoot(fn);
    BeFileName temp;
    BeTest::GetHost().GetTempDir(temp);
    IModelHubMock mock(fn);
    auto id = mock.CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(temp).AppendToPath(L"briefcase.bim");
    auto status = mock.AcquireBriefcase(id, briefcasePath);

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
    ASSERT_FALSE(mock.PushChangeset(rev, id).empty());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
TEST_F(IModelHubMockTestFixture, MergeChangesets)
    {
    BeFileName fn;
    BeTest::GetHost().GetOutputRoot(fn);
    BeFileName temp;
    BeTest::GetHost().GetTempDir(temp);
    IModelHubMock mock(fn);
    auto id = mock.CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(temp).AppendToPath(L"briefcase.bim");
    auto status = mock.AcquireBriefcase(id, briefcasePath);

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
    ASSERT_FALSE(mock.PushChangeset(rev, id).empty());

    ASSERT_TRUE(mock.ManualMergeAllChangesets(id)) << "Merging a changeset with correct iModelId and parent should succeed";
    mock.ClearStoredRevisions();
    ASSERT_TRUE(mock.ManualMergeAllChangesets(id)) << "Merging 0 changesets should succeed always";
    }

