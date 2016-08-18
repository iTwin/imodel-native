/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_SQLITE

DgnDbTestUtils::SeedDbInfo DgnDbTestFixture::s_seedFileInfo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    DgnDbTestFixture::s_seedFileInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(true, false));
    }
//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void DgnDbTestFixture::TearDownTestCase()
    {
    //DgnDbTestUtils::EmptySubDirectory(DgnDbTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Umar.Hayat                   07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDbTestFixture::GetSeedDbCopy(BeFileNameR actualName, WCharCP newName)
    {
    BeFileName outPath;
    DgnDbStatus status = DgnDbTestUtils::MakeSeedDbCopy(outPath, DgnDbTestFixture::s_seedFileInfo.fileName, newName);
    BeTest::GetHost().GetOutputRoot(actualName);
    actualName.AppendToPath(outPath.c_str());
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that creates a copy of 3dMetricGeneral.ibim at Output
* Project file name is the name of the test, mode is ReadWrite and it is Briefcase
* @bsimethod                                     Majd.Uddin                   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupSeedProject()
    {
    WString fileName (TEST_NAME, BentleyCharEncoding::Utf8);
    fileName.append(L".bim");
    SetupSeedProject(fileName.c_str(), Db::OpenMode::ReadWrite);
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file after copying it to out
* baseProjFile is the existing file and testProjFile is what we get
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupSeedProject(WCharCP inFileName, BeSQLite::Db::OpenMode mode, bool needBriefcase)
    {
    // Note: We know that our group's SetUpTestCase() function has already created the group seed file. We can just ask for it.
    if (Db::OpenMode::ReadWrite == mode)
        m_db = DgnDbTestUtils::OpenSeedDbCopy(DgnDbTestFixture::s_seedFileInfo.fileName, inFileName);
    else
        m_db = DgnDbTestUtils::OpenSeedDb(s_seedFileInfo.fileName);

    if (needBriefcase)
        {
        ASSERT_TRUE(m_db->IsBriefcase());
        ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());
        }

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    m_defaultModelP = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(m_defaultModelP.IsValid());
    m_defaultModelP->FillModel();

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);

    m_db->SaveChanges();
    }
/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(DgnModelId mid, DgnCategoryId categoryId, DgnDbStatus* result, DgnCode elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el, result);
    }

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Render::GeometryParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, ep, mid, categoryId, elementCode,100);
    return m_db->Elements().Insert(*el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestFixture::CopyDb(WCharCP inputFileName, WCharCP outputFileName)
    {
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot (fullInputFileName);
    fullInputFileName.AppendToPath (inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName))
        return BeFileName();

    return fullOutputFileName;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::OpenDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode, bool needBriefcase)
    {
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE( db.IsValid() ) << WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result).c_str();
    ASSERT_EQ( BE_SQLITE_OK , result );
    if (needBriefcase)
        TestDataManager::MustBeBriefcase(db, mode);
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file after copying it to out
* baseProjFile is the existing file and testProjFile is what we get
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupWithPrePublishedFile(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode, bool needBriefcase, bool needTestDomain)
    {
    //** Force to copy the file in Sub-Directory of TestCase
    BeFileName testFileName(TEST_FIXTURE_NAME,BentleyCharEncoding::Utf8);
    testFileName.AppendToPath(testProjFile);

    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseProjFile, testFileName.c_str(), __FILE__));
    
    OpenDb(m_db, outFileName, mode, needBriefcase);

    if (needBriefcase)
        {
        ASSERT_TRUE(m_db->IsBriefcase());
        ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());
        }

    if (BeSQLite::Db::OpenMode::ReadWrite == mode && needTestDomain)
        {
        auto status = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db);
        ASSERT_TRUE(DgnDbStatus::Success == status);
        }

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    m_defaultModelP = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(m_defaultModelP.IsValid());
    m_defaultModelP->FillModel();

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnDbTestFixture::InsertElement2d(DgnModelId mid, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode, 100);

    return m_db->Elements().Insert(*el)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnDbTestFixture::InsertElementUsingGeometryPart2d(DgnCodeCR gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElement2dPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode, 100);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint2d::From(0.0, 0.0));

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(gpCode, *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementId();

    if (SUCCESS != builder->Finish(*geomElem))
        return DgnElementId();

    return m_db->Elements().Insert(*el)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnDbTestFixture::InsertElementUsingGeometryPart(DgnGeometryPartId gpId, DgnModelId mid, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    if (!(builder->Append(gpId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementId();

    if (SUCCESS != builder->Finish(*geomElem))
        return DgnElementId();

    return m_db->Elements().Insert(*el)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetUpCameraView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId)
    {
    CameraViewDefinition view(dgnDb, "TestView");
    view.SetModelSelector(*DgnDbTestUtils::InsertNewModelSelector(dgnDb, "TestView", model.GetModelId()));
    EXPECT_TRUE(view.Insert().IsValid());

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    CameraViewController viewController (view);
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(elementBox, nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::SmoothShade);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(model.GetModelId(), true);

    EXPECT_EQ(DgnDbStatus::Success, viewController.Save());
    }


