/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
//! DgnChangeSummaryTestFixture
//=======================================================================================
struct DgnChangeSummaryTestFixture : public ChangeTestFixture
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
    DgnAuthority::Code CreateCode(int iFloor, int iQuadrant);
        
    DgnElementId QueryElementId(int iFloor, int iQuadrant);

    void CreateSampleBuilding(WCharCP fileName);
    void InsertEmptyBuilding(WCharCP filename);
    void InsertFloor(int iFloor);
    void UpdateFloorGeometry(int iFloor);
    void DeleteFloor(int iFloor);

    static TxnManager::TxnId QueryFirstTxnId(DgnDbR dgndb, uint32_t sessionId);
    static TxnManager::TxnId QueryLastTxnId(DgnDbR dgndb, uint32_t sessionId);

    void CompareSessions(DgnChangeSummaryTestFixture::ChangedElements& changedElements, uint32_t startSession, uint32_t endSession);

public:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnAuthority::Code DgnChangeSummaryTestFixture::CreateCode(int iFloor, int iQuadrant)
    {
    Utf8PrintfString codeStr("Floor %d,Quadrant %d", iFloor, iQuadrant);
    return m_testAuthority->CreateCode(codeStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::InsertEmptyBuilding(WCharCP filename)
    {
    CreateDgnDb(filename);
    InsertModel();
    InsertCategory();
    InsertAuthority();
    CreateDefaultView();
    UpdateDgnDbExtents();
    m_testDb->SaveChanges("Inserted empty building");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnChangeSummaryTestFixture::QueryElementId(int iFloor, int iQuadrant)
    {
    DgnAuthority::Code code = CreateCode(iFloor, iQuadrant);
    return m_testDb->Elements().QueryElementIdByCode(code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::DeleteFloor(int iFloor)
    {
    for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
        {
        DgnElementId elementId = QueryElementId(iFloor, iQuadrant);
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
            physicalElementPtr->SetCode(CreateCode(iFloor, iQuadrant));
            
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
        DgnElementId elementId = QueryElementId(iFloor, iQuadrant);
        ASSERT_TRUE(elementId.IsValid());

        PhysicalElementPtr physicalElement = m_testDb->Elements().GetForEdit<PhysicalElement>(elementId);
        Placement3d newPlacement = physicalElement->GetPlacement();
        newPlacement.GetOriginR().x += (iFloor + 1) * 1.0;

        physicalElement->SetPlacement(newPlacement);
        
        DgnDbStatus dbStatus;
        physicalElement->Update(&dbStatus);
        ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
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

    /*
    printf("-----------------------------------------\n");
    printf("Change summary between sessions (%d, %d]:\n", startSessionId, endSessionId);
    printf("-----------------------------------------\n");
    dgnChangeSummary.Dump();
    printf("\n\n\n");
    */

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
    EXPECT_EQ(changedElements.m_inserts.size(), 0+2); // category and sub-category...
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 0);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 1, 2); // [1, 2]
    EXPECT_EQ(changedElements.m_inserts.size(), 4+2); // category and sub-category...
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 4);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 1, 6); // [1, 6]
    EXPECT_EQ(changedElements.m_inserts.size(), 20+2); // category and sub-category...
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 20);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 7, 7); // [7, 7]
    EXPECT_EQ(changedElements.m_inserts.size(), 0);
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 4);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 4); // TODO: Updates due to LastMod change. Needs a fix. 

    CompareSessions(changedElements, 8, 8); // [8, 8]
    EXPECT_EQ(changedElements.m_inserts.size(), 0);
    EXPECT_EQ(changedElements.m_deletes.size(), 4);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 0);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);
    }
