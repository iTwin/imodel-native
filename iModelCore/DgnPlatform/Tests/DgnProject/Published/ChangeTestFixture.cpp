//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "ChangeTestFixture.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateSeedDgnDb(BeFileNameR seedPathname)
    {
    seedPathname = DgnDbTestDgnManager::GetOutputFilePath(L"ChangeTestSeed.dgndb");
    if (seedPathname.DoesPathExist())
        return;

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);

    DbResult createStatus;
    DgnDbPtr seedDgnDb = DgnDb::CreateDgnDb(&createStatus, seedPathname, createProjectParams);
    ASSERT_TRUE(seedDgnDb.IsValid()) << "Could not create seed project";

    seedDgnDb->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateDgnDb(WCharCP testFileName)
    {
    // Note: Since creating the DgnDb everytime consumes too much time, we instead
    // just copy one we have created the first time around. 

    BeFileName pathname = DgnDbTestDgnManager::GetOutputFilePath(testFileName);
    if (pathname.DoesPathExist())
        BeFileName::BeDeleteFile(pathname);

    BeFileName seedPathname;
    CreateSeedDgnDb(seedPathname);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(seedPathname.c_str(), pathname.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    OpenDgnDb(testFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::OpenDgnDb(WCharCP testFileName)
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_testDb = DgnDb::OpenDgnDb(&openStatus, DgnDbTestDgnManager::GetOutputFilePath(testFileName), openParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

    DgnModelId modelId = m_testDb->Models().QueryFirstModelId();
    if (modelId.IsValid())
        m_testModel = m_testDb->Models().GetModel(modelId).get();

    TestDataManager::MustBeBriefcase(m_testDb, Db::OpenMode::ReadWrite);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CloseDgnDb()
    {
    m_testDb->CloseDb();
    m_testModel = nullptr;
    m_testDb = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::InsertModel()
    {
    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
    DgnClassId classId = m_testDb->Domains().GetClassId(handler);
    m_testModel = handler.Create(DgnModel::CreateParams(*m_testDb, classId, DgnModel::CreateModelCode("ChangeSetModel")));

    DgnDbStatus status = m_testModel->Insert();
    ASSERT_TRUE(DgnDbStatus::Success == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateDefaultView()
    {
    ASSERT_TRUE(m_testModel.IsValid());

    CameraViewDefinition viewRow(CameraViewDefinition::CreateParams(*m_testDb, "Default", ViewDefinition::Data(m_testModel->GetModelId(), DgnViewSource::Generated)));
    ASSERT_TRUE(viewRow.Insert().IsValid());

    PhysicalViewController viewController(*m_testDb, viewRow.GetViewId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);

    for (auto const& catId : DgnCategory::QueryCategories(*m_testDb))
        viewController.ChangeCategoryDisplay(catId, true);

    DgnModels::Iterator modIter = m_testDb->Models().MakeIterator();
    for (auto& entry : modIter)
        {
        DgnModelId modelId = entry.GetModelId();
        viewController.ChangeModelDisplay(modelId, true);
        }

    auto result = viewController.Save();
    ASSERT_TRUE(BE_SQLITE_OK == result);
    UNUSED_VARIABLE(result);

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
    m_testDb->Units().SaveProjectExtents(physicalExtents);

    PhysicalViewDefinitionCPtr view = dynamic_cast<PhysicalViewDefinitionCP>(ViewDefinition::QueryView("Default", *m_testDb).get());
    ASSERT_TRUE(view.IsValid());

    ViewControllerPtr viewController = view->LoadViewController(ViewDefinition::FillModels::No);
    viewController->LookAtVolume(physicalExtents);
    DbResult result = viewController->Save();
    ASSERT_TRUE(result == BE_SQLITE_OK);

    m_testDb->SaveSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::InsertCategory()
    {
    DgnCategory category(DgnCategory::CreateParams(*m_testDb, "ChangeSetTestCategory", DgnCategory::Scope::Physical, DgnCategory::Rank::Application));

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    ASSERT_TRUE(persistentCategory.IsValid());

    m_testCategoryId = persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::InsertAuthority()
    {
    m_testAuthority = NamespaceAuthority::CreateNamespaceAuthority("ChangeTestAuthority", *m_testDb);
    ASSERT_TRUE(DgnDbStatus::Success == m_testAuthority->Insert());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId ChangeTestFixture::InsertElement(int x, int y, int z)
    {
    PhysicalModelP physicalTestModel = dynamic_cast<PhysicalModelP> (m_testModel.get());
    BeAssert(physicalTestModel != nullptr);
    BeAssert(m_testCategoryId.IsValid());

    PhysicalElementPtr testElement = PhysicalElement::Create(*physicalTestModel, m_testCategoryId);

    DPoint3d sizeOfBlock = DPoint3d::From(1, 1, 1);
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(0, 0, 0), sizeOfBlock, true);
    ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(testGeomPtr.IsValid());

    DPoint3d centerOfBlock = DPoint3d::From(x, y, z);
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*physicalTestModel, m_testCategoryId, centerOfBlock, YawPitchRollAngles());
    builder->Append(*testGeomPtr);
    BentleyStatus status = builder->SetGeomStreamAndPlacement(*testElement);
    BeAssert(status == SUCCESS);

    DgnElementId elementId = m_testDb->Elements().Insert(*testElement)->GetElementId();
    return elementId;
    }
