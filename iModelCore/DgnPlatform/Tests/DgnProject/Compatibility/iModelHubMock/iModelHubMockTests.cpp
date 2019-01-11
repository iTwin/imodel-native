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
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));
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
    ASSERT_TRUE(GetMock()->AcquireBriefcase(id, briefcasePath));

    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
    auto res = db->BriefcaseManager().LockDb(LockLevel::Exclusive);

    SubjectCPtr rootSubject = db->Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, "myPartition");
    auto code = PhysicalPartition::CreateCode(*rootSubject, "myPartition");
    DgnDbStatus insStatus;
    db->BriefcaseManager().ReserveCode(code);
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
TEST_F(IModelHubMockTestFixture, OverflowIssue)
    {
    auto id = GetMock()->CreateiModel("test");
    ASSERT_TRUE(id.IsValid());
    auto briefcasePath = BeFileName(GetTempPath()).AppendToPath(L"master.bim");
    auto status = GetMock()->AcquireBriefcase(id, briefcasePath);

    DbResult stat;
    auto db = DgnDb::OpenDgnDb(&stat, briefcasePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    auto res = db->BriefcaseManager().LockDb(LockLevel::Exclusive);
    Utf8CP domainName = "TestDomain";
    Utf8CP className = "TestEl";
    Utf8CP partitionName = "myPartition";
    Utf8CP categoryName = "myCategory";
    std::vector<DgnElementId> elements;
    Utf8CP v01_00_00 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestDomain" alias="td" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                    <ECEntityClass typeName="TestEl">
                        <BaseClass>bis:PhysicalElement</BaseClass>

                        <!-- bis_GeometricElement3d -->
                        <ECProperty propertyName="p01" typeName="int"/>
                        <ECProperty propertyName="p02" typeName="int"/>
                        <ECProperty propertyName="p03" typeName="int"/>
                        <ECProperty propertyName="p04" typeName="int"/>
                        <ECProperty propertyName="p05" typeName="int"/>
                        <ECProperty propertyName="p06" typeName="int"/>
                        <ECProperty propertyName="p07" typeName="int"/>
                        <ECProperty propertyName="p08" typeName="int"/>
                        <ECProperty propertyName="p09" typeName="int"/>
                        <ECProperty propertyName="p10" typeName="int"/>
                        <ECProperty propertyName="p11" typeName="int"/>
                        <ECProperty propertyName="p12" typeName="int"/>
                        <ECProperty propertyName="p13" typeName="int"/>
                        <ECProperty propertyName="p14" typeName="int"/>
                        <ECProperty propertyName="p15" typeName="int"/>
                        <ECProperty propertyName="p16" typeName="int"/>
                        <ECProperty propertyName="p17" typeName="int"/>
                        <ECProperty propertyName="p18" typeName="int"/>
                        <ECProperty propertyName="p19" typeName="int"/>
                        <ECProperty propertyName="p20" typeName="int"/>
                        <ECProperty propertyName="p21" typeName="int"/>
                        <ECProperty propertyName="p22" typeName="int"/>
                        <ECProperty propertyName="p23" typeName="int"/>
                        <ECProperty propertyName="p24" typeName="int"/>
                        <ECProperty propertyName="p25" typeName="int"/>
                        <ECProperty propertyName="p26" typeName="int"/>
                        <ECProperty propertyName="p27" typeName="int"/>
                        <ECProperty propertyName="p28" typeName="int"/>
                        <ECProperty propertyName="p29" typeName="int"/>
                        <ECProperty propertyName="p30" typeName="int"/>
                        <ECProperty propertyName="p31" typeName="int"/>
                        <ECProperty propertyName="p32" typeName="int"/>		
                    </ECEntityClass>
                </ECSchema>)xml";

    DgnDbStatus insStatus;
    DgnModelId modelId;
    if (true)
        {
        auto readContext = ECN::ECSchemaReadContext::CreateContext();
        const ECN::IECSchemaLocater& schemaLocator = db->Schemas();
        readContext->AddSchemaLocater(const_cast<ECN::IECSchemaLocater&>(schemaLocator));
        ECN::ECSchemaPtr ts_v01_00_00;
        ECN::ECSchema::ReadFromXmlString(ts_v01_00_00, v01_00_00, *readContext);
        bvector<ECN::ECSchemaCP> schemas = {ts_v01_00_00.get()};
        ASSERT_EQ(SchemaStatus::Success, db->ImportSchemas(schemas));


        SubjectCPtr rootSubject = db->Elements().GetRootSubject();
        PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, partitionName);
        auto code = PhysicalPartition::CreateCode(*rootSubject, partitionName);

        auto resStatus = db->BriefcaseManager().ReserveCode(code);
        partition->Insert(&insStatus);
        ASSERT_TRUE(partition.IsValid());
        PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
        ASSERT_TRUE(model.IsValid());
        EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
        modelId = model->GetModelId();

        DefinitionModelR dictionary = db->GetDictionaryModel();
        SpatialCategory category(dictionary, categoryName);
        uint32_t weight = 10;
        double trans = 0.5;
        uint32_t dp = 1;
        code  = SpatialCategory::CreateCode(dictionary, categoryName);
        resStatus = db->BriefcaseManager().ReserveCode(code);

        DgnSubCategory::Appearance appearence;
        appearence.SetInvisible(false);
        appearence.SetColor(ColorDef::DarkRed());
        appearence.SetWeight(weight);
        appearence.SetTransparency(trans);
        appearence.SetDisplayPriority(dp);
        category.Insert(appearence);
        }


    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(db->GetDictionaryModel(), categoryName);
    DgnClassId classId = DgnClassId(db->Schemas().GetClass(domainName, className)->GetId());
    if (true)
        {
        GeometricElement3d::CreateParams param = GeometricElement3d::CreateParams(*db, modelId, classId, categoryId);
        auto handler = dgn_ElementHandler::Element::FindHandler(*db, param.m_classId);
        for (int i = 0; i < 10; ++i)
            {           
            auto newEl = handler->Create(param);        
            ((PhysicalElementP) (newEl.get()))->SetCategoryId(categoryId);
            for (ECN::ECPropertyCP property : newEl->GetElementClass()->GetProperties(false))
                {
                newEl->SetPropertyValue(property->GetName().c_str(), rand());
                }

            elements.push_back(newEl->Insert(&insStatus)->GetElementId());
            }
        }

    db->SaveChanges("test1");
    auto rev1 = db->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev1.IsValid());
    ASSERT_EQ(RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_FALSE(GetMock()->PushChangeset(rev1, id).empty());
    db->BriefcaseManager().RelinquishLocks();
    db->CloseDb();
    db = nullptr;

    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id));
    //=========================================================================================================================================
    //Schema Client Make changes to client
    Utf8CP v01_00_01 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestDomain" alias="td" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                    <ECEntityClass typeName="TestEl">
                        <BaseClass>bis:PhysicalElement</BaseClass>

                        <!-- bis_GeometricElement3d -->
                        <ECProperty propertyName="p01" typeName="int"/>
                        <ECProperty propertyName="p02" typeName="int"/>
                        <ECProperty propertyName="p03" typeName="int"/>
                        <ECProperty propertyName="p04" typeName="int"/>
                        <ECProperty propertyName="p05" typeName="int"/>
                        <ECProperty propertyName="p06" typeName="int"/>
                        <ECProperty propertyName="p07" typeName="int"/>
                        <ECProperty propertyName="p08" typeName="int"/>
                        <ECProperty propertyName="p09" typeName="int"/>
                        <ECProperty propertyName="p10" typeName="int"/>
                        <ECProperty propertyName="p11" typeName="int"/>
                        <ECProperty propertyName="p12" typeName="int"/>
                        <ECProperty propertyName="p13" typeName="int"/>
                        <ECProperty propertyName="p14" typeName="int"/>
                        <ECProperty propertyName="p15" typeName="int"/>
                        <ECProperty propertyName="p16" typeName="int"/>
                        <ECProperty propertyName="p17" typeName="int"/>
                        <ECProperty propertyName="p18" typeName="int"/>
                        <ECProperty propertyName="p19" typeName="int"/>
                        <ECProperty propertyName="p20" typeName="int"/>
                        <ECProperty propertyName="p21" typeName="int"/>
                        <ECProperty propertyName="p22" typeName="int"/>
                        <ECProperty propertyName="p23" typeName="int"/>
                        <ECProperty propertyName="p24" typeName="int"/>
                        <ECProperty propertyName="p25" typeName="int"/>
                        <ECProperty propertyName="p26" typeName="int"/>
                        <ECProperty propertyName="p27" typeName="int"/>
                        <ECProperty propertyName="p28" typeName="int"/>
                        <ECProperty propertyName="p29" typeName="int"/>
                        <ECProperty propertyName="p30" typeName="int"/>
                        <ECProperty propertyName="p31" typeName="int"/>
                        <ECProperty propertyName="p32" typeName="int"/>		

                        <!-- bis_GeometricElement3d_overflow -->
                        <ECProperty propertyName="o33" typeName="int"/>		
                        <ECProperty propertyName="o34" typeName="int"/>		
                        <ECProperty propertyName="o35" typeName="int"/>		
                    </ECEntityClass>
                </ECSchema>)xml";


    auto briefcasePathA = BeFileName(GetTempPath()).AppendToPath(L"client_with_schema_changes.bim");
    status = GetMock()->AcquireBriefcase(id, briefcasePathA);
    auto dbA = DgnDb::OpenDgnDb(&stat, briefcasePathA, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    dbA->BriefcaseManager().LockDb(LockLevel::Exclusive);
    if (true)
        {
        auto readContext = ECN::ECSchemaReadContext::CreateContext();
        const ECN::IECSchemaLocater& schemaLocator = dbA->Schemas();
        readContext->AddSchemaLocater(const_cast<ECN::IECSchemaLocater&>(schemaLocator));
        ECN::ECSchemaPtr ts_v01_00_01;
        ECN::ECSchema::ReadFromXmlString(ts_v01_00_01, v01_00_01, *readContext);
        bvector<ECN::ECSchemaCP> schemas = {ts_v01_00_01.get()};
        ASSERT_EQ(SchemaStatus::Success, dbA->ImportSchemas(schemas));
        }

    if (true)
        {
        GeometricElement3d::CreateParams param = GeometricElement3d::CreateParams(*dbA, modelId, classId, categoryId);
        ElementHandlerP handler = dgn_ElementHandler::Element::FindHandler(*dbA, param.m_classId);
        for (int i = 0; i < 10; ++i)
            {
            auto newEl = handler->Create(param);
            ((PhysicalElementP) (newEl.get()))->SetCategoryId(categoryId);
            for (ECN::ECPropertyCP property : newEl->GetElementClass()->GetProperties(false))
                {
                newEl->SetPropertyValue(property->GetName().c_str(), rand());
                }

            elements.push_back(newEl->Insert()->GetElementId());

            }
        }

    dbA->SaveChanges("test2");
    auto rev2 = dbA->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev2.IsValid());
    ASSERT_EQ(RevisionStatus::Success, dbA->Revisions().FinishCreateRevision());
    dbA->BriefcaseManager().RelinquishLocks();
    ASSERT_FALSE(GetMock()->PushChangeset(rev2, id).empty());
    dbA->CloseDb();
    dbA = nullptr;

    //========================================================================================================================
    auto briefcasePathB = BeFileName(GetTempPath()).AppendToPath(L"client_with_data_changes.bim");
    status = GetMock()->AcquireBriefcase(id, briefcasePathB);
    auto dbB = DgnDb::OpenDgnDb(&stat, briefcasePathB, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    res = dbB->BriefcaseManager().LockDb(LockLevel::Exclusive);
    if (true)
        {
        GeometricElement3d::CreateParams param = GeometricElement3d::CreateParams(*dbB, modelId, classId, categoryId);
        ElementHandlerP handler = dgn_ElementHandler::Element::FindHandler(*dbB, param.m_classId);
        for (int i = 0; i < 10; ++i)
            {
            auto newEl = handler->Create(param);
            ((PhysicalElementP) (newEl.get()))->SetCategoryId(categoryId);
            for (ECN::ECPropertyCP property : newEl->GetElementClass()->GetProperties(false))
                {
                newEl->SetPropertyValue(property->GetName().c_str(), rand());
                }

            elements.push_back(newEl->Insert()->GetElementId());
            }
        }

    dbB->SaveChanges("test3");
    dbB->BriefcaseManager().RelinquishLocks();
    dbB->CloseDb();   
    dbB = nullptr;

    auto openParam = DgnDb::OpenParams(Db::OpenMode::ReadWrite);
    openParam.GetSchemaUpgradeOptionsR().SetUpgradeFromRevision(*rev2);
    dbB = DgnDb::OpenDgnDb(&stat, briefcasePathB, openParam);
    dbB->SaveChanges("test4");

    auto rev3 = dbB->Revisions().StartCreateRevision();
    ASSERT_TRUE(rev3.IsValid());
    ASSERT_EQ(RevisionStatus::Success, dbB->Revisions().FinishCreateRevision());
    
    ASSERT_FALSE(GetMock()->PushChangeset(rev3, id).empty());    
    dbB->CloseDb();
    dbB = nullptr;
    ASSERT_TRUE(GetMock()->ManualMergeAllChangesets(id));

    auto briefcasePathC = BeFileName(GetTempPath()).AppendToPath(L"client.bim");
    status = GetMock()->AcquireBriefcase(id, briefcasePathC);
    auto dbC = DgnDb::OpenDgnDb(&stat, briefcasePathC, DgnDb::OpenParams(Db::OpenMode::Readonly));
    for (DgnElementId el : elements)
        {
        ASSERT_TRUE(dbC->Elements().GetElement(el).IsValid());
        }

    dbC->CloseDb();
    dbC = nullptr;
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
