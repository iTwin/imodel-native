/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

//=======================================================================================
//! DgnChangeSummaryTestFixture
//=======================================================================================
struct DgnChangeSummaryTestFixture : public ChangeTestFixture
{
    DEFINE_T_SUPER(ChangeTestFixture)
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
    DgnCode CreateCode(int iFloor, int iQuadrant);
        
    DgnElementId QueryElementId(int iFloor, int iQuadrant);

    void CreateSampleBuilding();
    void InsertFloor(int iFloor);
    void UpdateFloorGeometry(int iFloor);
    void DeleteFloor(int iFloor);

    static TxnManager::TxnId QueryFirstTxnId(DgnDbR dgndb, uint32_t sessionId);
    static TxnManager::TxnId QueryLastTxnId(DgnDbR dgndb, uint32_t sessionId);

    void CompareSessions(DgnChangeSummaryTestFixture::ChangedElements& changedElements, uint32_t startSession, uint32_t endSession);

public:
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;
    static void SetUpTestCase();
    DgnChangeSummaryTestFixture() {}
};

DgnPlatformSeedManager::SeedDbInfo DgnChangeSummaryTestFixture::s_seedFileInfo;
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
void DgnChangeSummaryTestFixture::SetUpTestCase()
    {
    //Start from parent fixture's seed Db
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(ChangeTestFixture::s_seedFileInfo.id, DgnPlatformSeedManager::SeedDbOptions(true, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    DgnChangeSummaryTestFixture::s_seedFileInfo = rootSeedInfo;
    DgnChangeSummaryTestFixture::s_seedFileInfo.fileName.SetName(L"DgnChangeSummaryTestFixture/DgnChangeSummaryTestFixture.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, DgnChangeSummaryTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    ASSERT_EQ(SchemaStatus::Success, DgnPlatformTestDomain::GetDomain().ImportSchema(*db));
    TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    CreateDefaultView(*db);
    DgnDbTestUtils::UpdateProjectExtents(*db);
    db->SaveChanges();

    // Create a dummy revision to purge transaction table for the test
    DgnRevisionPtr rev = db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());
    db->Revisions().FinishCreateRevision();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnCode DgnChangeSummaryTestFixture::CreateCode(int iFloor, int iQuadrant)
    {
    Utf8PrintfString codeStr("Floor %d,Quadrant %d", iFloor, iQuadrant);
    return m_defaultCodeSpec->CreateCode(codeStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnChangeSummaryTestFixture::QueryElementId(int iFloor, int iQuadrant)
    {
    DgnCode code = CreateCode(iFloor, iQuadrant);
    return m_db->Elements().QueryElementIdByCode(code);
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
        m_db->Elements().Delete(elementId);
        }

    Utf8PrintfString desc("Deleted floor %d", iFloor);
    m_db->SaveChanges(desc.c_str());
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

            PhysicalModelR model = *m_defaultModel->ToPhysicalModelP();
            GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(model, m_defaultCategoryId);
            physicalElementPtr->SetCode(CreateCode(iFloor, iQuadrant));
            
            DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), blockSizeRange, true);
            ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
            BeAssert(geomPtr.IsValid());

            GeometryBuilderPtr builder = GeometryBuilder::Create(*m_defaultModel, m_defaultCategoryId, center, YawPitchRollAngles());
            builder->Append(*geomPtr);
            BentleyStatus status = builder->Finish(*physicalElementPtr);
            BeAssert(status == SUCCESS);

            m_db->Elements().Insert(*physicalElementPtr);
            }
        }

    // UpdateDgnDbExtents();

    Utf8PrintfString desc("Inserted floor %d", iFloor);
    m_db->SaveChanges(desc.c_str());
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

        PhysicalElementPtr physicalElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
        Placement3d newPlacement = physicalElement->GetPlacement();
        newPlacement.GetOriginR().x += (iFloor + 1) * 1.0;

        physicalElement->SetPlacement(newPlacement);
        
        DgnDbStatus dbStatus;
        physicalElement->Update(&dbStatus);
        ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
        }

    // UpdateDgnDbExtents();

    Utf8PrintfString desc("Updated geometry of floor %d", iFloor);
    m_db->SaveChanges(desc.c_str());
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
void DgnChangeSummaryTestFixture::CreateSampleBuilding()
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
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

    TxnManager::TxnId endTxnId = QueryLastTxnId(*m_db, endSessionId);
    ASSERT_TRUE(endTxnId.IsValid());
    TxnManager::TxnId reverseToTxnId = m_db->Txns().QueryNextTxnId(endTxnId);

    TxnManager::TxnId startTxnId = QueryFirstTxnId(*m_db, startSessionId);
    ASSERT_TRUE(startTxnId.IsValid());
    
    if (reverseToTxnId.IsValid())
        {
        DgnDbStatus dbStatus = m_db->Txns().ReverseTo(reverseToTxnId, TxnManager::AllowCrossSessions::Yes);
        ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
        }

    DgnChangeSummary dgnChangeSummary(*m_db);
    BentleyStatus status = m_db->Txns().GetChangeSummary(dgnChangeSummary, startTxnId);
    ASSERT_TRUE(status == SUCCESS);

    changedElements.Clear();
    dgnChangeSummary.GetChangedElements(changedElements.m_inserts, ChangeSummary::QueryDbOpcode::Insert);
    dgnChangeSummary.GetChangedElements(changedElements.m_deletes, ChangeSummary::QueryDbOpcode::Delete);
    dgnChangeSummary.GetChangedElements(changedElements.m_businessUpdates, ChangeSummary::QueryDbOpcode::Update);
    dgnChangeSummary.GetElementsWithGeometryUpdates(changedElements.m_geometryUpdates);

    /*
    printf("-----------------------------------------\n");
    printf("Change summary between sessions (%d, %d]:\n", startSessionId, endSessionId);
    printf("-----------------------------------------\n");
    dgnChangeSummary.Dump();
    printf("\n\n\n");
    */

    DgnDbStatus dbStatus = m_db->Txns().ReinstateTxn();
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnChangeSummaryTestFixture, CreateSampleDataSet)
    {
    SetupDgnDb(DgnChangeSummaryTestFixture::s_seedFileInfo.fileName, L"CreateSampleDataSet.bim");
    CreateSampleBuilding();

    printf("Created sample data set: CreateSampleDataSet.bim\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnChangeSummaryTestFixture, ValidateChangeSummaries)
    {
    SetupDgnDb(DgnChangeSummaryTestFixture::s_seedFileInfo.fileName, L"ValidateChangeSummaries.bim");
    BeFileName fullFileName = BeFileName(m_db->GetDbFileName(), true);
    CreateSampleBuilding();

    OpenDgnDb(fullFileName);

    /*
    Dump of TxnTable -
    (Session 1) 4294967296	4294967296	False	False	Inserted floor 0	False	2017-03-10 14:54:33.026	[BLOB_DATA]
    (Session 2) 8589934592	8589934592	False	False	Inserted floor 1	False	2017-03-10 14:54:33.666	[BLOB_DATA]
    (Session 3) 12884901888	12884901888	False	False	Inserted floor 2	False	2017-03-10 14:54:34.350	[BLOB_DATA]
    (Session 4) 17179869184	17179869184	False	False	Inserted floor 3	False	2017-03-10 14:54:34.930	[BLOB_DATA]
    (Session 5) 21474836480	21474836480	False	False	Inserted floor 4	False	2017-03-10 14:54:35.524	[BLOB_DATA]
    (Session 6) 25769803776	25769803776	False	False	Updated geometry of floor 1	False	2017-03-10 14:54:36.168	[BLOB_DATA]
    (Session 7) 30064771072	30064771072	False	False	Deleted floor 3	False	2017-03-10 14:54:36.756	[BLOB_DATA]
    (Session 8) 34359738368	34359738368	False	False	Inserted floor 5	False	2017-03-10 14:54:37.349	[BLOB_DATA]
    */

    DgnChangeSummaryTestFixture::ChangedElements changedElements;
    
    CompareSessions(changedElements, 1, 5);
    EXPECT_EQ(changedElements.m_inserts.size(), 16);
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    // NEEDSWORK: EXPECT_EQ(changedElements.m_geometryUpdates.size(), 20);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);

    CompareSessions(changedElements, 6, 6);
    EXPECT_EQ(changedElements.m_inserts.size(), 4);// TODO: Updates due to LastMod change. Needs a fix. 
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    // NEEDSWORK: EXPECT_EQ(changedElements.m_geometryUpdates.size(), 4);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0); 

    CompareSessions(changedElements, 7, 7);
    EXPECT_EQ(changedElements.m_inserts.size(), 4);
    EXPECT_EQ(changedElements.m_deletes.size(), 0);
    EXPECT_EQ(changedElements.m_geometryUpdates.size(), 0);
    EXPECT_EQ(changedElements.m_businessUpdates.size(), 0);
    }
