//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "ChangeTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTestFixture::ChangeTestFixture(WCharCP testFileName, bool wantTestDomain) : m_testFileName (testFileName), m_wantTestDomain(wantTestDomain)
    {
    if (wantTestDomain)
        DgnDomains::RegisterDomain(DPTest::DgnPlatformTestDomain::GetDomain());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateSeedDgnDb(BeFileNameR seedPathname)
    {
    seedPathname = DgnDbTestDgnManager::GetOutputFilePath(L"ChangeTestSeed.bim");
    if (seedPathname.DoesPathExist())
        return;

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetProjectName("ChangeTestFixture");
    createProjectParams.SetOverwriteExisting(true);

    DbResult createStatus;
    DgnDbPtr seedDgnDb = DgnDb::CreateDgnDb(&createStatus, seedPathname, createProjectParams);
    ASSERT_TRUE(seedDgnDb.IsValid()) << "Could not create seed project";

    seedDgnDb->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::_CreateDgnDb()
    {
    // Note: Since creating the DgnDb everytime consumes too much time, we instead
    // just copy one we have created the first time around. 

    //BeFileName pathname = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    //if (pathname.DoesPathExist())
    //    BeFileName::BeDeleteFile(pathname);

    //BeFileName seedPathname;
    //CreateSeedDgnDb(seedPathname);

    //BeFileNameStatus fileStatus = BeFileName::BeCopyFile(seedPathname.c_str(), pathname.c_str());
    //ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    //DbResult openStatus;
    //DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    //m_testDb = DgnDb::OpenDgnDb(&openStatus, DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str()), openParams);
    BeFileName newName(TEST_NAME, true);
    newName.AppendString(L".ibim");

    //SetupWithPrePublishedFile(L"3dMetricGeneral.ibim", newName.c_str(), BeSQLite::Db::OpenMode::ReadWrite, true, m_wantTestDomain);
    SetupSeedProject();
    m_testDb = m_db;
    m_testFileName = BeFileName(m_db->GetDbFileName(),true);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

    if (m_wantTestDomain)
        ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*m_testDb));

    TestDataManager::MustBeBriefcase(m_testDb, Db::OpenMode::ReadWrite);

    SubjectCPtr rootSubject = m_testDb->Elements().GetRootSubject();
    SubjectCPtr modelSubject = Subject::CreateAndInsert(*rootSubject, "TestSubject"); // create a placeholder Subject for the DgnModel to describe
    ASSERT_TRUE(modelSubject.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*modelSubject, DgnModel::CreateModelCode("TestModel"));
    ASSERT_TRUE(model.IsValid());
    m_testModelId = model->GetModelId();

    m_testModel = m_testDb->Models().Get<SpatialModel>(m_testModelId);
    ASSERT_TRUE(m_testModel.IsValid());

    m_testCategoryId =  InsertCategory("TestCategory");
    ASSERT_TRUE(m_testCategoryId.IsValid());

    m_testAuthorityId = InsertNamespaceAuthority("TestAuthority");
    ASSERT_TRUE(m_testAuthorityId.IsValid());
    
    m_testAuthority = m_testDb->Authorities().Get<NamespaceAuthority>(m_testAuthorityId);
    ASSERT_TRUE(m_testAuthority.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::OpenDgnDb()
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    //m_testDb = DgnDb::OpenDgnDb(&openStatus, DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str()), openParams);
    m_testDb = DgnDb::OpenDgnDb(&openStatus, m_testFileName, openParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

    if (!m_testModelId.IsValid())
        {
        m_testModelId = m_testDb->Models().QueryModelId(DgnModel::CreateModelCode("TestModel"));
        ASSERT_TRUE(m_testModelId.IsValid());
        }

    m_testModel = m_testDb->Models().Get<SpatialModel>(m_testModelId);
    ASSERT_TRUE(m_testModel.IsValid());

    if (!m_testAuthorityId.IsValid())
        {
        m_testAuthorityId = m_testDb->Authorities().QueryAuthorityId("TestAuthority");
        ASSERT_TRUE(m_testAuthorityId.IsValid());
        }

    m_testAuthority = m_testDb->Authorities().Get<NamespaceAuthority>(m_testAuthorityId);
    ASSERT_TRUE(m_testAuthority.IsValid());

    if (!m_testCategoryId.IsValid())
        {
        m_testCategoryId = DgnCategory::QueryCategoryId(DgnCategory::CreateCategoryCode("TestCategory"), *m_testDb);
        ASSERT_TRUE(m_testCategoryId.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CloseDgnDb()
    {
    m_testDb->CloseDb();
    m_testDb = nullptr;
    m_testModel = nullptr;
    m_testAuthority = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnCategoryId ChangeTestFixture::InsertCategory(Utf8CP categoryName)
    {
    DgnCategory category(DgnCategory::CreateParams(*m_testDb, categoryName, DgnCategory::Scope::Physical, DgnCategory::Rank::Application));

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
DgnAuthorityId ChangeTestFixture::InsertNamespaceAuthority(Utf8CP authorityName)
    {
    RefCountedPtr<NamespaceAuthority> testAuthority = NamespaceAuthority::CreateNamespaceAuthority(authorityName, *m_testDb);

    DgnDbStatus status = testAuthority->Insert();
    BeAssert(status == DgnDbStatus::Success);

    return testAuthority->GetAuthorityId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId ChangeTestFixture::InsertPhysicalElement(SpatialModelR model, DgnCategoryId categoryId, int x, int y, int z)
    {
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(model, categoryId);

    DPoint3d sizeOfBlock = DPoint3d::From(1, 1, 1);
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(0, 0, 0), sizeOfBlock, true);
    ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(testGeomPtr.IsValid());

    DPoint3d centerOfBlock = DPoint3d::From(x, y, z);
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, centerOfBlock, YawPitchRollAngles());
    builder->Append(*testGeomPtr);
    BentleyStatus status = builder->Finish(*testElement);
    BeAssert(status == SUCCESS);

    DgnElementId elementId = m_testDb->Elements().Insert(*testElement)->GetElementId();
    return elementId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateDefaultView(DgnModelId defaultModelId)
    {
    CameraViewDefinition viewRow(*m_testDb, "Default");
    viewRow.SetModelSelector(*DgnDbTestUtils::InsertNewModelSelector(*m_testDb, "Default", defaultModelId));
    ASSERT_TRUE(viewRow.Insert().IsValid());

    CameraViewControllerPtr viewController = viewRow.LoadViewController();
    viewController->SetStandardViewRotation(StandardView::Iso);
    viewController->GetViewFlagsR().SetRenderMode(Render::RenderMode::SmoothShade);

    for (auto const& catId : DgnCategory::QueryCategories(*m_testDb))
        viewController->ChangeCategoryDisplay(catId, true);

    DgnModels::Iterator modIter = m_testDb->Models().MakeIterator();
    for (auto& entry : modIter)
        {
        DgnModelId modelId = entry.GetModelId();
        viewController->ChangeModelDisplay(modelId, true);
        }

    ASSERT_TRUE(DgnDbStatus::Success == viewController->Save());

    DgnViewId viewId = viewRow.GetViewId();
    m_testDb->SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    m_testDb->SaveSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::UpdateDgnDbExtents()
    {
    AxisAlignedBox3d physicalExtents;
    physicalExtents = m_testDb->Units().ComputeProjectExtents();
    m_testDb->Units().SetProjectExtents(physicalExtents);

    SpatialViewDefinitionCPtr view = dynamic_cast<SpatialViewDefinitionCP>(ViewDefinition::QueryView("Default", *m_testDb).get());
    ASSERT_TRUE(view.IsValid());

    ViewControllerPtr viewController = view->LoadViewController(ViewDefinition::FillModels::No);
    viewController->LookAtVolume(physicalExtents);
    ASSERT_TRUE(DgnDbStatus::Success == viewController->Save());
    }
