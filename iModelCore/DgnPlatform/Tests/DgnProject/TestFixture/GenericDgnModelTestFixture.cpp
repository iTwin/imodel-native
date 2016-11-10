/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GenericDgnModelTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "GenericDgnModelTestFixture.h"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

DgnPlatformSeedManager::SeedDbInfo GenericDgnModelTestFixture::s_seedFileInfo;
DgnPlatformSeedManager::SeedDbInfo GenericDgnModel2dTestFixture::s_seedFileInfo;
DgnModelId GenericDgnModel2dTestFixture::s_drawingModelId;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Umar.Hayat           06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericBaseFixture::TearDown()
    {
    if (!m_dgnDb.IsNull())
        m_dgnDb->SaveChanges();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Umar.Hayat           06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr GenericBaseFixture::GetDgnDb(WCharCP seedName)
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnPlatformSeedManager::OpenSeedDbCopy(seedName);

    return m_dgnDb;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Umar.Hayat           06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr GenericBaseFixture::GetDgnDb(WCharCP seedName, WCharCP newName)
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnPlatformSeedManager::OpenSeedDbCopy(seedName, newName);

    return m_dgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericDgnModelTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    GenericDgnModelTestFixture::s_seedFileInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void GenericDgnModel2dTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;

    //TODO: Need to add new enum value in SeedDbId , for 2dModel fixuture it should use flag OneSpatialModel,
    // For 2D there seed should have a geomtric 2D model by default

    //  Request a root seed file.
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));

    GenericDgnModel2dTestFixture::s_seedFileInfo = rootSeedInfo;
    GenericDgnModel2dTestFixture::s_seedFileInfo.fileName.SetName(L"Seed_GenericDgnModel2d/DgnDbTestUtils_OneSpatialMoelWithOneGeometricModel2d.bim");

    // Make a copy of the root seed which will be customized as a seed for tests in this group
    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, GenericDgnModel2dTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    // Create a 2d model
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, TEST_MODEL2D_NAME);
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    s_drawingModelId = drawingModel->GetModelId();
    
    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void GenericDgnModelTestFixture::TearDownTestCase()
    {
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void GenericDgnModel2dTestFixture::TearDownTestCase()
    {
    }

END_DGNDB_UNIT_TESTS_NAMESPACE
