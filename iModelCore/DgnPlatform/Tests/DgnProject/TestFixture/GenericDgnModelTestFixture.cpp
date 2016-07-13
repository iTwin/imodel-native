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
DgnDbPtr GenericDgnModelTestFixture::GetDgnDb()
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName);

    return m_dgnDb;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Umar.Hayat           06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr GenericDgnModel2dTestFixture::GetDgnDb()
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName);

    return m_dgnDb;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BETEST_TC_SETUP(GenericDgnModelTestFixture)
    {
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));

    GenericDgnModelTestFixture::s_seedFileInfo = rootSeedInfo;
    GenericDgnModelTestFixture::s_seedFileInfo.fileName.SetName(L"GenericDgnModelTestFixture/GenericDgnModelTestFixture.bim");

    //// Make a copy of the root seed which will be customized as a seed for tests in this group
    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootSeedInfo.fileName, GenericDgnModelTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    db->SaveChanges();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
BETEST_TC_SETUP(GenericDgnModel2dTestFixture)
    {
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));

    GenericDgnModel2dTestFixture::s_seedFileInfo = rootSeedInfo;
    GenericDgnModel2dTestFixture::s_seedFileInfo.fileName.SetName(L"GenericDgnModel2dTestFixture/GenericDgnModel2dTestFixture.bim");

    //// Make a copy of the root seed which will be customized as a seed for tests in this group
    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootSeedInfo.fileName, GenericDgnModel2dTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    // Create a 2d model
    DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricModel2d)), DgnModel::CreateModelCode(TEST_MODEL2D_NAME)));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    ASSERT_TRUE(model->GetModelId().IsValid());

    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(GenericDgnModelTestFixture)
    {
    DgnDbTestUtils::EmptySubDirectory(GenericDgnModelTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }
//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(GenericDgnModel2dTestFixture)
    {
    DgnDbTestUtils::EmptySubDirectory(GenericDgnModel2dTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }


END_DGNDB_UNIT_TESTS_NAMESPACE
