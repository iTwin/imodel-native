/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnHandlers/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
//! DgnChangeSummaryTestFixture
//=======================================================================================
struct DgnChangeSummaryTestFixture : public GenericDgnModelTestFixture
{
    struct ChangedElements
    {
    DgnElementIdSet m_inserts;
    DgnElementIdSet m_deletes;
    DgnElementIdSet m_geometryUpdates;
    DgnElementIdSet m_businessUpdates;

    void Clear()
        {
        m_inserts.clear();
        m_deletes.clear();
        m_geometryUpdates.clear();
        m_businessUpdates.clear();
        }
    };

protected:
    DgnDbPtr m_testDb;
    DgnModelPtr m_testModel;
    DgnCategoryId m_testCategoryId;
    DgnElementId m_testElementId;

    Utf8String GetLabel(int iFloor, int iQuadrant) const;

    void CreateDgnDb(WCharCP filename);
    void OpenDgnDb(WCharCP filename);
    void CloseDgnDb();
    void CopyDgnDb(WCharCP original, WCharCP copy);

    void InsertModel();
    void InsertCategory();

    void CreateSampleBuilding(WCharCP fileName);
    void InsertEmptyBuilding(WCharCP filename);
    void InsertFloor(int iFloor);
    void UpdateFloorGeometry(int iFloor);
    void DeleteFloor(int iFloor);
    DgnElementId QueryElementIdByLabel(Utf8CP label);

    void CreateDefaultView();
    void UpdateDgnDbExtents();

    static TxnManager::TxnId QueryFirstTxnId(DgnDbR dgndb, uint32_t sessionId);
    static TxnManager::TxnId QueryLastTxnId(DgnDbR dgndb, uint32_t sessionId);

    void CompareSessions(DgnChangeSummaryTestFixture::ChangedElements& changedElements, uint32_t startSession, uint32_t endSession);

public:
    DgnChangeSummaryTestFixture() : GenericDgnModelTestFixture(__FILE__, true) {}
    virtual ~DgnChangeSummaryTestFixture() {}
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

//=======================================================================================
//! SampleChangeSet
//=======================================================================================
struct SqlChangeSet : BeSQLite::ChangeSet
    {
    virtual ConflictResolution _OnConflict(ConflictCause cause, Changes::Change iter)
        {
        BeAssert(false);
        fprintf(stderr, "Conflict \"%s\"\n", ChangeSet::InterpretConflictCause(cause).c_str());
        return ConflictResolution::Skip;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CreateDgnDb(WCharCP filename)
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);

    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile(DgnDbTestDgnManager::GetOutputFilePath(filename));

