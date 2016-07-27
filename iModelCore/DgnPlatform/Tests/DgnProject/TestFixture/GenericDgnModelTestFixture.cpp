/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GenericDgnModelTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "GenericDgnModelTestFixture.h"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

DgnDbTestUtils::SeedDbInfo GenericDgnModelTestFixture::s_seedFileInfo;
DgnDbTestUtils::SeedDbInfo GenericDgnModel2dTestFixture::s_seedFileInfo;

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
        m_dgnDb = DgnDbTestUtils::OpenSeedDbCopy(seedName);

    return m_dgnDb;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Umar.Hayat           06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr GenericBaseFixture::GetDgnDb(WCharCP seedName, WCharCP newName)
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnDbTestUtils::OpenSeedDbCopy(seedName, newName);

    return m_dgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericDgnModelTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    GenericDgnModelTestFixture::s_seedFileInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));
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
    DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));

    GenericDgnModel2dTestFixture::s_seedFileInfo = rootSeedInfo;
    GenericDgnModel2dTestFixture::s_seedFileInfo.fileName.SetName(L"Seed_GenericDgnModel2d/DgnDbTestUtils_OneSpatialMoelWithOneGeometricModel2d.bim");

    //// Make a copy of the root seed which will be customized as a seed for tests in this group
    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootSeedInfo.fileName, GenericDgnModel2dTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    // Create a 2d model
    DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_GeometricModel2d)), DgnModel::CreateModelCode(TEST_MODEL2D_NAME)));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    ASSERT_TRUE(model->GetModelId().IsValid());

    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void GenericDgnModelTestFixture::TearDownTestCase()
    {
    //DgnDbTestUtils::EmptySubDirectory(GenericDgnModelTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }
//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void GenericDgnModel2dTestFixture::TearDownTestCase()
    {
    //DgnDbTestUtils::EmptySubDirectory(GenericDgnModel2dTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }


END_DGNDB_UNIT_TESTS_NAMESPACE
