/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_SQLITE

DgnPlatformSeedManager::SeedDbInfo DgnDbTestFixture::s_seedFileInfo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    DgnDbTestFixture::s_seedFileInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(true, false));
    }
//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void DgnDbTestFixture::TearDownTestCase()
    {
    //DgnPlatformSeedManager::EmptySubDirectory(DgnDbTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Umar.Hayat                   07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDbTestFixture::GetSeedDbCopy(BeFileNameR actualName, WCharCP newName)
    {
    BeFileName outPath;
    DgnDbStatus status = DgnPlatformSeedManager::MakeSeedDbCopy(outPath, DgnDbTestFixture::s_seedFileInfo.fileName, newName);
    BeTest::GetHost().GetOutputRoot(actualName);
    actualName.AppendToPath(outPath.c_str());
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that creates a copy of 3dMetricGeneral.ibim at Output
* Project file name is the name of the test, mode is ReadWrite and it is Briefcase
* @bsimethod                                     Majd.Uddin                   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupSeedProject(BeSQLite::Db::OpenMode mode, bool needBriefcase)
    {
    WString fileName (TEST_NAME, BentleyCharEncoding::Utf8);
    fileName.append(L".bim");
    SetupSeedProject(fileName.c_str(), mode, needBriefcase);
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
        m_db = DgnPlatformSeedManager::OpenSeedDbCopy(DgnDbTestFixture::s_seedFileInfo.fileName, inFileName);
    else
        m_db = DgnPlatformSeedManager::OpenSeedDb(s_seedFileInfo.fileName);

    if (needBriefcase)
        {
        TestDataManager::MustBeBriefcase(m_db, mode);
        ASSERT_TRUE(m_db->IsBriefcase());
        ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());
        }

    DgnCode physicalPartitionCode = PhysicalPartition::CreateCode(*m_db->Elements().GetRootSubject(), s_seedFileInfo.physicalPartitionName);
    m_defaultModelId = m_db->Models().QuerySubModelId(physicalPartitionCode);
    ASSERT_TRUE(m_defaultModelId.IsValid());

    m_defaultCategoryId = SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), s_seedFileInfo.categoryName);
    ASSERT_TRUE(m_defaultCategoryId.IsValid());

    m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(DgnModelId modelId, DgnCategoryId categoryId, DgnDbStatus* result, DgnCode elementCode)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, modelId, categoryId, elementCode);
    return m_db->Elements().Insert(*el, result);
    }

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Render::GeometryParamsCR ep, DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, ep, modelId, categoryId, elementCode,100);
    return m_db->Elements().Insert(*el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Utf8CP elementCode, DgnModelId modelId, DgnCategoryId categoryId)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, modelId, categoryId, elementCode);
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
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnDbTestFixture::InsertElement2d(DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement2d::Create(*m_db, modelId, categoryId, elementCode, 100);
    return m_db->Elements().Insert(*el)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnDbTestFixture::InsertElementUsingGeometryPart2d(DgnCodeCR gpCode, DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElement2dPtr el = TestElement2d::Create(*m_db, modelId, categoryId, elementCode, 100);

    DgnModelPtr model = m_db->Models().GetModel(modelId);
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint2d::From(0.0, 0.0));

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(*m_db, gpCode);
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
DgnElementId DgnDbTestFixture::InsertElementUsingGeometryPart(DgnGeometryPartId gpId, DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode)
    {
    if (!modelId.IsValid())
        modelId = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, modelId, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(modelId).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    if (!(builder->Append(gpId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementId();

    if (SUCCESS != builder->Finish(*geomElem))
        return DgnElementId();

    return m_db->Elements().Insert(*el)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr DgnDbTestFixture::GetDefaultPhysicalModel()
    {
    PhysicalModelPtr model = GetDgnDb().Models().Get<PhysicalModel>(m_defaultModelId);
    BeAssert(model.IsValid());
    return model;
    }

//////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PerfTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    //  Request a root seed file.
    DgnDbTestFixture::s_seedFileInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, false));
    }
//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Umar.Hayat             07/2016
//---------------------------------------------------------------------------------------
void PerfTestFixture::TearDownTestCase()
    {
    //DgnPlatformSeedManager::EmptySubDirectory(DgnDbTestFixture::s_seedFileInfo.fileName.GetDirectoryName());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Umar.Hayat                   07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerfTestFixture::GetSeedDbCopy(BeFileNameR actualName, WCharCP newName)
    {
    BeFileName outPath;
    DgnDbStatus status = DgnPlatformSeedManager::MakeSeedDbCopy(outPath, PerfTestFixture::s_seedFileInfo.fileName, newName);
    BeTest::GetHost().GetOutputRoot(actualName);
    actualName.AppendToPath(outPath.c_str());
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that creates a copy of 3dMetricGeneral.ibim at Output
* Project file name is the name of the test, mode is ReadWrite and it is Briefcase
* @bsimethod                                     Majd.Uddin                   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PerfTestFixture::SetupSeedProject(BeSQLite::Db::OpenMode mode, bool needBriefcase)
    {
    WString fileName(TEST_NAME, BentleyCharEncoding::Utf8);
    fileName.append(L".bim");
    SetupSeedProject(fileName.c_str(), mode, needBriefcase);
    }

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file after copying it to out
* baseProjFile is the existing file and testProjFile is what we get
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerfTestFixture::SetupSeedProject(WCharCP inFileName, BeSQLite::Db::OpenMode mode, bool needBriefcase)
    {
    // Note: We know that our group's SetUpTestCase() function has already created the group seed file. We can just ask for it.
    if (Db::OpenMode::ReadWrite == mode)
        m_db = DgnPlatformSeedManager::OpenSeedDbCopy(DgnDbTestFixture::s_seedFileInfo.fileName, inFileName);
    else
        m_db = DgnPlatformSeedManager::OpenSeedDb(s_seedFileInfo.fileName);

    if (needBriefcase)
        {
        TestDataManager::MustBeBriefcase(m_db, mode);
        ASSERT_TRUE(m_db->IsBriefcase());
        ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());
        }

    //DgnCode physicalPartitionCode = PhysicalPartition::CreateCode(*m_db->Elements().GetRootSubject(), s_seedFileInfo.physicalPartitionName);
    //m_defaultModelId = m_db->Models().QuerySubModelId(physicalPartitionCode);
    //ASSERT_TRUE(m_defaultModelId.IsValid());

    //m_defaultCategoryId = SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), s_seedFileInfo.categoryName);
    //ASSERT_TRUE(m_defaultCategoryId.IsValid());

    m_db->SaveChanges();
    }

DgnPlatformSeedManager::SeedDbInfo PerfTestFixture::s_seedFileInfo;