    DbResult createStatus;
    m_testDb = DgnDb::CreateDgnDb(&createStatus, DgnDbTestDgnManager::GetOutputFilePath(filename), createProjectParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not create test project";

    m_testDb->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CopyDgnDb(WCharCP original, WCharCP copy)
    {
    BeFileNameStatus status = BeFileName::BeCopyFile(DgnDbTestDgnManager::GetOutputFilePath(original), DgnDbTestDgnManager::GetOutputFilePath(copy));
    ASSERT_TRUE(status == BeFileNameStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::OpenDgnDb(WCharCP filename)
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_testDb = DgnDb::OpenDgnDb(&openStatus, DgnDbTestDgnManager::GetOutputFilePath(filename), openParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

    DgnModelId modelId = m_testDb->Models().QueryFirstModelId();
    m_testModel = m_testDb->Models().GetModel(modelId).get();

    m_testDb->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CloseDgnDb()
    {
    m_testDb->CloseDb();
    m_testModel = nullptr;
    m_testDb = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::InsertModel()
    {
    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
    DgnClassId classId = m_testDb->Domains().GetClassId(handler);
    m_testModel = handler.Create(DgnModel::CreateParams(*m_testDb, classId, "ChangeSetModel"));

    DgnDbStatus status = m_testModel->Insert();
    ASSERT_TRUE(DgnDbStatus::Success == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::InsertCategory()
    {
    DgnCategories::Category category("ChangeSetTestCategory", DgnCategories::Scope::Physical);
    category.SetRank(DgnCategories::Rank::Application);

    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    DbResult result = m_testDb->Categories().Insert(category, appearance);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    m_testCategoryId = category.GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::UpdateDgnDbExtents()
    {
    AxisAlignedBox3d physicalExtents;
    physicalExtents = m_testDb->Units().ComputeProjectExtents();
    m_testDb->Units().SaveProjectExtents(physicalExtents);

    DgnViews::Iterator iter = m_testDb->Views().MakeIterator((int) DgnViewType::Physical);
    BeAssert(iter.begin() != iter.end());

    DgnViewId viewId = iter.begin().GetDgnViewId();

    ViewControllerPtr viewController = m_testDb->Views().LoadViewController(viewId, DgnViews::FillModels::No);
    viewController->LookAtVolume(physicalExtents);
    DbResult result = viewController->Save();
    BeAssert(result == BE_SQLITE_OK);

    m_testDb->SaveSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CreateDefaultView()
    {
    DgnDbR dgndb = *m_testDb;

    DgnViews::View viewRow;
    DgnClassId classId(dgndb.Schemas().GetECClassId("dgn", "CameraView"));
    viewRow.SetDgnViewType(classId, DgnViewType::Physical);
    viewRow.SetDgnViewSource(DgnViewSource::Generated);
    viewRow.SetName("Default");
    viewRow.SetBaseModelId(m_testModel->GetModelId());
    DbResult result = dgndb.Views().Insert(viewRow);
    BeAssert(BE_SQLITE_OK == result);
    BeAssert(viewRow.GetId().IsValid());

    PhysicalViewController viewController(dgndb, viewRow.GetId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);
    DgnCategories::Iterator catIter = dgndb.Categories().MakeIterator();
    for (auto& entry : catIter)
        {
        DgnCategoryId categoryId = entry.GetCategoryId();
        viewController.ChangeCategoryDisplay(categoryId, true);
        }
    DgnModels::Iterator modIter = dgndb.Models().MakeIterator();
    for (auto& entry : modIter)
        {
        DgnModelId modelId = entry.GetModelId();
        viewController.ChangeModelDisplay(modelId, true);
        }
    result = viewController.Save();
    BeAssert(BE_SQLITE_OK == result);

    DgnViewId viewId = viewRow.GetId();
    dgndb.SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    dgndb.SaveSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::InsertEmptyBuilding(WCharCP filename)
    {
    CreateDgnDb(filename);
    InsertModel();
    InsertCategory();
    CreateDefaultView();
    UpdateDgnDbExtents();
    m_testDb->SaveChanges("Inserted empty building");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnChangeSummaryTestFixture::QueryElementIdByLabel(Utf8CP label)
    {
    CachedStatementPtr stmt = m_testDb->GetCachedStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Label=? LIMIT 1"); // find first if label not unique
    stmt->BindText(1, label, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnElementId() : stmt->GetValueId<DgnElementId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
Utf8String DgnChangeSummaryTestFixture::GetLabel(int iFloor, int iQuadrant) const
    {
    return Utf8PrintfString("Floor %d,Quadrant %d", iFloor, iQuadrant);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::DeleteFloor(int iFloor)
    {
    for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
        {
        Utf8String label = GetLabel(iFloor, iQuadrant);
        DgnElementId elementId = QueryElementIdByLabel(label.c_str());
        BeAssert(elementId.IsValid());
        m_testDb->Elements().Delete(elementId);
        }

    Utf8PrintfString desc("Deleted floor %d", iFloor);
    m_testDb->SaveChanges(desc.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::InsertFloor(int iFloor)
    {
    double blockSizeX = 2.0, blockSizeY = 2.0, blockSizeZ = 2.0;
    DPoint3d blockSizeRange = DPoint3d::From(blockSizeX, blockSizeY, blockSizeZ);

    double centerZ = iFloor * blockSizeZ - (blockSizeZ / 2.0);

    int numSquareBlocksPerQuadrant = 1;
    double limX = numSquareBlocksPerQuadrant * blockSizeX - blockSizeX * 0.5;
    double limY = numSquareBlocksPerQuadrant * blockSizeY - blockSizeY * 0.5;
    for (double centerX = -limX; centerX <= limX; centerX += blockSizeX)
        {
        for (double centerY = -limY; centerY <= limY; centerY += blockSizeY)
            {
            int iQuadrant = (centerX > 0) ? ((centerY > 0) ? 1 : 2) : ((centerY > 0) ? 4 : 3);
            DPoint3d center = DPoint3d::From(centerX, centerY, centerZ);

            PhysicalModelR physicalModel = *(dynamic_cast<PhysicalModelP> (m_testModel.get()));
            PhysicalElementPtr physicalElementPtr = PhysicalElement::Create(physicalModel, m_testCategoryId);

            Utf8String label = GetLabel(iFloor, iQuadrant);
            physicalElementPtr->SetLabel(label.c_str());
            
            DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), blockSizeRange, true);
            ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
            BeAssert(geomPtr.IsValid());

            ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*m_testModel, m_testCategoryId, center, YawPitchRollAngles());
            builder->Append(*geomPtr);
            BentleyStatus status = builder->SetGeomStreamAndPlacement(*physicalElementPtr);
            BeAssert(status == SUCCESS);

            m_testDb->Elements().Insert(*physicalElementPtr);
            }
        }

    // UpdateDgnDbExtents();

    Utf8PrintfString desc("Inserted floor %d", iFloor);
    m_testDb->SaveChanges(desc.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::UpdateFloorGeometry(int iFloor)
    {
    for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
        {
        Utf8String label = GetLabel(iFloor, iQuadrant);
        DgnElementId elementId = QueryElementIdByLabel(label.c_str());

        PhysicalElementPtr physicalElement = m_testDb->Elements().GetForEdit<PhysicalElement>(elementId);
        Placement3d newPlacement = physicalElement->GetPlacement();
        newPlacement.GetOriginR().x += (iFloor + 1) * 1.0;

        physicalElement->SetPlacement(newPlacement);
        
        DgnDbStatus dbStatus;
        physicalElement->Update(&dbStatus);
        BeAssert(dbStatus == DgnDbStatus::Success);
        }

    // UpdateDgnDbExtents();

    Utf8PrintfString desc("Updated geometry of floor %d", iFloor);
    m_testDb->SaveChanges(desc.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
// static
TxnManager::TxnId DgnChangeSummaryTestFixture::QueryFirstTxnId(DgnDbR dgndb, uint32_t sessionId)
    {
    if (sessionId == 0)
        {
        BeAssert(false);
        return TxnManager::TxnId(0);
        }
    TxnManager::TxnId startTxnId(TxnManager::SessionId ((uint32_t) sessionId), 0);

    CachedStatementPtr stmt = dgndb.GetCachedStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id>=? ORDER BY Id ASC LIMIT 1");
    stmt->BindInt64(1, startTxnId.GetValue());
    DbResult rc = stmt->Step();
    if (rc != BE_SQLITE_ROW)
        return TxnManager::TxnId();

    TxnManager::TxnId firstTxnId = TxnManager::TxnId(stmt->GetValueInt64(0));
    return firstTxnId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// static
TxnManager::TxnId DgnChangeSummaryTestFixture::QueryLastTxnId(DgnDbR dgndb, uint32_t sessionId)
    {
    if (sessionId == 0)
        {
        BeAssert(false);
        return TxnManager::TxnId(0);
        }
    TxnManager::TxnId nextTxnId(TxnManager::SessionId((uint32_t) (sessionId + 1)), 0); // start with first TxnId in next Session

    CachedStatementPtr stmt = dgndb.GetCachedStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id<? ORDER BY Id DESC LIMIT 1");
    stmt->BindInt64(1, nextTxnId.GetValue());
    DbResult rc = stmt->Step();
    if (rc != BE_SQLITE_ROW)
        return TxnManager::TxnId();

    TxnManager::TxnId firstTxnId = TxnManager::TxnId(stmt->GetValueInt64(0));
    return firstTxnId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CreateSampleBuilding(WCharCP fileName)
    {
    InsertEmptyBuilding(fileName);
    CloseDgnDb();

    for (int ii = 0; ii < 5; ii++)
        {
        OpenDgnDb(fileName);
        InsertFloor(ii);
        CloseDgnDb();
        }

    OpenDgnDb(fileName);
    UpdateFloorGeometry(1);
    CloseDgnDb();

    OpenDgnDb(fileName);
    DeleteFloor(3);
    CloseDgnDb();

    OpenDgnDb(fileName);
    InsertFloor(5);
    CloseDgnDb();

    OpenDgnDb(fileName);
    UpdateFloorItem(2);
    CloseDgnDb();
    }

//---------------------------------------------------------------------------------------
// Gets the changed elements between the two sessions [startSession, endSession]. 
// Gets changes from start of startSession to end of endSession. 
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::CompareSessions(DgnChangeSummaryTestFixture::ChangedElements& changedElements, uint32_t startSessionId, uint32_t endSessionId)
    {
    ASSERT_TRUE(startSessionId != 0);
    ASSERT_TRUE(endSessionId >= startSessionId);

    TxnManager::TxnId endTxnId = QueryLastTxnId(*m_testDb, endSessionId);
    ASSERT_TRUE(endTxnId.IsValid());
    TxnManager::TxnId reverseToTxnId = m_testDb->Txns().QueryNextTxnId(endTxnId);

    TxnManager::TxnId startTxnId = QueryFirstTxnId(*m_testDb, startSessionId);
    ASSERT_TRUE(startTxnId.IsValid());
    
    if (reverseToTxnId.IsValid())
        {
        DgnDbStatus dbStatus = m_testDb->Txns().ReverseTo(reverseToTxnId, TxnManager::AllowCrossSessions::Yes);
        ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
        }

    DgnChangeSummary dgnChangeSummary(*m_testDb);
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(dgnChangeSummary, startTxnId);
    ASSERT_TRUE(status == DgnDbStatus::Success);

    changedElements.Clear();
    dgnChangeSummary.GetChangedElements(changedElements.m_inserts, ChangeSummary::QueryDbOpcode::Insert);
    dgnChangeSummary.GetChangedElements(changedElements.m_deletes, ChangeSummary::QueryDbOpcode::Delete);
    dgnChangeSummary.GetChangedElements(changedElements.m_businessUpdates, ChangeSummary::QueryDbOpcode::Update);
    dgnChangeSummary.GetElementsWithItemUpdates(changedElements.m_businessUpdates);
    dgnChangeSummary.GetElementsWithGeometryUpdates(changedElements.m_geometryUpdates);

    printf("-----------------------------------------\n");
    printf("Change summary between sessions (%d, %d]:\n", startSessionId, endSessionId);
    printf("-----------------------------------------\n");
    dgnChangeSummary.Dump();
    printf("\n\n\n");

    status = m_testDb->Txns().ReinstateTxn();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnChangeSummaryTestFixture, CreateSampleDataSet)
    {
    WCharCP fileName = L"SampleBuilding.dgndb";
    CreateSampleBuilding(fileName);

    BeFileName pathname = DgnDbTestDgnManager::GetOutputFilePath(fileName);
    printf("Created sample data set: %s\n", pathname.GetNameUtf8().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnChangeSummaryTestFixture, ValidateChangeSummaries)
    {
    WCharCP fileName = L"SampleBuildingTest.dgndb";
    CreateSampleBuilding(fileName);
    OpenDgnDb(fileName);

    /*
    Dump of TxnTable - 
    (Session 1) 4294967296	False	False	Inserted empty building	2015-09-09 18:33:21.688	[BLOB_DATA]
    (Session 2) 8589934592	False	False	Inserted floor 0	2015-09-09 18:33:21.923	[BLOB_DATA]
    (Session 3) 12884901888	False	False	Inserted floor 1	2015-09-09 18:33:22.173	[BLOB_DATA]
    (Session 4) 17179869184	False	False	Inserted floor 2	2015-09-09 18:33:22.407	[BLOB_DATA]
    (Session 5) 21474836480	False	False	Inserted floor 3	2015-09-09 18:33:22.640	[BLOB_DATA]
    (Session 6) 25769803776	False	False	Inserted floor 4	2015-09-09 18:33:22.859	[BLOB_DATA]
    (Session 7) 30064771072	False	False	Updated geometry of floor 1	2015-09-09 18:33:23.078	[BLOB_DATA]
    (Session 8) 34359738368	False	False	Deleted floor 3	2015-09-09 18:33:23.250	[BLOB_DATA]
    (Session 9) 38654705664	False	False	Inserted floor 5	2015-09-09 18:33:23.545	[BLOB_DATA]
    */

    DgnChangeSummaryTestFixture::ChangedElements changedElements;
    
    CompareSessions(changedElements, 1, 1); // [1, 1]
    ASSERT_EQ(changedElements.m_inserts.size(), 0);
    ASSERT_EQ(changedElements.m_deletes.size(), 0);
    ASSERT_EQ(changedElements.m_geometryUpdates.size(), 0);
    ASSERT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 1, 2); // [1, 2]
    ASSERT_EQ(changedElements.m_inserts.size(), 4);
    ASSERT_EQ(changedElements.m_deletes.size(), 0);
    ASSERT_EQ(changedElements.m_geometryUpdates.size(), 4);
    ASSERT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 1, 6); // [1, 6]
    ASSERT_EQ(changedElements.m_inserts.size(), 20);
    ASSERT_EQ(changedElements.m_deletes.size(), 0);
    ASSERT_EQ(changedElements.m_geometryUpdates.size(), 20);
    ASSERT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 7, 7); // [7, 7]
    ASSERT_EQ(changedElements.m_inserts.size(), 0);
    ASSERT_EQ(changedElements.m_deletes.size(), 0);
    ASSERT_EQ(changedElements.m_geometryUpdates.size(), 4);
    ASSERT_EQ(changedElements.m_businessUpdates.size(), 4); // TODO: Updates due to LastMod change. Needs a fix. 

    CompareSessions(changedElements, 8, 8); // [8, 8]
    ASSERT_EQ(changedElements.m_inserts.size(), 0);
    ASSERT_EQ(changedElements.m_deletes.size(), 4);
    ASSERT_EQ(changedElements.m_geometryUpdates.size(), 0);
    ASSERT_EQ(changedElements.m_businessUpdates.size(), 0);
    }
