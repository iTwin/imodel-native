//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.cpp $
//  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "ChangeTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

// #define DEBUG_REVISION_TEST_COMPRESSION 1

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
void ChangeTestFixture::_SetupDgnDb()
    {
    SetupSeedProject(m_testFileName.c_str());

    m_testFileName = BeFileName(m_db->GetDbFileName(), true);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    if (m_wantTestDomain)
        ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*m_db));

    TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*m_db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    m_db->SaveChanges();

    // Create a dummy revision to purge transaction table for the test
    DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());
    m_db->Revisions().FinishCreateRevision();

    CloseDgnDb();
    OpenDgnDb();    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::OpenDgnDb()
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_db = DgnDb::OpenDgnDb(&openStatus, m_testFileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CloseDgnDb()
    {
    m_db->CloseDb();
    m_db = nullptr;
    m_defaultModel = nullptr;
    m_defaultCodeSpec = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnCategoryId ChangeTestFixture::InsertCategory(Utf8CP categoryName)
    {
    SpatialCategory category(*m_db, categoryName, DgnCategory::Rank::Application);

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
double randFraction()
    {
#ifdef DEBUG_REVISION_TEST_COMPRESSION
    return (double) rand() / RAND_MAX;
#else
    return 0.0;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId ChangeTestFixture::InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int x, int y, int z)
    {
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(model, categoryId);

    DPoint3d sizeOfBlock = DPoint3d::From(1 + randFraction(), 1 + randFraction(), 1 + randFraction());
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(randFraction(), randFraction(), randFraction()), sizeOfBlock, true);
    ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(testGeomPtr.IsValid());

    DPoint3d centerOfBlock = DPoint3d::From(x + randFraction(), y + randFraction(), z + randFraction());
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, centerOfBlock, YawPitchRollAngles());
    builder->Append(*testGeomPtr);
    BentleyStatus status = builder->Finish(*testElement);
    BeAssert(status == SUCCESS);

    DgnElementId elementId = m_db->Elements().Insert(*testElement)->GetElementId();
    return elementId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateDefaultView(DgnModelId defaultModelId)
    {
    auto categories = new CategorySelector(*m_db,"");
    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(*m_db))
        categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

    auto style = new DisplayStyle3d(*m_db,"");
    auto flags = style->GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    style->SetViewFlags(flags);

    auto models = new ModelSelector(*m_db,"");
    ModelIterator modIter = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel));
    for (ModelIteratorEntryCR entry : modIter)
        {
        auto id = entry.GetModelId();
        auto model = m_db->Models().GetModel(id);

        if (model.IsValid())
            models->AddModel(id);
        }

    CameraViewDefinition view(*m_db, "Default", *categories, *style, *models);
    view.SetStandardViewRotation(StandardView::Iso);

    ASSERT_TRUE(view.Insert().IsValid());

    DgnViewId viewId = view.GetViewId();
    m_db->SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::UpdateDgnDbExtents()
    {
    m_db->Units().InitializeProjectExtents();
    AxisAlignedBox3d physicalExtents = m_db->Units().GetProjectExtents();

    auto view = ViewDefinition::Get(*m_db, "Default");
    ASSERT_TRUE(view.IsValid());
    auto editView = view->MakeCopy<SpatialViewDefinition>();
    ASSERT_TRUE(editView.IsValid());

    editView->LookAtVolume(physicalExtents);
    ASSERT_TRUE(editView->Update().IsValid());
    }
